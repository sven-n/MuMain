#pragma once

// Stride matching: a player's walking and running steps play as fast as it really moves, so the feet don't slide
// (CharacterAnimation), and a fade between two steppings starts the new one where the old was in the stride (PoseFade).
namespace Engine::Characters
{
struct Stride
{
    // How far the planted foot moves back per key (model units), 0 for an action that doesn't step.
    float stepPerKey = -1.f;
    // Where in the action (0 to 1 of its cycle) the first foot is furthest forward: two steppings line up by it.
    float leftForwardAt = 0.f;
};

// An action's stride (the player's), measured from the skeleton once per action: the faster half of each foot's
// backward moves between keys, the larger of the two feet.
const Stride& MeasureStride(int action);

// How far an action's planted foot moves back per key (model units), 0 for an action that doesn't step.
float StepPerKey(int action);
} // namespace Engine::Characters
