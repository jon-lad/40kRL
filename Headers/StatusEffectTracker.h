#pragma once
// StatusEffectTracker.h — Tracks active status effects on an actor.
// Implements stacking (refresh if new > existing, keep permanent), idempotent removal,
// and query interface for active effects.

#include "StatusEffects.h"
#include <vector>
#include <string>
#include <algorithm>

class Actor; // forward declaration

class StatusEffectTracker {
public:
    StatusEffectTracker() = default;

    // Apply a new effect. Handles stacking:
    //   - If same type already permanent (duration 0): keep permanent, ignore new
    //   - If same type exists and new duration > existing: update to new duration
    //   - If same type exists and existing >= new: no change
    //   - If type is new: add to effects vector
    void apply(Actor* owner, StatusType type, int duration, const std::string& source);

    // Remove a specific effect type. Idempotent (no-op if not present).
    void remove(Actor* owner, StatusType type);

    // Called at the start of the actor's turn. Decrements durations, deals tick damage.
    // Returns true if actor can act this turn (false if Stunned).
    bool tickStartOfTurn(Actor* owner);

    // Called at the end of the actor's turn. Deals Bleeding damage (1 wound).
    void tickEndOfTurn(Actor* owner);

    // Query interface
    bool has(StatusType type) const;
    int getRemainingDuration(StatusType type) const;
    const std::vector<StatusEffect>& getActiveEffects() const;
    bool canAct() const;               // false if Stunned
    bool isMovementHalved() const;     // true if any Missing_Leg
    int getWSModifier() const;         // sum of WS penalties from active effects
    int getBSModifier() const;         // sum of BS penalties from active effects
    int getEvasionModifier() const;    // Stunned: -20
    int getSModifier() const;          // Poisoned: -10
    int getTModifier() const;          // Poisoned: -10
    int getAgModifier() const;         // Poisoned: -10

    // Returns the modifier applied to ranged attacks targeting this actor (Prone: -10)
    int getRangedTargetingModifier() const;

    // Returns the bonus attackers receive when targeting this actor (Blinded: +30)
    int getAttackerBonusAgainstMe() const;

    // Returns true if the actor cannot equip weapons in the given slot
    bool isSlotDisabled(int slot) const;

    // Returns true if the actor cannot equip two-handed weapons (any Missing_Arm)
    bool isTwoHandedBlocked() const;

    // Serialization — saves/loads all active effects to/from a TCODZip archive.
    void save(TCODZip& zip);
    void load(TCODZip& zip);

    // Reapply all characteristic modifiers after load (call after load + owner is available)
    void reapplyModifiers(Actor* owner);

    // Modifier management — applies/removes characteristic overlays on owner
    void applyModifiers(Actor* owner, StatusType type);
    void removeModifiers(Actor* owner, StatusType type);

    // Logging — posts messages to the GUI message log on status apply/expire
    void logApplication(Actor* owner, StatusType type);
    void logExpiry(Actor* owner, StatusType type);

private:
    std::vector<StatusEffect> effects_;
};
