#include "main.hpp"

#include <algorithm>

ActionBudget::ActionBudget()
	: ap_(MAX_AP), reactions_(MAX_REACTIONS), aimBonus_(0) {
}

// ── Turn lifecycle ──

void ActionBudget::beginTurn() {
	ap_ = MAX_AP;
	refreshReaction();
	clearAimBonus();
}

void ActionBudget::endTurn() {
	clearAimBonus();
}

// ── AP management ──

int ActionBudget::getAP() const {
	return ap_;
}

bool ActionBudget::canAfford(int cost) const {
	return ap_ >= cost;
}

bool ActionBudget::spend(int cost) {
	if (!canAfford(cost)) {
		return false;
	}
	ap_ -= cost;
	return true;
}

void ActionBudget::setAP(int value) {
	ap_ = value;
}

// ── Reaction management ──

bool ActionBudget::hasReaction() const {
	return reactions_ > 0;
}

void ActionBudget::useReaction() {
	reactions_ = 0;
}

void ActionBudget::forfeitReaction() {
	reactions_ = 0;
}

void ActionBudget::refreshReaction() {
	reactions_ = MAX_REACTIONS;
}

// ── Aim bonus ──

int ActionBudget::getAimBonus() const {
	return aimBonus_;
}

void ActionBudget::addAimBonus() {
	aimBonus_ = std::min(aimBonus_ + AIM_PER_ACTION, MAX_AIM_BONUS);
}

void ActionBudget::consumeAimBonus() {
	aimBonus_ = 0;
}

void ActionBudget::clearAimBonus() {
	aimBonus_ = 0;
}

// ── Serialization ──

void ActionBudget::save(TCODZip& zip) {
	zip.putInt(ap_);
	zip.putInt(reactions_);
	zip.putInt(aimBonus_);
}

void ActionBudget::load(TCODZip& zip) {
	ap_ = zip.getInt();
	reactions_ = zip.getInt();
	aimBonus_ = zip.getInt();
}
