#include "stdafx.h"

#ifdef _EDITOR

#include "EffectWorldPreview.h"

#include "Audio/EditorSoundMute.h"
#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCommon.h"
#include "Scenes/SceneCore.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace MuEditor::Effects
{
namespace
{
constexpr float DegreesToRadians = 3.14159265f / 180.0f;
// Lightning runs to the chest of its target.
constexpr float CharacterChest = 80.0f;
// Repeat waits this long after what was created ended (WorldTime, ms).
constexpr double RepeatPauseMs = 300.0;

// The catapult stones of every SubType knock the character back when they
// land near it and tell the server (SubTypes 0 and 1 land as 88 and 99).
// SubType 0 of the class change stops the character when it ends. SubType 1
// of the summoner's Lagul takes its owner for a lightning object.
constexpr std::array<RefusedWorldPreview, 4> Refused = {{
    {MODEL_FLY_BIG_STONE1, -1},
    {MODEL_FLY_BIG_STONE2, -1},
    {MODEL_CHANGE_UP_EFF, 0},
    {MODEL_SUMMONER_SUMMON_LAGUL, 1},
}};

// Removes an effect as the game does when its life ends, with its trails.
void RemoveGameEffect(OBJECT& effect)
{
    RemoveObjectBlurs(&effect, 0);
    EffectDestructor(&effect);
}
} // namespace

WorldPreviewCall DefaultWorldPreviewCall(Data::Effects::EffectKind kind)
{
    WorldPreviewCall call;
    if (kind != Data::Effects::EffectKind::Effect)
        call.height = WorldPreviewChestHeight;
    return call;
}

PreviewVector PlaceInFrontOf(const PreviewVector& position, float yawDegrees, float distance)
{
    const float yaw = yawDegrees * DegreesToRadians;
    return {position[0] + std::sin(yaw) * distance, position[1] - std::cos(yaw) * distance, position[2]};
}

std::span<const RefusedWorldPreview> GetRefusedWorldPreviews()
{
    return Refused;
}

bool IsRefusedInWorld(const WorldPreviewRequest& request)
{
    return request.kind == Data::Effects::EffectKind::Effect &&
           std::any_of(
               Refused.begin(), Refused.end(), [&](const RefusedWorldPreview& refused)
               { return refused.type == request.type && (refused.subType < 0 || refused.subType == request.subType); });
}

bool IsWorldReadyForPreview()
{
    return SceneFlag == MAIN_SCENE && LoadingWorld == 0 && EnableMainRender && Hero != nullptr && Hero->Object.Live;
}

OBJECT* FindNearestCharacter(std::span<CHARACTER> characters, const PreviewVector& position, float range)
{
    OBJECT* nearest = nullptr;
    float nearestDistance = range * range;
    for (CHARACTER& character : characters)
    {
        OBJECT& o = character.Object;
        if (!o.Live || (o.Kind != KIND_MONSTER && o.Kind != KIND_NPC) || o.CurrentAction == MONSTER01_DIE)
            continue;
        const float dx = o.Position[0] - position[0];
        const float dy = o.Position[1] - position[1];
        const float distance = dx * dx + dy * dy;
        if (distance <= nearestDistance)
        {
            nearest = &o;
            nearestDistance = distance;
        }
    }
    return nearest;
}

void EffectWorldPreview::Start(const WorldPreviewRequest& request)
{
    if (m_running && (m_running->kind != request.kind || m_running->type != request.type))
        Stop();
    if (!m_running && Hero != nullptr)
        m_owner = Hero->Object;
    m_notesFor = EffectTypeRef{request.kind, request.type};
    m_notes = 0;
    if (IsRefusedInWorld(request))
    {
        m_notes = WorldNoteRefused;
        return;
    }
    // A monster or NPC is chosen once a run; a run with another one ends
    // first, so no object follows a copy that changed.
    if (request.call.target == WorldPreviewTarget::NearestCharacter && Hero != nullptr)
    {
        const PreviewVector position = {Hero->Object.Position[0], Hero->Object.Position[1], Hero->Object.Position[2]};
        OBJECT* nearest =
            FindNearestCharacter({CharactersClient, MAX_CHARACTERS_CLIENT}, position, NearestCharacterRange);
        if (nearest != m_targetFollowed && m_running)
            Stop();
        m_targetFollowed = nearest;
        if (nearest != nullptr)
            m_targetCopy = *nearest;
        else
            m_notes |= WorldNoteNoCharacterNear;
    }
    m_running = request;
    m_createPending = true;
    m_repeatAt = 0.0;
    ApplyMute();
}

void EffectWorldPreview::Stop()
{
    if (!m_tracker.IsEmpty())
        m_tracker.RemoveAll(GetGamePools(), &RemoveGameEffect);
    End();
}

void EffectWorldPreview::End()
{
    m_running.reset();
    m_createPending = false;
    m_repeatAt = 0.0;
    ApplyMute();
}

void EffectWorldPreview::SetMute(bool mute)
{
    m_mute = mute;
    ApplyMute();
}

void EffectWorldPreview::KeepOnly(const std::optional<EffectTypeRef>& shown)
{
    if (m_running && (!shown || shown->kind != m_running->kind || shown->type != m_running->type))
        Stop();
    if (m_notesFor != shown)
    {
        m_notesFor.reset();
        m_notes = 0;
    }
}

void EffectWorldPreview::AfterFrame(bool browserOpen, bool worldReady)
{
    if (!m_running && m_tracker.IsEmpty())
        return;
    if (!browserOpen || !worldReady)
    {
        Stop();
        return;
    }
    // The copies follow the character and the monster or NPC while it lives;
    // what the type's code wrote into them stays.
    VectorCopy(Hero->Object.Position, m_owner.Position);
    m_owner.Live = true;
    if (m_targetFollowed != nullptr && m_targetFollowed->Live)
        VectorCopy(m_targetFollowed->Position, m_targetCopy.Position);
    m_targetCopy.Live = true;
    const EffectPools pools = GetGamePools();
    m_tracker.Update(pools);
    if (std::exchange(m_createPending, false))
    {
        Create(pools);
        return;
    }
    if (!m_running)
        return;
    // A sprite lives until the next RenderSprites, so it is made every frame.
    if (m_running->kind == Data::Effects::EffectKind::Sprite)
    {
        Create(pools);
        return;
    }
    const bool createdLive = m_tracker.AnyCreatedLive(pools);
    if (++m_framesSinceCreate == 1 && m_lastCallFilled && !createdLive)
        m_notes |= WorldNoteEndedAtOnce;
    if (createdLive)
        return;
    if (!m_repeat)
    {
        // What the call created ended; its followers may still run.
        if (m_tracker.IsEmpty())
            End();
        return;
    }
    if (m_repeatAt == 0.0)
        m_repeatAt = WorldTime + RepeatPauseMs;
    else if (WorldTime >= m_repeatAt)
        Create(pools);
}

void EffectWorldPreview::OnWorldClearing()
{
    Stop();
}

// What the call is aimed at: a copy, or none.
OBJECT* EffectWorldPreview::TargetOf(const WorldPreviewCall& call)
{
    switch (call.target)
    {
    case WorldPreviewTarget::None:
        return nullptr;
    case WorldPreviewTarget::NearestCharacter:
        if (m_targetFollowed != nullptr)
            return &m_targetCopy;
        break;
    case WorldPreviewTarget::Character:
        break;
    }
    return &m_owner;
}

// The game's create call of the kind, with copies of the vectors: creation
// code writes into them.
void EffectWorldPreview::Create(const EffectPools& pools)
{
    const WorldPreviewRequest request = *m_running;
    const WorldPreviewCall& call = request.call;
    const OBJECT& hero = Hero->Object;
    const PreviewVector front =
        PlaceInFrontOf({hero.Position[0], hero.Position[1], hero.Position[2]}, hero.Angle[2], call.distance);
    vec3_t position = {front[0], front[1], RequestTerrainHeight(front[0], front[1]) + call.height};
    vec3_t angle = {0.0f, 0.0f, hero.Angle[2]};
    if (call.randomAngle)
        Vector(static_cast<float>(rand() % 360), 0.0f, static_cast<float>(rand() % 360), angle);
    vec3_t light = {call.light[0], call.light[1], call.light[2]};
    OBJECT* target = TargetOf(call);
    m_tracker.BeginCreate(pools);
    switch (request.kind)
    {
    case Data::Effects::EffectKind::Effect:
        CreateEffect(request.type, position, angle, light, request.subType, target, -1, 0, 0, 0, call.scale);
        break;
    case Data::Effects::EffectKind::Particle:
        CreateParticle(request.type, position, angle, light, request.subType, call.scale > 0.0f ? call.scale : 1.0f,
                       target);
        break;
    case Data::Effects::EffectKind::Joint:
    {
        // Without a target, lightning runs to where the character stands.
        const OBJECT& aimedAt = target != nullptr ? *target : hero;
        vec3_t targetPosition = {aimedAt.Position[0], aimedAt.Position[1], aimedAt.Position[2] + CharacterChest};
        CreateJoint(request.type, position, targetPosition, angle, request.subType, target,
                    call.scale > 0.0f ? call.scale : 10.0f, static_cast<short>(call.pk),
                    static_cast<WORD>(call.skillIndex), 0, -1, light);
        break;
    }
    case Data::Effects::EffectKind::Sprite:
        CreateSprite(request.type, position, call.scale > 0.0f ? call.scale : 1.0f, light, target, 0.0f,
                     request.subType);
        break;
    }
    const int filled = m_tracker.EndCreate(pools);
    m_framesSinceCreate = 0;
    m_lastCallFilled = filled > 0;
    m_repeatAt = 0.0;
    if (filled == 0)
    {
        m_notes |= WorldNoteNothingCreated;
        // What earlier calls created still runs until it ends or is stopped.
        if (m_tracker.IsEmpty())
            End();
    }
}

void EffectWorldPreview::ApplyMute() const
{
    Audio::EditorMute::SetMuted(m_running.has_value() && m_mute);
}
} // namespace MuEditor::Effects

#endif // _EDITOR
