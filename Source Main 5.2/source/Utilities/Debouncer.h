#pragma once

#include <chrono>
#include <functional>

class Debouncer {
public:
    explicit Debouncer(int period): 
        interval(std::chrono::milliseconds(period)), 
        resetInactivityPeriod(2 * interval),
        created(std::chrono::high_resolution_clock::now()),
        firstCall(true) {}

    void debounce(std::function<void()> callback) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = now - created;
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

        // Reset firstCall if inactive for twice the debounce period
        if (elapsedMs > resetInactivityPeriod) {
            firstCall = true;  // Allow immediate function call again
        }

        // Call immediately on the first invocation
        if (firstCall) {
            callback();
            created = now;
            firstCall = false;
            return;
        }

        // Only call if the debounce period has passed
        if (elapsedMs > interval) {
            callback();
            created = now;
        }
    }

private:
    std::chrono::milliseconds interval;
    std::chrono::milliseconds resetInactivityPeriod;
    std::chrono::high_resolution_clock::time_point created;
    bool firstCall;
};


