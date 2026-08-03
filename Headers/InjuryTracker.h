#pragma once

#include <array>
#include "HitLocation.h"
#include "Persistent.h"

class Actor;
class TCODZip;

// Tracks active critical injuries on an Actor and manages their stat modifiers.
// Stub header for TDD — implementation in Source/InjuryTracker.cpp (task 3.5).
class InjuryTracker : public Persistent {
public:
    static constexpr int MAX_LOCATIONS = static_cast<int>(HitLocation::COUNT);
    static constexpr int MAX_MAGNITUDE = 4; // non-fatal range

    InjuryTracker();

    // Applies or escalates an injury at the given location.
    // magnitude is clamped to [1, 4] before applying.
    // Returns true if the injury was applied/escalated within non-fatal range.
    // Returns false if escalation would exceed magnitude 4 (caller handles fatality).
    bool applyInjury(Actor* owner, HitLocation loc, int magnitude);

    // Removes all injuries and reverses all debuffs on the owner.
    void clearAll(Actor* owner);

    // Returns the current magnitude at a location (0 = no injury).
    int getMagnitude(HitLocation loc) const;

    // Returns true if any injury is active.
    bool hasInjuries() const;

    // Returns count of active injury locations.
    int activeCount() const;

    void save(TCODZip& zip) override;
    void load(TCODZip& zip) override;

    // Reapplies all debuffs to owner (used after load to restore modifier state).
    void reapplyDebuffs(Actor* owner);

private:
    // Magnitude per location. 0 = no injury, 1-4 = active.
    std::array<int, MAX_LOCATIONS> magnitudes_;

    // Removes debuff for a specific location (if any) from the owner's characteristics.
    void removeDebuff(Actor* owner, HitLocation loc, int magnitude);

    // Applies debuff for a specific location/magnitude to the owner's characteristics.
    void applyDebuff(Actor* owner, HitLocation loc, int magnitude);
};
