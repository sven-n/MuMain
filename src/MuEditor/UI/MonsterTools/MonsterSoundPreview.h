#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <span>

// The monster tools' previews of sounds (the Play buttons of Monster Sounds,
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

// Plays the set's sound events for `animation` at `frame` at the preview
// volume, as the game plays them, on any map and at any distance, with the
// death effect counted as armed: every sound the animation and the cues of
// the shown model can play (Q-MN74, Q-MN79).
void PlayEvents(const Data::Monsters::MonsterSoundSet& set, std::span<const Data::Monsters::MonsterModelCue> cues,
                int animation, float frame);

// The set's sound events at `moment` at the preview volume, as the game
// plays them, on any map and at any distance (Q-MN76).
void PlayMoment(const Data::Monsters::MonsterSoundSet& set, Data::Monsters::MonsterSoundMoment moment);

// Plays the sound a set's event names once at the preview volume; a sound of
// the set that is not loaded is loaded first, as a model loads it.
void PlaySoundOfSet(const Data::Monsters::MonsterSoundSet& set, const std::string& name);

// The preview volume's slider, with a note on what it is.
void RenderVolume();
} // namespace MuEditor::Monsters::SoundPreview

#endif // _EDITOR
