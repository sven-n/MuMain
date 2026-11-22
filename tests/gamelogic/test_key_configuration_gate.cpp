// Tests of when the client may save the selected character's key configuration.

#include "doctest.h"

#include "GameLogic/KeyConfiguration/KeyConfigurationGate.h"

using GameLogic::KeyConfiguration::KeyConfigurationGate;

namespace
{
const KeyConfigurationGate::Clock::time_point Join{};
const auto Before = Join + KeyConfigurationGate::Wait - std::chrono::milliseconds(1);
const auto After = Join + KeyConfigurationGate::Wait;
} // namespace

TEST_CASE("key configuration gate: a save before the join waits until the configuration or the wait is known")
{
    KeyConfigurationGate gate;
    CHECK_FALSE(gate.RequestSave(Join));
    CHECK_FALSE(gate.TakeDueSave(After));
    gate.Joined(Join);
    CHECK_FALSE(gate.RequestSave(Before));
    CHECK_FALSE(gate.TakeDueSave(Before));
    CHECK(gate.TakeDueSave(After));
    CHECK_FALSE(gate.TakeDueSave(After));
    CHECK(gate.RequestSave(After));
}

TEST_CASE("key configuration gate: the stored configuration allows saving at once and drops a waiting save")
{
    KeyConfigurationGate gate;
    gate.Joined(Join);
    CHECK_FALSE(gate.RequestSave(Join));
    gate.Received(true);
    CHECK_FALSE(gate.TakeDueSave(After));
    CHECK(gate.RequestSave(Join));
}

TEST_CASE("key configuration gate: a waiting save goes at once when the configuration didn't restore everything")
{
    KeyConfigurationGate gate;
    gate.Joined(Join);
    CHECK_FALSE(gate.RequestSave(Join));
    gate.Received(false);
    CHECK(gate.TakeDueSave(Join));
    CHECK_FALSE(gate.TakeDueSave(Join));
    // Nothing waited: nothing to send.
    KeyConfigurationGate idle;
    idle.Joined(Join);
    idle.Received(false);
    CHECK_FALSE(idle.TakeDueSave(Join));
}

// The order the client handles them in doesn't matter: a configuration before the join still counts.
TEST_CASE("key configuration gate: the configuration may come before the join, and a later join doesn't undo it")
{
    KeyConfigurationGate gate;
    gate.Received(true);
    CHECK(gate.RequestSave(Join));
    gate.Joined(Join);
    CHECK(gate.RequestSave(Join));
    CHECK_FALSE(gate.TakeDueSave(After));
}

TEST_CASE("key configuration gate: another character forgets the join, the configuration and the waiting save")
{
    KeyConfigurationGate gate;
    gate.Joined(Join);
    gate.Received(true);
    gate.Reset();
    CHECK_FALSE(gate.RequestSave(After));
    gate.Reset();
    gate.Joined(Join);
    CHECK_FALSE(gate.RequestSave(Before));
    gate.Reset();
    CHECK_FALSE(gate.TakeDueSave(After));
}

// Leaving can't wait for the 2 seconds: before then nothing is saved, after them (or once the configuration
// arrived) the save goes, and nothing is left waiting by asking.
TEST_CASE("key configuration gate: a save on leaving goes only when the configuration is known")
{
    KeyConfigurationGate gate;
    gate.Joined(Join);
    CHECK_FALSE(gate.MaySaveNow(Before));
    CHECK_FALSE(gate.TakeDueSave(After));
    CHECK(gate.MaySaveNow(After));
    KeyConfigurationGate received;
    received.Received(true);
    CHECK(received.MaySaveNow(Before));
    CHECK_FALSE(KeyConfigurationGate{}.MaySaveNow(After));
}
