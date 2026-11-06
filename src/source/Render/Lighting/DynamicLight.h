// DynamicLight.h: one dynamic terrain light of the current frame, as a point light.
#pragma once

namespace Render::Lighting
{

// Height of a dynamic light above the terrain under it, in world units: about the chest of a
// standing character. The game's light calls carry no height.
inline constexpr float kDynamicLightHeightAboveTerrain = 100.0f;

// A light the game added to the terrain light grid (AddTerrainLight into PrimaryTerrainLight).
// The terrain light of a grid vertex at distance d (in cells) grows by
// color * (range - d) / range; a point light of radius range * cell size gives models the same
// falloff. Colours may be negative: such lights darken.
struct DynamicLight
{
    // World position: the light call's x and y, z is the terrain height plus
    // kDynamicLightHeightAboveTerrain.
    float position[3]{};
    float color[3]{};
    // Range of the light call, in terrain cells.
    int range = 0;
    // range in world units.
    float radius = 0.0f;
};

} // namespace Render::Lighting
