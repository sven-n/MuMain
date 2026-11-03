#include "stdafx.h"

#include "MonsterSlotSound.h"

#include "Render/Models/ZzzBMD.h"

namespace Engine::Object::MonsterSlotSound
{
int SlotFor(int slot, int subType)
{
    return subType == SecondSetSubType ? SecondSetSlot + slot : slot;
}

std::optional<ESound> Find(const BMD& model, int slot)
{
    const short sound = model.Sounds[slot];
    if (sound == NoSound)
    {
        return std::nullopt;
    }
    return static_cast<ESound>(SOUND_MONSTER + sound);
}
} // namespace Engine::Object::MonsterSlotSound
