#include "stdafx.h"

#include "Render/Lighting/ModelLight.h"

#include "Core/Math/ZzzMathLib.h"
#include "Render/Lighting/DynamicLightsSetting.h"
#include "Render/Lighting/LightMoodSetting.h"
#include "Render/Renderer/MuRenderer.h"
#include "Scenes/SceneCore.h"
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
    // Models shown in the UI keep their light; in the world it follows the frame's sun. (The game draws
    // every model with the highlight light.)
    if (!FrameSun() || SceneFlag != MAIN_SCENE || mu::GetRenderer().InScreenSpaceSampling())
    {
        return vector;
    }
    return FollowFrameSun(vector);
}

} // namespace Render::Lighting
