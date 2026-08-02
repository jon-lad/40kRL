#pragma once

#include "Persistent.h"
#include "Characteristics.h"
#include "CareerProgression.h"
#include <memory>

// Unified character component for the player. Owns both characteristics and career data.
// Attached to the player Actor after character generation. NPCs use standalone Characteristics.
class CharacterSheet : public Persistent {
public:
	Characteristics characteristics{25};
	CareerProgression career;

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};
