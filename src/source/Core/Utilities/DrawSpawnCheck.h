#pragma once

#include <string>
#include <vector>

// Finds what the draw still spawns or animates: while the frame draws, every spawn function notes who called it, so a
// sweep of the maps lists the spawns the update has not taken over (control socket "drawspawns").
namespace Diagnostics::DrawSpawnCheck
{
#ifdef MU_ENABLE_CONTROL_SOCKET
// Marks the frame's draw for as long as it lives.
class DrawScope
{
public:
    DrawScope();
    ~DrawScope();
    DrawScope(const DrawScope&) = delete;
    DrawScope& operator=(const DrawScope&) = delete;
};

// Called first by each spawn function; records its callers when checking and inside a draw.
void Note(const char* kind);

// Clears the counts and starts checking.
void Start();

struct Site
{
    std::string kind;
    // The spawn function's callers, nearest first.
    std::string callers;
    int count = 0;
};
// The sites seen since Start, most frequent first, and the draws they were counted over.
std::vector<Site> Report(int& draws);
#else
class DrawScope
{
};
inline void Note(const char*) {}
#endif
} // namespace Diagnostics::DrawSpawnCheck
