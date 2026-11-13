#pragma once

class CHARACTER;

// Resolves the current attack target tile from the selection (or the ground
// cursor) and records it on the character. `skill` and `distance` are the skill
// being cast and its range, used to keep a direction skill's aim within range.
// Extracted from ZzzInterface.cpp.
namespace GameLogic::Combat
{
bool CheckTarget(CHARACTER* c, int skill, float distance);

// CheckTarget's ground branch: aims `c` at the ground point `ground` (x, y, z in world units), pulled into range for a
// direction skill, and sets TargetX/TargetY to its tile.
void AimAtGround(CHARACTER* c, int skill, float distance, const float ground[3]);
}
