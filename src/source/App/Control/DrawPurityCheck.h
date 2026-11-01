#pragma once

#include <string>
#include <vector>

// Finds the game state the draw still writes: while listing, the fields of the characters, effects, items, map
// objects, mounts, boids, particles and joints (visibility and drawn-frame stamps included) and the frame's view are
// compared before and after each draw pass, the scene's and the offscreen one, and so is whether the pass drew from the
// random sequences the update draws from (control socket "drawpurity").
namespace Diagnostics::DrawPurityCheck
{
#ifdef MU_ENABLE_CONTROL_SOCKET
// Around each draw pass; both do nothing unless listing.
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
