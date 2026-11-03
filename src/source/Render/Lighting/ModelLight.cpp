#include "stdafx.h"

#include "Render/Lighting/ModelLight.h"

#include "Core/Math/ZzzMathLib.h"
#include "Render/Lighting/DynamicLightsSetting.h"
#include "Render/Lighting/LightMoodSetting.h"
#include "World/MapInfra/MapManager.h"

namespace Render::Lighting
{

glm::vec3 ModelLightVector(bool highlight)
{
    vec3_t position;
    vec3_t angle = {0.f, 0.f, 0.f};
    if (highlight)
    {
        Vector(1.3f, 0.f, 2.f, position);
    }
    else if (gMapManager.InBattleCastle())
    {
        Vector(0.5f, -1.f, 1.f, position);
        Vector(0.f, 0.f, -45.f, angle);
    }
    else
    {
        Vector(0.f, -1.5f, 0.f, position);
    }

    // The light mood's sun scales the directional part of the model light.
    LightMood::ScaleSunDirection(LightMood::Active(), position);
    float matrix[3][4];
    AngleMatrix(angle, matrix);
    vec3_t light;
    VectorIRotate(position, matrix, light);
    const glm::vec3 vector(light[0], light[1], light[2]);
    return highlight ? vector : FollowFrameSun(vector);
}

} // namespace Render::Lighting
