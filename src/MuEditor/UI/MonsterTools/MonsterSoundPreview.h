#pragma once

#ifdef _EDITOR

namespace Data::Monsters
{
struct MonsterModelDefinition;
}

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

// Plays the model's sound events for `animation` at `frame` at the preview
// volume, as the game plays them, on any map and at any distance, with the
// death effect counted as armed: every sound the animation can play (Q-MN74).
void PlayEvents(const Data::Monsters::MonsterModelDefinition& model, int animation, float frame);

// The preview volume's slider, with a note on what it is.
void RenderVolume();
} // namespace MuEditor::Monsters::SoundPreview

#endif // _EDITOR
