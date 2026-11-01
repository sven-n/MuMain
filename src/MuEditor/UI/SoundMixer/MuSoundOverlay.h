#pragma once

#ifdef _EDITOR

#include "Audio/SoundMixer.h"
#include "Audio/SoundOverlay.h"
#include "MuSoundMixerUI.h"
#include "imgui.h"

#include <array>
#include <string>
#include <vector>

// Draws the Sound Mixer's overlay over the game
// (docs/superpowers/specs/2026-11-01-sound-mixer-design.md, Q-SM9, Q-SM10).
namespace MuEditor::SoundOverlay
{
// Whether a table row names the source: the same kind and number and, for a
// monster, the same set (an attack's row names no set).
bool Names(const SoundMixer::HeardRow& row, Audio::SoundKind kind, const std::string& name, int sourceNumber);

// The drawn sources to show: all, or while a filter is set those a shown row
// names.
std::vector<Audio::Overlay::DrawnSource> Followed(const std::vector<Audio::Overlay::DrawnSource>& drawn,
                                                  const std::vector<SoundMixer::HeardRow>& shown, bool filtered);

// A pixel of the game window in ImGui's coordinates: the window's viewport is
// at `origin`, `size` big, for a window of `windowWidth` x `windowHeight`
// pixels. With the editor's viewports ImGui counts from the desktop's corner,
// so the window's place matters.
ImVec2 ToDisplay(float windowX, float windowY, const ImVec2& origin, const ImVec2& size, unsigned int windowWidth,
                 unsigned int windowHeight);

// Whether the point (x, y) is within the source's inner ring (a monster:
// heard).
bool IsInside(const Audio::Overlay::DrawnSource& source, float x, float y);

// Whether a shown row follows a start: a row of its sound for a sound of the
// game's own code (Q-SM11), else a row that names its source.
bool Follows(const Audio::Mixer::PlacedStart& start, const std::vector<SoundMixer::HeardRow>& shown);

// A place where the game's own code plays sounds, with no range (Q-SM11).
struct CodeMarker
{
    Audio::SoundKind kind = Audio::SoundKind::MapObject;
    int sourceNumber = -1;
    std::array<float, 3> position{};
    // The sound numbers, in the order of the starts given.
    std::vector<int> sounds;
};

// One marker for each place: the starts of the same kind, number and
// position share it.
std::vector<CodeMarker> GroupCodeSounds(const std::vector<Audio::Mixer::PlacedStart>& starts);

// Whether the hovered row lights up the marker: a row of one of its sounds.
bool Highlights(const SoundMixer::HeardRow& hovered, const CodeMarker& marker);

// Takes the frame's drawn sources and, while the overlay is on, draws them,
// the places of the game's own code's sounds and the starts of the last
// second behind the editor's windows. `hovered`: the table row under the
// mouse, or nullptr.
void Render(const std::vector<SoundMixer::HeardRow>& shown, bool filtered, const SoundMixer::HeardRow* hovered);
} // namespace MuEditor::SoundOverlay

#endif // _EDITOR
