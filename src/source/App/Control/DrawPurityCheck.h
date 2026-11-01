#pragma once

#include <string>
#include <vector>

// Finds the game state the draw still writes: while listing, the gameplay fields of the characters, effects, items,
// map objects, mounts, boids, particles and joints are compared before and after each frame's draw (control socket
// "drawpurity"). The draw's own bookkeeping (visibility, drawn-frame stamps, bounds) is left out.
namespace Diagnostics::DrawPurityCheck
{
#ifdef MU_ENABLE_CONTROL_SOCKET
// Around the frame's draw; both do nothing unless listing.
void BeforeDraw();
void AfterDraw();

// Clears the counts and starts listing.
void Start();

struct Change
{
    // "effect 312 LifeTime": the kind of object, its type and the field the draw changed.
    std::string what;
    // How many objects' fields changed, summed over the draws.
    int count = 0;
};
// The changes seen since Start, most frequent first, and the draws they were counted over.
std::vector<Change> Report(int& draws);
#else
inline void BeforeDraw() {}
inline void AfterDraw() {}
#endif
} // namespace Diagnostics::DrawPurityCheck
