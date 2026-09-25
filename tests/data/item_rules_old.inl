// One-time copy of the item rule code before phase 3 (see test_item_rules_equivalence.cpp).
// Copy of Check_ItemAction from ZzzInventory.cpp.
sItemAct OldSet_ItemActOption(int _nIndex, int _nOption)
{
    sItemAct	sItem;
    // eITEM_PERSONALSHOP = 개인상점, eITEM_STORE = 창고, eITEM_TRADE = 거래, eITEM_DROP = 버리기, eITEM_SELL = 판매, eITEM_REPAIR = 수리
    int	nItemOption[][eITEM_END] = { 0, 1, 1, 0, 0, 0,
                                        0, 0, 0, 0, 1, 0,
        -1 };

    sItem.s_nItemIndex = _nIndex;

    for (int i = 0; i < eITEM_END; i++)
    {
        sItem.s_bType[i] = nItemOption[_nOption][i];
    }
    return sItem;
}

bool OldCheck_ItemAction(const ITEM* _pItem, ITEMSETOPTION _eAction, bool _bType = false)
{
    std::vector<sItemAct>			sItem;
    std::vector<sItemAct>::iterator li;
    int		i = 0;

    // Restricted ITEM_HELPER special items starting at local index 135.
    for (i = 0; i < RESTRICTED_SPECIAL_MISC_COUNT; i++)
    {
        sItem.push_back(OldSet_ItemActOption(ITEM_HELPER + RESTRICTED_SPECIAL_MISC_START_INDEX + i, ITEM_ACTION_BLOCK_STORAGE_TRADE));
    }
    // Restricted ITEM_POTION special jewels starting at local index 160.
    for (i = 0; i < RESTRICTED_SPECIAL_JEWEL_COUNT; i++)
    {
        sItem.push_back(OldSet_ItemActOption(ITEM_POTION + RESTRICTED_SPECIAL_JEWEL_START_INDEX + i, ITEM_ACTION_BLOCK_STORAGE_TRADE));
    }
    for (i = 0; i < LUCKY_SET_ARMOR_COUNT; i++)
    {
        sItem.push_back(OldSet_ItemActOption(ITEM_ARMOR + LUCKY_SET_ARMOR_START_INDEX + i, ITEM_ACTION_BLOCK_SELL_ONLY));
        sItem.push_back(OldSet_ItemActOption(ITEM_HELM + LUCKY_SET_ARMOR_START_INDEX + i, ITEM_ACTION_BLOCK_SELL_ONLY));
        sItem.push_back(OldSet_ItemActOption(ITEM_BOOTS + LUCKY_SET_ARMOR_START_INDEX + i, ITEM_ACTION_BLOCK_SELL_ONLY));
        sItem.push_back(OldSet_ItemActOption(ITEM_GLOVES + LUCKY_SET_ARMOR_START_INDEX + i, ITEM_ACTION_BLOCK_SELL_ONLY));
        sItem.push_back(OldSet_ItemActOption(ITEM_PANTS + LUCKY_SET_ARMOR_START_INDEX + i, ITEM_ACTION_BLOCK_SELL_ONLY));
    }

    for (li = sItem.begin(); li != sItem.end(); li++)
    {
        if (li->s_nItemIndex == _pItem->Type)
        {
            _bType = (li->s_bType[_eAction]) ^ (!_bType);
            return _bType;
        }
    }

    // 등록되지 않은 아이템은 무시.
    return false;
}

namespace OldRules
{
    // The old header.
    bool IsWingItem(const ITEM* pItem);
    bool IsSecondTierWingExceptCape(const ITEM* pItem);
    bool IsThirdTierWing(const ITEM* pItem);
    bool IsSmallWing(const ITEM* pItem);
    bool IsClothWing(const ITEM* pItem);
    bool IsClothWingModel(int modelType);
    bool IsRideableMount(const ITEM* pItem);
    bool IsRideableMountModel(int modelType);
    bool IsHornMountModel(int modelType);
    bool IsFlyingMount(const ITEM* pItem);
    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing);
    bool IsDarkLordPet(const ITEM* pItem);
    bool IsDarkLordPetType(int itemType);
    bool IsDemonOrSpiritOfGuardian(const ITEM* pItem);
    bool IsDemonOrSpiritOfGuardianType(int itemType);
    bool IsDemonOrSpiritOfGuardianModel(int modelType);
    bool IsPandaOrSkeletonItem(const ITEM* pItem);
    bool IsJewelItem(const ITEM* pItem);
    bool IsRefineStone(const ITEM* pItem);
    bool IsRefineStoneModel(int modelType);
    bool IsWingMixCharm(const ITEM* pItem);
    bool IsWingMixCharmType(int itemType);
    bool IsWingMixCharmModel(int modelType);
    bool IsSocketSeedOrSphere(const ITEM* pItem);
    bool IsSocketSeedOrSphereType(int itemType);
    bool IsSocketSeedOrSphereModel(int modelType);
    bool IsSocketSeed(const ITEM* pItem);
    bool IsSocketSeedModel(int modelType);
    bool IsSocketSphere(const ITEM* pItem);
    bool IsSocketSphereModel(int modelType);
    bool IsSocketSeedSphere(const ITEM* pItem);
    bool IsSocketSeedSphereType(int itemType);
    bool IsSocketSeedSphereModel(int modelType);
    bool IsHealingPotion(const ITEM* pItem);
    bool IsHealingPotionType(int itemType);
    bool IsManaPotionType(int itemType);
    bool IsComplexPotion(const ITEM* pItem);
    bool IsComplexPotionType(int itemType);
    bool IsElitePotion(const ITEM* pItem);
    bool IsElitePotionType(int itemType);
    bool IsElitePotionModel(int modelType);
    bool IsElixir(const ITEM* pItem);
    bool IsElixirType(int itemType);
    bool IsElixirModel(int modelType);
    bool IsBuffScroll(const ITEM* pItem);
    bool IsBuffScrollType(int itemType);
    bool IsBuffScrollModel(int modelType);
    bool IsBattleOrStrengthScroll(const ITEM* pItem);
    bool IsBattleOrStrengthScrollType(int itemType);
    bool IsBattleOrStrengthScrollModel(int modelType);
    bool IsResetFruit(const ITEM* pItem);
    bool IsResetFruitType(int itemType);
    bool IsResetFruitModel(int modelType);
    bool IsSeal(const ITEM* pItem);
    bool IsSealType(int itemType);
    bool IsSealModel(int modelType);
    bool IsHealingOrDivinitySeal(const ITEM* pItem);
    bool IsHealingOrDivinitySealType(int itemType);
    bool IsHealingOrDivinitySealModel(int modelType);
    bool IsAmmunition(const ITEM* pItem);
    bool IsAmmunitionType(int itemType);
    bool IsAmmunitionModel(int modelType);
    bool IsEventTicket(const ITEM* pItem);
    bool IsEventTicketType(int itemType);
    bool IsEventTicketModel(int modelType);
    bool IsDoppelgangerOrVarkaTicket(const ITEM* pItem);
    bool IsDoppelgangerOrVarkaTicketType(int itemType);
    bool IsDoppelgangerOrVarkaTicketModel(int modelType);
    bool IsRareItemTicket(const ITEM* pItem);
    bool IsLuckyItemTicket(const ITEM* pItem);
    bool IsLuckyItemTicketModel(int modelType);
    bool IsAccountServiceItem(const ITEM* pItem);
    bool IsAccountServiceItemModel(int modelType);
    bool IsDayPass(const ITEM* pItem);
    bool IsDayPassModel(int modelType);
    bool IsHourPass(const ITEM* pItem);
    bool IsHourPassModel(int modelType);
    bool IsPackageBox(const ITEM* pItem);
    bool IsPackageBoxModel(int modelType);
    bool IsSilverOrGoldKey(const ITEM* pItem);
    bool IsSilverOrGoldKeyModel(int modelType);
    bool IsGemJewelry(const ITEM* pItem);
    bool IsGambleItem(const ITEM* pItem);
    bool IsGambleItemType(int itemType);
    bool IsGambleItemModel(int modelType);
    bool IsCharacterCard(const ITEM* pItem);
    bool IsCharacterCardModel(int modelType);
    bool IsPartChargeItem(const ITEM* pItem);
    bool IsDevilSquareItem(const ITEM* pItem);
    bool IsDevilSquareItemType(int itemType);
    bool IsBloodCastleTicketPart(const ITEM* pItem);
    bool IsBloodCastleTicketPartType(int itemType);
    bool IsBloodCastleTicketPartModel(int modelType);
    bool IsChocolateBox(const ITEM* pItem);
    bool IsChocolateBoxType(int itemType);
    bool IsChocolateBoxModel(int modelType);
    bool IsRibbonBox(const ITEM* pItem);
    bool IsRibbonBoxType(int itemType);
    bool IsRibbonBoxModel(int modelType);
    bool IsSecondClassQuestItem(const ITEM* pItem);
    bool IsSecondClassQuestItemType(int itemType);
    bool IsThirdClassQuestItem(const ITEM* pItem);
    bool IsSecromiconQuestItem(const ITEM* pItem);
    bool IsSecromiconQuestItemModel(int modelType);
    bool IsDivineArchangelWeapon(const ITEM* pItem);
    bool IsDivineArchangelWeaponType(int itemType);
    bool IsDivineArchangelWeaponModel(int modelType);
    bool IsSummonerBook(const ITEM* pItem);
    bool IsSummonerBookType(int itemType);
    bool IsSummonerBookModel(int modelType);
    bool IsSummonerStickModel(int modelType);
    bool IsSummonerSkillParchmentModel(int modelType);
    bool IsRageFighterSkillParchmentModel(int modelType);
    bool IsHighValueItem(const ITEM* pItem);
    bool IsTradeBan(const ITEM* pItem);
    bool IsDropBan(const ITEM* pItem);
    bool IsStoreBan(const ITEM* pItem);
    bool IsPersonalShopBan(const ITEM* pItem);
    bool IsSellingBan(const ITEM* pItem);
    bool IsRepairBan(const ITEM* pItem);



