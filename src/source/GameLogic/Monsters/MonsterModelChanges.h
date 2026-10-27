#pragma once

#include <span>

class BMD;
class OBJECT;

// What the game does when the monster editor opened changed models again
// (MonsterDataHandler::ReopenChangedModels, monsters design D55).
namespace GameLogic::Monsters::ModelChanges
{
// Keeps the object inside the model's animations: an animation the model
// no longer has goes back to the first, a frame past the end of its
// animation back to the start. A model without animations leaves the object
// at the first animation's start.
void ClampAnimation(OBJECT& object, const BMD& model);

// ClampAnimation for each living character of these model slots. Their
// bones are their own and stay.
void ClampAnimations(std::span<const int> slots);
} // namespace GameLogic::Monsters::ModelChanges
