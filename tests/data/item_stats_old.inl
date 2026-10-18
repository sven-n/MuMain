// One-time copy of the item stat and requirement code before the refactor commits of phase 3c
// (see test_item_stats_equivalence.cpp): the stat and requirement functions of ZzzInfomation.cpp,
// the strength the option level adds, GameLogic/Items/ItemLevelBonus and the columns of the item
// help table (ZzzInventory.cpp, ComputeItemInfo), renamed Old... so the copy keeps working when
// the originals go. It has the fixes 3c made in commits of their own before them: no Cape of Lord
// defense in code, and the help table's defense rate cap and command requirement. It still has
// the dead Dark Raven branch of the command requirement, which never runs.

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

// Engine/Object/ZzzInfomation.cpp.
void OldCalcRequirements(ITEM* ip, ITEM_ATTRIBUTE* p)
{
    bool isExcellent = ip->ExcellentFlags > 0;
    bool isAncientItem = ip->AncientDiscriminator > 0;
    int ItemLevel = p->Level;

    if (isExcellent)
    {
        ItemLevel = p->Level + 25;
    }
    else if (isAncientItem)
    {
        ItemLevel = p->Level + 30;
    }

    int addValue = 4;

    if (GameLogic::Items::IsSecondTierWingExceptCape(ip))
    {
        addValue = 5;
    }

    // These keep their base level requirement; the ones in the next branch grow with the + level.
    const bool isWeaponOrArmor = ip->Type >= ITEM_SWORD && ip->Type < ITEM_WING;
    const bool isFromOrbsToThirdTierWings = ip->Type >= ITEM_ORB_OF_TWISTING_SLASH && ip->Type <= ITEM_CAPE_OF_EMPEROR;
    const bool isLateWingGroupKind =
        ip->Type >= ITEM_WING_OF_DIMENSION && ip->Type < ITEM_HELPER && ip->Type != ITEM_CAPE_OF_FIGHTER;
    const bool keepsBaseLevelRequirement =
        isWeaponOrArmor || ip->Type == ITEM_HORN_OF_FENRIR || isFromOrbsToThirdTierWings || isLateWingGroupKind;

    if (p->RequireLevel && keepsBaseLevelRequirement)
    {
        ip->RequireLevel = p->RequireLevel;
    }
    else if (p->RequireLevel && ((ip->Type >= ITEM_WING && ip->Type <= ITEM_ORB_OF_TWISTING_SLASH)
        || (ip->Type >= ITEM_WING_OF_CURSE && ip->Type <= ITEM_WINGS_OF_DESPAIR)
        || (ip->Type == ITEM_CAPE_OF_FIGHTER)
        || ip->Type >= ITEM_HELPER))
    {
        ip->RequireLevel = p->RequireLevel + ip->Level * addValue;
    }
    else
        ip->RequireLevel = 0;

    if (p->RequireStrength)
        ip->RequireStrength = 20 + (p->RequireStrength) * (ItemLevel + ip->Level * 3) * 3 / 100;
    else	ip->RequireStrength = 0;

    if (p->RequireDexterity)
        ip->RequireDexterity = 20 + (p->RequireDexterity) * (ItemLevel + ip->Level * 3) * 3 / 100;
    else	ip->RequireDexterity = 0;

    if (p->RequireVitality)
        ip->RequireVitality = 20 + (p->RequireVitality) * (ItemLevel + ip->Level * 3) * 3 / 100;
    else	ip->RequireVitality = 0;

    if (p->RequireEnergy)
    {
        if (GameLogic::Items::IsSummonerBook(ip))
        {
            ip->RequireEnergy = 20 + (p->RequireEnergy) * (ItemLevel + ip->Level * 1) * 3 / 100;
        }
        else

            if ((p->RequireLevel > 0) && (ip->Type >= ITEM_ETC && ip->Type < ITEM_ETC + MAX_ITEM_INDEX))
            {
                ip->RequireEnergy = 20 + (p->RequireEnergy) * (p->RequireLevel) * 4 / 100;
            }
            else

            {
                ip->RequireEnergy = 20 + (p->RequireEnergy) * (ItemLevel + ip->Level * 3) * 4 / 100;
            }
    }
    else
    {
        ip->RequireEnergy = 0;
    }

    if (p->RequireCharisma)
        ip->RequireCharisma = 20 + (p->RequireCharisma) * (ItemLevel + ip->Level * 3) * 3 / 100;
    else	ip->RequireCharisma = 0;

    if (ip->Type == ITEM_ORB_OF_SUMMONING)
    {
        WORD Energy = 0;

        switch (ip->Level)
        {
        case 0:Energy = 30; break;
        case 1:Energy = 60; break;
        case 2:Energy = 90; break;
        case 3:Energy = 130; break;
        case 4:Energy = 170; break;
        case 5:Energy = 210; break;
        case 6:Energy = 300; break;
        case 7:Energy = 500; break;
        }
        ip->RequireEnergy = Energy;
    }

    if (p->RequireCharisma)
    {
        if (ip->Type == MODEL_DARK_RAVEN_ITEM)
            ip->RequireCharisma = (185 + (p->RequireCharisma * 15));
        else
            ip->RequireCharisma = p->RequireCharisma;
    }

    if (ip->Type == ITEM_TRANSFORMATION_RING)
    {
        if (ip->Level <= 2)
            ip->RequireLevel = 20;
        else
            ip->RequireLevel = 50;
    }

    if ((ip->Type >= ITEM_DRAGON_KNIGHT_HELM && ip->Type <= ITEM_SUNLIGHT_MASK) ||
        (ip->Type >= ITEM_DRAGON_KNIGHT_ARMOR && ip->Type <= ITEM_SUNLIGHT_ARMOR) ||
        (ip->Type >= ITEM_DRAGON_KNIGHT_PANTS && ip->Type <= ITEM_SUNLIGHT_PANTS) ||
        (ip->Type >= ITEM_DRAGON_KNIGHT_GLOVES && ip->Type <= ITEM_SUNLIGHT_GLOVES) ||
        (ip->Type >= ITEM_DRAGON_KNIGHT_BOOTS && ip->Type <= ITEM_SUNLIGHT_BOOTS) || ip->Type == ITEM_BONE_BLADE ||
        ip->Type == ITEM_EXPLOSION_BLADE || ip->Type == ITEM_GRAND_VIPER_STAFF || ip->Type == ITEM_SYLPH_WIND_BOW ||
        ip->Type == ITEM_SOLEIL_SCEPTER || ITEM_STORM_BLITZ_STICK == ip->Type || ITEM_AURA_HELM == ip->Type ||
        ITEM_AURA_ARMOR == ip->Type || ITEM_AURA_PANTS == ip->Type || ITEM_AURA_GLOVES == ip->Type ||
        ITEM_AURA_BOOTS == ip->Type || GameLogic::Items::IsLuckyItemType(ip->Type))
    {
        isExcellent = false;
    }

    if (isExcellent && ip->RequireLevel > 0 && !GameLogic::Items::IsWingItem(ip) && ip->Type != ITEM_HORN_OF_FENRIR)
    {
        ip->RequireLevel += 20;
    }
}

