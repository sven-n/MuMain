#include "stdafx.h"
#include "App/Control/ControlTaps.h"

#include "App/Control/ControlEvents.h"
#include "GameLogic/Automation/Attack.h"
#include "App/Control/ControlObjects.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Server/WSclient.h"
#include "World/MapInfra/MapManager.h"

#include <cstring>
#include <string>
#include <utility>

namespace
{
// Damage type the server marks a critical hit with, as the client's own
// damage presentation reads it.
constexpr int CriticalDamageType = 2;
constexpr int ExcellentDamageType = 3;

int KeyOfIndex(int characterIndex)
{
    if (characterIndex < 0 || characterIndex >= MAX_CHARACTERS_CLIENT)
    {
        return -1;
    }
    return CharactersClient[characterIndex].Key;
}

} // namespace

namespace App::Control::Events
{
void RecordAttackDamage(int attackerIndex, int targetKey, int damage, int shieldDamage, int damageType)
{
    if (!IsEnabled())
    {
        return;
    }

    const int attackerKey = KeyOfIndex(attackerIndex);
    const bool heroWasHit = targetKey == HeroKey;

    // The damage packet names only the target, so the attacker comes from
    // the action packet that preceded it (`AttackPlayer`), and otherwise
    // from the target this character is acting on — but only while its own
    // swing is playing. `ActionTarget` alone is sticky: it outlives the
    // swing that set it, and a third party hitting the same monster would
    // then be reported as this character's blow.
    const bool swinging = GameLogic::Automation::IsSwingInProgress();
    const bool heroAttacked =
        !heroWasHit &&
        (attackerKey == HeroKey || (swinging && ActionTarget >= 0 && ActionTarget == FindCharacterIndex(targetKey)));

    if (!heroAttacked && !heroWasHit)
    {
        // A fight between two other objects is noise for a scenario about
        // this client's character.
        return;
    }

    const bool critical = damageType == CriticalDamageType || damageType == ExcellentDamageType;
    // `AttackPlayer` is whoever acted last, which is not the attacker when
    // the character itself is swinging; name the hero in that case.
    const int reportedAttacker = heroAttacked ? HeroKey : attackerKey;
    // The packet's success bit marks the presentation the client plays, not
    // whether the blow landed; zero damage is what a miss looks like.
    const bool missed = damage <= 0 && shieldDamage <= 0;
    RecordHit(heroAttacked ? Direction::Dealt : Direction::Received, reportedAttacker, targetKey, damage, shieldDamage,
              critical, missed);

    if (heroWasHit)
    {
        RecordHeroStats();
    }
}

void RecordDeathOf(int victimKey, int killerKey)
{
    RecordDeath(victimKey, killerKey);
}

void RecordExperienceGain(std::uint64_t experience, int damage)
{
    if (!IsEnabled())
    {
        return;
    }

    // `experience_gained` rather than `experience`: `state` reports the
    // character's cumulative experience under that name, and two different
    // numbers must not arrive under one name.
    RecordStat("experience_gained", static_cast<long long>(experience), -1);
    if (damage > 0)
    {
        RecordStat("damage_dealt", damage, -1);
    }
}

void RecordLevelUp(int level)
{
    RecordStat("level", level, -1);
}

void RecordHeroStats()
{
    if (!IsEnabled() || CharacterAttribute == nullptr)
    {
        return;
    }

    RecordStat("life", CharacterAttribute->Life, CharacterAttribute->LifeMax);
    RecordStat("mana", CharacterAttribute->Mana, CharacterAttribute->ManaMax);
    RecordStat("sd", CharacterAttribute->Shield, CharacterAttribute->ShieldMax);
    RecordStat("ag", CharacterAttribute->SkillMana, CharacterAttribute->SkillManaMax);
}

void RecordChatLine(const wchar_t* sender, const wchar_t* text, const char* kind)
{
    RecordChat(sender, text, kind != nullptr ? kind : "public");
}

void RecordDropAppeared(int itemSlot)
{
    if (!IsEnabled() || itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return;
    }

    // The slot is the id `pickup` takes; the drop's own key is not what the
    // pickup packet carries.
    const std::wstring name = App::Control::DropName(itemSlot);
    const std::pair<int, int> tile = App::Control::DropTile(itemSlot);
    RecordDrop(itemSlot, name.c_str(), tile.first, tile.second);
}

void RecordDropVanished(int itemSlot, const char* reason)
{
    if (!IsEnabled() || itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return;
    }

    RecordDropGone(itemSlot, reason != nullptr ? reason : "gone");
}

void RecordViewCleared(const char* reason)
{
    if (!IsEnabled())
    {
        return;
    }

    const char* why = reason != nullptr ? reason : "the view was cleared";

    for (int slot = 0; slot < MAX_ITEMS; ++slot)
    {
        if (Items[slot].Object.Live && Items[slot].Item.Type >= 0)
        {
            RecordDropGone(slot, why);
        }
    }

    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (character.Object.Live && character.Key != HeroKey)
        {
            RecordViewLeave(character.Key);
        }
    }
}

void RecordMapChange()
{
    if (!IsEnabled())
    {
        return;
    }

    const int map = gMapManager.WorldActive;
    RecordMap(map, gMapManager.GetMapName(map), Hero != nullptr ? Hero->PositionX : -1,
              Hero != nullptr ? Hero->PositionY : -1);
}

void RecordTeleportPacket()
{
    NoteTeleport();
}

void RecordViewEnterKey(int key)
{
    RecordViewEnter(key);
}

void RecordViewLeaveKey(int key)
{
    if (!IsEnabled())
    {
        return;
    }

    // Only what the client still knows: the delete packet arrives for keys
    // that were never in view, and for keys a teleport has already reported
    // as gone.
    if (FindCharacterIndex(key) == MAX_CHARACTERS_CLIENT)
    {
        return;
    }

    RecordViewLeave(key);
}

void RecordPartyChange(const char* change, const wchar_t* name)
{
    RecordParty(change != nullptr ? change : "changed", name != nullptr ? name : L"");
}

void RecordDisconnected(const char* reason)
{
    RecordDisconnect(reason != nullptr ? reason : "the server closed the connection");
}
} // namespace App::Control::Events
