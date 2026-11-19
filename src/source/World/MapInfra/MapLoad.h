#pragma once

#include "Core/Utilities/StagedLoad.h"

#include <functional>
#include <vector>

// A map change from a server packet (join, revival, teleport), loaded a few steps per frame between frames.
// While it runs, the packets after the one that started it wait in the queue, in order.
namespace World::MapLoad
{
// Starts a map's load steps (CMapManager::LoadWorldSteps); `afterLoad` runs once they are done. A load still running
// finishes first.
void Begin(std::vector<Core::StagedLoad::Step> steps, std::function<void()> afterLoad);

bool Active();

// Between frames: the next steps, and once the map is ready, its after-load code. `holdBack` ends the frame's steps
// early, as at startup.
void Pump(const std::function<bool()>& holdBack = {});

// True once a load has run long enough that a loading screen beats holding the last frame.
bool LoadingScreenDue();

// The session is torn down or the client quits: the load, what its steps hold and its after-load code are dropped
// now, before what they reference is released.
void Cancel();
} // namespace World::MapLoad
