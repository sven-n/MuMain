#include "Render/Lighting/SunShadow.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>

namespace Render::Lighting
{
namespace
{
// The box's size and depth move in these steps, so a walking camera keeps the same box (and the
// same texels) until it has gone far enough.
constexpr float kSizeStep = 256.0f;
constexpr float kDepthStep = 512.0f;

// World to light space without translation: light-space x and y stay fixed in the world, so
// snapping them to texels keeps the shadows still while the camera moves.
glm::mat4 LightRotation(const glm::vec3& toSun)
{
    const glm::vec3 up = std::abs(toSun.z) > 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
    return glm::lookAtRH(glm::vec3(0.0f), -toSun, up);
}
} // namespace

glm::vec3 SunDirection(float fallAngleDegrees, float fallPerHeight)
{
    const float angle = glm::radians(fallAngleDegrees);
    // Shadows fall away from the sun: it stands on the other side, fallPerHeight across per unit up.
    return glm::normalize(glm::vec3(-std::cos(angle) * fallPerHeight, -std::sin(angle) * fallPerHeight, 1.0f));
}

SunShadowView BuildSunShadowView(const glm::vec3& toSun, const glm::mat4& sceneViewProjection, float groundLow,
                                 float groundHigh, std::uint32_t resolution)
{
    const glm::dmat4 sceneFromClip = glm::inverse(glm::dmat4(sceneViewProjection));
    const auto unproject = [&](double x, double y, double z)
    {
        const glm::dvec4 point = sceneFromClip * glm::dvec4(x, y, z, 1.0);
        return glm::dvec3(point) / point.w;
    };

    // Where the screen's corner rays cross the lowest and highest ground.
    const glm::mat4 rotation = LightRotation(toSun);
    glm::vec3 low(INFINITY);
    glm::vec3 high(-INFINITY);
    for (const double x : {-1.0, 1.0})
    {
        for (const double y : {-1.0, 1.0})
        {
            const glm::dvec3 nearPoint = unproject(x, y, 0.0);
            const glm::dvec3 farPoint = unproject(x, y, 1.0);
            const double rise = farPoint.z - nearPoint.z;
            for (const float height : {groundLow, groundHigh})
            {
                const double t = std::abs(rise) > 1e-6 ? std::clamp((height - nearPoint.z) / rise, 0.0, 1.0) : 1.0;
                const glm::vec3 lightPoint =
                    glm::vec3(rotation * glm::vec4(glm::vec3(nearPoint + (farPoint - nearPoint) * t), 1.0f));
                low = glm::min(low, lightPoint);
                high = glm::max(high, lightPoint);
            }
        }
    }

    SunShadowView view;
    SunShadowBox& box = view.box;
    const float extent = (std::max)(high.x - low.x, high.y - low.y);
    box.size = (std::floor(extent / kSizeStep) + 2.0f) * kSizeStep;
    const float texel = box.size / static_cast<float>((std::max)(resolution, 1u));
    box.origin = glm::floor(glm::vec2(low) / texel) * texel;
    // The light looks down its -z: the nearest point to the sun has the largest z.
    box.nearDistance = std::floor((-high.z - kSunShadowCasterHeight) / kDepthStep) * kDepthStep;
    box.farDistance = std::ceil((-low.z + kDepthStep) / kDepthStep) * kDepthStep;

    const glm::mat4 projection = glm::orthoRH_ZO(box.origin.x, box.origin.x + box.size, box.origin.y,
                                                 box.origin.y + box.size, box.nearDistance, box.farDistance);
    view.viewProjection = 2.0f * projection * rotation;
    view.sunFromScene = glm::mat4(glm::dmat4(view.viewProjection) * sceneFromClip);
    return view;
}

} // namespace Render::Lighting
