#pragma once

#include <string>
#include "HitLocation.h"

namespace CriticalEffects {
	struct CritEffect {
		std::string description;
		bool fatal;
	};

	// Returns the critical effect for a given location and damage magnitude.
	// magnitude is clamped to [1, 10] for table lookup.
	CritEffect resolve(HitLocation location, int magnitude);
}
