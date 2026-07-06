#include <algorithm>
#include <sstream>
#include "main.h"
#include "LevelCache.h"

LevelCache::LevelCache(int maxCapacity)
	: maxCapacity(std::clamp(maxCapacity, 2, 200)) {
}

void LevelCache::store(int level, std::vector<char> snapshot) {
	++accessCounter;

	// If the level already exists, overwrite it
	auto it = entries.find(level);
	if (it != entries.end()) {
		it->second.data = std::move(snapshot);
		it->second.lastAccess = accessCounter;
		return;
	}

	// Evict LRU entry if at capacity
	if (static_cast<int>(entries.size()) >= maxCapacity) {
		evictLRU();
	}

	// Insert new entry
	entries[level] = CacheEntry{ std::move(snapshot), accessCounter };
}

std::optional<std::vector<char>> LevelCache::retrieve(int level) {
	auto it = entries.find(level);
	if (it == entries.end()) {
		return std::nullopt;
	}

	++accessCounter;
	it->second.lastAccess = accessCounter;
	return it->second.data;
}

bool LevelCache::contains(int level) const {
	return entries.find(level) != entries.end();
}

void LevelCache::evictLRU() {
	if (entries.empty()) return;

	// Linear scan for the entry with minimum lastAccess
	auto lru = entries.begin();
	for (auto it = entries.begin(); it != entries.end(); ++it) {
		if (it->second.lastAccess < lru->second.lastAccess) {
			lru = it;
		}
	}

	entries.erase(lru);
}

void LevelCache::clear() {
	entries.clear();
	accessCounter = 0;
}

int LevelCache::size() const {
	return static_cast<int>(entries.size());
}

void LevelCache::setMaxCapacity(int cap) {
	maxCapacity = std::clamp(cap, 2, 200);
}

int LevelCache::getMaxCapacity() const {
	return maxCapacity;
}

void LevelCache::save(TCODZip& zip) const {
	zip.putInt(static_cast<int>(entries.size()));
	for (const auto& [level, entry] : entries) {
		zip.putInt(level);
		zip.putInt(static_cast<int>(entry.data.size()));
		zip.putData(static_cast<int>(entry.data.size()), entry.data.data());
	}
}

void LevelCache::load(TCODZip& zip) {
	int entryCount = zip.getInt();
	if (entryCount < 0) {
		engine.gui->message(Colors::damage, "Warning: level cache entry count negative, skipping.");
		return;
	}

	static constexpr int MAX_SNAPSHOT_BYTES = 10 * 1024 * 1024; // 10 MB

	for (int i = 0; i < entryCount; ++i) {
		int level = zip.getInt();
		int byteCount = zip.getInt();

		if (byteCount < 0) {
			engine.gui->message(Colors::damage,
				"Warning: discarding corrupted cache entry for level # (negative byte count).", level);
			continue;
		}

		if (byteCount > MAX_SNAPSHOT_BYTES) {
			engine.gui->message(Colors::damage,
				"Warning: discarding cache entry for level # (byte count # exceeds limit).", level, byteCount);
			// Skip the data bytes in the stream
			zip.skipBytes(static_cast<uint32_t>(byteCount));
			continue;
		}

		std::vector<char> data(byteCount);
		int bytesRead = zip.getData(byteCount, data.data());
		if (bytesRead < byteCount) {
			engine.gui->message(Colors::damage,
				"Warning: discarding cache entry for level # (read failure).", level);
			continue;
		}

		++accessCounter;
		entries[level] = CacheEntry{ std::move(data), accessCounter };
	}
}
