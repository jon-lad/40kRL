#pragma once

#include <array>
#include <vector>
#include "HitLocation.hpp"
#include "Persistent.hpp"

class Actor;
class TCODZip;

// Records a single debuff applied from a specific critical hit event.
struct InjuryRecord {
    HitLocation location;  // where the crit landed
    int magnitude;         // total magnitude at time of this crit (determines debuff severity)
};

// Tracks critical injury state for an Actor.
// Uses a single cumulative magnitude counter. Each crit adds to the total,
// and the debuff applied depends on (hitLocation, newTotalMagnitude).
// Healing reduces magnitude first, then excess heals HP.
// Debuffs are permanent once applied (cleared only by future healing systems).
class InjuryTracker : public Persistent {
public:
    static constexpr int MAX_MAGNITUDE = 9;   // magnitudes 1-9 are non-fatal
    static constexpr int FATAL_MAGNITUDE = 10; // magnitude 10 = death

    InjuryTracker();

    // Returns current cumulative magnitude (0 = no crits taken).
    int getMagnitude() const;

    // Applies a critical hit: adds critMagnitude to the total, applies debuff
    // for (location, newTotal). Returns false if new total >= FATAL_MAGNITUDE
    // (caller handles death). Returns true if non-fatal.
    bool applyCrit(Actor* owner, HitLocation loc, int critMagnitude);

    // Records a fatal critical hit's location for display purposes WITHOUT applying
    // characteristic debuffs (the actor is dying/dead — no living characteristics to
    // modify). Pushes an InjuryRecord{loc, clampedMagnitude} and sets the cumulative
    // magnitude to reflect the killing blow. The magnitude is clamped to
    // [1, MAX_MAGNITUDE] so it round-trips symmetrically through save/load.
    // Engine-free / test-safe: never touches engine globals or calls die().
    void recordFatalCrit(HitLocation loc, int magnitude);

    // Reduces magnitude by the given amount. Returns excess (amount beyond reducing
    // magnitude to 0) which should be applied as HP healing.
    // Does NOT remove existing debuffs — they are permanent.
    int healMagnitude(int amount);

    // Returns true if any debuffs are active.
    bool hasInjuries() const;

    // Returns the number of active injury records (debuffs applied).
    int activeCount() const;

    // Returns the list of active injury records.
    const std::vector<InjuryRecord>& getRecords() const;

    // Removes all debuffs and resets magnitude to 0 (used for level transitions / full clear).
    void clearAll(Actor* owner);

    void save(TCODZip& zip) override;
    void load(TCODZip& zip) override;

    // Reapplies all debuffs to owner (used after load to restore modifier state).
    void reapplyDebuffs(Actor* owner);

private:
    int magnitude_;  // cumulative crit magnitude (0-9 non-fatal, 10 = fatal)
    std::vector<InjuryRecord> records_;  // each crit event that applied a debuff

    // Applies debuff for a specific location/magnitude to the owner's characteristics.
    void applyDebuff(Actor* owner, HitLocation loc, int magnitude);

    // Removes debuff for a specific location/magnitude from the owner's characteristics.
    void removeDebuff(Actor* owner, HitLocation loc, int magnitude);
};
