#include <fstream>
#include <sstream>
#include <filesystem>
#include "main.h"

// ─── Engine ──────────────────────────────────────────────────────────────────

std::vector<char> Engine::serializeCurrentLevel() const
{
	TCODZip zip;

	// Write map dimensions so the snapshot is self-describing.
	zip.putInt(map->getWidth());
	zip.putInt(map->getHeight());

	// Write full map state (sentinel, levelType, seed, tiles, currentScentValue).
	map->save(zip);

	// Count non-player actors.
	int actorCount = 0;
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() != player) actorCount++;
	}
	zip.putInt(actorCount);

	// Serialize each non-player actor.
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() != player) {
			actorPtr->save(zip);
		}
	}

	// TCODZip has no save-to-buffer API — save to a temp file, read it back.
	static constexpr const char* TEMP_FILE = "_level_cache_temp.sav";
	zip.saveToFile(TEMP_FILE);

	std::ifstream file(TEMP_FILE, std::ios::binary | std::ios::ate);
	const auto fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(static_cast<size_t>(fileSize));
	file.read(buffer.data(), fileSize);
	file.close();

	std::filesystem::remove(TEMP_FILE);

	return buffer;
}

bool Engine::deserializeLevel(const std::vector<char>& snapshot)
{
	// Write the byte vector to a temp file (TCODZip only supports loadFromFile).
	static constexpr const char* TEMP_FILE = "_level_cache_temp.sav";
	{
		std::ofstream file(TEMP_FILE, std::ios::binary);
		file.write(snapshot.data(), static_cast<std::streamsize>(snapshot.size()));
	}

	TCODZip zip;
	zip.loadFromFile(TEMP_FILE);
	std::filesystem::remove(TEMP_FILE);

	// Read map dimensions written by serializeCurrentLevel.
	const int mapWidth  = zip.getInt();
	const int mapHeight = zip.getInt();

	// Create new Map and load its state.
	map = std::make_unique<Map>(mapWidth, mapHeight);
	map->load(zip);

	// Read actor count and load each actor.
	const int actorCount = zip.getInt();
	for (int i = 0; i < actorCount; i++) {
		auto actor = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
		actor->load(zip);
		actors.emplace_front(std::move(actor));
	}

	// Assign stair pointers by scanning for glyph '<' and '>'.
	stairsUp = nullptr;
	stairsDown = nullptr;
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() == player) continue;
		if (actorPtr->getGlyph() == '<') stairsUp = actorPtr.get();
		if (actorPtr->getGlyph() == '>') stairsDown = actorPtr.get();
	}

	// Validate stair presence based on depth.
	// Depth 0 has no up stairs, depth 20 has no down stairs — those are expected nullptr.
	// But if depth > 0 and no stairsUp, or depth < 20 and no stairsDown, snapshot is corrupted.
	const bool needsUp   = (dungeonLevel > 0);
	const bool needsDown = (dungeonLevel < 20);

	if ((needsUp && stairsUp == nullptr) || (needsDown && stairsDown == nullptr)) {
		// Corrupted snapshot — discard restored actors (except player) and signal failure.
		for (auto it = actors.begin(); it != actors.end(); ) {
			it = (it->get() != player) ? actors.erase(it) : std::next(it);
		}
		map.reset();
		stairsUp = nullptr;
		stairsDown = nullptr;
		return false;
	}

	// Move decoration actors (no ai, no destructible, no attacker) to the front
	// so they render beneath gameplay actors. emplace_front during load reverses
	// the serialized order, so decorations (which were at the front pre-save) end
	// up at the back post-load without this pass.
	for (auto it = actors.begin(); it != actors.end(); ) {
		Actor* actor = it->get();
		if (actor != player && !actor->ai && !actor->destructible && !actor->attacker) {
			auto node = std::move(*it);
			it = actors.erase(it);
			actors.push_front(std::move(node));
		} else {
			++it;
		}
	}

	// Order dead actors behind living actors using sendToBack().
	for (auto it = actors.begin(); it != actors.end(); ++it) {
		Actor* actor = it->get();
		if (actor != player && actor->destructible && actor->destructible->isDead()) {
			sendToBack(actor);
		}
	}

	// Update camera dimensions to match restored map.
	camera->mapWidth  = map->getWidth();
	camera->mapHeight = map->getHeight();

	return true;
}