namespace
{
    bool IsClothWingType(int itemType)
    {
        return itemType == ITEM_CAPE_OF_LORD
            || itemType == ITEM_WING_OF_RUIN
            || itemType == ITEM_CAPE_OF_EMPEROR
            || itemType == ITEM_CAPE_OF_FIGHTER
            || itemType == ITEM_CAPE_OF_OVERRULE
            || itemType == ITEM_SMALL_CAPE_OF_LORD
            || itemType == ITEM_LITTLE_WARRIORS_CLOAK;
    }

    bool IsRideableMountType(int itemType)
    {
        return itemType == ITEM_HORN_OF_UNIRIA
            || itemType == ITEM_HORN_OF_DINORANT
            || itemType == ITEM_DARK_HORSE_ITEM
            || itemType == ITEM_HORN_OF_FENRIR;
    }

    bool IsHornMountType(int itemType)
    {
        return itemType == ITEM_HORN_OF_UNIRIA
            || itemType == ITEM_HORN_OF_DINORANT
            || itemType == ITEM_HORN_OF_FENRIR;
    }

    bool IsCharacterCardType(int itemType)
    {
        return itemType == ITEM_MAGIC_GLADIATOR_CHARACTER_CARD
            || itemType == ITEM_DARK_LORD_CHARACTER_CARD
            || itemType == ITEM_SUMMONER_CHARACTER_CARD;
    }

    bool IsRefineStoneType(int itemType)
    {
        return itemType == ITEM_LOWER_REFINE_STONE
            || itemType == ITEM_HIGHER_REFINE_STONE;
    }

    bool IsLuckyItemTicketType(int itemType)
    {
        return itemType >= ITEM_FIRST_LUCKY_ARMOR_TICKET && itemType <= ITEM_HELPER + 145;
    }

    bool IsAccountServiceItemType(int itemType)
    {
        return itemType >= ITEM_MASTER_SKILL_RESET && itemType <= ITEM_PREMIUM_PACKAGE;
    }

    bool IsDayPassType(int itemType)
    {
        return itemType >= ITEM_30_DAY_PASS && itemType <= ITEM_90_DAY_PASS_POINTS;
    }

    bool IsHourPassType(int itemType)
    {
        return itemType == ITEM_3_HOUR_PASS
            || itemType == ITEM_5_HOUR_PASS
            || itemType == ITEM_10_HOUR_PASS;
    }

    bool IsPackageBoxType(int itemType)
    {
        return itemType >= ITEM_PACKAGE_BOX_A && itemType <= ITEM_PACKAGE_BOX_F;
    }

    bool IsSilverOrGoldKeyType(int itemType)
    {
        return itemType == ITEM_SILVER_KEY
            || itemType == ITEM_GOLD_KEY;
    }

    bool IsGemJewelryType(int itemType)
    {
        return itemType >= ITEM_SAPPHIRE_RING && itemType <= ITEM_SAPPHIRE_NECKLACE;
    }

    bool IsPandaOrSkeletonItemType(int itemType)
    {
        return itemType == ITEM_PET_PANDA
            || itemType == ITEM_PANDA_TRANSFORMATION_RING
            || itemType == ITEM_PET_SKELETON
            || itemType == ITEM_SKELETON_TRANSFORMATION_RING;
    }

    bool IsThirdClassQuestItemType(int itemType)
    {
        return itemType == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
            || itemType == ITEM_HORN_OF_HELL_MAINE
            || itemType == ITEM_FEATHER_OF_DARK_PHOENIX
            || itemType == ITEM_EYE_OF_ABYSSAL;
    }

    bool IsSecromiconQuestItemType(int itemType)
    {
        return itemType >= ITEM_SUSPICIOUS_SCRAP_OF_PAPER && itemType <= ITEM_COMPLETE_SECROMICON;
    }

