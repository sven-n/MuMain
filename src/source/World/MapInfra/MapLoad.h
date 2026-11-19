#pragma once

#include <functional>

// A map change from a server packet (join, revival, teleport), loaded a few steps per frame between frames.
// While it runs, the packets after the one that started it wait in the queue, in order.
namespace World::MapLoad
{
// Starts loading `map`; `afterLoad` runs once it is ready. A load still running finishes first.
void Begin(int map, std::function<void()> afterLoad);

bool Active();

// Between frames: the next steps, and once the map is ready, its after-load code.
void Pump();

// True once a load has run long enough that a loading screen beats holding the last frame.
bool LoadingScreenDue();

// The session is torn down: the load and its after-load code are dropped.
void Cancel();
} // namespace World::MapLoad
