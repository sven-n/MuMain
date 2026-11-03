// ModelLight.h: the directional light models are lit with, whatever format they come from.
#pragma once

#include <glm/glm.hpp>

namespace Render::Lighting
{

// World-space vector towards the models' light; its length is the directional part's strength
// (Classic lights a face max(0.8 × dot(n, L) + 0.4, 0.2) with it), so the normals it meets must be
// world-oriented. `highlight`: the brighter fixed light of previews (BMD::HighLight). Otherwise the
// map's baked direction, scaled by the light mood's sun and, with Enhanced dynamic lights, following
// the frame's sun (FollowFrameSun).
[[nodiscard]] glm::vec3 ModelLightVector(bool highlight);

} // namespace Render::Lighting