    bool IsSummonerStickType(int itemType)
    {
        return itemType >= ITEM_MISTERY_STICK && itemType <= ITEM_ETERNAL_WING_STICK;
    }

    bool IsSummonerSkillParchmentType(int itemType)
    {
        return itemType >= ITEM_CHAIN_LIGHTNING_PARCHMENT && itemType <= ITEM_INNOVATION_PARCHMENT;
    }

    bool IsRageFighterSkillParchmentType(int itemType)
    {
        return itemType >= ITEM_CHAIN_DRIVE_PARCHMENT && itemType <= ITEM_INCREASE_BLOCK_PARCHMENT;
    }

    bool IsSocketSeedType(int itemType)
    {
        return itemType >= ITEM_SEED_FIRE && itemType <= ITEM_SEED_EARTH;
    }

    bool IsSocketSphereType(int itemType)
    {
        return itemType >= ITEM_SPHERE_MONO && itemType <= ITEM_SPHERE_5;
    }
}


    bool IsWingItem(const ITEM* pItem)
    {
        switch (pItem->Type)
        {
        case ITEM_WING:
        case ITEM_WINGS_OF_HEAVEN:
        case ITEM_WINGS_OF_SATAN:
        case ITEM_WINGS_OF_SPIRITS:
        case ITEM_WINGS_OF_SOUL:
        case ITEM_WINGS_OF_DRAGON:
        case ITEM_WINGS_OF_DARKNESS:
        case ITEM_CAPE_OF_LORD:
        case ITEM_WING_OF_STORM:
        case ITEM_WING_OF_ETERNAL:
        case ITEM_WING_OF_ILLUSION:
        case ITEM_WING_OF_RUIN:
        case ITEM_CAPE_OF_EMPEROR:
        case ITEM_WING_OF_CURSE:
        case ITEM_WINGS_OF_DESPAIR:
        case ITEM_WING_OF_DIMENSION:
        case ITEM_SMALL_CAPE_OF_LORD:
        case ITEM_SMALL_WING_OF_CURSE:
        case ITEM_SMALL_WINGS_OF_ELF:
        case ITEM_SMALL_WINGS_OF_HEAVEN:
        case ITEM_SMALL_WINGS_OF_SATAN:
        case ITEM_CAPE_OF_FIGHTER:
        case ITEM_CAPE_OF_OVERRULE:
        case ITEM_LITTLE_WARRIORS_CLOAK:
            return true;
        }

        return false;
    }

    bool IsSecondTierWingExceptCape(const ITEM* pItem)
    {
        return (pItem->Type >= ITEM_WINGS_OF_SPIRITS && pItem->Type <= ITEM_WINGS_OF_DARKNESS)
            || pItem->Type == ITEM_WINGS_OF_DESPAIR;
    }

    bool IsThirdTierWing(const ITEM* pItem)
    {
        return (pItem->Type >= ITEM_WING_OF_STORM && pItem->Type <= ITEM_CAPE_OF_EMPEROR)
            || pItem->Type == ITEM_WING_OF_DIMENSION
            || pItem->Type == ITEM_CAPE_OF_OVERRULE;
    }

    bool IsSmallWing(const ITEM* pItem)
    {
        return pItem->Type >= ITEM_SMALL_CAPE_OF_LORD && pItem->Type <= ITEM_LITTLE_WARRIORS_CLOAK;
    }

    bool IsClothWing(const ITEM* pItem)
    {
        return IsClothWingType(pItem->Type);
    }

    bool IsClothWingModel(int modelType)
    {
        return IsClothWingType(modelType - MODEL_ITEM);
    }

    bool IsRideableMount(const ITEM* pItem)
    {
        return IsRideableMountType(pItem->Type);
    }

    bool IsRideableMountModel(int modelType)
    {
        return IsRideableMountType(modelType - MODEL_ITEM);
    }

    bool IsHornMountModel(int modelType)
    {
        return IsHornMountType(modelType - MODEL_ITEM);
    }

    bool IsFlyingMount(const ITEM* pItem)
    {
        return pItem->Type == ITEM_HORN_OF_DINORANT
            || pItem->Type == ITEM_DARK_HORSE_ITEM
            || pItem->Type == ITEM_HORN_OF_FENRIR;
    }

    bool HasFlightEquipment(const ITEM* pItemHelper, const ITEM* pItemWing)
    {
        return IsWingItem(pItemWing) || IsFlyingMount(pItemHelper);
    }

    bool IsJewelItem(const ITEM* pItem)
    {
        return pItem->Type == ITEM_JEWEL_OF_BLESS
            || pItem->Type == ITEM_JEWEL_OF_SOUL
            || pItem->Type == ITEM_JEWEL_OF_LIFE
            || pItem->Type == ITEM_JEWEL_OF_CHAOS
            || pItem->Type == ITEM_JEWEL_OF_CREATION
            || pItem->Type == ITEM_JEWEL_OF_GUARDIAN;
    }

    bool IsWingMixCharmType(int itemType)
    {
        return itemType >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN && itemType <= ITEM_TYPE_CHARM_MIXWING + EWS_END;
    }

    bool IsWingMixCharm(const ITEM* pItem)
    {
        return IsWingMixCharmType(pItem->Type);
    }

    bool IsWingMixCharmModel(int modelType)
    {
        return IsWingMixCharmType(modelType - MODEL_ITEM);
    }

    bool IsSocketSeedOrSphereType(int itemType)
    {
        return IsSocketSeedType(itemType)
            || IsSocketSphereType(itemType)
            || IsSocketSeedSphereType(itemType);
    }

    bool IsSocketSeedOrSphere(const ITEM* pItem)
    {
        return IsSocketSeedOrSphereType(pItem->Type);
    }

    bool IsSocketSeedOrSphereModel(int modelType)
    {
        return IsSocketSeedOrSphereType(modelType - MODEL_ITEM);
    }

    bool IsSealType(int itemType)
    {
        return itemType == ITEM_SEAL_OF_ASCENSION
            || itemType == ITEM_SEAL_OF_WEALTH
            || itemType == ITEM_SEAL_OF_SUSTENANCE;
    }

    bool IsSeal(const ITEM* pItem)
    {
        return IsSealType(pItem->Type);
    }

    bool IsSealModel(int modelType)
    {
        return IsSealType(modelType - MODEL_ITEM);
    }

    bool IsGambleItemType(int itemType)
    {
        return itemType == ITEM_GAMBLE_SWORD_MACE_SPEAR
            || itemType == ITEM_GAMBLE_STAFF
            || itemType == ITEM_GAMBLE_BOW_CROSSBOW
            || itemType == ITEM_GAMBLE_SCEPTER
            || itemType == ITEM_GAMBLE_STICK;
    }

    bool IsGambleItem(const ITEM* pItem)
    {
        return IsGambleItemType(pItem->Type);
    }

    bool IsGambleItemModel(int modelType)
    {
        return IsGambleItemType(modelType - MODEL_ITEM);
    }

    bool IsCharacterCard(const ITEM* pItem)
    {
        return IsCharacterCardType(pItem->Type);
    }

    bool IsCharacterCardModel(int modelType)
    {
        return IsCharacterCardType(modelType - MODEL_ITEM);
    }

    bool IsDevilSquareItemType(int itemType)
    {
        return itemType == ITEM_DEVILS_EYE
            || itemType == ITEM_DEVILS_KEY
            || itemType == ITEM_DEVILS_INVITATION;
    }

    bool IsDevilSquareItem(const ITEM* pItem)
    {
        return IsDevilSquareItemType(pItem->Type);
    }

    bool IsDivineArchangelWeapon(const ITEM* pItem)
    {
        return IsDivineArchangelWeaponType(pItem->Type);
    }

    bool IsDivineArchangelWeaponType(int itemType)
    {
        return itemType == ITEM_DIVINE_SWORD_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_CB_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_STAFF_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_STICK_OF_ARCHANGEL
            || itemType == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL;
    }

    bool IsDivineArchangelWeaponModel(int modelType)
    {
        return IsDivineArchangelWeaponType(modelType - MODEL_ITEM);
    }

    bool IsPartChargeItem(const ITEM* pItem)
    {
        return IsEventTicket(pItem)
            || pItem->Type == ITEM_CHAOS_CARD
            || IsRareItemTicket(pItem)
            || IsDoppelgangerOrVarkaTicket(pItem)
            || pItem->Type == ITEM_TALISMAN_OF_LUCK
            || IsSeal(pItem)
            || IsElitePotion(pItem)
            || IsBuffScroll(pItem)
            || pItem->Type == ITEM_SEAL_OF_MOBILITY
            || IsResetFruit(pItem)
            || IsElixir(pItem)
            || pItem->Type == ITEM_INDULGENCE
            || pItem->Type == ITEM_ILLUSION_TEMPLE_TICKET
            || pItem->Type == ITEM_SUMMONER_CHARACTER_CARD
            || pItem->Type == ITEM_CHAOS_CARD_GOLD
            || pItem->Type == ITEM_CHAOS_CARD_RARE
            || pItem->Type == ITEM_CHAOS_CARD_MINI
            || pItem->Type == ITEM_MEDIUM_ELITE_HEALING_POTION
            || IsHealingOrDivinitySeal(pItem)
            || IsBattleOrStrengthScroll(pItem)
            || pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY
            || IsDemonOrSpiritOfGuardian(pItem)
            || pItem->Type == ITEM_TALISMAN_OF_RESURRECTION
            || pItem->Type == ITEM_TALISMAN_OF_MOBILITY
            || pItem->Type == ITEM_TALISMAN_OF_GUARDIAN
            || pItem->Type == ITEM_TALISMAN_OF_ITEM_PROTECTION
            || pItem->Type == ITEM_MASTER_SEAL_OF_ASCENSION
            || pItem->Type == ITEM_MASTER_SEAL_OF_WEALTH
            || pItem->Type == ITEM_LETHAL_WIZARDS_RING
            || pItem->Type == ITEM_MAX_AG_BOOST_AURA
            || pItem->Type == ITEM_MAX_SD_BOOST_AURA
            || pItem->Type == ITEM_PARTY_EXP_SCROLL
            || pItem->Type == ITEM_ELITE_SD_POTION
            || IsGemJewelry(pItem)
            || IsSilverOrGoldKey(pItem)
            || pItem->Type == ITEM_GOBLIN_GOLD_COIN
            || pItem->Type == ITEM_GOLDEN_BOX
            || pItem->Type == ITEM_SILVER_BOX
            || IsPackageBox(pItem)
            || IsSmallWing(pItem)
            || pItem->Type == ITEM_HELPER + 116
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_PAID_CHANNEL_ACCESS_TICKET
            || IsAccountServiceItem(pItem)
            || IsDayPass(pItem)
            || IsHourPass(pItem)
            || pItem->Type == ITEM_OPEN_ACCESS_TICKET_TO_CHAOS_CASTLE
            || pItem->Type == ITEM_SCROLL_OF_HEALING;
    }

    bool IsHighValueItem(const ITEM* pItem)
    {
        int iLevel = pItem->Level;

        if (pItem->Type == ITEM_HORN_OF_DINORANT
            || IsJewelItem(pItem)
            || pItem->Type == ITEM_PACKED_JEWEL_OF_BLESS
            || pItem->Type == ITEM_PACKED_JEWEL_OF_SOUL
            || IsWingItem(pItem)
            || IsDarkLordPet(pItem)
            || pItem->AncientDiscriminator > 0
            || IsDivineArchangelWeapon(pItem)
            || pItem->Type == ITEM_LOCHS_FEATHER
            || pItem->Type == ITEM_FRUITS
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || pItem->Type == ITEM_SPIRIT
            || (pItem->Type >= ITEM_GEMSTONE && pItem->Type <= ITEM_HIGHER_REFINE_STONE)
            || (iLevel > 6 && pItem->Type < ITEM_WING)
            || pItem->ExcellentFlags > 0
            || (pItem->Type >= ITEM_CLAW_OF_BEAST && pItem->Type <= ITEM_HORN_OF_FENRIR)
            || pItem->Type == ITEM_FLAME_OF_CONDOR
            || pItem->Type == ITEM_FEATHER_OF_CONDOR
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            || IsPandaOrSkeletonItem(pItem)
            || IsDemonOrSpiritOfGuardian(pItem)
            || IsGemJewelry(pItem)
            || IsSilverOrGoldKey(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && iLevel == 0)
            || COMGEM::isCompiledGem(pItem))
        {
            if (pItem->bPeriodItem && !pItem->bExpiredPeriod)
            {
                return false;
            }

            if (IsPandaOrSkeletonItem(pItem) || IsDemonOrSpiritOfGuardian(pItem))
            {
                return pItem->bPeriodItem && pItem->bExpiredPeriod;
            }

            return true;
        }

        return false;
    }

    bool IsHealingPotionType(int itemType)
    {
        return itemType >= ITEM_APPLE && itemType <= ITEM_LARGE_HEALING_POTION;
    }

    bool IsHealingPotion(const ITEM* pItem)
    {
        return IsHealingPotionType(pItem->Type);
    }

    bool IsManaPotionType(int itemType)
    {
        return itemType >= ITEM_SMALL_MANA_POTION && itemType <= ITEM_LARGE_MANA_POTION;
    }

    bool IsComplexPotionType(int itemType)
    {
        return itemType >= ITEM_SMALL_COMPLEX_POTION && itemType <= ITEM_LARGE_COMPLEX_POTION;
    }

    bool IsComplexPotion(const ITEM* pItem)
    {
        return IsComplexPotionType(pItem->Type);
    }

    bool IsElitePotionType(int itemType)
    {
        return itemType == ITEM_ELITE_HEALING_POTION
            || itemType == ITEM_ELITE_MANA_POTION;
    }

    bool IsElitePotion(const ITEM* pItem)
    {
        return IsElitePotionType(pItem->Type);
    }

    bool IsElitePotionModel(int modelType)
    {
        return IsElitePotionType(modelType - MODEL_ITEM);
    }

    bool IsElixirType(int itemType)
    {
        return itemType >= ITEM_ELIXIR_OF_STRENGTH && itemType <= ITEM_ELIXIR_OF_CONTROL;
    }

    bool IsElixir(const ITEM* pItem)
    {
        return IsElixirType(pItem->Type);
    }

    bool IsElixirModel(int modelType)
    {
        return IsElixirType(modelType - MODEL_ITEM);
    }

    bool IsBuffScrollType(int itemType)
    {
        return itemType >= ITEM_SCROLL_OF_QUICKNESS && itemType <= ITEM_SCROLL_OF_MANA;
    }

    bool IsBuffScroll(const ITEM* pItem)
    {
        return IsBuffScrollType(pItem->Type);
    }

    bool IsBuffScrollModel(int modelType)
    {
        return IsBuffScrollType(modelType - MODEL_ITEM);
    }

    bool IsBattleOrStrengthScrollType(int itemType)
    {
        return itemType == ITEM_SCROLL_OF_BATTLE
            || itemType == ITEM_SCROLL_OF_STRENGTH;
    }

    bool IsBattleOrStrengthScroll(const ITEM* pItem)
    {
        return IsBattleOrStrengthScrollType(pItem->Type);
    }

    bool IsBattleOrStrengthScrollModel(int modelType)
    {
        return IsBattleOrStrengthScrollType(modelType - MODEL_ITEM);
    }

    bool IsResetFruitType(int itemType)
    {
        return itemType >= ITEM_RESET_FRUIT_STRENGTH && itemType <= ITEM_RESET_FRUIT_CONTROL;
    }

    bool IsResetFruit(const ITEM* pItem)
    {
        return IsResetFruitType(pItem->Type);
    }

    bool IsResetFruitModel(int modelType)
    {
        return IsResetFruitType(modelType - MODEL_ITEM);
    }

    bool IsHealingOrDivinitySealType(int itemType)
    {
        return itemType == ITEM_SEAL_OF_HEALING
            || itemType == ITEM_SEAL_OF_DIVINITY;
    }

    bool IsHealingOrDivinitySeal(const ITEM* pItem)
    {
        return IsHealingOrDivinitySealType(pItem->Type);
    }

    bool IsHealingOrDivinitySealModel(int modelType)
    {
        return IsHealingOrDivinitySealType(modelType - MODEL_ITEM);
    }

    bool IsRefineStone(const ITEM* pItem)
    {
        return IsRefineStoneType(pItem->Type);
    }

    bool IsRefineStoneModel(int modelType)
    {
        return IsRefineStoneType(modelType - MODEL_ITEM);
    }

    bool IsAmmunitionType(int itemType)
    {
        return itemType == ITEM_BOLT
            || itemType == ITEM_ARROWS;
    }

    bool IsAmmunition(const ITEM* pItem)
    {
        return IsAmmunitionType(pItem->Type);
    }

    bool IsAmmunitionModel(int modelType)
    {
        return IsAmmunitionType(modelType - MODEL_ITEM);
    }

    bool IsEventTicketType(int itemType)
    {
        return itemType == ITEM_DEVIL_SQUARE_TICKET
            || itemType == ITEM_BLOOD_CASTLE_TICKET
            || itemType == ITEM_KALIMA_TICKET;
    }

    bool IsEventTicket(const ITEM* pItem)
    {
        return IsEventTicketType(pItem->Type);
    }

    bool IsEventTicketModel(int modelType)
    {
        return IsEventTicketType(modelType - MODEL_ITEM);
    }

    bool IsDoppelgangerOrVarkaTicketType(int itemType)
    {
        return itemType == ITEM_OPEN_ACCESS_TICKET_TO_DOPPELGANGER
            || itemType == ITEM_OPEN_ACCESS_TICKET_TO_VARKA
            || itemType == ITEM_OPEN_ACCESS_TICKET_TO_VARKA_7;
    }

    bool IsDoppelgangerOrVarkaTicket(const ITEM* pItem)
    {
        return IsDoppelgangerOrVarkaTicketType(pItem->Type);
    }

    bool IsDoppelgangerOrVarkaTicketModel(int modelType)
    {
        return IsDoppelgangerOrVarkaTicketType(modelType - MODEL_ITEM);
    }

    bool IsRareItemTicket(const ITEM* pItem)
    {
        return (pItem->Type >= ITEM_RARE_ITEM_TICKET_1 && pItem->Type <= ITEM_RARE_ITEM_TICKET_5)
            || (pItem->Type >= ITEM_RARE_ITEM_TICKET_7 && pItem->Type <= ITEM_RARE_ITEM_TICKET_12);
    }

    bool IsLuckyItemTicket(const ITEM* pItem)
    {
        return IsLuckyItemTicketType(pItem->Type);
    }

    bool IsLuckyItemTicketModel(int modelType)
    {
        return IsLuckyItemTicketType(modelType - MODEL_ITEM);
    }

    bool IsBloodCastleTicketPartType(int itemType)
    {
        return itemType == ITEM_SCROLL_OF_ARCHANGEL
            || itemType == ITEM_BLOOD_BONE;
    }

    bool IsBloodCastleTicketPart(const ITEM* pItem)
    {
        return IsBloodCastleTicketPartType(pItem->Type);
    }

    bool IsBloodCastleTicketPartModel(int modelType)
    {
        return IsBloodCastleTicketPartType(modelType - MODEL_ITEM);
    }

    bool IsAccountServiceItem(const ITEM* pItem)
    {
        return IsAccountServiceItemType(pItem->Type);
    }

    bool IsAccountServiceItemModel(int modelType)
    {
        return IsAccountServiceItemType(modelType - MODEL_ITEM);
    }

    bool IsDayPass(const ITEM* pItem)
    {
        return IsDayPassType(pItem->Type);
    }

    bool IsDayPassModel(int modelType)
    {
        return IsDayPassType(modelType - MODEL_ITEM);
    }

    bool IsHourPass(const ITEM* pItem)
    {
        return IsHourPassType(pItem->Type);
    }

    bool IsHourPassModel(int modelType)
    {
        return IsHourPassType(modelType - MODEL_ITEM);
    }

    bool IsPackageBox(const ITEM* pItem)
    {
        return IsPackageBoxType(pItem->Type);
    }

    bool IsPackageBoxModel(int modelType)
    {
        return IsPackageBoxType(modelType - MODEL_ITEM);
    }

    bool IsSilverOrGoldKey(const ITEM* pItem)
    {
        return IsSilverOrGoldKeyType(pItem->Type);
    }

    bool IsSilverOrGoldKeyModel(int modelType)
    {
        return IsSilverOrGoldKeyType(modelType - MODEL_ITEM);
    }

    bool IsGemJewelry(const ITEM* pItem)
    {
        return IsGemJewelryType(pItem->Type);
    }

    bool IsChocolateBoxType(int itemType)
    {
        return itemType == ITEM_PINK_CHOCOLATE_BOX
            || itemType == ITEM_RED_CHOCOLATE_BOX
            || itemType == ITEM_BLUE_CHOCOLATE_BOX;
    }

    bool IsChocolateBox(const ITEM* pItem)
    {
        return IsChocolateBoxType(pItem->Type);
    }

    bool IsChocolateBoxModel(int modelType)
    {
        return IsChocolateBoxType(modelType - MODEL_ITEM);
    }

    bool IsRibbonBoxType(int itemType)
    {
        return itemType == ITEM_RED_RIBBON_BOX
            || itemType == ITEM_GREEN_RIBBON_BOX
            || itemType == ITEM_BLUE_RIBBON_BOX;
    }

    bool IsRibbonBox(const ITEM* pItem)
    {
        return IsRibbonBoxType(pItem->Type);
    }

    bool IsRibbonBoxModel(int modelType)
    {
        return IsRibbonBoxType(modelType - MODEL_ITEM);
    }

    bool IsDarkLordPetType(int itemType)
    {
        return itemType == ITEM_DARK_HORSE_ITEM
            || itemType == ITEM_DARK_RAVEN_ITEM;
    }

    bool IsDarkLordPet(const ITEM* pItem)
    {
        return IsDarkLordPetType(pItem->Type);
    }

    bool IsDemonOrSpiritOfGuardianType(int itemType)
    {
        return itemType == ITEM_DEMON
            || itemType == ITEM_SPIRIT_OF_GUARDIAN;
    }

    bool IsDemonOrSpiritOfGuardian(const ITEM* pItem)
    {
        return IsDemonOrSpiritOfGuardianType(pItem->Type);
    }

    bool IsDemonOrSpiritOfGuardianModel(int modelType)
    {
        return IsDemonOrSpiritOfGuardianType(modelType - MODEL_ITEM);
    }

    bool IsPandaOrSkeletonItem(const ITEM* pItem)
    {
        return IsPandaOrSkeletonItemType(pItem->Type);
    }

    bool IsSecondClassQuestItemType(int itemType)
    {
        return itemType == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR
            || itemType == ITEM_BROKEN_SWORD_DARK_STONE
            || itemType == ITEM_TEAR_OF_ELF
            || itemType == ITEM_SOUL_SHARD_OF_WIZARD;
    }

    bool IsSecondClassQuestItem(const ITEM* pItem)
    {
        return IsSecondClassQuestItemType(pItem->Type);
    }

    bool IsThirdClassQuestItem(const ITEM* pItem)
    {
        return IsThirdClassQuestItemType(pItem->Type);
    }

    bool IsSecromiconQuestItem(const ITEM* pItem)
    {
        return IsSecromiconQuestItemType(pItem->Type);
    }

    bool IsSecromiconQuestItemModel(int modelType)
    {
        return IsSecromiconQuestItemType(modelType - MODEL_ITEM);
    }

    bool IsSummonerBookType(int itemType)
    {
        return itemType >= ITEM_BOOK_OF_SAHAMUTT && itemType <= ITEM_STAFF + 29;
    }

    bool IsSummonerBook(const ITEM* pItem)
    {
        return IsSummonerBookType(pItem->Type);
    }

    bool IsSummonerBookModel(int modelType)
    {
        return IsSummonerBookType(modelType - MODEL_ITEM);
    }

    bool IsSummonerStickModel(int modelType)
    {
        return IsSummonerStickType(modelType - MODEL_ITEM);
    }

    bool IsSummonerSkillParchmentModel(int modelType)
    {
        return IsSummonerSkillParchmentType(modelType - MODEL_ITEM);
    }

    bool IsRageFighterSkillParchmentModel(int modelType)
    {
        return IsRageFighterSkillParchmentType(modelType - MODEL_ITEM);
    }

    bool IsSocketSeed(const ITEM* pItem)
    {
        return IsSocketSeedType(pItem->Type);
    }

    bool IsSocketSeedModel(int modelType)
    {
        return IsSocketSeedType(modelType - MODEL_ITEM);
    }

    bool IsSocketSphere(const ITEM* pItem)
    {
        return IsSocketSphereType(pItem->Type);
    }

    bool IsSocketSphereModel(int modelType)
    {
        return IsSocketSphereType(modelType - MODEL_ITEM);
    }

    bool IsSocketSeedSphereType(int itemType)
    {
        return itemType >= ITEM_SEED_SPHERE_FIRE_1 && itemType <= ITEM_SEED_SPHERE_EARTH_5;
    }

    bool IsSocketSeedSphere(const ITEM* pItem)
    {
        return IsSocketSeedSphereType(pItem->Type);
    }

    bool IsSocketSeedSphereModel(int modelType)
    {
        return IsSocketSeedSphereType(modelType - MODEL_ITEM);
    }




    bool IsTradeBan(const ITEM* pItem)
    {
        if (pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || IsSecondClassQuestItem(pItem)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || pItem->Type == ITEM_CURSED_CASTLE_WATER
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            )
        {
            return true;
        }

        if (pItem->Type == ITEM_GM_GIFT)
        {
            if (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) ||
                (Hero->CtlCode == CTLCODE_20OPERATOR) || (Hero->CtlCode == CTLCODE_08OPERATOR))
                return false;
            else
                return true;
        }
        if (OldCheck_ItemAction(pItem, eITEM_TRADE))	return true;

        return false;
    }

    bool IsDropBan(const ITEM* pItem)
    {
        if ((!pItem->bPeriodItem) &&
            (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY
                || pItem->Type == ITEM_CHAOS_CARD
                || IsDemonOrSpiritOfGuardian(pItem)
                || IsPandaOrSkeletonItem(pItem)
                || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
                ))
        {
            return false;
        }

        if (pItem->Type == ITEM_GOLDEN_BOX || pItem->Type == ITEM_SILVER_BOX)
        {
            return false;
        }

        if (IsSecondClassQuestItem(pItem)
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsWingMixCharm(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            )
        {
            return true;
        }

        if (OldCheck_ItemAction(pItem, eITEM_DROP))	return true;

        return false;
    }

    bool IsStoreBan(const ITEM* pItem)
    {
        if (IsSecondClassQuestItem(pItem)
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || pItem->Type == ITEM_MASTER_SEAL_OF_ASCENSION
            || pItem->Type == ITEM_MASTER_SEAL_OF_WEALTH
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || IsThirdClassQuestItem(pItem)
            || (pItem->Type == ITEM_TALISMAN_OF_MOBILITY && pItem->Durability == 1)
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
            || pItem->bPeriodItem
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
            )
        {
            return true;
        }

        if (OldCheck_ItemAction(pItem, eITEM_STORE))	return true;

        return false;
    }




    bool IsPersonalShopBan(const ITEM* pItem)
    {
        if (pItem == NULL)
        {
            return false;
        }

        if ((!pItem->bPeriodItem) &&
            pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
            )
        {
            return false;
        }

        if (pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
            || (pItem->Type == ITEM_RENA && pItem->Level != 3)
            || IsSecondClassQuestItem(pItem)
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
            || IsSeal(pItem)
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
            || IsThirdClassQuestItem(pItem)
            || IsPartChargeItem(pItem)
            || IsCharacterCard(pItem)
            || pItem->Type == ITEM_HELPER + 99
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY)
            || IsGemJewelry(pItem)
            )
        {
            return true;
        }
        if (OldCheck_ItemAction(pItem, eITEM_PERSONALSHOP))	return true;

        return false;
    }

    bool IsSellingBan(const ITEM* pItem)
    {
        int Level = pItem->Level;

        if (IsSilverOrGoldKey(pItem)
            || pItem->Type == ITEM_SEALED_GOLDEN_BOX
            || pItem->Type == ITEM_SEALED_SILVER_BOX
            || pItem->Type == ITEM_GOLDEN_BOX
            || pItem->Type == ITEM_SILVER_BOX
            || IsSmallWing(pItem)
            || IsPandaOrSkeletonItem(pItem)
            || IsDemonOrSpiritOfGuardian(pItem)
            || IsGemJewelry(pItem)
            || ((pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_LETHAL_WIZARDS_RING)
            )
        {
            if (pItem->bPeriodItem && pItem->bExpiredPeriod)
            {
                return false;
            }
        }

        if (pItem->Type == ITEM_BOX_OF_LUCK
            || (pItem->Type == ITEM_RENA && Level == 1)
            || (pItem->bPeriodItem && !pItem->bExpiredPeriod && pItem->Type == ITEM_WIZARDS_RING && Level == 0)
            || (pItem->Type == ITEM_WIZARDS_RING && (Level == 1 || Level == 2))
            || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
            || (pItem->Type == ITEM_REMEDY_OF_LOVE && Level >= 1 && Level <= 5)
            || IsPartChargeItem(pItem)
            || IsWingMixCharm(pItem)
            || IsPandaOrSkeletonItem(pItem)
            || (pItem->Type == ITEM_PET_UNICORN)
            || (pItem->Type == ITEM_LETHAL_WIZARDS_RING)
            )
        {
            return true;
        }

        if (OldCheck_ItemAction(pItem, eITEM_SELL))	return true;

        return false;
    }

    bool IsRepairBan(const ITEM* pItem)
    {
        if (g_ChangeRingMgr->CheckRepair(pItem->Type))
        {
            return true;
        }
        if (IsPartChargeItem(pItem) || IsWingMixCharm(pItem))
        {
            return true;
        }

        if ((pItem->Type >= ITEM_GREEN_CHAOS_BOX && pItem->Type <= ITEM_PURPLE_CHAOS_BOX)
            || IsSeal(pItem)
            || (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HORN_OF_DINORANT)
            || IsAmmunition(pItem)
            || pItem->Type >= ITEM_POTION
            || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
            || (pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL)
            || pItem->Type == ITEM_RENA
            || IsDarkLordPet(pItem)
            || pItem->Type == ITEM_MOONSTONE_PENDANT
            || pItem->Type == ITEM_PET_RUDOLF
            || IsPandaOrSkeletonItem(pItem)
            || pItem->Type == ITEM_PET_UNICORN
            || pItem->Type == ITEM_CHERRY_BLOSSOM_PLAYBOX
            || pItem->Type == ITEM_CHERRY_BLOSSOM_WINE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_RICE_CAKE
            || pItem->Type == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL
            || pItem->Type == ITEM_WHITE_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_RED_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH
            || pItem->Type == ITEM_CONTRACT_SUMMON
            || pItem->Type == ITEM_TRANSFORMATION_RING
            || pItem->Type == ITEM_LIFE_STONE_ITEM
            || pItem->Type == ITEM_WIZARDS_RING
            || pItem->Type == ITEM_ARMOR_OF_GUARDSMAN
            || pItem->Type == ITEM_SPLINTER_OF_ARMOR
            || pItem->Type == ITEM_BLESS_OF_GUARDIAN
            || pItem->Type == ITEM_CLAW_OF_BEAST
            || pItem->Type == ITEM_FRAGMENT_OF_HORN
            || pItem->Type == ITEM_BROKEN_HORN
            || pItem->Type == ITEM_HORN_OF_FENRIR
            || pItem->Type == ITEM_OLD_SCROLL
            || pItem->Type == ITEM_ILLUSION_SORCERER_COVENANT
            || pItem->Type == ITEM_SCROLL_OF_BLOOD
            || pItem->Type == ITEM_INVITATION_TO_SANTA_VILLAGE
            || IsGambleItem(pItem)
            )
        {
            return true;
        }

        if (OldCheck_ItemAction(pItem, eITEM_REPAIR))	return true;

        return false;
    }

}

