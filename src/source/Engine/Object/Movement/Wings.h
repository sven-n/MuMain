#pragma once

// A player's wings on its back: they hang from Bone05, on the lower spine (Bip01 Spine), so the head's turn reaches
// them only by that bone's small share; they turn with the back too and trail behind a turn (UpdateTurnLag).
namespace Engine::Characters
{
// Puts a player's wings on its back, in `parentMatrix` (their link bone's transform, as RenderLinkObject placed them):
// turned with the back's share of the head's turn and the turn lag, about the upright through Spine1, and on a Uniria
// or a Dinorant sitting on the spine as they do standing.
void PlaceWingsOnBack(const CHARACTER* c, int linkBone, float parentMatrix[3][4]);
} // namespace Engine::Characters
