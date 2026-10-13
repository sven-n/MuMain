#pragma once

#include <span>
#include <string>
#include <vector>

namespace UI::Diagnostics
{
struct FrameSummary
{
    bool details = false;
    bool counters = false;
    bool fpsOnly = false;
    double fps = 0;
    double averageFps = 0;
    double lowFps = 0;
    double slowestFps = 0;
    double frameMs = 0;
    double cpu = 0;
    bool vsync = false;
    std::span<const float> history;
    int oldestSample = 0;
    // Which build runs where, and what the camera sees: one readable line each.
    std::vector<std::string> info;
};

void Initialize();
void Update(const FrameSummary& summary);
void Release();
}
