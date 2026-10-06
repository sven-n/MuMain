#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"
#include "EffectPreviewCamera.h"
#include "EffectPreviewTracker.h"
#include "Engine/Object/w_CharacterInfo.h"

#include <cstdint>
#include <optional>
#include <span>

namespace MuEditor::Effects
{
// What the world preview's call is aimed at: its owner (the target of
// particles and lightning).
enum class WorldPreviewTarget : std::uint8_t
{
    // A copy of the character, which follows it.
    Character,
    // None, as the game passes for some calls.
    None,
    // A copy of the monster or NPC nearest to the character.
    NearestCharacter,
};

// How far in front of the character the preview creates a type: two tiles;
// particles, lightning and sprites start at about the height of the chest.
inline constexpr float WorldPreviewDistance = 200.0f;
inline constexpr float WorldPreviewChestHeight = 100.0f;

// The values of the world preview's call besides the type and the SubType.
struct WorldPreviewCall
{
    // Where the call starts the type: this far in front of the character and
    // this high above the ground.
    float distance = WorldPreviewDistance;
    float height = 0.0f;
    // 0: the default of the create function (effects 0.9, particles 1,
    // lightning 10); sprites, whose create function has none, get 1.
    float scale = 0.0f;
    PreviewVector light{1.0f, 1.0f, 1.0f};
    // Lightning only: whether the call passes the light as its colour. Most
    // of the game's calls pass none, and some types then choose their colour
    // by SubType.
    bool jointColour = false;
    // A random angle for each call, as many game calls give lightning.
    bool randomAngle = false;
    WorldPreviewTarget target = WorldPreviewTarget::Character;
    // Lightning only: the values some SubTypes read from PK and SkillIndex.
    int pk = -1;
    int skillIndex = 0;

    bool operator==(const WorldPreviewCall&) const = default;
};

// The values the preview starts with for a kind.
WorldPreviewCall DefaultWorldPreviewCall(Data::Effects::EffectKind kind);

// What the world preview creates: a type of a kind with a SubType (for a
// sprite, its blend) and the values of the call.
struct WorldPreviewRequest
{
    Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
    int type = 0;
    int subType = 0;
    WorldPreviewCall call;

    bool operator==(const WorldPreviewRequest&) const = default;
};

// What the world preview says about its last create call, as flags.
enum WorldPreviewNote : std::uint16_t
{
    // The call filled no slot.
    WorldNoteNothingCreated = 1 << 0,
    // What the call created ended in its first frame.
    WorldNoteEndedAtOnce = 1 << 1,
    // The type's code changes the character or tells the server.
    WorldNoteRefused = 1 << 2,
    // No monster or NPC near the character: the copy of the character is
    // the target.
    WorldNoteNoCharacterNear = 1 << 3,
    // What the last call created does not keep a value of the call: its code
    // put it elsewhere, it moved to its owner, it chose its size or its
    // light.
    WorldNoteOwnPlace = 1 << 4,
    WorldNoteFollowsOwner = 1 << 5,
    WorldNoteOwnSize = 1 << 6,
    WorldNoteOwnLight = 1 << 7,
};

// A type and SubType the world preview does not create (-1: every SubType).
struct RefusedWorldPreview
{
    int type = 0;
    int subType = -1;
};

// A lightning SubType whose creation code reads the colour without checking
// that the call passed one: the preview passes white when none is chosen.
struct JointNeedingColour
{
    int type = 0;
    int subType = 0;
};

// The point `distance` in front of `position` for a yaw in degrees: the
// game's forward direction (0, -1, 0) turned by the yaw, as AngleMatrix does.
PreviewVector PlaceInFrontOf(const PreviewVector& position, float yawDegrees, float distance);

// The types whose code changes the character or tells the server, whoever
// owns them.
std::span<const RefusedWorldPreview> GetRefusedWorldPreviews();
bool IsRefusedInWorld(const WorldPreviewRequest& request);
std::span<const JointNeedingColour> GetJointsNeedingColour();

// Whether a character stands in a map, so a type can be created in front of
// it.
bool IsWorldReadyForPreview();

// How far around the character the preview looks for a monster or NPC.
inline constexpr float NearestCharacterRange = 1000.0f;

// The live monster or NPC of `characters` nearest to `position`, within
// `range`; nullptr without one.
OBJECT* FindNearestCharacter(std::span<CHARACTER> characters, const PreviewVector& position, float range);

// The selected type created in the game world in front of the character,
// with the game's own create call. Its owner (the target of particles and
// lightning) is a copy of the character's object that follows the character:
// code that writes into its owner changes the copy, and the branches the game
// runs only for the character's own effects (the skill effects, the catapult
// camera, ...) stay off. It follows what it created and removes it when it
// stops: on Stop, when another type is shown, when the browser closes and
// before the game clears its pools.
class EffectWorldPreview
{
public:
    // Creates the type at the end of this frame, again on every call; with
    // other values, what earlier calls created stays.
    void Start(const WorldPreviewRequest& request);
    // Removes what the preview created.
    void Stop();
    // The SubType and the call values the running preview creates with from
    // now on (Repeat, sprites); for another type nothing changes.
    void UpdateRunning(const WorldPreviewRequest& request);
    void SetRepeat(bool repeat)
    {
        m_repeat = repeat;
    }
    bool GetRepeat() const
    {
        return m_repeat;
    }
    // Mutes the game's sound effects while the preview runs.
    void SetMute(bool mute);
    bool GetMute() const
    {
        return m_mute;
    }
    // Stops when another type than the running one is shown.
    void KeepOnly(const std::optional<EffectTypeRef>& shown);
    // Once a frame, after the game's move and draw, so what it creates shows
    // from the next frame. Stops when the browser is closed or no character
    // stands in a map.
    void AfterFrame(bool browserOpen, bool worldReady);
    // The game is about to clear its pools.
    void OnWorldClearing();

    bool IsRunning() const
    {
        return m_running.has_value();
    }
    // The notes of the type shown last (KeepOnly).
    std::uint16_t GetNotes() const
    {
        return m_notes;
    }
    EffectPreviewCounts GetCounts() const
    {
        return m_tracker.Count();
    }
    // The owner of what the preview creates.
    const OBJECT& GetOwner() const
    {
        return m_owner;
    }

private:
    void Create(const EffectPools& pools);
    OBJECT* TargetOf(const WorldPreviewCall& call);
    void ChooseTarget(const WorldPreviewCall& call);
    void NoteWhatWasKept(const EffectPools& pools, const PreviewVector& position, const WorldPreviewRequest& request);
    void NoteFollowing(const EffectPools& pools);
    void End();
    void ApplyMute() const;

    EffectPreviewTracker m_tracker;
    OBJECT m_owner;
    // The copy of the monster or NPC the call is aimed at, and the one it
    // follows while that lives.
    OBJECT m_targetCopy;
    OBJECT* m_targetFollowed = nullptr;
    std::optional<WorldPreviewRequest> m_running;
    std::optional<EffectTypeRef> m_notesFor;
    bool m_createPending = false;
    bool m_repeat = false;
    bool m_mute = false;
    // Frames since the last create call that filled a slot, to tell what
    // ended at once.
    int m_framesSinceCreate = 0;
    bool m_lastCallFilled = false;
    // Where the last call put the type.
    PreviewVector m_lastPosition{};
    // When Repeat creates the type again (WorldTime), 0 while waiting for
    // what was created to end.
    double m_repeatAt = 0.0;
    std::uint16_t m_notes = 0;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