void Engine::save()
{
	if (player->destructible->isDead()) {
		// Dead players don't keep their save — clear it.
		std::remove("game.sav");
		levelCache.clear();
		return;
	}

	TCODZip zip;

	// Write in a strict order that Engine::load must mirror exactly.
	zip.putInt(map->getWidth());
	zip.putInt(map->getHeight());
	map->save(zip);
	camera->save(zip);
	player->save(zip);

	// Save player equipment slot assignments (after player save, since items are in inventory)
	if (player->equipment) {
		zip.putInt(1); // has equipment
		player->equipment->save(zip, *player->container);
	} else {
		zip.putInt(0);
	}

	// Dual stairs serialization — sentinel-based presence flags.
	static constexpr int STAIR_PRESENT = 1;
	static constexpr int STAIR_ABSENT = -1;

	if (stairsUp) {
		zip.putInt(STAIR_PRESENT);
		stairsUp->save(zip);
	} else {
		zip.putInt(STAIR_ABSENT);
	}

	if (stairsDown) {
		zip.putInt(STAIR_PRESENT);
		stairsDown->save(zip);
	} else {
		zip.putInt(STAIR_ABSENT);
	}

	// Save all actors except player and stairs (they are saved above).
	int count = 0;
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() != player && actorPtr.get() != stairsUp && actorPtr.get() != stairsDown) count++;
	}
	zip.putInt(count);
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() != player && actorPtr.get() != stairsUp && actorPtr.get() != stairsDown) {
			actorPtr->save(zip);
		}
	}

	gui->save(zip);
	zip.putInt(dungeonLevel);

	// Level cache sentinel and data — signals presence of cache section to Engine::load().
	static constexpr int LEVEL_CACHE_SENTINEL = 0x4C564C43; // "LVLC"
	zip.putInt(LEVEL_CACHE_SENTINEL);
	levelCache.save(zip);

	zip.saveToFile("game.sav");
}

