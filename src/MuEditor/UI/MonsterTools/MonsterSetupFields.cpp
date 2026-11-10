#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSetupFields.h"

#include "I18N/All.h"

#include <algorithm>
#include <array>

namespace MuEditor::Monsters::SetupFields
{
namespace
{
using enum Section;
using enum Takes;

// The fields a living monster takes at once are those of its look and of
// its way of moving (D55, Q-MN39); the rest is set when it appears. A
// facing step or tiles take effect at its next position packet, which counts
// as its next spawn here.
constexpr std::array Fields{
    Field{"scale", &I18N::Editor::Scale, Look, AtOnce},
    Field{"outline", &I18N::Editor::Outline, Look, AtOnce},
    Field{"subType", &I18N::Editor::Skin, Look, AtOnce},
    Field{"lookLevel", &I18N::Editor::RenderTier, Look, AtOnce},
    Field{"glowMesh", &I18N::Editor::GlowMesh, Look, AtOnce},
    Field{"glowLight", &I18N::Editor::GlowLight, Look, AtOnce},
    Field{"hiddenMesh", &I18N::Editor::HiddenMesh, Look, AtOnce},
    Field{"shadow", &I18N::Editor::Shadow, Look, AtOnce},
    Field{"hideShadow", &I18N::Editor::HideShadow, Look, AtOnce},
    Field{"alphaTarget", &I18N::Editor::Alpha, Look, AtOnce},
    Field{"light", &I18N::Editor::LitByTheLight, Look, AtOnce},
    Field{"pkColor", &I18N::Editor::PKColour, Look, AtOnce},
    Field{"helperPlate", &I18N::Editor::HelperPlateLights, Look, AtOnce},
    Field{"class", &I18N::Editor::PlayerClass, Look, NextSpawn},
    Field{"classStep", &I18N::Editor::ClassStep, Look, NextSpawn},
    Field{"equipment", &I18N::Editor::Equipment, Look, AtOnce},
    Field{"startAction", &I18N::Editor::StartAction, Spawn, NextSpawn},
    Field{"startFrame", &I18N::Editor::StartFrame, Spawn, NextSpawn},
    Field{"positionOffset", &I18N::Editor::PositionOffset, Spawn, NextSpawn},
    Field{"tileOffset", &I18N::Editor::TileOffset, Spawn, NextSpawn},
    Field{"heightOffset", &I18N::Editor::HeightAboveGround, Spawn, NextSpawn},
    Field{"height", &I18N::Editor::FixedHeight, Spawn, NextSpawn},
    Field{"keepGroundHeight", &I18N::Editor::KeepGroundHeight, Spawn, NextSpawn},
    Field{"boundingBox", &I18N::Editor::BoundingBox, Spawn, NextSpawn},
    Field{"facing", &I18N::Editor::FixedFacing, Spawn, AtOnce},
    Field{"facingStep", &I18N::Editor::FacingStep, Spawn, NextSpawn},
    Field{"facingByTile", &I18N::Editor::FacingByTile, Spawn, NextSpawn},
    Field{"randomLook", &I18N::Editor::RandomSkin, Spawn, NextSpawn},
    Field{"randomStart", &I18N::Editor::RandomStart, Spawn, NextSpawn},
    Field{"spawnBuffs", &I18N::Editor::SpawnBuffs, Spawn, NextSpawn},
    Field{"spawnSound", &I18N::Editor::SpawnSound, Spawn, NextSpawn},
    Field{"spawnEffects", &I18N::Editor::SpawnEffects, Spawn, NextSpawn},
    Field{"appear", &I18N::Editor::Appear, Appear, NextSpawn},
    Field{"teleport", &I18N::Editor::Teleport, Appear, NextSpawn},
    Field{"moveSpeed", &I18N::Editor::MoveSpeed, Behaviour, AtOnce},
    Field{"noTurnOnHit", &I18N::Editor::NoTurnOnAMagicHit, Behaviour, AtOnce},
    Field{"noBloodPool", &I18N::Editor::NoBloodPool, Behaviour, AtOnce},
    Field{"selectable", &I18N::Editor::CanBePointedAt, Behaviour, AtOnce},
    Field{"live", &I18N::Editor::Shown, Behaviour, NextSpawn},
    Field{"deathEffect", &I18N::Editor::DeathEffect, Behaviour, NextSpawn},
    Field{"gate", &I18N::Editor::CastleGate, GateAndHooks, NextSpawn},
    Field{"hooks", &I18N::Editor::Hooks, GateAndHooks, NextSpawn},
    Field{"stateVariants", &I18N::Editor::StateVariants, GateAndHooks, AtOnce},
};
} // namespace

std::span<const Field> GetFields()
{
    return Fields;
}

const Field* Find(std::string_view key)
{
    const auto found = std::ranges::find(Fields, key, &Field::key);
    return found != Fields.end() ? &*found : nullptr;
}

const Data::Monsters::MonsterSetupField* FindSetupField(std::string_view key)
{
    const std::span<const Data::Monsters::MonsterSetupField> fields = Data::Monsters::GetMonsterSetupFields();
    const auto found = std::ranges::find(fields, key, &Data::Monsters::MonsterSetupField::key);
    return found != fields.end() ? &*found : nullptr;
}

std::optional<Data::Monsters::MonsterValueField> GetValueField(std::string_view key)
{
    const std::span<const Data::Monsters::MonsterSetupField> fields = Data::Monsters::GetMonsterSetupFields();
    const auto found = std::ranges::find(fields, key, &Data::Monsters::MonsterSetupField::key);
    if (found == fields.end())
        return std::nullopt;
    return Data::Monsters::GetSetupValueField(static_cast<size_t>(found - fields.begin()));
}

bool IsSet(std::string_view key, const Data::Monsters::MonsterSetup& setup)
{
    const Data::Monsters::MonsterSetupField* field = FindSetupField(key);
    return field != nullptr && field->isSet(setup);
}

void CopyField(std::string_view key, Data::Monsters::MonsterSetup& to, const Data::Monsters::MonsterSetup& from)
{
    if (const Data::Monsters::MonsterSetupField* field = FindSetupField(key))
        field->copy(to, from);
}

const char* GetLabel(std::string_view key)
{
    const Field* field = Find(key);
    return field != nullptr ? *field->label : key.data();
}

void ClearField(std::string_view key, Data::Monsters::MonsterSetup& setup)
{
    CopyField(key, setup, Data::Monsters::MonsterSetup{});
}
} // namespace MuEditor::Monsters::SetupFields

#endif // _EDITOR
