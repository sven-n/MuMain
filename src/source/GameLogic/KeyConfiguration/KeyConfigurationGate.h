#pragma once

#include <chrono>
#include <optional>

// When the client may save the selected character's key configuration. Until the stored one has arrived, the
// hotkeys in memory are the reset (empty) ones, and saving them would overwrite the character's stored layout. A
// character with a stored configuration gets it right after the join (OpenMU sends F3 30 just after F3 03, so a slow
// link delays both; the client handles it once the map has loaded, as the packets after F3 03 wait for the load); one
// without (a new character) gets none, so `Wait` after Joined (the map loaded) the configuration counts as known. A
// save asked before then waits and goes once the wait is over. Received and Joined may come in either order.
namespace GameLogic::KeyConfiguration
{
class KeyConfigurationGate
{
public:
    using Clock = std::chrono::steady_clock;
    static constexpr std::chrono::seconds Wait{2};

    // Another character is selected: nothing is known, and no save waits.
    void Reset();

    // The selected character joined the map server.
    void Joined(Clock::time_point now);

    // The stored configuration arrived. A waiting save is dropped when everything in memory is now the stored
    // one; when a part wasn't restored (a chat box byte that stores no size or alpha), the change made before is
    // still to be saved, and goes at the next TakeDueSave.
    void Received(bool everythingRestored);

    // A save is asked: true when it may go now; otherwise it waits (see TakeDueSave).
    bool RequestSave(Clock::time_point now);

    // A save that can't wait (the session ends): true when it may go now. A waiting save is not kept, as nothing
    // asks for it once the session is over.
    bool MaySaveNow(Clock::time_point now) const;

    // Asked each frame: true once, when a waiting save may go now.
    bool TakeDueSave(Clock::time_point now);

private:
    bool IsKnown(Clock::time_point now) const;

    bool m_received = false;
    bool m_pending = false;
    std::optional<Clock::time_point> m_joined;
};
} // namespace GameLogic::KeyConfiguration