void Engine::load()
{
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::MenuItemCode::NEW_GAME, "New Game");
	if (std::filesystem::exists("game.sav")) {
		engine.gui->menu.addItem(Menu::MenuItemCode::CONTINUE, "Continue");
	}
	engine.gui->menu.addItem(Menu::MenuItemCode::EXIT, "Exit");

	const Menu::MenuItemCode choice = engine.gui->menu.pick();

	if (choice == Menu::MenuItemCode::EXIT || choice == Menu::MenuItemCode::NONE) {
		exit(0);
	}

	if (choice == Menu::MenuItemCode::NEW_GAME) {
		engine.term();
		engine.init();
		return;
	}

	// "Continue" — restore from save file.
	TCODZip zip;
	engine.term();
	zip.loadFromFile("game.sav");

	// Reconstruct in the same order as save().
	const int mapWidth  = zip.getInt();
	const int mapHeight = zip.getInt();
	map = std::make_unique<Map>(mapWidth, mapHeight);
	map->load(zip);

	camera = std::make_unique<Camera>(0, 0, 80, 43, map->getWidth(), map->getHeight());
	camera->load(zip);

	// Player must be emplaced before loading so engine.player is valid for subsequent loads.
	auto newPlayer = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
	player = newPlayer.get();
	actors.emplace_front(std::move(newPlayer));
	player->load(zip);

	// Load player equipment slot assignments
	int hasEquipment = zip.getInt();
	if (hasEquipment) {
		player->equipment = std::make_unique<Equipment>();
		player->equipment->load(zip, *player->container);
		// Reapply skill modifiers from equipped items
		for (int i = 0; i < static_cast<int>(EquipmentSlot::COUNT); i++) {
			Actor* equipped = player->equipment->getSlot(static_cast<EquipmentSlot>(i));
			if (equipped && equipped->equippable && equipped->equippable->modifiers.skill != 0) {
				player->attacker->addModifier(equipped->equippable->modifiers.skill);
			}
		}
	} else {
		player->equipment = std::make_unique<Equipment>();
	}

	// Dual stairs restoration — sentinel-based presence flags.
	static constexpr int STAIR_PRESENT = 1;

	int upFlag = zip.getInt();
	if (upFlag == STAIR_PRESENT) {
		auto newUp = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
		stairsUp = newUp.get();
		actors.emplace_front(std::move(newUp));
		stairsUp->load(zip);
	} else {
		stairsUp = nullptr;
	}

	int downFlag = zip.getInt();
	if (downFlag == STAIR_PRESENT) {
		auto newDown = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
		stairsDown = newDown.get();
		actors.emplace_front(std::move(newDown));
		stairsDown->load(zip);
	} else {
		stairsDown = nullptr;
	}

	int remainingActors = zip.getInt();
	while (remainingActors-- > 0) {
		auto actor = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
		actor->load(zip);
		actors.emplace_front(std::move(actor));
	}

	// Move decoration actors (no ai, no destructible, no attacker) to the front
	// so they render beneath gameplay actors. emplace_front during load reverses
	// the serialized order, so decorations need to be repositioned.
	for (auto it = actors.begin(); it != actors.end(); ) {
		Actor* actor = it->get();
		if (actor != player && !actor->ai && !actor->destructible && !actor->attacker) {
			auto node = std::move(*it);
			it = actors.erase(it);
			actors.push_front(std::move(node));
		} else {
			++it;
		}
	}

	gui->load(zip);

	// dungeonLevel appended at end of save stream; old saves yield 0 (archive exhausted).
	int loadedLevel = zip.getInt();
	dungeonLevel = (loadedLevel > 0) ? loadedLevel : 1;

	// Check for level cache sentinel after dungeonLevel.
	// If sentinel matches: restore cached level snapshots from save file.
	// If not: old save format — leave levelCache empty (already cleared by term()).
	static constexpr int LEVEL_CACHE_SENTINEL = 0x4C564C43; // "LVLC"
	int maybeSentinel = zip.getInt();
	if (maybeSentinel == LEVEL_CACHE_SENTINEL) {
		levelCache.load(zip);
	}
	// Old saves: getInt() returns 0 when exhausted, which won't match sentinel.

	// Note: The active level is saved separately in the main save format above,
	// so it won't be present in the cache. No removal needed.

	gameStatus = STARTUP; // force FOV recomputation on the first frame
}

// ─── Map ─────────────────────────────────────────────────────────────────────

void Map::save(TCODZip& zip)
{
	static constexpr int SAVE_VERSION_SENTINEL = 0x4F444F52; // "ODOR"
	zip.putInt(SAVE_VERSION_SENTINEL);
	zip.putInt(static_cast<int>(levelType));
	zip.putInt(seed);
	for (const auto& tile : tiles) {
		zip.putInt(tile.explored);
		zip.putInt(tile.scent);
	}
	zip.putInt(currentScentValue);

	// Extended format: dimensions and terrain data for snapshot self-description.
	static constexpr int DIMS_SENTINEL = 0x44494D53; // "DIMS"
	zip.putInt(DIMS_SENTINEL);
	zip.putInt(width);
	zip.putInt(height);
	// For OUTDOOR levels, persist terrain classification to avoid re-running Perlin.
	if (levelType == LevelType::OUTDOOR) {
		for (const auto& t : terrainTypes) {
			zip.putInt(static_cast<int>(t));
		}
	}
}

