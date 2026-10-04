#pragma once

#include "Core/Globals/_types.h"

#include <array>

class OBJECT;
class CHARACTER;

// The trails of the effect code: a blur follows the weapon swing of a
// character, an object blur follows an object. CreateBlur, CreateObjectBlur
// and RemoveObjectBlurs (ZzzEffect.h) change them; MoveBlurs, MoveObjectBlurs
// and the render functions run them every frame (ZzzEffectBlurSpark.cpp).
namespace Render::Effects
{
constexpr int MAX_BLURS = 100;
constexpr int MAX_BLUR_TAILS = 30;

constexpr int MAX_OBJECT_BLURS = 1000;
constexpr int MAX_OBJECT_BLUR_TAILS = 600;

struct Blur
{
    bool Live = false;
    int Type = 0;
    int LifeTime = 0;
    CHARACTER* Owner = nullptr;
    int Number = 0;
    vec3_t Light{};
    std::array<vec3_t, MAX_BLUR_TAILS> P1{};
    std::array<vec3_t, MAX_BLUR_TAILS> P2{};
    int SubType = 0;
};

struct ObjectBlur
{
    bool Live = false;
    int Type = 0;
    int LifeTime = 0;
    OBJECT* Owner = nullptr;
    int Number = 0;
    vec3_t Light{};
    int LimitLifeTime = 0;
    std::array<vec3_t, MAX_OBJECT_BLUR_TAILS> P1{};
    std::array<vec3_t, MAX_OBJECT_BLUR_TAILS> P2{};
    int SubType = 0;
};

extern std::array<Blur, MAX_BLURS> g_blurs;
extern std::array<ObjectBlur, MAX_OBJECT_BLURS> g_objectBlurs;
} // namespace Render::Effects
