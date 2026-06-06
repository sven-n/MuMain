#pragma once

class CHARACTER;

// Resolves the current attack target tile from the selection (or the ground
// cursor) and records it on the character. Extracted from ZzzInterface.cpp.
namespace GameLogic::Combat
{
    bool CheckTarget(CHARACTER* c);
}
