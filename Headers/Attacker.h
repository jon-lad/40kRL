#pragma once

// Gives an actor the ability to deal melee damage.
// Damage formula: effective_damage = power - target.defense
class Attacker : public Persistent {
public:
	float power; // raw attack power before the target's defence is applied

	explicit Attacker(float power);

	// Deals damage from owner to target and logs the result to the message log.
	void attack(Actor* owner, Actor* target);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};