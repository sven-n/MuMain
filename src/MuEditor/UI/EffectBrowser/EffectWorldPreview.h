#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"
#include "EffectPreviewCamera.h"
#include "EffectPreviewTracker.h"

#include <cstdint>
#include <optional>
#include <span>

namespace MuEditor::Effects
{
// What the world preview creates: a type of a kind with a SubType (for a
// sprite, its blend).
struct WorldPreviewRequest
{
    Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
    int type = 0;
    int subType = 0;

    bool operator==(const WorldPreviewRequest&) const = default;
};

// What the world preview says about its last create call, as flags.
enum WorldPreviewNote : std::uint16_t
{
    // The call filled no slot.
    WorldNoteNothingCreated = 1 << 0,
    // What the call created ended in its first frame.
    WorldNoteEndedAtOnce = 1 << 1,
    // The type changes the game when the character owns it.
    WorldNoteRefused = 1 << 2,
};

// A type and SubType the world preview does not create (-1: every SubType).
struct RefusedWorldPreview
{
    int type = 0;
    int subType = -1;
};

// How far in front of the character the preview creates a type: two tiles.
inline constexpr float WorldPreviewDistance = 200.0f;

// The point `distance` in front of `position` for a yaw in degrees: the
// game's forward direction (0, -1, 0) turned by the yaw, as AngleMatrix does.
PreviewVector PlaceInFrontOf(const PreviewVector& position, float yawDegrees, float distance);

// The types that change the game when the character owns them.
std::span<const RefusedWorldPreview> GetRefusedWorldPreviews();
bool IsRefusedInWorld(const WorldPreviewRequest& request);

// Whether a character stands in a map, so a type can be created in front of
// it.
bool IsWorldReadyForPreview();

// The selected type created in the game world in front of the character,
// with the game's own create call and the character as its owner (as the
// target of particles and lightning). It follows what it created and removes
// it when it stops: on Stop, when another type is shown, when the browser
// closes and before the game clears its pools.
class EffectWorldPreview
{
public:
    // Creates the type at the end of this frame, again on every call.
    void Start(const WorldPreviewRequest& request);
    // Removes what the preview created.
    void Stop();
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

private:
    void Create(const EffectPools& pools);
    void End();
    void ApplyMute() const;

    EffectPreviewTracker m_tracker;
    std::optional<WorldPreviewRequest> m_running;
    std::optional<EffectTypeRef> m_notesFor;
    bool m_createPending = false;
    bool m_repeat = false;
    bool m_mute = false;
    // Frames since the last create call, to tell what ended at once.
    int m_framesSinceCreate = 0;
    // When Repeat creates the type again (WorldTime), 0 while waiting for
    // what was created to end.
    double m_repeatAt = 0.0;
    std::uint16_t m_notes = 0;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
