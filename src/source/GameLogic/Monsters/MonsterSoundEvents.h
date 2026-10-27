#pragma once

class OBJECT;

// The sound events of the monster models (monsters design, MN7): the sounds
// that PlayMonsterSound plays on top of the sound slots, from the model
// entries of Data/Monsters/Models.json.
namespace GameLogic::Monsters::SoundEvents
{
// Plays the sound events of the object's model that its animation, frame and
// the map call for, when the object is within 500 units of the hero. False,
// with nothing done, when the model's entry has no sound events.
bool Play(OBJECT& object);
} // namespace GameLogic::Monsters::SoundEvents
