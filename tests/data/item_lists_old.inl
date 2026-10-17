// One-time copy of the item lists before phase 3b (see test_item_lists_equivalence.cpp).

// ClassAttack.cpp, AttackKnight, Power Slash: only when not riding.
bool OldPowerSlashNotRiding(int helperModel)
{
    return helperModel < MODEL_HORN_OF_UNIRIA ||
           helperModel > MODEL_DARK_HORSE_ITEM && helperModel != MODEL_HORN_OF_FENRIR;
}

// MyInventory.cpp, IsEquipable: no mounts or Dark Lord pets in Chaos Castle or as a Crywolf hero elf.
bool OldIsChaosCastleBannedHelper(int itemType)
{
    return (itemType >= ITEM_HORN_OF_UNIRIA && itemType <= ITEM_DARK_RAVEN_ITEM) || itemType == ITEM_HORN_OF_FENRIR;
}

// MyInventory.cpp, FindManaItemIndex and FindHealingItemIndex: the potions they look for, in order.
std::vector<int> OldManaPotionOrder()
{
    std::vector<int> order;
    for (int i = ITEM_LARGE_MANA_POTION; i >= ITEM_SMALL_MANA_POTION; i--)
    {
        order.push_back(i);
    }
    return order;
}

std::vector<int> OldHealingPotionOrder()
{
    std::vector<int> order;
    for (int i = ITEM_LARGE_HEALING_POTION; i >= ITEM_APPLE; i--)
    {
        order.push_back(i);
    }
    return order;
}

// ItemEnduranceInfo.cpp, HasHelperLifeFrame.
bool OldHasHelperLifeFrame(int helperModel)
{
    return helperModel >= MODEL_HELPER && helperModel <= MODEL_DARK_HORSE_ITEM ||
           GameLogic::Items::IsDemonOrSpiritOfGuardianModel(helperModel) || helperModel == MODEL_PET_RUDOLF ||
           helperModel == MODEL_PET_PANDA || helperModel == MODEL_PET_UNICORN || helperModel == MODEL_PET_SKELETON ||
           helperModel == MODEL_HORN_OF_FENRIR;
}

// ItemEnduranceInfo.cpp, SyncIcons: no durability icon for the arrows in the right hand or the bolts in the left.
bool OldSkipsAmmunition(int itemType, int hand)
{
    return (hand == EQUIPMENT_WEAPON_RIGHT && itemType == ITEM_ARROWS) ||
           (hand == EQUIPMENT_WEAPON_LEFT && itemType == ITEM_BOLT);
}

// ZzzInfomation.cpp, CHARACTER_MACHINE::CalculateDefense: the wing whose defense option counts.
bool OldIsCapeOfLordOrFighter(int itemType)
{
    return itemType == ITEM_CAPE_OF_LORD || itemType == ITEM_CAPE_OF_FIGHTER;
}

// ZzzInfomation.cpp, CalcDefense: the items whose defense grows by the second tier wings' value per level.
bool OldHasSecondTierWingDefensePerLevel(const ITEM* ip)
{
    return GameLogic::Items::IsSecondTierWingExceptCape(ip) || ip->Type == ITEM_CAPE_OF_LORD ||
           ip->Type == ITEM_CAPE_OF_FIGHTER;
}

// MixMgr.cpp, CMixItem::SetItem: the three wing flags.
struct OldWingFlags
{
    bool isWing = false;
    bool isUpgradedWing = false;
    bool is3rdUpgradedWing = false;
};

OldWingFlags OldMixWingFlags(int itemType)
{
    OldWingFlags flags;
    switch (itemType)
    {
    case ITEM_WING:
    case ITEM_WINGS_OF_HEAVEN:
    case ITEM_WINGS_OF_SATAN:
    case ITEM_WING_OF_CURSE:
        flags.isWing = true;
        break;
    case ITEM_WINGS_OF_SPIRITS:
    case ITEM_WINGS_OF_SOUL:
    case ITEM_WINGS_OF_DRAGON:
    case ITEM_WINGS_OF_DARKNESS:
    case ITEM_CAPE_OF_LORD:
    case ITEM_WINGS_OF_DESPAIR:
    case ITEM_CAPE_OF_FIGHTER:
        flags.isUpgradedWing = true;
        break;
    case ITEM_WING_OF_STORM:
    case ITEM_WING_OF_ETERNAL:
    case ITEM_WING_OF_ILLUSION:
    case ITEM_WING_OF_RUIN:
    case ITEM_CAPE_OF_EMPEROR:
    case ITEM_WING_OF_DIMENSION:
    case ITEM_CAPE_OF_OVERRULE:
        flags.is3rdUpgradedWing = true;
    }
    return flags;
}

// ZzzInterface.cpp, SendMacroChat: the chat text's animation only when not riding (the list has no Horn of Fenrir).
bool OldSendMacroChatAnimates(int helperModel, bool safeZone)
{
    return (helperModel < MODEL_HORN_OF_UNIRIA || helperModel > MODEL_DARK_HORSE_ITEM) || safeZone;
}

// ZzzInterface.cpp, MoveHero: operating an object (the helper part is always true).
bool OldMoveHeroCanOperate(int helperModel, bool safeZone)
{
    return safeZone || (helperModel < MODEL_HORN_OF_UNIRIA || helperModel > MODEL_DARK_HORSE_ITEM ||
                        helperModel != MODEL_HORN_OF_FENRIR);
}

// ZzzInventory.cpp, CalcMaxDurability.
WORD OldCalcMaxDurability(const ITEM* ip, ITEM_ATTRIBUTE* p, int Level)
{
    WORD maxDurability = p->Durability;

    if (ip->Type >= ITEM_STAFF && ip->Type < ITEM_STAFF + MAX_ITEM_INDEX)
    {
        maxDurability = p->MagicDur;
    }
    for (int i = 0; i < Level; i++)
    {
        if (ip->Type >= ITEM_SCROLL_OF_BLOOD)
        {
            break;
        }
        else if (i >= 14)
        {
            maxDurability = (maxDurability + 8 >= 255 ? 255 : maxDurability + 8);
        }
        else if (i >= 13)
        {
            maxDurability = (maxDurability + 7 >= 255 ? 255 : maxDurability + 7);
        }
        else if (i >= 12)
        {
            maxDurability += 6;
        }
        else if (i >= 11)
        {
            maxDurability += 5;
        }
        else if (i >= 10)
        {
            maxDurability += 4;
        }
        else if (i >= 9)
        {
            maxDurability += 3;
        }
        else if (i >= 4)
        {
            maxDurability += 2;
        }
        else
        {
            maxDurability++;
        }
    }

    if (GameLogic::Items::IsDarkLordPet(ip))
    {
        maxDurability = 255;
    }

    if (ip->AncientDiscriminator > 0)
    {
        maxDurability += 20;
    }
    else if (ip->ExcellentFlags > 0 && (ip->Type < ITEM_WINGS_OF_SPIRITS || ip->Type > ITEM_WINGS_OF_DARKNESS) &&
             !GameLogic::Items::IsDivineArchangelWeapon(ip) && ip->Type != ITEM_CAPE_OF_LORD &&
             (ip->Type < ITEM_WING_OF_STORM || ip->Type > ITEM_CAPE_OF_EMPEROR) &&
             (ip->Type < ITEM_WINGS_OF_DESPAIR || ip->Type > ITEM_WING_OF_DIMENSION) &&
             !(ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        maxDurability += 15;
    }

    if (Check_LuckyItem(ip->Type))
    {
        maxDurability = 255;
    }

    return maxDurability;
}
