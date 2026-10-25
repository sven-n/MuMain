#include "stdafx.h"

#include "MonsterSlotSound.h"

#include "Render/Models/ZzzBMD.h"

namespace Engine::Object::MonsterSlotSound
{
std::optional<ESound> Find(const BMD& model, int slot, int offset)
{
    const short sound = model.Sounds[slot];
    if (sound == NoSound)
    {
        return std::nullopt;
    }
    return static_cast<ESound>(SOUND_MONSTER + offset + sound);
}
} // namespace Engine::Object::MonsterSlotSound
