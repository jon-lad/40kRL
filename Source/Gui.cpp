
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
	hudConsole->setDefaultBackground(TCODColor::black);
	hudConsole->clear();

	renderBar(1, 1, constants::BAR_WIDTH, "HP",
		engine.player->destructible->hp,
		engine.player->destructible->maxHp,
		TCOD_light_red, TCOD_darker_red);

	PlayerAi* playerAi = static_cast<PlayerAi*>(engine.player->ai.get());
	if (playerAi) {
		std::stringstream xpLabel;
		xpLabel << "XP(" << playerAi->xpLevel << ")";
		renderBar(1, 5, constants::BAR_WIDTH, xpLabel.str(),
			static_cast<float>(engine.player->destructible->xp),
			static_cast<float>(playerAi->getNextLevelXp()),
			TCOD_light_violet, TCOD_darker_violet);
	}

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

	hudConsole->setDefaultForeground(TCOD_white);
	std::stringstream levelLabel;
	levelLabel << "Dungeon level " << engine.dungeonLevel;
	hudConsole->printf(3, 3, levelLabel.str().c_str());

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
	hudConsole->setDefaultForeground(TCOD_white);
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

	hudConsole->setDefaultForeground(TCODColor::lightGrey);
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
		TCODConsole::root->setDefaultForeground(TCODColor(200, 180, 50));
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
		int row = 0;
		for (const auto& item : items) {
			TCODConsole::root->setDefaultForeground(
				(row == selectedItem) ? TCOD_lighter_orange : TCOD_light_grey);
			TCODConsole::root->print(menuX, menuY + row * 3, item->label.c_str());
			row++;
		}
		TCODConsole::flush();

		pollInput(engine.inputState);
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
		// Outdoor: scroll by 1 tile when player is within margin of viewport edge
		int screenX = trackedActor->getX() + x;
		int screenY = trackedActor->getY() + y;

		if (screenX < scrollMargin)
			x += 1;
		else if (screenX >= width - scrollMargin)
			x -= 1;

		if (screenY < scrollMargin)
			y += 1;
		else if (screenY >= height - scrollMargin)
			y -= 1;
	}

	// Clamp to map bounds
	if (x > 0) x = 0;
	if (y > 0) y = 0;
	if (x < -(mapWidth - width)) x = -(mapWidth - width);
	if (y < -(mapHeight - height)) y = -(mapHeight - height);
}
