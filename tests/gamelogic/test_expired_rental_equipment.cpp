#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Items/ItemCategories.h"

namespace
{
constexpr int AnyItemType = 100;

void EmptyEquipment(CHARACTER_MACHINE& machine)
{
    for (ITEM& item : machine.Equipment)
    {
        item.Type = -1;
    }
}

void Equip(CHARACTER_MACHINE& machine, int slot, WORD walkSpeed)
{
    ITEM& item = machine.Equipment[slot];
    item.Type = AnyItemType;
    item.Durability = 50;
    item.WalkSpeed = walkSpeed;
}

void Rent(ITEM& item, bool expired)
{
    item.bPeriodItem = true;
    item.bExpiredPeriod = expired;
}
} // namespace

TEST_CASE("A rented item is an expired rental only once its time ran out [gamelogic][items][rental]")
{
    ITEM item{};
    item.Type = AnyItemType;
    CHECK_FALSE(GameLogic::Items::IsExpiredRental(&item));

    Rent(item, false);
    CHECK_FALSE(GameLogic::Items::IsExpiredRental(&item));

    Rent(item, true);
    CHECK(GameLogic::Items::IsExpiredRental(&item));
    CHECK_FALSE(GameLogic::Items::IsExpiredRental(nullptr));
}

TEST_CASE("An expired rented item counts as an empty equipment slot [gamelogic][items][rental]")
{
    CHARACTER_MACHINE machine{};
    EmptyEquipment(machine);
    Equip(machine, EQUIPMENT_WING, 15);

    CHECK(machine.GetEffectiveEquipment(EQUIPMENT_WING) == &machine.Equipment[EQUIPMENT_WING]);

    Rent(machine.Equipment[EQUIPMENT_WING], false);
    CHECK(machine.GetEffectiveEquipment(EQUIPMENT_WING) == &machine.Equipment[EQUIPMENT_WING]);

    Rent(machine.Equipment[EQUIPMENT_WING], true);
    const ITEM* effective = machine.GetEffectiveEquipment(EQUIPMENT_WING);
    CHECK(effective->Type == -1);
    CHECK(effective->Durability == 0);
    // The item itself stays where it is, for the inventory.
    CHECK(machine.Equipment[EQUIPMENT_WING].Type == AnyItemType);

    // An empty slot gives an empty item, never nullptr.
    CHECK(machine.GetEffectiveEquipment(EQUIPMENT_HELM)->Type == -1);
}

TEST_CASE("The stats leave out an expired rented item [gamelogic][items][rental]")
{
    CHARACTER_MACHINE machine{};
    EmptyEquipment(machine);
    Equip(machine, EQUIPMENT_BOOTS, 10);
    Equip(machine, EQUIPMENT_WING, 15);

    machine.CalculateWalkSpeed();
    CHECK(machine.Character.WalkSpeed == 25);

    Rent(machine.Equipment[EQUIPMENT_WING], true);
    machine.CalculateWalkSpeed();
    CHECK(machine.Character.WalkSpeed == 10);
}
