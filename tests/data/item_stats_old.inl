// One-time copy of the item stat code before phase 3c-1 (see test_item_stats_equivalence.cpp):
// the stat functions of ZzzInfomation.cpp, GameLogic/Items/ItemLevelBonus and the stat columns
// of the item help table (ZzzInventory.cpp, ComputeItemInfo), renamed Old... so the copy keeps
// working when the originals go.

// GameLogic/Items/ItemLevelBonus.h and .cpp.
constexpr int OldLevelBonusPerLevelUpToNine = 3;
constexpr int OldFirstLevelBonusStepAboveNine = 4;
constexpr int OldThirdTierWingFirstLevelBonusStepAboveNine = 5;
constexpr int OldLastLevelWithLinearBonus = 9;
constexpr int OldMaxLevelsAboveNine = 6;

int OldGetItemLevelBonus(int level, int perLevelUpToNine = OldLevelBonusPerLevelUpToNine,
                         int firstStepAboveNine = OldFirstLevelBonusStepAboveNine)
{
    int bonus = perLevelUpToNine * std::clamp(level, 0, OldLastLevelWithLinearBonus);

    // Each level above +9 adds a step one larger than the step of the level before.
    const int levelsAboveNine = std::clamp(level - OldLastLevelWithLinearBonus, 0, OldMaxLevelsAboveNine);
    for (int step = 0; step < levelsAboveNine; ++step)
    {
        bonus += firstStepAboveNine + step;
    }
    return bonus;
}

// Engine/Object/ZzzInfomation.cpp.
int OldGetDropLevel(ITEM_ATTRIBUTE* p)
{
    return p->Level + 30;
}

int OldGetExcellentAddValue(ITEM *ip)
{
    if (ip->Type == ITEM_CHAOS_DRAGON_AXE)
    {
        return 15;
    }

    if (ip->Type == ITEM_CHAOS_NATURE_BOW)
    {
        return 30;
    }

    if (ip->Type == ITEM_CHAOS_LIGHTNING_STAFF)
    {
        return 25;
    }

    return 0;
}

// MagicPower and SuccessfulBlocking are BYTEs: a value past 255 shows as 255, not wrapped.
constexpr int OldMaxByteStat = 255;
// Second tier wings and the Capes of Lord and Fighter grow their defense by 2 per level up to
// +9, third tier wings by 4 (other items by 3).
constexpr int OldSecondTierWingDefensePerLevel = 2;
constexpr int OldThirdTierWingDefensePerLevel = 4;

// Excellent and ancient items get the excellent bonus of damage, magic power,
// blocking and defense. Wings keep their wing options in ExcellentFlags, but they
// are not excellent. Excellent archangel weapons get it, as in OpenMU; the client
// before 815b8828 left them out with the wings.
bool OldHasExcellentBonus(const ITEM* ip)
{
    return (ip->ExcellentFlags > 0 && !GameLogic::Items::IsWingItem(ip)) || ip->AncientDiscriminator > 0;
}

void OldCalcDamageMin(ITEM* ip, ITEM_ATTRIBUTE* p, int excelAddValue)
{
    //ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    if (p->DamageMin <= 0)
    {
        return;
    }

    if (OldHasExcellentBonus(ip))
    {
        if (p->Level)
        {
            if (excelAddValue)
                ip->DamageMin += excelAddValue;
            else
                ip->DamageMin += p->DamageMin * 25 / p->Level + 5;
        }
    }

    if (ip->AncientDiscriminator > 0)
    {
        ip->DamageMin += 5 + (OldGetDropLevel(p) / 40);
    }

    ip->DamageMin += OldGetItemLevelBonus(ip->Level);
}

void OldCalcDamageMax(ITEM* ip, ITEM_ATTRIBUTE* p, int excelAddValue)
{
    if (p->DamageMax <= 0)
    {
        return;
    }

    if (OldHasExcellentBonus(ip))
    {
        if (p->Level)
        {
            if (excelAddValue)
                ip->DamageMax += excelAddValue;
            else
                ip->DamageMax += p->DamageMin * 25 / p->Level + 5;
        }
    }

    if (ip->AncientDiscriminator > 0)
    {
        ip->DamageMax += 5 + (OldGetDropLevel(p) / 40);
    }

    ip->DamageMax += OldGetItemLevelBonus(ip->Level);
}

