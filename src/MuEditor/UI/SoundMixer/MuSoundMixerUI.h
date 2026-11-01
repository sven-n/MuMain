#pragma once

#ifdef _EDITOR

#include "Audio/SoundMixer.h"

#include <string>

namespace MuEditor
{
struct ToolWindows;
}

// The Sound Mixer window (Windows menu, docs/superpowers/specs/2026-11-01-sound-mixer-design.md):
// mute, solo and a volume for each kind of sound and each sound heard in the
// last ten seconds, for the session; the editor's previews are not touched.
class MuSoundMixerUI
{
public:
    void Render(bool* open);

private:
    void RenderKinds();
    void RenderHeard();
};

extern MuSoundMixerUI g_MuSoundMixerUI;

namespace MuEditor::SoundMixer
{
// "2 muted, 1 solo, 1 lowered"; "" for nothing changed.
std::string ChangesText(const Audio::Mixer::Changes& changes);

// The kind's name in the window.
const char* KindName(Audio::SoundKind kind);

// Where a heard sound came from: its kind, and the set and monster or the map
// object type.
std::string SourceText(const Audio::Mixer::HeardSound& sound);

// The file a heard sound plays below Data/Sound, or its number.
std::string SoundText(int number);

// The menu bar's reminder (Q-SM5): while anything is changed, "Sound Mixer:
// …", which opens the window.
void RenderReminder(ToolWindows& windows);

// The mixer's controls as the window's buttons change them: they apply at once.
void ToggleMute(Audio::Mixer::Control& control);
void ToggleSolo(Audio::Mixer::Control& control);
void SetVolume(Audio::Mixer::Control& control, float volume);
} // namespace MuEditor::SoundMixer

#endif // _EDITOR
