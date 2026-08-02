
#include <list>
#include <memory>
#include <string>
#include <sstream>
#include <tuple>
#include "main.h"

// ─── Gui ─────────────────────────────────────────────────────────────────────

Gui::Gui()
{
	// hudConsole is used as a scratch buffer for rendering the HP bar graphic (1 row)
	hudConsole = std::make_unique<TCODConsole>(constants::BAR_WIDTH + 1, 1);
	msgLogConsole = std::make_unique<TCODConsole>(layout::VIEWPORT_WIDTH, layout::MSG_LOG_HEIGHT);
	rightSidebarConsole = std::make_unique<TCODConsole>(layout::RIGHT_SIDEBAR_WIDTH, layout::SCREEN_HEIGHT);

	// Only create left sidebar console when enabled; otherwise remains nullptr
	if constexpr (layout::LEFT_SIDEBAR_ENABLED) {
		leftSidebarConsole = std::make_unique<TCODConsole>(layout::LEFT_SIDEBAR_WIDTH, layout::SCREEN_HEIGHT);
	}
}

void Gui::render()
{
	// ─── Render order: left sidebar → right sidebar → HUD (message log + HP bar + skill bar) ───

	// 1. Left sidebar (no-op when disabled)
	renderLeftSidebar();

	// 2. Right sidebar
	renderRightSidebar();

	// 3. HUD: Message log
	renderMessageLog();

	// 4. HUD: HP bar + status info rendered directly to root console
	//    Positioned at the row between message log and skill bar (row 48).
	{
		const int hpBarRow = layout::VIEWPORT_HEIGHT + layout::MSG_LOG_HEIGHT;
		const int hpBarX = layout::VIEWPORT_X;

		// Clear the HP bar row
		TCODConsole::root->setDefaultBackground(Colors::black);
		for (int col = hpBarX; col < hpBarX + layout::VIEWPORT_WIDTH; ++col) {
			TCODConsole::root->putChar(col, hpBarRow, ' ', TCOD_BKGND_SET);
		}

		// HP bar (rendered into hudConsole for the bar visual, then blit just 1 row)
		hudConsole->setDefaultBackground(Colors::black);
		hudConsole->clear();

		renderBar(0, 0, constants::BAR_WIDTH, "HP",
			engine.player->destructible->hp,
			engine.player->destructible->maxHp,
			Colors::damageLight, Colors::damageDark);

		// Blit just the HP bar row from hudConsole to root at the correct position
		TCODConsole::blit(hudConsole.get(), 0, 0, constants::BAR_WIDTH + 1, 1,
			TCODConsole::root, hpBarX, hpBarRow);

		// Dungeon level label beside the HP bar
		TCODConsole::root->setDefaultForeground(Colors::white);
		std::stringstream levelLabel;
		levelLabel << "Dungeon level " << engine.dungeonLevel;
		TCODConsole::root->printf(hpBarX + constants::BAR_WIDTH + 2, hpBarRow, levelLabel.str().c_str());

		// Ammo display beside dungeon level
		if (engine.player->equipment) {
			Actor* weapon = engine.player->equipment->getSlot(EquipmentSlot::WEAPON);
			if (weapon && weapon->equippable && weapon->equippable->rangedStats.has_value()) {
				std::stringstream ammoLabel;
				ammoLabel << "Ammo: " << weapon->equippable->currentAmmo
				          << "/" << weapon->equippable->rangedStats->clipSize;
				TCODConsole::root->setDefaultForeground(Colors::white);
				TCODConsole::root->printf(hpBarX + constants::BAR_WIDTH + 22, hpBarRow, ammoLabel.str().c_str());
			}
		}

		// XP display
		if (engine.player->career) {
			std::stringstream xpLabel;
			xpLabel << "XP: " << engine.player->career->availableXp()
			        << "/" << engine.player->career->xpPool
			        << " Rank " << engine.player->career->currentRank;
			TCODConsole::root->setDefaultForeground(Colors::uiText);
			TCODConsole::root->printf(hpBarX + constants::BAR_WIDTH + 42, hpBarRow, xpLabel.str().c_str());
		}
	}

	// 5. HUD: Skill bar
	std::vector<SkillBarEntry> skills; // empty until skill system is implemented
	renderSkillBar(skills);
}

