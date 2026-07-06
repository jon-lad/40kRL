#include "CriticalEffects.h"
#include <algorithm>
#include <array>

namespace CriticalEffects {

// Static lookup table: [HitLocation][magnitude-1] -> {description, fatal}
// Magnitudes 1-4 are non-fatal, magnitudes 5-10 are always fatal.
struct CritEntry {
	const char* description;
	bool fatal;
};

static constexpr int NUM_LOCATIONS = static_cast<int>(HitLocation::COUNT);
static constexpr int MAX_MAGNITUDE = 10;

// Table indexed by [location][magnitude-1]
static const CritEntry critTable[NUM_LOCATIONS][MAX_MAGNITUDE] = {
	// HEAD (0)
	{
		{"A glancing blow rattles the skull, causing disorientation.", false},
		{"A solid strike to the head draws blood and blurs vision.", false},
		{"The blow cracks bone, sending teeth flying.", false},
		{"A brutal hit caves in the helm, leaving the target reeling.", false},
		{"The skull is shattered by the force of the blow. Death is instant.", true},
		{"The head is split open, spraying gore across the battlefield.", true},
		{"A devastating strike pulverises the skull into fragments.", true},
		{"The head is severed clean from the shoulders.", true},
		{"The cranium explodes in a shower of bone and brain matter.", true},
		{"The blow obliterates the head entirely, leaving nothing recognisable.", true},
	},
	// RIGHT_ARM (1)
	{
		{"The arm is jarred painfully, numbing the fingers.", false},
		{"A deep gash opens along the forearm, blood flowing freely.", false},
		{"Bone cracks audibly as the weapon strikes the upper arm.", false},
		{"The arm hangs limp, tendons severed by the vicious blow.", false},
		{"The arm is cleaved off at the elbow in a spray of arterial blood.", true},
		{"The limb is torn away at the shoulder, shock setting in instantly.", true},
		{"The arm disintegrates under the catastrophic force of the strike.", true},
		{"The blow passes through the arm and deep into the torso.", true},
		{"The entire right side caves in as the arm is ripped away.", true},
		{"The strike annihilates the arm and shoulder, severing the torso.", true},
	},
	// LEFT_ARM (2)
	{
		{"The arm is jarred painfully, numbing the fingers.", false},
		{"A deep gash opens along the forearm, blood flowing freely.", false},
		{"Bone cracks audibly as the weapon strikes the upper arm.", false},
		{"The arm hangs limp, tendons severed by the vicious blow.", false},
		{"The arm is cleaved off at the elbow in a spray of arterial blood.", true},
		{"The limb is torn away at the shoulder, shock setting in instantly.", true},
		{"The arm disintegrates under the catastrophic force of the strike.", true},
		{"The blow passes through the arm and deep into the torso.", true},
		{"The entire left side caves in as the arm is ripped away.", true},
		{"The strike annihilates the arm and shoulder, severing the torso.", true},
	},
	// BODY (3)
	{
		{"A solid hit to the torso knocks the wind out of the target.", false},
		{"Ribs crack under the impact, causing sharp pain with every breath.", false},
		{"The blow punches through armour, leaving a ragged wound in the flesh.", false},
		{"Internal organs are bruised and battered, blood pooling inside.", false},
		{"The torso is split open, spilling viscera onto the ground.", true},
		{"The spine is severed by the force, the body crumpling lifelessly.", true},
		{"The blow rips through the chest cavity, destroying the heart.", true},
		{"The torso is nearly bisected, held together by scraps of flesh.", true},
		{"Organs and bone explode outward from the devastating impact.", true},
		{"The body is torn apart completely, painting the ground crimson.", true},
	},
	// RIGHT_LEG (4)
	{
		{"The leg buckles from a painful strike to the thigh.", false},
		{"A deep wound opens on the shin, making movement agonising.", false},
		{"The kneecap cracks from the impact, destabilising the target.", false},
		{"The leg is mangled, bone protruding through torn muscle.", false},
		{"The leg is severed below the knee, blood jetting from the stump.", true},
		{"The limb is hewn off at the hip, the target collapsing instantly.", true},
		{"The leg explodes into fragments of bone and gristle.", true},
		{"The blow severs the femoral artery, death follows in seconds.", true},
		{"The entire lower body is shattered by the tremendous strike.", true},
		{"The leg and pelvis are destroyed, the body torn asunder.", true},
	},
	// LEFT_LEG (5)
	{
		{"The leg buckles from a painful strike to the thigh.", false},
		{"A deep wound opens on the shin, making movement agonising.", false},
		{"The kneecap cracks from the impact, destabilising the target.", false},
		{"The leg is mangled, bone protruding through torn muscle.", false},
		{"The leg is severed below the knee, blood jetting from the stump.", true},
		{"The limb is hewn off at the hip, the target collapsing instantly.", true},
		{"The leg explodes into fragments of bone and gristle.", true},
		{"The blow severs the femoral artery, death follows in seconds.", true},
		{"The entire lower body is shattered by the tremendous strike.", true},
		{"The leg and pelvis are destroyed, the body torn asunder.", true},
	},
};

CritEffect resolve(HitLocation location, int magnitude) {
	// Clamp magnitude to [1, 10]
	int clamped = std::clamp(magnitude, 1, MAX_MAGNITUDE);

	int locIndex = static_cast<int>(location);
	// Safety check for invalid location
	if (locIndex < 0 || locIndex >= NUM_LOCATIONS) {
		locIndex = static_cast<int>(HitLocation::BODY);
	}

	const CritEntry& entry = critTable[locIndex][clamped - 1];
	return CritEffect{std::string(entry.description), entry.fatal};
}

} // namespace CriticalEffects