void OldCalcMagicPower(ITEM* ip, ITEM_ATTRIBUTE* p, int excelAddValue)
{
    if (p->MagicPower <= 0)
    {
        return;
    }

    // MagicPower is a BYTE: the sum can pass 255 before it is halved (an excellent +15
    // Divine Staff of Archangel: 156 + 42 + 66), so it is added up in an int.
    int magicPower = ip->MagicPower;
    if (OldHasExcellentBonus(ip))
    {
        if (p->Level)
        {
            if (excelAddValue)
                magicPower += excelAddValue;
            else
                magicPower += p->MagicPower * 25 / p->Level + 5;
        }
    }

    if (ip->AncientDiscriminator > 0)
    {
        magicPower += 2 + (OldGetDropLevel(p) / 60);
    }

    magicPower += OldGetItemLevelBonus(ip->Level);
    magicPower /= 2;

    if (!GameLogic::Items::IsSceptreType(ip->Type))
    {
        magicPower += ip->Level * 2;
    }
    ip->MagicPower = static_cast<BYTE>(std::min(magicPower, OldMaxByteStat));
}

void OldCalcSuccessfulBlocking(ITEM* ip, ITEM_ATTRIBUTE* p)
{
    if (p->SuccessfulBlocking == 0)
    {
        return;
    }

    int blocking = ip->SuccessfulBlocking;
    if (OldHasExcellentBonus(ip))
    {
        if (p->Level)
            blocking += p->SuccessfulBlocking * 25 / p->Level + 5;
    }
    blocking += OldGetItemLevelBonus(ip->Level);
    ip->SuccessfulBlocking = static_cast<BYTE>(std::min(blocking, OldMaxByteStat));
}

void OldCalcDefense(ITEM* ip, ITEM_ATTRIBUTE* p)
{
    if (p->MagicDefense > 0)
    {
        ip->MagicDefense += OldGetItemLevelBonus(ip->Level);
    }

    if (p->Defense == 0)
    {
        return;
    }

    auto isAncientItem = ip->AncientDiscriminator > 0;
    auto isExcellent = OldHasExcellentBonus(ip);
    auto setItemDropLevel = OldGetDropLevel(p);

    if (ip->Type >= ITEM_SHIELD && ip->Type < ITEM_SHIELD + MAX_ITEM_INDEX)
    {
        ip->Defense += ip->Level;
        if (isAncientItem)
        {
            ip->Defense = ip->Defense + (ip->Defense * 20 / setItemDropLevel + 2);
        }

        return;
    }

    if (isExcellent && p->Level > 0)
    {
        ip->Defense += p->Defense * 12 / p->Level + 4 + p->Level / 5;
    }

    if (isAncientItem)
    {
        ip->Defense += ip->Defense * 3 / setItemDropLevel + 2 + setItemDropLevel / 30;
    }

    int perLevelUpToNine = OldLevelBonusPerLevelUpToNine;
    int firstStepAboveNine = OldFirstLevelBonusStepAboveNine;
    if (GameLogic::Items::IsSecondTierWing(ip))
    {
        perLevelUpToNine = OldSecondTierWingDefensePerLevel;
    }
    else if (GameLogic::Items::IsThirdTierWing(ip))
    {
        perLevelUpToNine = OldThirdTierWingDefensePerLevel;
        firstStepAboveNine = OldThirdTierWingFirstLevelBonusStepAboveNine;
    }
    ip->Defense += OldGetItemLevelBonus(ip->Level, perLevelUpToNine, firstStepAboveNine);
}

// Engine/Object/ZzzInventory.cpp, ComputeItemInfo: the stat columns of the row of one level.
struct OldHelpTableStats
{
    int DamageMin;
    int DamageMax;
    int Magic;
    int Defense;
    int Blocking;
};

OldHelpTableStats OldComputeItemInfoStats(int iHelpItem, int Level)
{
    ITEM_ATTRIBUTE* p = &ItemAttribute[iHelpItem];
    int DamageMin = p->DamageMin;
    int DamageMax = p->DamageMax;
    int Defense = p->Defense;
    int Magic = p->MagicPower;
    int Blocking = p->SuccessfulBlocking;

    if (DamageMin > 0)
    {
        DamageMin += OldGetItemLevelBonus(Level);
    }
    if (DamageMax > 0)
    {
        DamageMax += OldGetItemLevelBonus(Level);
    }

    if (Magic > 0)
    {
        Magic += OldGetItemLevelBonus(Level);
        Magic /= 2;

        if (!GameLogic::Items::IsSceptreType(iHelpItem))
        {
            Magic += Level * 2;
        }
    }

    if (Defense > 0)
    {
        if (iHelpItem >= ITEM_SHIELD && iHelpItem < ITEM_SHIELD + MAX_ITEM_INDEX)
        {
            Defense += Level;
        }
        else
        {
            Defense += OldGetItemLevelBonus(Level);
        }
    }
    if (Blocking > 0)
    {
        // Like the item's defense rate, a BYTE that stops at 255.
        Blocking = std::min(Blocking + OldGetItemLevelBonus(Level), 255);
    }

    return {DamageMin, DamageMax, Magic, Defense, Blocking};
}
