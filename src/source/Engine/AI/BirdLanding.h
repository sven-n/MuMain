#pragma once

namespace Engine::AI::BirdLanding
{
// Whether the hero stands still in this action. With the player model that is any pose SetPlayerStop gives,
// sitting, healing or posing. A hero in a monster form (a Transformation Ring) uses that monster's actions,
// and only its stop actions count.
// A landed bird stays while the hero is still and flies off when it walks, attacks or uses a skill.
bool IsHeroStill(int heroAction, bool playerModel);

// Whether a bird may sit on a terrain cell with these TerrainWall flags. The terrain height ignores what
// stands on it, so a cell under a house, tree or fence, off the ground, in water or with a character on it
// is refused.
bool CanLandOn(unsigned int terrainWallFlags);
} // namespace Engine::AI::BirdLanding
