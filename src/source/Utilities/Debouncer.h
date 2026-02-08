#pragma once

#include <chrono>
#include <functional>

class DebouncedAction 
{
public:
    explicit DebouncedAction(std::function<void()> callback, int period)
        : interval(std::chrono::milliseconds(period)),
        lastCallTime(),
        callback(std::move(callback)) {}

    void invoke()
    {
        auto now = std::chrono::high_resolution_clock::now();

        // Call immediately on the first invocation or on the next interval
        if (lastCallTime == std::chrono::high_resolution_clock::time_point() ||
            now - lastCallTime >= interval) 
        {
            callback();
            lastCallTime = now;
        }
    }

private:
    std::chrono::milliseconds interval;
    std::chrono::high_resolution_clock::time_point lastCallTime;
    std::function<void()> callback;
};
