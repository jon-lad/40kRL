#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

class TCODZip;

// Stores serialized level snapshots keyed by dungeon depth with LRU eviction.
// Snapshots are binary blobs (TCODZip archives saved to memory) rather than live
// objects, keeping memory predictable and avoiding dangling pointer issues.
class LevelCache {
public:
	explicit LevelCache(int maxCapacity = 30);

	// Store a snapshot for the given dungeon level. Evicts LRU if at capacity.
	void store(int level, std::vector<char> snapshot);

	// Retrieve a snapshot if cached. Marks as most-recently-accessed.
	std::optional<std::vector<char>> retrieve(int level);

	// Check if a level is cached without affecting access order.
	bool contains(int level) const;

	// Remove all entries.
	void clear();

	// Persist cache to/from save file.
	void save(TCODZip& zip) const;
	void load(TCODZip& zip);

	// Number of cached entries.
	int size() const;

	// Update max capacity (clamped to [2, 200]).
	void setMaxCapacity(int cap);
	int getMaxCapacity() const;

private:
	struct CacheEntry {
		std::vector<char> data;
		uint64_t lastAccess = 0;
	};

	std::unordered_map<int, CacheEntry> entries;
	int maxCapacity;
	uint64_t accessCounter = 0;

	void evictLRU();
};
