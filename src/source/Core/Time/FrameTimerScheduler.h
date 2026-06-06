#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Core::Time
{
    // Portable replacement for Win32 SetTimer/WM_TIMER. Subsystems register a
    // repeating timer by id; the main loop calls Tick() once per frame, which
    // fires every timer whose interval has elapsed.
    //
    // SetRepeating with an id that already exists replaces it (matching
    // SetTimer); Kill removes it (matching KillTimer). Timers fire on the main
    // thread from the main loop, exactly like the old WM_TIMER dispatch through
    // WndProc. The id space is shared, as it was with SetTimer(g_hWnd, id, ...).
    class FrameTimerScheduler
    {
    public:
        using TimerId = std::uintptr_t;
        using Callback = std::function<void()>;

        static FrameTimerScheduler& Instance();

        // Register or replace a repeating timer firing every intervalMs.
        void SetRepeating(TimerId id, unsigned intervalMs, Callback callback);

        // Remove a timer. No-op if the id is not registered.
        void Kill(TimerId id);

        // Fire all timers whose interval has elapsed. Call once per frame.
        void Tick();

    private:
        FrameTimerScheduler() = default;

        struct Timer
        {
            unsigned intervalMs;
            double nextDueMs;
            Callback callback;
        };

        static double NowMs();

        std::unordered_map<TimerId, Timer> m_timers;
    };
}
