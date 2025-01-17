#pragma once

#include <chrono>
#include <functional>

class Debouncer {
public:
    explicit Debouncer(int period)
        : interval(std::chrono::milliseconds(period)),
        lastCallTime() {
    }

    void debounce(std::function<void()> callback) {
        auto now = std::chrono::high_resolution_clock::now();

        // Call immediately on the first invocation (lastCallTime is uninitialized)
        if (lastCallTime == std::chrono::high_resolution_clock::time_point() ||
            now - lastCallTime >= interval) {
            callback();
            lastCallTime = now;
        }
    }

private:
    std::chrono::milliseconds interval;
    std::chrono::high_resolution_clock::time_point lastCallTime;
};
