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

// The drawn sources to show: those of a shown type (Q-SM14), all of them or,
// while a filter is set, those a shown row names.
std::vector<Audio::Overlay::DrawnSource> Followed(const std::vector<Audio::Overlay::DrawnSource>& drawn,
                                                  const std::vector<SoundMixer::HeardRow>& shown, bool filtered);

// A pixel of the game window in ImGui's coordinates: the window's viewport is
// at `origin`, `size` big, for a window of `windowWidth` x `windowHeight`
// pixels. With the editor's viewports ImGui counts from the desktop's corner,
// so the window's place matters.
ImVec2 ToDisplay(float windowX, float windowY, const ImVec2& origin, const ImVec2& size, unsigned int windowWidth,
                 unsigned int windowHeight);

// Whether the point (x, y) is within the source's inner ring (a monster:
// heard); always for a source without one (a set heard anywhere).
bool IsInside(const Audio::Overlay::DrawnSource& source, float x, float y);

// Whether a shown row follows a start: a row of its sound for a sound of the
// game's own code (Q-SM11), else a row that names its source.
bool Follows(const Audio::Mixer::PlacedStart& start, const std::vector<SoundMixer::HeardRow>& shown);

// Whether the overlay draws a start's marker or flash: one of a shown type
// (Q-SM14) and, while a filter is set, one a shown row follows.
bool Draws(const Audio::Mixer::PlacedStart& start, const std::vector<SoundMixer::HeardRow>& shown, bool filtered);

// The line of a label beside its marker.
enum class LabelLine
{
    // Centred on the marker: a drawn source's set or file.
    Beside,
    // Right below that: the sounds the game's own code plays there, so that a
    // monster's or an NPC's set and its code's sounds both stay readable.
    Below,
};

// The top of a label `textHeight` high on its line beside a marker at
// `markerY`; the labels' backgrounds, 2 pixels beyond the text, do not
// overlap.
float LabelTop(float markerY, float textHeight, LabelLine line);

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

// The overlay's log of starts without a place (Q-SM15): so many lines.
constexpr int LogLines = 10;

// The log's lines: the newest `LogLines` of `starts` (newest first) of a
// shown type and, while a filter is set, of a sound a shown row names (a
// music track, with no row, then none).
std::vector<Audio::Mixer::PlacelessStart> Logged(const std::vector<Audio::Mixer::PlacelessStart>& starts,
                                                 const std::vector<SoundMixer::HeardRow>& shown, bool filtered);

// The music's file below Data/Music.
std::string MusicText(const std::string& music);

// A log line: how long ago it started, the file and its type
// ("0.2 s  iButtonClick.wav (Interface)").
std::string LogText(const Audio::Mixer::PlacelessStart& start, double now);

// Takes the frame's drawn sources and, while the overlay is on, draws the log
// of starts without a place at the game's top left and, where the world is
// drawn, the sources, the places of the game's own code's sounds and the
// starts of the last second, of the types shown, behind the editor's
// windows. `hovered`: the table row under the mouse, or nullptr.
void Render(const std::vector<SoundMixer::HeardRow>& shown, bool filtered, const SoundMixer::HeardRow* hovered);
} // namespace MuEditor::SoundOverlay

#endif // _EDITOR
