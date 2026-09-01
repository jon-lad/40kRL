#pragma once

enum class HitLocation : int {
	HEAD = 0,
	RIGHT_ARM,
	LEFT_ARM,
	BODY,
	RIGHT_LEG,
	LEFT_LEG,
	COUNT
};

namespace HitLocationTable {
	// Reverses the d100 roll digits and maps to a HitLocation.
	// Input: the original d100 roll (1-100).
	HitLocation resolve(int d100Roll);

	// Returns display name for a HitLocation ("Head", "Body", etc.)
	const char* name(HitLocation loc);
}