// Inventory filter of RepairAllGold: true when the item is skipped.
bool OldRepairAllSkips(const ITEM* pItem)
{

            if (pItem->Type >= ITEM_GREEN_CHAOS_BOX && pItem->Type <= ITEM_PURPLE_CHAOS_BOX)
            {
                return true;
            }
            //. item filtering
            if ((pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_DARK_RAVEN_ITEM) || pItem->Type == ITEM_TRANSFORMATION_RING || pItem->Type == ITEM_SPIRIT)
                return true;
            if (OldRules::IsAmmunition(pItem) || pItem->Type >= ITEM_POTION)
                return true;
            if (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
                return true;
            if ((pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL) || pItem->Type == ITEM_RENA)
                return true;
            if (pItem->Type == ITEM_WIZARDS_RING)
                return true;
            if (pItem->Type == ITEM_MOONSTONE_PENDANT)
                return true;

            if (OldRules::IsEventTicket(pItem))
            {
                return true;
            }
            if (OldRules::IsDoppelgangerOrVarkaTicket(pItem))
            {
                return true;
            }
            if (pItem->Type >= ITEM_RARE_ITEM_TICKET_7 && pItem->Type <= ITEM_RARE_ITEM_TICKET_12)
            {
                return true;
            }
            if (pItem->Type >= ITEM_RARE_ITEM_TICKET_1 && pItem->Type <= ITEM_RARE_ITEM_TICKET_5)
            {
                return true;
            }
            if (pItem->Type == ITEM_TALISMAN_OF_LUCK)
            {
                return true;
            }
            if (OldRules::IsSeal(pItem))
            {
                return true;
            }
            if (OldRules::IsElitePotion(pItem))
            {
                return true;
            }
            if (OldRules::IsBuffScroll(pItem))
            {
                return true;
            }
            if (pItem->Type == ITEM_SEAL_OF_MOBILITY)
            {
                return true;
            }
            if (OldRules::IsResetFruit(pItem))
            {
                return true;
            }
            if (pItem->Type == ITEM_INDULGENCE)
            {
                return true;
            }
            if (pItem->Type == ITEM_ILLUSION_TEMPLE_TICKET)
            {
                return true;
            }
            if (pItem->Type == ITEM_SUMMONER_CHARACTER_CARD)
            {
                return true;
            }
            if (pItem->Type >= ITEM_CHAOS_CARD_GOLD && pItem->Type <= ITEM_CHAOS_CARD_RARE)
            {
                return true;
            }
            if (pItem->Type == ITEM_CHAOS_CARD_MINI)
            {
                return true;
            }
            if (pItem->Type == ITEM_CHAOS_CARD_MINI)
            {
                return true;
            }
            if (OldRules::IsHealingOrDivinitySeal(pItem))
            {
                return true;
            }
            if (OldRules::IsBattleOrStrengthScroll(pItem))
            {
                return true;
            }
            if (pItem->Type == ITEM_SCROLL_OF_HEALING)
            {
                return true;
            }
            if (pItem->Type == ITEM_TALISMAN_OF_CHAOS_ASSEMBLY)
            {
                return true;
            }
            if (OldRules::IsDemonOrSpiritOfGuardian(pItem))
            {
                return true;
            }
            if (pItem->Type == ITEM_PET_RUDOLF)
            {
                return true;
            }
            if (pItem->Type == ITEM_PET_PANDA)
            {
                return true;
            }
            if (pItem->Type == ITEM_PET_UNICORN)
            {
                return true;
            }
            if (pItem->Type == ITEM_PET_SKELETON)
            {
                return true;
            }
            if (pItem->Type == ITEM_SNOWMAN_TRANSFORMATION_RING)
            {
                return true;
            }
            if (pItem->Type == ITEM_PANDA_TRANSFORMATION_RING)
            {
                return true;
            }
            if (pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING)
            {
                return true;
            }
            if (pItem->Type == ITEM_TALISMAN_OF_RESURRECTION)
                return true;
            if (pItem->Type == ITEM_TALISMAN_OF_MOBILITY)
                return true;

            if (pItem->Type == ITEM_HORN_OF_FENRIR)
                return true;

            if (pItem->Type == ITEM_INVITATION_TO_SANTA_VILLAGE)
                return true;

            if (OldRules::IsGambleItem(pItem))
                return true;

            if (pItem->Type == ITEM_TALISMAN_OF_GUARDIAN)
                return true;
            if (pItem->Type == ITEM_TALISMAN_OF_ITEM_PROTECTION)
                return true;
            if (pItem->Type == ITEM_MASTER_SEAL_OF_ASCENSION)
                return true;
            if (pItem->Type == ITEM_MASTER_SEAL_OF_WEALTH)
                return true;

            if (OldRules::IsWingMixCharm(pItem))
            {
                return true;
            }
            if (OldRules::IsCharacterCard(pItem))
                return true;

            if (pItem->Type == ITEM_OPEN_ACCESS_TICKET_TO_CHAOS_CASTLE)
                return true;

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
            if (g_pMyInventory->IsInvenItem(pItem->Type))
                return true;

#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

            if (pItem->Type >= ITEM_SMALL_CAPE_OF_LORD && pItem->Type <= ITEM_SMALL_WINGS_OF_SATAN)
                return true;
            if (pItem->Type == ITEM_SAPPHIRE_RING)
                return true;
            if (pItem->Type == ITEM_RUBY_RING)
                return true;
            if (pItem->Type == ITEM_TOPAZ_RING)
                return true;
            if (pItem->Type == ITEM_AMETHYST_RING)
                return true;
            if (pItem->Type == ITEM_RUBY_NECKLACE)
                return true;
            if (pItem->Type == ITEM_EMERALD_NECKLACE)
                return true;
            if (pItem->Type == ITEM_SAPPHIRE_NECKLACE)
                return true;
            if (pItem->Type == ITEM_LETHAL_WIZARDS_RING)
                return true;

            if (OldCheck_ItemAction(pItem, eITEM_REPAIR))	return true;
    return false;
}

// Early returns of RenderRepairInfo: true when no repair info is shown.
bool OldRepairInfoHidden(const ITEM* ip)
{
    if (OldRules::IsRepairBan(ip) == true)
    {
        return true;
    }
    if (OldRules::IsWingMixCharm(ip))
    {
        return true;
    }
    if (ip->Type == ITEM_LETHAL_WIZARDS_RING)
    {
        return true;
    }
    if (ip->Type == ITEM_MAX_AG_BOOST_AURA)
    {
        return true;
    }
    if (ip->Type == ITEM_MAX_SD_BOOST_AURA)
    {
        return true;
    }
    if (ip->Type == ITEM_PARTY_EXP_SCROLL)
    {
        return true;
    }
    if (ip->Type == ITEM_ELITE_SD_POTION)
    {
        return true;
    }
    if (ip->Type == MODEL_SAPPHIRE_RING)
    {
        return true;
    }
    if (ip->Type == MODEL_RUBY_RING)
    {
        return true;
    }
    if (ip->Type == MODEL_TOPAZ_RING)
    {
        return true;
    }
    if (ip->Type == MODEL_AMETHYST_RING)
    {
        return true;
    }
    if (ip->Type == MODEL_RUBY_NECKLACE)
    {
        return true;
    }
    if (ip->Type == MODEL_EMERALD_NECKLACE)
    {
        return true;
    }
    if (ip->Type == MODEL_SAPPHIRE_NECKLACE)
    {
        return true;
    }
    if (ip->Type == MODEL_SILVER_KEY)
    {
        return true;
    }
    if (ip->Type == MODEL_GOLD_KEY)
    {
        return true;
    }
    if (ip->Type == ITEM_GOBLIN_GOLD_COIN)
    {
        return true;
    }
    if (ip->Type == ITEM_SEALED_GOLDEN_BOX)
    {
        return true;
    }
    if (ip->Type == ITEM_SEALED_SILVER_BOX)
    {
        return true;
    }
    if (ITEM_GOLDEN_BOX == ip->Type)
    {
        return true;
    }
    if (ITEM_SILVER_BOX == ip->Type)
    {
        return true;
    }
    if (OldRules::IsPackageBox(ip))
    {
        return true;
    }

    if (OldRules::IsSmallWing(ip))
    {
        return true;
    }

    if (OldRules::IsAccountServiceItem(ip))
    {
        return true;
    }
    if (OldRules::IsDayPass(ip))
    {
        return true;
    }
    if (OldRules::IsHourPass(ip))
    {
        return true;
    }
    if (ITEM_OPEN_ACCESS_TICKET_TO_CHAOS_CASTLE == ip->Type)
    {
        return true;
    }

    return false;
}
