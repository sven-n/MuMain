#pragma once

#ifdef _EDITOR

// The monster tools' previews of sounds (the Play buttons of Monster Model,
// the animation sounds of Monster Preview) at a volume of their own, so they
// are heard while the game's effect volume is down. The volume lasts for the
// session; it is not a setting.
namespace MuEditor::Monsters::SoundPreview
{
// 0 to 1.
float GetVolume();
void SetVolume(float volume);

// Plays the sound with this number once at the preview volume.
void Play(int number);

// The preview volume's slider, with a note on what it is.
void RenderVolume();
} // namespace MuEditor::Monsters::SoundPreview

#endif // _EDITOR
