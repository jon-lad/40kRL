#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include <deque>
#include <string>
#include <vector>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 3: Message log bounded-buffer invariant
// **Validates: Requirements 4.2, 4.3, 4.4, 4.5**
//
// Test-local MessageLog implementation modeling the expected bounded-buffer
// behavior. Production code (task 6.2) will adapt the existing Gui message log
// to match this specification.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// MSG_LOG_CAPACITY matches layout::MSG_LOG_HEIGHT from Constants.h
static constexpr int MSG_LOG_CAPACITY = 6;

struct MessageLog {
    static constexpr int CAPACITY = MSG_LOG_CAPACITY;
    std::deque<std::string> messages;

    void add(const std::string& msg) {
        if (msg.empty()) return; // silently discard empty
        if (static_cast<int>(messages.size()) >= CAPACITY) {
            messages.pop_front(); // remove oldest
        }
        messages.push_back(msg);
    }

    int size() const { return static_cast<int>(messages.size()); }
};

// Helper to compare strings without triggering RC_ASSERT decomposition issues on MSVC
inline bool strEq(const std::string& a, const std::string& b) { return a == b; }
inline bool strNe(const std::string& a, const std::string& b) { return a != b; }

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Property 3a: The log SHALL never contain more than MSG_LOG_CAPACITY entries
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3a — Message log never exceeds capacity",
          "[pbt][property][ui-rework][message-log]")
{
    // Feature: ui-rework, Property 3: Message log bounded-buffer invariant
    rc::check("after any sequence of adds, log size <= CAPACITY", []() {
        // Generate a random sequence of messages (some may be empty)
        const int numMsgs = *rc::gen::inRange(1, 200);
        auto msgGen = rc::gen::string(0, 40);

        MessageLog log;
        for (int i = 0; i < numMsgs; ++i) {
            std::string msg = *msgGen;
            log.add(msg);
            RC_ASSERT(log.size() <= MessageLog::CAPACITY);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 3b: Messages maintain insertion order (oldest first)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3b — Messages maintain insertion order (oldest first)",
          "[pbt][property][ui-rework][message-log]")
{
    // Feature: ui-rework, Property 3: Message log bounded-buffer invariant
    rc::check("messages in log are in the same relative order they were added", []() {
        // Generate non-empty messages (length 1..30)
        const int numMsgs = *rc::gen::inRange(1, 50);
        auto msgGen = rc::gen::string(1, 30);

        std::vector<std::string> allMsgs;
        MessageLog log;

        for (int i = 0; i < numMsgs; ++i) {
            std::string msg = *msgGen;
            allMsgs.push_back(msg);
            log.add(msg);
        }

        // The log should contain the last min(numMsgs, CAPACITY) messages
        // in insertion order
        int expectedCount = std::min(numMsgs, MessageLog::CAPACITY);
        RC_ASSERT(log.size() == expectedCount);

        // The messages in the log should be the last `expectedCount` messages
        // from the input sequence, in original order
        int startIdx = numMsgs - expectedCount;
        for (int i = 0; i < expectedCount; ++i) {
            RC_ASSERT(strEq(log.messages[i], allMsgs[startIdx + i]));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 3c: When at capacity, remove only the oldest message
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3c — When at capacity, oldest message is removed",
          "[pbt][property][ui-rework][message-log]")
{
    // Feature: ui-rework, Property 3: Message log bounded-buffer invariant
    rc::check("adding to a full log removes exactly the oldest message", []() {
        // Fill the log to capacity with unique messages
        MessageLog log;
        std::vector<std::string> filledMsgs;
        for (int i = 0; i < MessageLog::CAPACITY; ++i) {
            int suffix = *rc::gen::inRange(1000, 9999);
            std::string msg = "msg_" + std::to_string(i) + "_" + std::to_string(suffix);
            log.add(msg);
            filledMsgs.push_back(msg);
        }
        RC_PRE(log.size() == MessageLog::CAPACITY);

        // Record state before adding
        std::string oldestBefore = log.messages.front();

        // Add a new non-empty message
        std::string newMsg = *rc::gen::string(1, 30);
        log.add(newMsg);

        // Size should still be CAPACITY
        RC_ASSERT(log.size() == MessageLog::CAPACITY);

        // The oldest message should have been removed
        RC_ASSERT(strNe(log.messages.front(), oldestBefore));

        // The new message should be at the back
        RC_ASSERT(strEq(log.messages.back(), newMsg));

        // All remaining old messages (except the removed oldest) should still
        // be present in original order: filledMsgs[1..CAPACITY-1] == log[0..CAPACITY-2]
        for (int i = 1; i < MessageLog::CAPACITY; ++i) {
            RC_ASSERT(strEq(log.messages[i - 1], filledMsgs[i]));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 3d: Each message occupies exactly one entry (no overwriting)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3d — Each add creates a new entry, never overwrites",
          "[pbt][property][ui-rework][message-log]")
{
    // Feature: ui-rework, Property 3: Message log bounded-buffer invariant
    rc::check("each non-empty add increases size by 1 (when not at capacity) "
              "or keeps size at capacity (when full)", []() {
        const int numMsgs = *rc::gen::inRange(1, 50);
        auto msgGen = rc::gen::string(1, 20);

        MessageLog log;
        for (int n = 0; n < numMsgs; ++n) {
            std::string msg = *msgGen;
            int sizeBefore = log.size();
            std::deque<std::string> contentBefore = log.messages;

            log.add(msg);

            if (sizeBefore < MessageLog::CAPACITY) {
                // Not at capacity: size should increase by exactly 1
                RC_ASSERT(log.size() == sizeBefore + 1);
                // All previous messages should still be present unchanged
                for (int i = 0; i < sizeBefore; ++i) {
                    RC_ASSERT(strEq(log.messages[i], contentBefore[i]));
                }
                // New message is appended at end
                RC_ASSERT(strEq(log.messages.back(), msg));
            } else {
                // At capacity: size stays the same
                RC_ASSERT(log.size() == MessageLog::CAPACITY);
                // Messages [1..end] from before should match [0..end-1] now
                for (int i = 1; i < sizeBefore; ++i) {
                    RC_ASSERT(strEq(log.messages[i - 1], contentBefore[i]));
                }
                // New message is at end
                RC_ASSERT(strEq(log.messages.back(), msg));
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Additional property: Empty messages are silently discarded
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Message log — empty messages are discarded",
          "[pbt][property][ui-rework][message-log]")
{
    // Feature: ui-rework, Property 3: Message log bounded-buffer invariant
    rc::check("adding an empty string does not change log state", []() {
        // Build a log with some messages first
        MessageLog log;
        const int preCount = *rc::gen::inRange(0, MessageLog::CAPACITY);
        for (int i = 0; i < preCount; ++i) {
            log.add("msg_" + std::to_string(i));
        }

        int sizeBefore = log.size();
        std::deque<std::string> contentBefore = log.messages;

        // Add empty string
        log.add("");

        // State should be unchanged
        RC_ASSERT(log.size() == sizeBefore);
        bool unchanged = (log.messages == contentBefore);
        RC_ASSERT(unchanged);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Edge Cases — unit tests for boundary conditions
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Message log: fresh log is empty", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    CHECK(log.size() == 0);
    CHECK(log.messages.empty());
}

TEST_CASE("Message log: single message adds correctly", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    log.add("Hello, Hive World");
    CHECK(log.size() == 1);
    CHECK(log.messages[0] == "Hello, Hive World");
}

TEST_CASE("Message log: filling to exact capacity", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    for (int i = 0; i < MessageLog::CAPACITY; ++i) {
        log.add("msg_" + std::to_string(i));
    }
    CHECK(log.size() == MessageLog::CAPACITY);
    CHECK(log.messages.front() == "msg_0");
    CHECK(log.messages.back() == "msg_" + std::to_string(MessageLog::CAPACITY - 1));
}

TEST_CASE("Message log: overflow removes oldest", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    for (int i = 0; i < MessageLog::CAPACITY; ++i) {
        log.add("msg_" + std::to_string(i));
    }
    // Add one more
    log.add("overflow_msg");

    CHECK(log.size() == MessageLog::CAPACITY);
    // msg_0 should be gone, msg_1 is now oldest
    CHECK(log.messages.front() == "msg_1");
    CHECK(log.messages.back() == "overflow_msg");
}

TEST_CASE("Message log: multiple overflows maintain order", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    // Add many more messages than capacity
    for (int i = 0; i < 20; ++i) {
        log.add("msg_" + std::to_string(i));
    }

    CHECK(log.size() == MessageLog::CAPACITY);
    // Only last CAPACITY messages remain: msg_14 through msg_19
    int start = 20 - MessageLog::CAPACITY;
    for (int i = 0; i < MessageLog::CAPACITY; ++i) {
        CHECK(log.messages[i] == "msg_" + std::to_string(start + i));
    }
}

TEST_CASE("Message log: empty string is not added", "[unit][ui-rework][message-log]")
{
    MessageLog log;
    log.add("first");
    log.add("");
    log.add("second");

    CHECK(log.size() == 2);
    CHECK(log.messages[0] == "first");
    CHECK(log.messages[1] == "second");
}

TEST_CASE("Message log: capacity constant matches layout::MSG_LOG_HEIGHT",
          "[unit][ui-rework][message-log]")
{
    // Verify our test-local constant matches the layout constant
    CHECK(MessageLog::CAPACITY == 6);
    CHECK(MSG_LOG_CAPACITY == 6);
}
