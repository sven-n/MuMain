#include "stdafx.h"

#include "Audio/SoundEventPlay.h"

#include "Audio/SoundPlaces.h"

#include "Character/CharacterManager.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Engine/Object/ZzzCharacter.h"

namespace Audio
{
std::optional<ESound> PickEventSound(std::string_view event, const Data::Sounds::SoundEventContext& context)
{
    const Data::Sounds::SoundEvent* found = g_SoundEventDatabase.Find(event);
    if (found == nullptr)
    {
        return std::nullopt;
    }
    const Data::Sounds::SoundEventRule* rule = Data::Sounds::MatchRule(*found, context);
    if (rule == nullptr || rule->numbers.empty())
    {
        return std::nullopt;
    }
    // One sound draws nothing, as the code played it.
    const size_t pick = rule->numbers.size() > 1 ? static_cast<size_t>(Random::Rand()) % rule->numbers.size() : 0;
    return static_cast<ESound>(rule->numbers[pick]);
}

void PlayInterfaceEventSound(std::string_view event, const Data::Sounds::SoundEventContext& context)
{
    if (const std::optional<ESound> sound = PickEventSound(event, context))
    {
        const ScopedSoundPlace ui(SoundKind::Interface, -1, nullptr);
        PlayBuffer(*sound);
    }
}

Data::Sounds::HeldWeapon HeldWeaponOf(const CHARACTER& c)
{
    using Data::Sounds::HeldWeapon;
    CHARACTER& character = const_cast<CHARACTER&>(c);
    const int bowType = gCharacterManager.GetEquipedBowType(&character);
    if (bowType == BOWTYPE_BOW)
    {
        return HeldWeapon::Bow;
    }
    if (bowType == BOWTYPE_CROSSBOW)
    {
        return HeldWeapon::Crossbow;
    }
    const int right = c.Weapon[0].Type;
    if (right >= MODEL_BLUEWING_CROSSBOW && right < MODEL_ARROWS)
    {
        return HeldWeapon::MagicCrossbow;
    }
    if (right == MODEL_LIGHT_SABER || right == MODEL_SPEAR)
    {
        return HeldWeapon::SaberOrSpear;
    }
    if (right != -1 || c.Weapon[1].Type != -1)
    {
        return HeldWeapon::Other;
    }
    return HeldWeapon::None;
}
} // namespace Audio
