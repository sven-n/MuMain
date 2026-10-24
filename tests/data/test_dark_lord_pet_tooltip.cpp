#include "stdafx.h"

#include "doctest.h"

#include "Engine/Object/ZzzInventory.h"

#include <optional>

TEST_CASE("A Dark Lord pet's tooltip asks for the pet it is drawn for, also after another pet's [data][items][tooltip]")
{
    // The raven in the left hand, then the horse as the helper, then the raven again, from other storages.
    const std::optional<PetInfoRequest> raven = GetPetInfoRequest(ITEM_DARK_RAVEN_ITEM, EQUIPMENT_WEAPON_LEFT, 0);
    REQUIRE(raven.has_value());
    CHECK(raven->pet == PetType::DarkRaven);
    CHECK(raven->slot == EQUIPMENT_WEAPON_LEFT);
    CHECK(raven->storage == static_cast<StorageType>(0));

    const std::optional<PetInfoRequest> horse = GetPetInfoRequest(ITEM_DARK_HORSE_ITEM, EQUIPMENT_HELPER, 2);
    REQUIRE(horse.has_value());
    CHECK(horse->pet == PetType::DarkHorse);
    CHECK(horse->slot == EQUIPMENT_HELPER);
    CHECK(horse->storage == static_cast<StorageType>(2));

    const std::optional<PetInfoRequest> ravenAgain = GetPetInfoRequest(ITEM_DARK_RAVEN_ITEM, EQUIPMENT_WEAPON_LEFT, 1);
    REQUIRE(ravenAgain.has_value());
    CHECK(ravenAgain->pet == PetType::DarkRaven);
    CHECK(ravenAgain->storage == static_cast<StorageType>(1));

    // A pet pointed at anywhere else asks for nothing.
    CHECK_FALSE(GetPetInfoRequest(ITEM_DARK_HORSE_ITEM, EQUIPMENT_WEAPON_LEFT, 0).has_value());
    CHECK_FALSE(GetPetInfoRequest(ITEM_DARK_RAVEN_ITEM, EQUIPMENT_HELPER, 0).has_value());
    CHECK_FALSE(GetPetInfoRequest(ITEM_DARK_HORSE_ITEM, MAX_EQUIPMENT + 3, 0).has_value());
}
