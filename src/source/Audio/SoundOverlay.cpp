#include "stdafx.h"

#ifdef _EDITOR

#include "Audio/SoundOverlay.h"

#include "Camera/CameraState.h"

#include <array>
#include <optional>
#include <utility>

namespace Audio::Overlay
{
namespace
{
// A frame never draws more; the list stays bounded while nobody takes it.
constexpr size_t MaxDrawn = 2048;

struct State
{
    bool on = false;
    std::array<bool, SoundKindCount> shown = []
    {
        std::array<bool, SoundKindCount> all{};
        all.fill(true);
        return all;
    }();
    std::vector<DrawnSource> drawn;
    std::optional<CameraState> camera;
};

State& Overlay()
{
    static State state;
    return state;
}
} // namespace

bool IsOn()
{
    return Overlay().on;
}

void SetOn(bool on)
{
    Overlay().on = on;
    if (!on)
    {
        Overlay().drawn.clear();
        Overlay().camera.reset();
    }
}

bool IsShown(SoundKind kind)
{
    return Overlay().shown[static_cast<size_t>(kind)];
}

void SetShown(SoundKind kind, bool shown)
{
    Overlay().shown[static_cast<size_t>(kind)] = shown;
}

void NoteDrawn(const DrawnSource& source)
{
    State& state = Overlay();
    if (state.on && state.drawn.size() < MaxDrawn)
    {
        state.drawn.push_back(source);
    }
}

std::vector<DrawnSource> TakeDrawn()
{
    return std::exchange(Overlay().drawn, {});
}

void KeepCamera(const CameraState& camera)
{
    if (Overlay().on)
    {
        Overlay().camera = camera;
    }
}

const CameraState* KeptCamera()
{
    const State& state = Overlay();
    return state.camera ? &*state.camera : nullptr;
}
} // namespace Audio::Overlay

#endif // _EDITOR
