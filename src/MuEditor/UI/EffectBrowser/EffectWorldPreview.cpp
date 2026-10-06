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
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <optional>
#include <utility>

namespace MuEditor::Effects
{
namespace
{
constexpr float DegreesToRadians = 3.14159265f / 180.0f;
// Lightning runs to the chest of its target.
constexpr float CharacterChest = 80.0f;
// How far from where the call put it an object may start before its code
// counts as putting it elsewhere (many add a random offset), and how near
// its owner it must be a frame later to count as following it.
constexpr float OwnPlaceDistance = 50.0f;
constexpr float FollowingDistance = 30.0f;
// The notes about what the last call created.
constexpr std::uint16_t KeptNotes = WorldNoteOwnPlace | WorldNoteFollowsOwner | WorldNoteOwnSize | WorldNoteOwnLight;
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

// CreateJoint copies the colour of these without checking that there is one;
// the game always passes one there.
constexpr std::array<JointNeedingColour, 3> NeedColour = {{
    {BITMAP_JOINT_THUNDER, 27},
    {BITMAP_JOINT_THUNDER, 28},
    {MODEL_SPEARSKILL, 14},
}};

bool NeedsJointColour(int type, int subType)
{
    return std::any_of(NeedColour.begin(), NeedColour.end(),
                       [&](const JointNeedingColour& entry) { return entry.type == type && entry.subType == subType; });
}

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

std::span<const JointNeedingColour> GetJointsNeedingColour()
{
    return NeedColour;
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
    ChooseTarget(request.call);
    m_running = request;
    m_createPending = true;
    m_repeatAt = 0.0;
    ApplyMute();
}

// The monster or NPC nearest to the character becomes the target; the copy
// takes it over, so what aims at the copy turns to it.
void EffectWorldPreview::ChooseTarget(const WorldPreviewCall& call)
{
    if (call.target != WorldPreviewTarget::NearestCharacter || Hero == nullptr)
        return;
    const PreviewVector position = {Hero->Object.Position[0], Hero->Object.Position[1], Hero->Object.Position[2]};
    OBJECT* nearest = FindNearestCharacter({CharactersClient, MAX_CHARACTERS_CLIENT}, position, NearestCharacterRange);
    m_targetFollowed = nearest;
    if (nearest != nullptr)
    {
        m_targetCopy = *nearest;
        m_notes &= static_cast<std::uint16_t>(~WorldNoteNoCharacterNear);
    }
    else
    {
        m_notes |= WorldNoteNoCharacterNear;
    }
}

void EffectWorldPreview::UpdateRunning(const WorldPreviewRequest& request)
{
    if (!m_running || m_running->kind != request.kind || m_running->type != request.type)
        return;
    // A SubType the preview refuses ends the run: Repeat would create it.
    if (IsRefusedInWorld(request))
    {
        Stop();
        m_notes = WorldNoteRefused;
        return;
    }
    if (request.call.target == WorldPreviewTarget::NearestCharacter &&
        (m_running->call.target != WorldPreviewTarget::NearestCharacter || m_targetFollowed == nullptr))
        ChooseTarget(request.call);
    m_running->subType = request.subType;
    m_running->call = request.call;
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
    if (++m_framesSinceCreate == 1 && m_lastCallFilled)
    {
        if (!createdLive)
            m_notes |= WorldNoteEndedAtOnce;
        NoteFollowing(pools);
    }
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
        // Without a chosen colour the call passes none, as most of the game's
        // calls do, so the type's code chooses it.
        const float* colour = call.jointColour || NeedsJointColour(request.type, request.subType) ? light : nullptr;
        CreateJoint(request.type, position, targetPosition, angle, request.subType, target,
                    call.scale > 0.0f ? call.scale : 10.0f, static_cast<short>(call.pk),
                    static_cast<WORD>(call.skillIndex), 0, -1, colour);
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
    m_lastPosition = {position[0], position[1], position[2]};
    if (request.kind != Data::Effects::EffectKind::Sprite)
        NoteWhatWasKept(pools, m_lastPosition, request);
    m_repeatAt = 0.0;
    if (filled == 0)
    {
        m_notes |= WorldNoteNothingCreated;
        // What earlier calls created still runs until it ends or is stopped.
        if (m_tracker.IsEmpty())
            End();
    }
}

namespace
{
// What the code of a created object kept of the call.
struct Kept
{
    PreviewVector position;
    float scale;
    PreviewVector light;
};

template <typename Object> Kept KeptBy(const Object& o)
{
    return {{o.Position[0], o.Position[1], o.Position[2]}, o.Scale, {o.Light[0], o.Light[1], o.Light[2]}};
}

std::optional<Kept> KeptAt(const EffectPools& pools, const EffectPoolSlot& slot)
{
    const auto index = static_cast<size_t>(slot.index);
    switch (slot.pool)
    {
    case EffectPool::Effect:
        return KeptBy(pools.effects[index]);
    case EffectPool::SkillEffect:
        return KeptBy(pools.skillEffects[index]);
    case EffectPool::Particle:
        return KeptBy(pools.particles[index]);
    case EffectPool::Joint:
        return KeptBy(pools.joints[index]);
    case EffectPool::Sprite:
        break;
    }
    return std::nullopt;
}

float DistanceBetween(const PreviewVector& a, const PreviewVector& b)
{
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    const float dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool SameLight(const PreviewVector& a, const PreviewVector& b)
{
    return std::abs(a[0] - b[0]) < 0.01f && std::abs(a[1] - b[1]) < 0.01f && std::abs(a[2] - b[2]) < 0.01f;
}
} // namespace

// Right after the call: what its code changed of the place, the size and the
// light the preview gave it. Only the values the user chose count: a size of
// 0, white light and lightning without a colour are the defaults.
void EffectWorldPreview::NoteWhatWasKept(const EffectPools& pools, const PreviewVector& position,
                                         const WorldPreviewRequest& request)
{
    const WorldPreviewCall& call = request.call;
    m_notes &= static_cast<std::uint16_t>(~KeptNotes);
    const auto first = std::find_if(m_tracker.GetCreated().begin(), m_tracker.GetCreated().end(),
                                    [&](const EffectPoolSlot& slot) { return KeptAt(pools, slot).has_value(); });
    if (first == m_tracker.GetCreated().end())
        return;
    const Kept kept = *KeptAt(pools, *first);
    if (DistanceBetween(kept.position, position) > OwnPlaceDistance)
        m_notes |= WorldNoteOwnPlace;
    if (call.scale > 0.0f && std::abs(kept.scale - call.scale) > 0.01f * call.scale)
        m_notes |= WorldNoteOwnSize;
    const PreviewVector white = {1.0f, 1.0f, 1.0f};
    const bool lightGiven = request.kind != Data::Effects::EffectKind::Joint || call.jointColour;
    if (lightGiven && !SameLight(call.light, white) && !SameLight(kept.light, call.light))
        m_notes |= WorldNoteOwnLight;
}

// A frame later: an object put away from its owner that is at its owner now
// follows it.
void EffectWorldPreview::NoteFollowing(const EffectPools& pools)
{
    const OBJECT* owner = TargetOf(m_running->call);
    if (owner == nullptr)
        return;
    const PreviewVector ownerPosition = {owner->Position[0], owner->Position[1], owner->Position[2]};
    if (DistanceBetween(m_lastPosition, ownerPosition) <= OwnPlaceDistance)
        return;
    for (const EffectPoolSlot& slot : m_tracker.GetCreated())
    {
        const std::optional<Kept> kept = KeptAt(pools, slot);
        if (kept && DistanceBetween(kept->position, ownerPosition) <= FollowingDistance)
        {
            m_notes |= WorldNoteFollowsOwner;
            m_notes &= static_cast<std::uint16_t>(~WorldNoteOwnPlace);
            return;
        }
    }
}

void EffectWorldPreview::ApplyMute() const
{
    Audio::EditorMute::SetMuted(m_running.has_value() && m_mute);
}
} // namespace MuEditor::Effects

#endif // _EDITOR
