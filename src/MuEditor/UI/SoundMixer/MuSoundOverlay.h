#pragma once

#ifdef _EDITOR

#include "Audio/SoundOverlay.h"
#include "MuSoundMixerUI.h"

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

// Whether the point (x, y) is within the source's inner ring (a monster:
// heard).
bool IsInside(const Audio::Overlay::DrawnSource& source, float x, float y);

// Takes the frame's drawn sources and, while the overlay is on, draws them
// and the starts of the last second behind the editor's windows. `hovered`:
// the table row under the mouse, or nullptr.
void Render(const std::vector<SoundMixer::HeardRow>& shown, bool filtered, const SoundMixer::HeardRow* hovered);
} // namespace MuEditor::SoundOverlay

#endif // _EDITOR
