#include "stdafx.h"
#include "Engine/Object/Movement/Stride.h"

#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <vector>

namespace Engine::Characters
{
namespace
{
// A stepping action's planted foot moves back at least this much per key (model units); slower ones (flying, riding,
// standing) don't step and keep their own speed.
constexpr float MinStepPerKey = 10.f;
} // namespace

float StepPerKey(int action)
{
    return MeasureStride(action).stepPerKey;
}

const Stride& MeasureStride(int action)
{
    BMD& player = Models[MODEL_PLAYER];
    static std::vector<Stride> measured;
    static const Stride none{0.f, 0.f};
    if (action < 0 || action >= player.NumActions || player.BoneFoot[0] < 0 || player.BoneFoot[1] < 0)
    {
        return none;
    }
    if (measured.size() != static_cast<size_t>(player.NumActions))
    {
        measured.assign(player.NumActions, Stride{});
    }
    if (measured[action].stepPerKey >= 0.f)
    {
        return measured[action];
    }
    constexpr int MaxKeys = 64;
    const int keys = std::min<int>(player.Actions[action].NumAnimationKeys, MaxKeys);
    float step = 0.f;
    float leftForwardAt = 0.f;
    if (keys >= 2)
    {
        static float bones[MAX_BONES][3][4];
        vec3_t noAngle = {0.f, 0.f, 0.f};
        vec3_t noHead = {0.f, 0.f, 0.f};
        float forward[2][MaxKeys];
        for (int key = 0; key < keys; ++key)
        {
            player.Animation(bones, static_cast<float>(key), static_cast<float>(key),
                             static_cast<unsigned short>(action), noAngle, noHead, false, false, nullptr,
                             static_cast<short>(action));
            for (int foot = 0; foot < 2; ++foot)
            {
                // A model faces -Y.
                forward[foot][key] = -bones[player.BoneFoot[foot]][1][3];
            }
        }
        int frontKey = 0;
        for (int key = 1; key < keys; ++key)
        {
            if (forward[0][key] > forward[0][frontKey])
            {
                frontKey = key;
            }
        }
        leftForwardAt = static_cast<float>(frontKey) / static_cast<float>(keys);
        for (int foot = 0; foot < 2; ++foot)
        {
            float back[MaxKeys];
            int count = 0;
            for (int key = 0; key < keys; ++key)
            {
                const float moved = forward[foot][key] - forward[foot][(key + 1) % keys];
                if (moved > 0.f)
                {
                    back[count++] = moved;
                }
            }
            std::sort(back, back + count);
            float sum = 0.f;
            for (int i = count / 2; i < count; ++i)
            {
                sum += back[i];
            }
            if (count - count / 2 > 0)
            {
                step = std::max(step, sum / static_cast<float>(count - count / 2));
            }
        }
    }
    measured[action] = {step >= MinStepPerKey ? step : 0.f, leftForwardAt};
    return measured[action];
}
} // namespace Engine::Characters
