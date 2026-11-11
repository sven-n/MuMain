#pragma once

typedef struct tagITEM ITEM;

namespace GameLogic::Items
{
// Maps where the character must keep flight equipment on. Only Icarus: the
// server requires it there (OpenMU: CanFly), not in the Kanturu event map.
bool RequiresFlight(int world);

// Whether the item in `equipmentSlot` may be taken off in `world`. Where the
// map requires flight, the last flight equipment (see HasFlightEquipment)
// cannot be taken off. The server has to enforce this as well; the client
// check only spares the player the fall.
bool CanTakeOff(int equipmentSlot, int world, const ITEM* pItemHelper, const ITEM* pItemWing);
} // namespace GameLogic::Items