void Map::load(TCODZip& zip)
{
	static constexpr int SAVE_VERSION_SENTINEL = 0x4F444F52;
	int firstValue = zip.getInt();
	if (firstValue == SAVE_VERSION_SENTINEL) {
		// New format: sentinel followed by levelType then seed
		levelType = static_cast<LevelType>(zip.getInt());
		seed = zip.getInt();
	} else {
		// Old format: first int is the seed directly
		levelType = LevelType::BSP;
		seed = firstValue;
	}
	init(false, levelType); // regenerate geometry from seed
	for (auto& tile : tiles) {
		tile.explored = static_cast<bool>(zip.getInt());
		tile.scent    = zip.getInt();
	}
	currentScentValue = zip.getInt();

	// Extended format: check for DIMS sentinel to read dimensions and terrain.
	static constexpr int DIMS_SENTINEL = 0x44494D53;
	int maybeDims = zip.getInt();
	if (maybeDims == DIMS_SENTINEL) {
		width  = zip.getInt();
		height = zip.getInt();
		// For OUTDOOR levels, restore terrain classification from saved data.
		if (levelType == LevelType::OUTDOOR) {
			terrainTypes.resize(width * height);
			for (int i = 0; i < width * height; i++) {
				terrainTypes[i] = static_cast<TerrainType>(zip.getInt());
			}
		}
	}
	// If sentinel not present: old format, terrain was already regenerated from seed via init().
}

// ─── Actor ───────────────────────────────────────────────────────────────────

void Actor::save(TCODZip& zip)
{
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(glyph);
	zip.putColor(&color);
	zip.putString(name.c_str());
	zip.putInt(blocks);
	zip.putInt(fovOnly);
	zip.putString(description.c_str());

	// Write presence flags before payloads so load knows what to expect.
	zip.putInt(attacker     != nullptr);
	zip.putInt(destructible != nullptr);
	zip.putInt(ai           != nullptr);
	zip.putInt(pickable     != nullptr);
	zip.putInt(container    != nullptr);
	zip.putInt(equippable   != nullptr);  // NOTE: breaks old save format

	if (attacker)     attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai)           ai->save(zip);
	if (pickable)     pickable->save(zip);
	if (container)    container->save(zip);
	if (equippable)   equippable->save(zip);

	// Characteristics presence flag — appended AFTER existing components for backward
	// compatibility (old saves that lack this field will read 0 from archive end).
	zip.putInt(characteristics != nullptr);
	if (characteristics) characteristics->save(zip);
}

void Actor::load(TCODZip& zip)
{
	x      = zip.getInt();
	y      = zip.getInt();
	glyph  = zip.getInt();
	color  = zip.getColor();
	name   = zip.getString();
	blocks  = zip.getInt();
	fovOnly = zip.getInt();
	description = zip.getString();

	const bool hasAttacker     = zip.getInt();
	const bool hasDestructible = zip.getInt();
	const bool hasAi           = zip.getInt();
	const bool hasPickable     = zip.getInt();
	const bool hasContainer    = zip.getInt();
	const bool hasEquippable   = zip.getInt();  // NOTE: breaks old save format

	if (hasAttacker) {
		attacker = std::make_unique<Attacker>(0.0f);
		attacker->load(zip);
	}
	if (hasDestructible) {
		destructible = Destructible::create(zip);
	}
	if (hasAi) {
		ai = Ai::create(zip);
	}
	if (hasPickable) {
		// Read whether effect is present (new format writes hasEffect flag first)
		const bool hasEffect = zip.getInt();
		std::unique_ptr<Effect> effect;
		if (hasEffect) {
			effect = Effect::create(zip);
		}
		pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f),
			std::move(effect));
		pickable->load(zip);
	}
	if (hasContainer) {
		container = std::make_unique<Container>(0);
		container->load(zip);
	}
	if (hasEquippable) {
		equippable = std::make_shared<Equippable>(EquipmentSlot::WEAPON, StatModifiers{}, 0.0f, 0);
		equippable->load(zip);
	}

	// Characteristics presence flag — appended AFTER existing components for backward
	// compatibility (old saves that lack this field will read 0 from archive end).
	const bool hasCharacteristics = zip.getInt();
	if (hasCharacteristics) {
		characteristics = std::make_shared<Characteristics>();
		characteristics->load(zip);
	}
}

// ─── Attacker ────────────────────────────────────────────────────────────────
// save/load moved to Source/Attacker.cpp (sentinel-based format with skillValue)

// ─── Destructible ────────────────────────────────────────────────────────────