void Gui::renderSkillBar(const std::vector<SkillBarEntry>& skills)
{
	// Skill bar renders at row VIEWPORT_HEIGHT + MSG_LOG_HEIGHT + 1 on the root console.
	const int barY = layout::VIEWPORT_HEIGHT + layout::MSG_LOG_HEIGHT + 1;
	const int barX = layout::VIEWPORT_X;
	const int maxWidth = layout::VIEWPORT_WIDTH;

	// Clear the skill bar row
	TCODConsole::root->setDefaultBackground(Colors::black);
	for (int col = barX; col < barX + maxWidth; ++col) {
		TCODConsole::root->putChar(col, barY, ' ', TCOD_BKGND_SET);
	}

	int cursorX = barX;
	for (const auto& skill : skills) {
		// Format: "[key] Name  " — 2 brackets + 1 char + 1 space + name + 2 trailing spaces
		std::string entry = "[" + std::string(1, skill.keybinding) + "] " + skill.name;
		int entryWidth = static_cast<int>(entry.size()) + 2; // +2 for spacing between entries

		// Truncate if more skills than fit on screen width
		if (cursorX + static_cast<int>(entry.size()) > barX + maxWidth) {
			break;
		}

		// Apply color based on availability
		TCODColor color = skill.available
			? Colors::white
			: TCODColor(SkillBarColors::DIMMED_R, SkillBarColors::DIMMED_G, SkillBarColors::DIMMED_B);

		TCODConsole::root->setDefaultForeground(color);
		TCODConsole::root->printf(cursorX, barY, entry.c_str());

		cursorX += entryWidth;
	}
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

void Gui::renderMessageLog()
{
	// Render the message log into its dedicated sub-console, then blit to root.
	// Position: (VIEWPORT_X, VIEWPORT_HEIGHT), size: (VIEWPORT_WIDTH × MSG_LOG_HEIGHT)
	msgLogConsole->setDefaultBackground(Colors::black);
	msgLogConsole->clear();

	// Draw messages oldest-at-top, newest-at-bottom.
	// Apply a fading effect: oldest messages are dimmer, newest are brighter.
	int row = 0;
	float dimFactor = 0.4f;
	float dimStep = (log.size() > 1) ? (0.6f / (static_cast<float>(log.size()) - 1.0f)) : 0.0f;

	for (const auto& msg : log) {
		msgLogConsole->setDefaultForeground(msg->col * dimFactor);
		msgLogConsole->printf(1, row, msg->text.c_str());
		row++;
		dimFactor += dimStep;
	}

	// Blit the message log console to the root console at the dedicated region
	TCODConsole::blit(msgLogConsole.get(), 0, 0,
		layout::VIEWPORT_WIDTH, layout::MSG_LOG_HEIGHT,
		TCODConsole::root, layout::VIEWPORT_X, layout::VIEWPORT_HEIGHT);
}

void Gui::renderLeftSidebar()
{
	// No-op when the left sidebar is disabled (console is nullptr)
	if (!leftSidebarConsole) { return; }

	leftSidebarConsole->setDefaultBackground(Colors::black);
	leftSidebarConsole->clear();

	// Placeholder content until talent system is implemented
	leftSidebarConsole->setDefaultForeground(Colors::uiText);
	leftSidebarConsole->printf(1, 1, "No talents");

	// Blit to root console at (0, 0)
	TCODConsole::blit(leftSidebarConsole.get(), 0, 0,
		layout::LEFT_SIDEBAR_WIDTH, layout::SCREEN_HEIGHT,
		TCODConsole::root, 0, 0);
}

void Gui::renderRightSidebar()
{
	rightSidebarConsole->setDefaultBackground(Colors::black);
	rightSidebarConsole->clear();

	const int sidebarWidth = layout::RIGHT_SIDEBAR_WIDTH;
	int row = 1;

	// Handle null player gracefully
	if (!engine.player) {
		rightSidebarConsole->setDefaultForeground(Colors::uiText);
		rightSidebarConsole->printf(1, row, "No player data");
		TCODConsole::blit(rightSidebarConsole.get(), 0, 0,
			sidebarWidth, layout::SCREEN_HEIGHT,
			TCODConsole::root, layout::SCREEN_WIDTH - layout::RIGHT_SIDEBAR_WIDTH, 0);
		return;
	}

	// ─── Equipment Section ───────────────────────────────────────────────
	rightSidebarConsole->setDefaultForeground(Colors::white);
	rightSidebarConsole->printf(1, row, "-- Equipment --");
	row += 2;

	if (engine.player->equipment) {
		const char* slotNames[] = { "Weapon", "Offhand", "Head", "Body" };
		for (int i = 0; i < static_cast<int>(EquipmentSlot::COUNT); ++i) {
			EquipmentSlot slot = static_cast<EquipmentSlot>(i);
			Actor* item = engine.player->equipment->getSlot(slot);
			rightSidebarConsole->setDefaultForeground(Colors::uiText);
			if (item) {
				// Truncate name to fit sidebar width (leave room for slot label)
				std::string display = std::string(slotNames[i]) + ": " + item->name;
				if (static_cast<int>(display.size()) > sidebarWidth - 2) {
					display = display.substr(0, sidebarWidth - 5) + "...";
				}
				rightSidebarConsole->printf(1, row, display.c_str());
			} else {
				std::string display = std::string(slotNames[i]) + ": --empty--";
				rightSidebarConsole->printf(1, row, display.c_str());
			}
			row++;
		}
	} else {
		rightSidebarConsole->setDefaultForeground(Colors::uiText);
		rightSidebarConsole->printf(1, row, "No equipment");
		row++;
	}

	row++;

	// ─── Ammo Section ────────────────────────────────────────────────────
	rightSidebarConsole->setDefaultForeground(Colors::white);
	rightSidebarConsole->printf(1, row, "-- Ammo --");
	row += 2;

	if (engine.player->equipment) {
		bool hasRanged = false;
		const auto& slots = engine.player->equipment->getSlots();
		for (const auto* item : slots) {
			if (item && item->equippable && item->equippable->rangedStats.has_value()) {
				hasRanged = true;
				std::stringstream ammoLine;
				ammoLine << item->name << ": "
				         << item->equippable->currentAmmo << "/"
				         << item->equippable->rangedStats->clipSize;
				std::string display = ammoLine.str();
				if (static_cast<int>(display.size()) > sidebarWidth - 2) {
					display = display.substr(0, sidebarWidth - 5) + "...";
				}
				rightSidebarConsole->setDefaultForeground(Colors::uiText);
				rightSidebarConsole->printf(1, row, display.c_str());
				row++;
			}
		}
		if (!hasRanged) {
			rightSidebarConsole->setDefaultForeground(Colors::uiText);
			rightSidebarConsole->printf(1, row, "No ranged weapon");
			row++;
		}
	}

	row++;

	// ─── Characteristics Section ─────────────────────────────────────────
	rightSidebarConsole->setDefaultForeground(Colors::white);
	rightSidebarConsole->printf(1, row, "-- Characteristics --");
	row += 2;

	if (engine.player->characteristics) {
		// Display all 9 stats in rows of 3
		// Row 1: WS, BS, S
		// Row 2: T, Ag, Int
		// Row 3: Per, WP, Fel
		const CharId statOrder[] = {
			CharId::WS, CharId::BS, CharId::S,
			CharId::T, CharId::Ag, CharId::Int,
			CharId::Per, CharId::WP, CharId::Fel
		};

		for (int i = 0; i < 9; i += 3) {
			std::stringstream statLine;
			for (int j = 0; j < 3; ++j) {
				CharId id = statOrder[i + j];
				statLine << Characteristics::abbreviation(id) << " "
				         << engine.player->characteristics->get(id);
				if (j < 2) statLine << "  ";
			}
			rightSidebarConsole->setDefaultForeground(Colors::uiText);
			rightSidebarConsole->printf(1, row, statLine.str().c_str());
			row++;
		}
	} else {
		rightSidebarConsole->setDefaultForeground(Colors::uiText);
		rightSidebarConsole->printf(1, row, "No characteristics");
		row++;
	}

	row++;

	// ─── Skills Section ──────────────────────────────────────────────────
	rightSidebarConsole->setDefaultForeground(Colors::white);
	rightSidebarConsole->printf(1, row, "-- Skills --");
	row += 2;

	if (engine.player->career && !engine.player->career->skills.empty()) {
		for (const auto& [skillName, rank] : engine.player->career->skills) {
			if (row >= layout::SCREEN_HEIGHT - 1) break; // don't overflow sidebar
			rightSidebarConsole->setDefaultForeground(Colors::uiText);
			rightSidebarConsole->printf(1, row, skillName.c_str());
			row++;
		}
	} else {
		rightSidebarConsole->setDefaultForeground(Colors::uiText);
		rightSidebarConsole->printf(1, row, "No skills");
		row++;
	}

	// Blit sidebar to root console at (SCREEN_WIDTH - RIGHT_SIDEBAR_WIDTH, 0)
	TCODConsole::blit(rightSidebarConsole.get(), 0, 0,
		sidebarWidth, layout::SCREEN_HEIGHT,
		TCODConsole::root, layout::SCREEN_WIDTH - layout::RIGHT_SIDEBAR_WIDTH, 0);
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
