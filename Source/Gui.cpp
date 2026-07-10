
#include <list>
#include <memory>
#include <string>
#include <sstream>
#include <tuple>
#include "main.h"

// ─── Gui ─────────────────────────────────────────────────────────────────────

Gui::Gui()
{
	hudConsole = std::make_unique<TCODConsole>(engine.screenWidth, constants::PANEL_HEIGHT);
}

void Gui::render()
{
	hudConsole->setDefaultBackground(Colors::black);
	hudConsole->clear();

	renderBar(1, 1, constants::BAR_WIDTH, "HP",
		engine.player->destructible->hp,
		engine.player->destructible->maxHp,
		Colors::damageLight, Colors::damageDark);



	// Draw the message log — oldest messages are dim, newest are bright.
	int  row       = 1;
	float dimFactor = 0.4f;
	for (const auto& msg : log) {
		hudConsole->setDefaultForeground(msg->col * dimFactor);
		hudConsole->printf(constants::MSG_X, row, msg->text.c_str());
		row++;
		dimFactor += 0.3f;
	}

	renderMouseLook();

	hudConsole->setDefaultForeground(Colors::white);
	std::stringstream levelLabel;
	levelLabel << "Dungeon level " << engine.dungeonLevel;
	hudConsole->printf(3, 3, levelLabel.str().c_str());

	// Display ammo when a ranged weapon is equipped
	if (engine.player->equipment) {
		Actor* weapon = engine.player->equipment->getSlot(EquipmentSlot::WEAPON);
		if (weapon && weapon->equippable && weapon->equippable->rangedStats.has_value()) {
			std::stringstream ammoLabel;
			ammoLabel << "Ammo: " << weapon->equippable->currentAmmo
			          << "/" << weapon->equippable->rangedStats->clipSize;
			hudConsole->setDefaultForeground(Colors::white);
			hudConsole->printf(constants::BAR_WIDTH + 3, 3, ammoLabel.str().c_str());
		}
	}

	TCODConsole::blit(hudConsole.get(), 0, 0, engine.screenWidth, constants::PANEL_HEIGHT,
		TCODConsole::root, 0, engine.screenHeight - constants::PANEL_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, std::string_view name,
	float value, float maxValue,
	const TCODColor& barColor, const TCODColor& backColor)
{
	// Background (empty portion of the bar)
	hudConsole->setDefaultBackground(backColor);
	hudConsole->rect(x, y, width, 1, false, TCOD_BKGND_SET);

	// Filled portion
	const int filledWidth = static_cast<int>(value / maxValue * width);
	hudConsole->setDefaultBackground(barColor);
	hudConsole->rect(x, y, filledWidth, 1, false, TCOD_BKGND_SET);

	// Label centred over the bar
	hudConsole->setDefaultForeground(Colors::white);
	std::stringstream label;
	label << name << " : " << value << "/" << maxValue;
	hudConsole->printf(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER, label.str().c_str());
}

void Gui::renderMouseLook()
{
	auto [worldX, worldY] = engine.camera->getWorldLocation(engine.inputState.mouse.cellX, engine.inputState.mouse.cellY);
	if (!engine.map->isInFOV(worldX, worldY)) { return; }

	// Build a comma-separated list of actor names at the cursor tile.
	std::string actorNames;
	for (const auto& actorPtr : engine.actors) {
		Actor* actor = actorPtr.get();
		if (actor->getX() == worldX && actor->getY() == worldY) {
			if (!actorNames.empty()) { actorNames += ", "; }
			actorNames += actor->name;
		}
	}

	hudConsole->setDefaultForeground(Colors::uiText);
	hudConsole->printf(1, 0, actorNames.c_str());
}

bool Gui::replace(std::string& str, const std::string& from, const std::string& to)
{
	const size_t pos = str.find(from);
	if (pos == std::string::npos) { return false; }
	str.replace(pos, from.length(), to);
	return true;
}

Gui::Message::Message(std::string_view text, const TCODColor& col)
	: text{ text.data() }, col{ col }
{}

void Gui::clear()
{
	log.clear();
}

// ─── Menu ────────────────────────────────────────────────────────────────────

void Menu::clear()
{
	items.clear();
}

void Menu::addItem(MenuItemCode code, std::string_view label)
{
	auto item   = std::make_unique<MenuItem>();
	item->code  = code;
	item->label = label;
	items.emplace_back(std::move(item));
}

Menu::MenuItemCode Menu::pick(DisplayMode mode)
{
	int selectedItem = 0;
	int menuX, menuY;

	if (mode == DisplayMode::PAUSE) {
		menuX = engine.screenWidth  / 2 - constants::PAUSE_MENU_WIDTH  / 2;
		menuY = engine.screenHeight / 2 - constants::PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(Colors::menuFrame);
		TCODConsole::root->printFrame(menuX, menuY,
			constants::PAUSE_MENU_WIDTH, constants::PAUSE_MENU_HEIGHT,
			true, TCOD_BKGND_ALPHA(70), "menu");
		menuX += 2;
		menuY += 3;
	} else {
		static TCODImage backgroundImage("menu_background1.png");
		backgroundImage.blit2x(TCODConsole::root, 0, 0);
		menuX = 10;
		menuY = TCODConsole::root->getHeight() / 3;
	}

	while (!TCODConsole::isWindowClosed()) {
		pollInput(engine.inputState);

		int row = 0;
		for (const auto& item : items) {
			TCODConsole::root->setDefaultForeground(
				(row == selectedItem) ? Colors::menuHighlightAlt : Colors::uiText);
			TCODConsole::root->print(menuX, menuY + row * 3, item->label.c_str());
			row++;
		}
		TCODConsole::flush();

		if (engine.inputState.key.pressed) {
			switch (engine.inputState.key.key) {
			case SDLK_UP:
				selectedItem = (selectedItem > 0) ? selectedItem - 1 : static_cast<int>(items.size()) - 1;
				break;
			case SDLK_DOWN:
				selectedItem = (selectedItem + 1) % static_cast<int>(items.size());
				break;
			case SDLK_RETURN: {
				auto it = items.begin();
				std::advance(it, selectedItem);
				return (*it)->code;
			}
			default: break;
			}
		}

		if (engine.inputState.windowClosed) {
			return MenuItemCode::NONE;
		}
	}
	return MenuItemCode::NONE;
}

// ─── Camera ──────────────────────────────────────────────────────────────────

Camera::Camera(int x, int y, int width, int height, int mapWidth, int mapHeight)
	: x{ x }, y{ y }, width{ width }, height{ height }
	, mapWidth{ mapWidth }, mapHeight{ mapHeight }
{}

std::tuple<int, int> Camera::apply(int worldX, int worldY)
{
	return { worldX + x, worldY + y };
}

std::tuple<int, int> Camera::getWorldLocation(int screenX, int screenY)
{
	return { screenX - x, screenY - y };
}

void Camera::update(Actor* trackedActor, bool isOutdoor)
{
	if (!isOutdoor) {
		// BSP: always centre on player
		x = -trackedActor->getX() + width / 2;
		y = -trackedActor->getY() + height / 2;
	} else {
		// Outdoor: scroll by 1 tile when player is within margin of viewport edge.
		// First, check if camera is far from the player (e.g., just entered the level)
		// and snap to centre if so.
		int screenX = trackedActor->getX() + x;
		int screenY = trackedActor->getY() + y;

		// If player is completely outside the viewport, snap to centre
		if (screenX < 0 || screenX >= width || screenY < 0 || screenY >= height) {
			x = -trackedActor->getX() + width / 2;
			y = -trackedActor->getY() + height / 2;
		} else {
			// Gradual scroll when near edges
			if (screenX < scrollMargin)
				x += 1;
			else if (screenX >= width - scrollMargin)
				x -= 1;

			if (screenY < scrollMargin)
				y += 1;
			else if (screenY >= height - scrollMargin)
				y -= 1;
		}
	}

	// Clamp to map bounds
	if (x > 0) x = 0;
	if (y > 0) y = 0;
	if (x < -(mapWidth - width)) x = -(mapWidth - width);
	if (y < -(mapHeight - height)) y = -(mapHeight - height);
}
