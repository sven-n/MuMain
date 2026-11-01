#pragma once

#include <string>
#include <vector>

// Keeps the draw from changing the game: while the frame draws, every spawn function notes its call, and a spawn no
// AllowScope covers is a violation. Developer builds log each violating caller once; tests read the count; the control
// socket's "drawspawns" lists every draw spawn with its callers.
namespace Diagnostics::DrawSpawnCheck
{
// Marks the frame's draw for as long as it lives.
class DrawScope
{
public:
    DrawScope();
    ~DrawScope();
    DrawScope(const DrawScope&) = delete;
    DrawScope& operator=(const DrawScope&) = delete;
};

// Lets the draw spawn for as long as it lives, for what no update sees yet: a part's first frame, a preview.
class AllowScope
{
public:
    AllowScope();
    ~AllowScope();
    AllowScope(const AllowScope&) = delete;
    AllowScope& operator=(const AllowScope&) = delete;
};

// Called first by each spawn function.
void Note(const char* kind);

// The spawns inside a draw that no AllowScope covered, since the last reset.
int Violations();
void ResetViolations();

#ifdef MU_ENABLE_CONTROL_SOCKET
// Clears the counts and starts listing.
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
#endif
} // namespace Diagnostics::DrawSpawnCheck