void Destructible::save(TCODZip& zip)
{
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(defense);
	zip.putString(corpseName.c_str());
	zip.putInt(xp);
}

void Destructible::load(TCODZip& zip)
{
	maxHp      = zip.getFloat();
	hp         = zip.getFloat();
	defense    = zip.getFloat();
	corpseName = zip.getString();
	xp         = zip.getInt();
}

std::unique_ptr<Destructible> Destructible::create(TCODZip& zip)
{
	const auto type = static_cast<DestructibleType>(zip.getInt());
	std::unique_ptr<Destructible> d;
	switch (type) {
	case DestructibleType::MONSTER: d = std::make_unique<MonsterDestructible>(0.0f, 0.0f, "", 0); break;
	case DestructibleType::PLAYER:  d = std::make_unique<PlayerDestructible> (0.0f, 0.0f, "", 0); break;
	}
	d->load(zip);
	return d;
}

void MonsterDestructible::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(DestructibleType::MONSTER));
	Destructible::save(zip);
}

void PlayerDestructible::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(DestructibleType::PLAYER));
	Destructible::save(zip);
}

// ─── Ai ──────────────────────────────────────────────────────────────────────

std::unique_ptr<Ai> Ai::create(TCODZip& zip)
{
	const auto type = static_cast<AiType>(zip.getInt());
	std::unique_ptr<Ai> ai;
	switch (type) {
	case AiType::PLAYER:          ai = std::make_unique<PlayerAi>();         break;
	case AiType::MONSTER:         ai = std::make_unique<MonsterAi>();        break;
	case AiType::RANGED_MONSTER:  ai = std::make_unique<RangedAi>();         break;
	default: /* CONFUSED_MONSTER */ ai = std::make_unique<ConfusedMonsterAi>(0); break;
	}
	ai->load(zip);
	return ai;
}

auto TemporaryAi::create(TCODZip& zip)
{
	const auto type = static_cast<AiType>(zip.getInt());
	std::unique_ptr<TemporaryAi> ai;
	if (type == AiType::CONFUSED_MONSTER) {
		ai = std::make_unique<ConfusedMonsterAi>(0);
	}
	if (ai) { ai->load(zip); }
	return ai;
}

void PlayerAi::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(AiType::PLAYER));
	zip.putInt(xpLevel);
}
void PlayerAi::load(TCODZip& zip) { xpLevel = zip.getInt(); }

void MonsterAi::save(TCODZip& zip) { zip.putInt(static_cast<int>(AiType::MONSTER)); }
void MonsterAi::load(TCODZip& zip) {}

void RangedAi::save(TCODZip& zip) { zip.putInt(static_cast<int>(AiType::RANGED_MONSTER)); }
void RangedAi::load(TCODZip& zip) {}

void ConfusedMonsterAi::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(AiType::CONFUSED_MONSTER));
	zip.putInt(turnsRemaining);
	zip.putInt(oldAi != nullptr);
	if (oldAi) { oldAi->save(zip); }
}

void ConfusedMonsterAi::load(TCODZip& zip)
{
	turnsRemaining = zip.getInt();
	const bool hasOldAi = zip.getInt();
	if (hasOldAi) { oldAi = Ai::create(zip); }
}

// ─── Pickable ────────────────────────────────────────────────────────────────

// Save order: effect (with type discriminator), hasSelector flag, selector data,
// then sentinel-guarded weight/value fields.
// Load order: effect is consumed by Effect::create() in Actor::load before Pickable::load is called.
void Pickable::save(TCODZip& zip)
{
	zip.putInt(effect != nullptr);
	if (effect) { effect->save(zip); }
	zip.putInt(selector != nullptr);
	if (selector) { selector->save(zip); }

	// Version sentinel for weight/value fields (backward-compatible extension).
	// The sentinel -2 cannot collide with valid data: selector types and effects
	// are non-negative, and Container::size is always >= 0.
	static constexpr int PICKABLE_WEIGHT_VALUE_SENTINEL = -2;
	zip.putInt(PICKABLE_WEIGHT_VALUE_SENTINEL);
	zip.putFloat(weight);
	zip.putInt(value);
}

