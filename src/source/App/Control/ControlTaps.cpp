#include "stdafx.h"
#include "App/Control/ControlTaps.h"

#include "App/Control/ControlEvents.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Server/WSclient.h"
#include "World/MapInfra/MapManager.h"

#include <cstring>

namespace
{
// Damage type the server marks a critical hit with, as the client's own
// damage presentation reads it.
constexpr int CriticalDamageType = 2;
constexpr int ExcellentDamageType = 3;

// Longest item name the tables hold.
constexpr std::size_t ItemNameLength = 128;

int KeyOfIndex(int characterIndex)
{
    if (characterIndex < 0 || characterIndex >= MAX_CHARACTERS_CLIENT)
    {
        return -1;
    }
    return CharactersClient[characterIndex].Key;
}

const wchar_t* DropName(int itemSlot, wchar_t (&buffer)[ItemNameLength])
{
    if (itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return L"";
    }

    const ITEM& item = Items[itemSlot].Item;
    if (item.Type < 0)
    {
        return L"";
    }

    GetItemName(item.Type, item.Level, buffer);
    return buffer;
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

    // The damage packet names only the target: the client infers the
    // attacker from the action packet that preceded it (`AttackPlayer`) or,
    // for the character's own swing, from the target it is acting on.
    const bool heroAttacked =
        !heroWasHit && (attackerKey == HeroKey || (ActionTarget >= 0 && ActionTarget == FindCharacterIndex(targetKey)));

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

    RecordStat("experience", static_cast<long long>(experience), -1);
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

    wchar_t buffer[ItemNameLength] = {};
    const ITEM_t& drop = Items[itemSlot];
    // The slot is the id `pickup` takes; the drop's own key is not what the
    // pickup packet carries.
    RecordDrop(itemSlot, DropName(itemSlot, buffer), static_cast<int>(drop.Object.Position[0] / TERRAIN_SCALE),
               static_cast<int>(drop.Object.Position[1] / TERRAIN_SCALE));
}

void RecordDropVanished(int itemSlot, const char* reason)
{
    if (!IsEnabled() || itemSlot < 0 || itemSlot >= MAX_ITEMS)
    {
        return;
    }

    RecordDropGone(itemSlot, reason != nullptr ? reason : "gone");
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
