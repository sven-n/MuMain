#pragma once

#include "Audio/EditorSoundMute.h"

#include <cstdint>
#include <optional>

// The slots of the game's effect lists that belong to a view of an editor preview (design L51): the Style Editor's
// views create their items' things on bones with the game's own calls into the game's lists, and the game moves them,
// but the world doesn't draw them; each view draws only its own, after its item. Each slot carries the view it belongs
// to, 0 for the game's own. The draw loops ask Draws, the move loops Moves. In game builds nothing is marked.
namespace Render::Effects::PreviewSlots
{
enum class List : std::uint8_t
{
    // The effects, then the skill effects, as RenderEffects counts them.
    Effect,
    Particle,
    Joint,
    Sprite,
};

inline constexpr int ListCount = 4;

// A view of a preview; 0 is the game.
using View = std::uint8_t;

#ifdef _EDITOR
// Forgets every mark.
void Clear();
void Mark(List list, int index, View view);
// A create call took the slot for something new: whoever creates it, it is the game's until a view marks it. A view
// whose thing ended in the slot sees that the slot is no longer its own, even when the new thing has the same type.
void Born(List list, int index);
[[nodiscard]] View ViewOf(List list, int index);
// Whether the draw loops draw the slot now: the game's slots, or only the captured view's while one is captured.
[[nodiscard]] bool Draws(List list, int index);
// Whether the game moves the slot: not while its view's time is stopped (design L54).
[[nodiscard]] bool Moves(List list, int index);
void SetStopped(View view, bool stopped);
// Whether a view is being captured: its things are drawn whatever the world's camera sees.
[[nodiscard]] bool IsCapturing();

// While it lives, the draw loops draw only the slots of `view`.
class CaptureScope
{
public:
    explicit CaptureScope(View view);
    ~CaptureScope();
    CaptureScope(const CaptureScope&) = delete;
    CaptureScope& operator=(const CaptureScope&) = delete;

private:
    View m_previous;
};

// While it lives, a view's slot plays no sound as it moves: the views are muted, as the world preview is.
class MoveSoundScope
{
public:
    MoveSoundScope(List list, int index);
    ~MoveSoundScope();
    MoveSoundScope(const MoveSoundScope&) = delete;
    MoveSoundScope& operator=(const MoveSoundScope&) = delete;

private:
    std::optional<Audio::EditorMute::Scope> m_mute;
};
#else
inline void Born(List, int) {}
inline View ViewOf(List, int)
{
    return 0;
}
inline bool Draws(List, int)
{
    return true;
}
inline bool Moves(List, int)
{
    return true;
}
inline bool IsCapturing()
{
    return false;
}
class MoveSoundScope
{
public:
    MoveSoundScope(List, int) {}
};
#endif
} // namespace Render::Effects::PreviewSlots