void Pickable::load(TCODZip& zip)
{
	// Effect was already created and loaded by Effect::create() in Actor::load.
	const bool hasSelector = zip.getInt();
	if (hasSelector) {
		selector = std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f);
		selector->load(zip);
	}

	// Check for weight/value sentinel (new format).
	// Old saves won't have the sentinel; TCODZip::getInt() returns 0 when the
	// archive is exhausted or the next value is from a different section.
	// Since -2 never appears as a valid Container::size or actor field count,
	// any value != -2 means old format. Note: old saves should be deleted
	// after this change to avoid stream misalignment.
	static constexpr int PICKABLE_WEIGHT_VALUE_SENTINEL = -2;
	int maybeSentinel = zip.getInt();
	if (maybeSentinel == PICKABLE_WEIGHT_VALUE_SENTINEL) {
		weight = zip.getFloat();
		value  = zip.getInt();
	} else {
		// Old format — default both fields to zero.
		weight = 0.0f;
		value  = 0;
	}
}

void TargetSelector::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(type));
	zip.putFloat(range);
}
void TargetSelector::load(TCODZip& zip)
{
	type  = static_cast<SelectorType>(zip.getInt());
	range = zip.getFloat();
}

std::unique_ptr<Effect> Effect::create(TCODZip& zip)
{
	const auto type = static_cast<EffectType>(zip.getInt());
	std::unique_ptr<Effect> effect;
	switch (type) {
	case EffectType::HEALTH:
		effect = std::make_unique<HealthEffect>(0.0f, "", Colors::uiText);
		break;
	case EffectType::CHANGE_AI:
		effect = std::make_unique<AiChangeEffect>(
			std::make_unique<TemporaryAi>(0), " ", Colors::uiText);
		break;
	}
	effect->load(zip);
	return effect;
}

void HealthEffect::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(EffectType::HEALTH));
	zip.putFloat(amount);
	zip.putString(message.c_str());
	zip.putColor(&textCol);
}
void HealthEffect::load(TCODZip& zip)
{
	amount  = zip.getFloat();
	message = zip.getString();
	textCol = zip.getColor();
}

void AiChangeEffect::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(EffectType::CHANGE_AI));
	newAi->save(zip);
	zip.putString(message.c_str());
	zip.putColor(&textCol);
}
void AiChangeEffect::load(TCODZip& zip)
{
	newAi   = TemporaryAi::create(zip);
	message = zip.getString();
	textCol = zip.getColor();
}

// ─── Container ───────────────────────────────────────────────────────────────

void Container::save(TCODZip& zip)
{
	zip.putInt(size);
	zip.putInt(static_cast<int>(inventory.size()));
	for (const auto& itemPtr : inventory) {
		itemPtr->save(zip);
	}
}

void Container::load(TCODZip& zip)
{
	size = zip.getInt();
	int count = zip.getInt();
	while (count-- > 0) {
		auto actor = std::make_unique<Actor>(0, 0, 0, " ", Colors::white);
		actor->load(zip);
		inventory.emplace_back(std::move(actor));
	}
}

// ─── Gui ─────────────────────────────────────────────────────────────────────

void Gui::save(TCODZip& zip)
{
	zip.putInt(static_cast<int>(log.size()));
	for (const auto& msg : log) {
		zip.putString(msg->text.c_str());
		zip.putColor(&msg->col);
	}
}

void Gui::load(TCODZip& zip)
{
	int count = zip.getInt();
	while (count-- > 0) {
		const std::string text = zip.getString();
		const TCODColor   col  = zip.getColor();
		message(col, text);
	}
}

// ─── Camera ──────────────────────────────────────────────────────────────────

void Camera::save(TCODZip& zip)
{
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(width);
	zip.putInt(height);
	zip.putInt(mapWidth);
	zip.putInt(mapHeight);
}

void Camera::load(TCODZip& zip)
{
	x         = zip.getInt();
	y         = zip.getInt();
	width     = zip.getInt();
	height    = zip.getInt();
	mapWidth  = zip.getInt();
	mapHeight = zip.getInt();
}
