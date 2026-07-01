#include <sstream>
#include <filesystem>
#include "main.h"

// ─── Engine ──────────────────────────────────────────────────────────────────

void Engine::save()
{
	if (player->destructible->isDead()) {
		// Dead players don't keep their save — clear it.
		std::remove("game.sav");
		return;
	}

	TCODZip zip;

	// Write in a strict order that Engine::load must mirror exactly.
	zip.putInt(map->getWidth());
	zip.putInt(map->getHeight());
	map->save(zip);
	camera->save(zip);
	player->save(zip);
	stairs->save(zip);

	// Save all actors except player and stairs (they are saved above).
	zip.putInt(static_cast<int>(actors.size()) - 2);
	for (const auto& actorPtr : actors) {
		if (actorPtr.get() != player && actorPtr.get() != stairs) {
			actorPtr->save(zip);
		}
	}

	gui->save(zip);
	zip.putInt(dungeonLevel);
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
	auto newPlayer = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
	player = newPlayer.get();
	actors.emplace_front(std::move(newPlayer));
	player->load(zip);

	auto newStairs = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
	stairs = newStairs.get();
	actors.emplace_front(std::move(newStairs));
	stairs->load(zip);

	int remainingActors = zip.getInt();
	while (remainingActors-- > 0) {
		auto actor = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
		actor->load(zip);
		actors.emplace_front(std::move(actor));
	}

	gui->load(zip);

	// dungeonLevel appended at end of save stream; old saves yield 0 (archive exhausted).
	int loadedLevel = zip.getInt();
	dungeonLevel = (loadedLevel > 0) ? loadedLevel : 1;

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

	// Write presence flags before payloads so load knows what to expect.
	zip.putInt(attacker     != nullptr);
	zip.putInt(destructible != nullptr);
	zip.putInt(ai           != nullptr);
	zip.putInt(pickable     != nullptr);
	zip.putInt(container    != nullptr);

	if (attacker)     attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai)           ai->save(zip);
	if (pickable)     pickable->save(zip);
	if (container)    container->save(zip);
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

	const bool hasAttacker     = zip.getInt();
	const bool hasDestructible = zip.getInt();
	const bool hasAi           = zip.getInt();
	const bool hasPickable     = zip.getInt();
	const bool hasContainer    = zip.getInt();

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
		// Effect is read here by Effect::create; Pickable::load reads only the selector.
		pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f),
			Effect::create(zip));
		pickable->load(zip);
	}
	if (hasContainer) {
		container = std::make_unique<Container>(0);
		container->load(zip);
	}
}

// ─── Attacker ────────────────────────────────────────────────────────────────

void Attacker::save(TCODZip& zip) { zip.putFloat(power); }
void Attacker::load(TCODZip& zip) { power = zip.getFloat(); }

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
	case DestructibleType::MONSTER: d = std::make_unique<MonsterDestructible>(0, 0.0f, "", 0); break;
	case DestructibleType::PLAYER:  d = std::make_unique<PlayerDestructible> (0, 0.0f, "", 0); break;
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

// Save order: effect (with type discriminator), hasSelector flag, selector data.
// Load order: effect is consumed by Effect::create() in Actor::load before Pickable::load is called.
void Pickable::save(TCODZip& zip)
{
	effect->save(zip);
	zip.putInt(selector != nullptr);
	if (selector) { selector->save(zip); }
}

void Pickable::load(TCODZip& zip)
{
	// Effect was already created and loaded by Effect::create() in Actor::load.
	const bool hasSelector = zip.getInt();
	if (hasSelector) {
		selector = std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f);
		selector->load(zip);
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
		effect = std::make_unique<HealthEffect>(0.0f, "", TCOD_light_grey);
		break;
	case EffectType::CHANGE_AI:
		effect = std::make_unique<AiChangeEffect>(
			std::make_unique<TemporaryAi>(0), " ", TCOD_light_grey);
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
		auto actor = std::make_unique<Actor>(0, 0, 0, " ", TCOD_white);
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
