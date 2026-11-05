#include "stdafx.h"

#include "MonsterModelBody.h"

#include "Core/Math/ZzzMathLib.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Engine/Object/w_CharacterInfo.h"

#include <array>

namespace Engine::Object::MonsterBody
{
namespace
{
constexpr std::array<float, 3> DefaultBoxMin = {-60.f, -60.f, 0.f};
constexpr std::array<float, 3> DefaultBoxMax = {50.f, 50.f, 150.f};
constexpr int DefaultRightHandBone = 33;
constexpr int DefaultLeftHandBone = 42;

void SetPoint(const std::array<float, 3>& point, vec3_t target)
{
    Vector(point[0], point[1], point[2], target);
}
} // namespace

void ApplyBoundingBox(OBJECT& object, const Data::Monsters::MonsterModelBody& body)
{
    SetPoint(body.boxMin.value_or(DefaultBoxMin), object.BoundingBoxMin);
    SetPoint(body.boxMax.value_or(DefaultBoxMax), object.BoundingBoxMax);
}

void ApplyLinkBones(CHARACTER& character, const Data::Monsters::MonsterModelBody& body)
{
    if (!body.linkBones)
    {
        character.Weapon[0].LinkBone = DefaultRightHandBone;
        character.Weapon[1].LinkBone = DefaultLeftHandBone;
        return;
    }
    for (size_t hand = 0; hand < body.linkBones->size() && hand < 2; ++hand)
    {
        character.Weapon[hand].LinkBone = static_cast<BYTE>((*body.linkBones)[hand]);
    }
}
} // namespace Engine::Object::MonsterBody
