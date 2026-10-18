// One-time copy of the weapon checks before phase 3b-3 (see test_item_lists_equivalence.cpp).

// ZzzInfomation.cpp, IsCepterItem.
// ITEM_MACE + 21: Master Scepter-J.
bool OldIsCepterItem(int iType)
{
    if ((iType >= ITEM_BATTLE_SCEPTER && iType <= ITEM_SHINING_SCEPTER) || iType == ITEM_ABSOLUTE_SCEPTER || iType == ITEM_STRYKER_SCEPTER || iType == ITEM_MACE + 21)
    {
        return true;
    }
    return false;
}

// ZzzInfomation.cpp, CHARACTER_MACHINE::CalculateDamage: a staff in the right hand adds its damage to the left pair.
bool OldRightStaffAddsDamageLeft(const ITEM* Right)
{
    return Right->Type >= ITEM_STAFF && Right->Type < ITEM_STAFF + MAX_ITEM_INDEX;
}

// ZzzInfomation.cpp, CHARACTER_MACHINE::CalculateMagicDamage: a magic sword in the right hand.
bool OldIsRightMagicSword(const ITEM* Right)
{
    return Right->Type == ITEM_DARK_REIGN_BLADE
            || Right->Type == ITEM_RUNE_BLADE
            || Right->Type == ITEM_EXPLOSION_BLADE
            || Right->Type == ITEM_SWORD_DANCER
            || Right->Type == ITEM_IMPERIAL_SWORD
            ;
}

// ZzzInfomation.cpp, CHARACTER_MACHINE::CalculateMagicDamage: a weapon in the left hand adds its magic options.
bool OldLeftWeaponAddsMagic(const ITEM* Left)
{
    return Left->Type >= ITEM_SWORD && Left->Type < ITEM_SHIELD;
}

// ZzzInventory.cpp, GetAttackDamage: a staff in the right hand shows the left damage pair.
bool OldRightStaffShowsLeftDamage(const ITEM* r)
{
    return r->Type >= ITEM_STAFF && r->Type < ITEM_SHIELD;
}

// ZzzInventory.cpp, GetAttackDamage: one hand's half of the Dark Knight's and Magic Gladiator's dual wield.
bool OldDualWieldHand(const ITEM* hand)
{
    return hand->Type >= ITEM_SWORD && hand->Type < ITEM_STAFF + MAX_ITEM_INDEX;
}

// ZzzInventory.cpp, GetAttackDamage: one hand's half of the Rage Fighter's dual wield.
bool OldRageFighterDualWieldHand(const ITEM* hand)
{
    return hand->Type >= ITEM_SWORD && hand->Type < ITEM_MACE + MAX_ITEM_INDEX;
}

// ZzzInterface.cpp, MoveHero: a weapon in the left hand moves to the empty right hand.
bool OldMoveHeroMovesLeftWeapon(int LeftType)
{
    return ((LeftType >= ITEM_SWORD && LeftType < ITEM_MACE + MAX_ITEM_INDEX)
                    || (LeftType >= ITEM_STAFF && LeftType < ITEM_STAFF + MAX_ITEM_INDEX
                        && !GameLogic::Items::IsSummonerBookType(LeftType)
                        ));
}

// SkillCast.cpp, SkillWarrior: Death Stab needs a weapon that is not a staff in the right hand.
bool OldDeathStabRefused(int weapon)
{
    return !(weapon != -1 && (weapon < MODEL_STAFF || weapon >= MODEL_STAFF + MAX_ITEM_INDEX));
}

// SkillCast.cpp, SkillWarrior: Spiral Slash needs a sword in the right hand.
bool OldSpiralSlashRefused(int weapon)
{
    return weapon < MODEL_SWORD || weapon >= MODEL_SWORD + MAX_ITEM_INDEX;
}

// SkillCast.cpp, UseSkillWizard: Death Cannon needs a staff in the right hand.
bool OldDeathCannonRefused(int weapon)
{
    return weapon < MODEL_STAFF || weapon >= MODEL_STAFF + MAX_ITEM_INDEX;
}

// SkillList.cpp, HasOneHandedOrMeleeWeapon and IsHudSkillUsable; ClassAttack.cpp, AttackKnight:
// a weapon in the right hand and no staff in either hand.
bool OldWeaponWithoutStaff(int iTypeL, int iTypeR)
{
    return iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
           (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX);
}

// MyInventory.cpp, IsEquipable: a summoner may put a right-hand item that is not a staff in the left hand.
bool OldSummonerTakesInLeftHand(const ITEM* pItem)
{
    return !(pItem->Type >= ITEM_STAFF && pItem->Type < ITEM_STAFF + MAX_ITEM_INDEX);
}

// CharacterInfoWindow.cpp, BuildAttributeLines: the wizardry damage line of a staff or magic sword.
bool OldWizardryWeaponRight(const ITEM* pWeaponRight)
{
    return (pWeaponRight->Type >= ITEM_STAFF && pWeaponRight->Type < (ITEM_STAFF + MAX_ITEM_INDEX)) ||
            pWeaponRight->Type == ITEM_RUNE_BLADE || pWeaponRight->Type == ITEM_EXPLOSION_BLADE ||
            pWeaponRight->Type == ITEM_SWORD_DANCER || pWeaponRight->Type == ITEM_DARK_REIGN_BLADE ||
            pWeaponRight->Type == ITEM_IMPERIAL_SWORD;
}
