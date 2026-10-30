#pragma once

#include <functional>

class OBJECT;

namespace Data::Monsters
{
struct MonsterModelDefinition;
}

// The sound events of the monster models (monsters design, MN7): the sounds
// that PlayMonsterSound plays on top of the sound slots, from the model
// entries of Data/Monsters/Models.json.
namespace GameLogic::Monsters::SoundEvents
{
// Plays the sound events of the object's model that its animation, frame and
// the map call for, when the object is within 500 units of the hero. False,
// with nothing done, when the model's entry has no sound events.
bool Play(OBJECT& object);

// The model's sound events for a character in `animation` at `frame`, as Play
// plays them, but on any map and at any distance (Monster Preview, Q-MN74):
// each event's chance drawn and its pick picked, in order, and the number of
// each sound given to `play`. An event that needs the death effect plays
// while `deathEffectArmed`.
void PlayEvents(const Data::Monsters::MonsterModelDefinition& model, int animation, float frame, bool deathEffectArmed,
                const std::function<void(int)>& play);
} // namespace GameLogic::Monsters::SoundEvents
