#include "stdafx.h"

#include "StatDownDebuffLights.h"

#include "Render/Effects/ZzzEffect.h"

#include <optional>

namespace Render::Effects::StatDownDebuffLights
{
namespace
{
struct Lights
{
    int subType;
    const float* color;
};

std::optional<Lights> LightsOf(eBuffState debuff)
{
    switch (debuff)
    {
    case eDeBuff_AttackDown:
        return Lights{1, AttackDownColor};
    case eDeBuff_DefenseDown:
        return Lights{2, DefenseDownColor};
    default:
        return std::nullopt;
    }
}
} // namespace

void Create(OBJECT* o, eBuffState debuff)
{
    const auto lights = LightsOf(debuff);
    if (!lights || SearchEffect(BITMAP_SHINY + 6, o, lights->subType))
        return;

    vec3_t color;
    VectorCopy(lights->color, color);
    CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, color, lights->subType, o, -1, 0, 0, 0, 0.5f);
    CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, color, lights->subType, o, -1, 0, 0, 0, 1.f);
}

void Delete(OBJECT* o, eBuffState debuff)
{
    const auto lights = LightsOf(debuff);
    if (!lights)
        return;

    DeleteEffect(BITMAP_SHINY + 6, o, lights->subType);
    DeleteEffect(BITMAP_PIN_LIGHT, o, lights->subType);
}

void Update(OBJECT* o)
{
    for (const eBuffState debuff : {eDeBuff_AttackDown, eDeBuff_DefenseDown})
    {
        if (g_isCharacterBuff(o, debuff))
            Create(o, debuff);
        else
            Delete(o, debuff);
    }
}
} // namespace Render::Effects::StatDownDebuffLights
