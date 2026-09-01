#pragma once

// Tracks the action point budget, reaction availability, and aim bonus
// for a single actor across one turn/round.
class ActionBudget : public Persistent {
public:
	static constexpr int MAX_AP = 2;
	static constexpr int MAX_REACTIONS = 1;
	static constexpr int MAX_AIM_BONUS = 20;
	static constexpr int AIM_PER_ACTION = 10;

	ActionBudget();

	// ── Turn lifecycle ──
	void beginTurn();   // resets AP to MAX_AP, refreshes reaction, clears aim bonus
	void endTurn();     // clears any unused aim bonus

	// ── AP management ──
	int  getAP() const;
	bool canAfford(int cost) const;
	bool spend(int cost);       // returns false if insufficient AP
	void setAP(int value);      // for End_Turn (set to 0)

	// ── Reaction management ──
	bool hasReaction() const;
	void useReaction();
	void forfeitReaction();     // for All_Out_Attack
	void refreshReaction();     // called at start of actor's turn

	// ── Aim bonus ──
	int  getAimBonus() const;
	void addAimBonus();         // +AIM_PER_ACTION per call, capped at MAX_AIM_BONUS
	void consumeAimBonus();     // resets to 0 after attack
	void clearAimBonus();       // resets to 0 (called on turn end)

	// ── Serialization ──
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

private:
	int ap_ = MAX_AP;
	int reactions_ = MAX_REACTIONS;
	int aimBonus_ = 0;
};
