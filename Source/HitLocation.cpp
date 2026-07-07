#include "HitLocation.h"

namespace HitLocationTable {

HitLocation resolve(int d100Roll)
{
	// Special case: roll of 100 is treated as "00" for reversal purposes.
	// Reversed 00 maps to the 81-100 range (Left Leg).
	int reversed;
	if (d100Roll == 100) {
		reversed = 0;
	}
	else {
		reversed = (d100Roll % 10) * 10 + (d100Roll / 10);
	}

	// Map reversed value to hit location range.
	// A reversed value of 0 (from roll 100) falls in the 81-00 bucket.
	if (reversed == 0) {
		return HitLocation::LEFT_LEG;
	}
	else if (reversed <= 10) {
		return HitLocation::HEAD;
	}
	else if (reversed <= 20) {
		return HitLocation::RIGHT_ARM;
	}
	else if (reversed <= 30) {
		return HitLocation::LEFT_ARM;
	}
	else if (reversed <= 70) {
		return HitLocation::BODY;
	}
	else if (reversed <= 80) {
		return HitLocation::RIGHT_LEG;
	}
	else {
		return HitLocation::LEFT_LEG;
	}
}

const char* name(HitLocation loc)
{
	switch (loc) {
	case HitLocation::HEAD:      return "Head";
	case HitLocation::RIGHT_ARM: return "Right Arm";
	case HitLocation::LEFT_ARM:  return "Left Arm";
	case HitLocation::BODY:      return "Body";
	case HitLocation::RIGHT_LEG: return "Right Leg";
	case HitLocation::LEFT_LEG:  return "Left Leg";
	default:                     return "Unknown";
	}
}

} // namespace HitLocationTable
