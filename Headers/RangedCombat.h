#pragma once

#include <functional>
#include "HitLocation.h"
#include "Equippable.h"

class Actor;

namespace RangedCombat {

	struct RangedContext {
		Actor* shooter;
		Actor* target;
		RangedStats weaponStats;
		int currentAmmo;                       // ammo before firing
		std::function<int()> rollD100;         // injectable RNG: returns [1, 100]
		std::function<int(int)> rollDie;       // injectable RNG: takes sides, returns [1, sides]
	};

	struct RangedResult {
		bool hit = false;
		int doS = 0;
		int doF = 0;
		HitLocation location = HitLocation::BODY;
		bool dodged = false;
		int finalDamage = 0;
		int ammoConsumed = 0;
		bool targetKilled = false;
	};

	// Resolves a full ranged attack against a character (has Characteristics).
	RangedResult resolveCharacterAttack(const RangedContext& ctx);

	// Resolves a ranged attack against a destructible (no Characteristics).
	RangedResult resolveDestructibleAttack(const RangedContext& ctx);

	// Top-level dispatch: checks target type, resolves, logs messages, applies wounds.
	// If rollD100/rollDie are null, uses default random.
	void resolve(Actor* shooter, Actor* target,
	             std::function<int()> rollD100 = nullptr,
	             std::function<int(int)> rollDie = nullptr);
}
