#pragma once
#include <atomic>

class SpinLock {
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void lock()
    {
        while (locked.test_and_set(std::memory_order_acquire))
        {
            Sleep(0);
        }
    }

    void unlock()
    {
        locked.clear(std::memory_order_release);
    }
};