// Engine/Object/ZzzInfomation.cpp, SetItemAttributes: the lines of the option level block that
// add to the strength requirement (the specials it also writes are left out).
void OldAddOptionStrength(ITEM* ip)
{
    int Option3 = ip->OptionLevel;
    if (Option3)
    {
        if (ip->Type == ITEM_HORN_OF_DINORANT)
        {
        }
        else
        {
            if (ip->Type >= ITEM_SWORD && ip->Type < ITEM_BOW + MAX_ITEM_INDEX)
            {
                if (!GameLogic::Items::IsAmmunition(ip))
                {
                    ip->RequireStrength += Option3 * 5;
                }
            }
            if (ip->Type >= ITEM_STAFF && ip->Type < ITEM_STAFF + MAX_ITEM_INDEX)
            {
                ip->RequireStrength += Option3 * 5;
            }
            if (ip->Type >= ITEM_SHIELD && ip->Type < ITEM_SHIELD + MAX_ITEM_INDEX)
            {
                ip->RequireStrength += Option3 * 5;
            }
            if (ip->Type >= ITEM_HELM && ip->Type < ITEM_BOOTS + MAX_ITEM_INDEX)
            {
                ip->RequireStrength += Option3 * 5;
            }
        }
    }
}

// Engine/Object/ZzzInventory.cpp, ComputeItemInfo: the requirement columns of the row of one level.
struct OldHelpTableRequirements
{
    int RequireLevel;
    int RequireStrength;
    int RequireDexterity;
    int RequireEnergy;
    int RequireVitality;
    int RequireCharisma;
};

OldHelpTableRequirements OldComputeItemInfoRequirements(int iHelpItem, int Level)
{
    ITEM_ATTRIBUTE* p = &ItemAttribute[iHelpItem];
    int RequireStrength = 0;
    int RequireDexterity = 0;
    int RequireEnergy = 0;
    int RequireCharisma = 0;
    int RequireVitality = 0;
    int RequireLevel = 0;
    if (p->RequireLevel)
    {
        RequireLevel = p->RequireLevel;
    }
    else
    {
        RequireLevel = 0;
    }

    if (p->RequireStrength)
    {
        RequireStrength = 20 + p->RequireStrength * (p->Level + Level * 3) * 3 / 100;
    }
    else
    {
        RequireStrength = 0;
    }

    if (p->RequireDexterity)	RequireDexterity = 20 + p->RequireDexterity * (p->Level + Level * 3) * 3 / 100;
    else RequireDexterity = 0;

    if (p->RequireVitality)	RequireVitality = 20 + p->RequireVitality * (p->Level + Level * 3) * 3 / 100;
    else RequireVitality = 0;

    if (p->RequireEnergy)
    {
        if (GameLogic::Items::IsSummonerBookType(iHelpItem))
        {
            RequireEnergy = 20 + p->RequireEnergy * (p->Level + Level * 1) * 3 / 100;
        }
        else
        {
            if ((p->RequireLevel > 0) && (iHelpItem >= ITEM_ETC && iHelpItem < ITEM_ETC + MAX_ITEM_INDEX))
            {
                RequireEnergy = 20 + (p->RequireEnergy) * (p->RequireLevel) * 4 / 100;
            }
            else
            {
                RequireEnergy = 20 + p->RequireEnergy * (p->Level + Level * 3) * 4 / 100;
            }
        }
    }
    else
    {
        RequireEnergy = 0;
    }

    // Like the item: the command requirement stays the base value.
    RequireCharisma = p->RequireCharisma;

    return {RequireLevel, RequireStrength, RequireDexterity, RequireEnergy, RequireVitality, RequireCharisma};
}
