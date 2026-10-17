// One-time copy of the weapon lists before phase 3b-2 (see test_item_lists_equivalence.cpp).

// CharacterManager.cpp
template <typename T, typename U, typename V>
constexpr bool OldInRange(const T value, const U minValue, const V maxValue)
{
    using Common = std::common_type_t<T, U, V>;
    const Common commonValue = static_cast<Common>(value);
    return (commonValue >= static_cast<Common>(minValue)) && (commonValue <= static_cast<Common>(maxValue));
}

// CharacterManager.cpp, GetEquipedBowType(CHARACTER*): the bow and crossbow models.
// MODEL_BOW + 27 and ITEM_BOW + 27: Silver Bow-J.
constexpr bool OldIsBowModel(int type)
{
    return OldInRange(type, MODEL_BOW, MODEL_CHAOS_NATURE_BOW) ||
           type == MODEL_CELESTIAL_BOW ||
           OldInRange(type, MODEL_ARROW_VIPER_BOW, MODEL_STINGER_BOW) ||
           type == MODEL_AIR_LYN_BOW ||
           type == MODEL_BOW + 27;
}

constexpr bool OldIsCrossbowModel(int type)
{
    return OldInRange(type, MODEL_CROSSBOW, MODEL_AQUAGOLD_CROSSBOW) ||
           type == MODEL_SAINT_CROSSBOW ||
           OldInRange(type, MODEL_DIVINE_CB_OF_ARCHANGEL, MODEL_GREAT_REIGN_CROSSBOW);
}

// CharacterManager.cpp, the other GetEquipedBowType functions: the bow and crossbow items.
constexpr bool OldIsGeneralBowItem(int type)
{
    return OldInRange(type, ITEM_BOW, ITEM_CHAOS_NATURE_BOW) ||
           type == ITEM_CELESTIAL_BOW ||
           OldInRange(type, ITEM_ARROW_VIPER_BOW, ITEM_STINGER_BOW) ||
           type == ITEM_AIR_LYN_BOW ||
           type == ITEM_BOW + 27;
}

constexpr bool OldIsGeneralCrossbowItem(int type)
{
    return OldInRange(type, ITEM_CROSSBOW, ITEM_AQUAGOLD_CROSSBOW) ||
           type == ITEM_SAINT_CROSSBOW ||
           OldInRange(type, ITEM_DIVINE_CB_OF_ARCHANGEL, ITEM_GREAT_REIGN_CROSSBOW);
}

// MyInventory.cpp, IsEquipable: an elf with this item in the left hand may only hold arrows in the right.
bool OldElfLeftHandTakesOnlyArrows(int leftType)
{
    return leftType != ITEM_BOLT
        && (leftType >= ITEM_BOW && leftType < ITEM_BOW + MAX_ITEM_INDEX);
}

// ZzzInfomation.cpp, CalculateDamage; ZzzInventory.cpp, GetAttackDamage; ZzzCharacter.cpp,
// MoveCharacter (the hit sound): one hand's half of their check of both hands.
bool OldIsBowGroupItem(int type)
{
    return type >= ITEM_BOW && type < ITEM_BOW + MAX_ITEM_INDEX;
}

// ZzzInfomation.cpp, CHARACTER_MACHINE::CalculateDefense: the shield defense set option.
bool OldIsShieldInLeftHand(int leftType)
{
    return leftType >= ITEM_SHIELD && leftType < ITEM_SHIELD + MAX_ITEM_INDEX;
}

// ZzzInterface.cpp, Action, and Attack.cpp, EquippedAttackRange: a spear's reach.
bool OldHasSpearReach(int rightType)
{
    return rightType >= ITEM_SPEAR && rightType < ITEM_SPEAR + MAX_ITEM_INDEX;
}

// SkillList.cpp, IsHudSkillUsable: Impale while riding needs a spear in either hand.
bool OldImpaleUnusableWhileRiding(int leftType, int rightType)
{
    return (leftType < ITEM_SPEAR || leftType >= ITEM_BOW) && (rightType < ITEM_SPEAR || rightType >= ITEM_BOW);
}

// SkillCast.cpp, SkillWarrior: Impale needs a spear model in the right hand.
bool OldImpaleHasSpearModel(int rightModel)
{
    return rightModel != -1 &&
           (rightModel >= MODEL_SPEAR && rightModel < MODEL_SPEAR + MAX_ITEM_INDEX);
}
