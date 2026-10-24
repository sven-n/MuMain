#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Pets/CompanionCleanup.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "Network/Server/TransformViewportEntry.h"
#include "World/MapInfra/MapManager.h"

#include <filesystem>

namespace
{
struct AppearanceFixture
{
    std::filesystem::path previousDirectory = std::filesystem::current_path();
    int previousWorld = gMapManager.WorldActive;
    CHARACTER character;
    CHARACTER observer;
    PCREATE_TRANSFORM_EXTENDED entry{};

    AppearanceFixture()
    {
        std::filesystem::current_path(MU_TEST_ASSET_DIR);
        gMapManager.WorldActive = WD_0LORENCIA;
        g_petProcess = PetProcess::Make();
        character.Object.Type = MODEL_PLAYER;
        character.Object.Kind = KIND_PLAYER;
        character.Class = CLASS_KNIGHT;
        character.SkinIndex = static_cast<CLASS_SKIN_INDEX>(0);
        character.Change = true;
        std::fill(std::begin(entry.Equipment), std::end(entry.Equipment), BYTE{0xFF});
        entry.Flags = 0x10;
        // Two three-byte weapons, five three-byte body parts, wings, helper.
        entry.Equipment[0] = 0x00;
        entry.Equipment[1] = 1;
        entry.Equipment[2] = 0x98;
        entry.Equipment[3] = 0x60;
        entry.Equipment[4] = 1;
        entry.Equipment[5] = 0;
        entry.Equipment[21] = 0xC0;
        entry.Equipment[22] = 1;
        entry.Equipment[23] = 0xD0;
        entry.Equipment[24] = 123;
    }

    ~AppearanceFixture()
    {
        g_petProcess.reset();
        for (auto& mount : std::span(Mounts, MAX_MOUNTS))
            mount.Live = false;
        std::filesystem::current_path(previousDirectory);
        gMapManager.WorldActive = previousWorld;
    }
};

PetObjectPtr AddPet(PetProcess::PetList& pets, CHARACTER& owner, int itemType)
{
    auto pet = PetObject::Make();
    pet->Create(itemType, MODEL_PET_SKELETON, owner.Object.Position, &owner, 0, 0);
    pets.push_back(pet);
    return pet;
}
} // namespace

TEST_CASE("Transformed extended equipment preserves body and decodes real equipment [transform_appearance]")
{
    AppearanceFixture fixture;
    auto& c = fixture.character;
    c.BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + 20;
    for (const bool wornGloves : {false, true})
    {
        CAPTURE(wornGloves);
        fixture.entry.Equipment[15] = wornGloves ? 0xA0 : 0xFF;
        fixture.entry.Equipment[16] = wornGloves ? 1 : 0xFF;
        for (int cycle = 0; cycle < 5; ++cycle)
        {
            Network::Viewport::ApplyTransformEquipment(&c, fixture.entry);
            CHECK(c.Helper.Type == MODEL_PET_SKELETON);
            CHECK(c.Weapon[0].Type == MODEL_SWORD + 1);
            CHECK(c.Weapon[1].Type == MODEL_SHIELD + 1);
            CHECK(c.Wing.Type == MODEL_WING + 1);
            CHECK(c.ExtendState);
            CHECK(c.BodyPart[BODYPART_GLOVES].Type == MODEL_BODY_GLOVES + 20);
            CHECK(c.Change);
        }
    }
}

TEST_CASE("Normal equipment and owner mounts survive independent appearance changes [transform_appearance]")
{
    AppearanceFixture fixture;
    auto& c = fixture.character;
    c.Change = false;
    fixture.entry.Equipment[15] = 0xA0;
    fixture.entry.Equipment[16] = 1;
    fixture.entry.Equipment[17] = 0;
    Mounts[0].Live = true;
    Mounts[0].Owner = &c.Object;
    Mounts[1].Live = true;
    Mounts[1].Owner = &fixture.observer.Object;

    ReadEquipmentExtended(0, fixture.entry.Flags, fixture.entry.Equipment, &c);
    CHECK(c.BodyPart[BODYPART_GLOVES].Type == MODEL_GLOVES + 1);
    CHECK(c.Helper.Type == MODEL_PET_SKELETON);
    CHECK_FALSE(Mounts[0].Live);
    CHECK(Mounts[1].Live);

    fixture.entry.Equipment[23] = 0xFF;
    fixture.entry.Equipment[24] = 0xFF;
    c.Change = true;
    Network::Viewport::ApplyTransformEquipment(&c, fixture.entry);
    CHECK(c.Helper.Type == -1);
    CHECK(Mounts[1].Live);
}

TEST_CASE(
    "Extended equipment keeps a transformed body out of normal Chaos Castle appearance rules [transform_appearance]")
{
    AppearanceFixture fixture;
    auto& c = fixture.character;
    c.BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + 20;
    gMapManager.WorldActive = WD_18CHAOS_CASTLE;
    Network::Viewport::ApplyTransformEquipment(&c, fixture.entry);
    CHECK(c.BodyPart[BODYPART_GLOVES].Type == MODEL_BODY_GLOVES + 20);
    CHECK(c.Change);
}

TEST_CASE("Wildcard all-delete releases every matching companion and preserves observers [transform_appearance]")
{
    CHARACTER owner;
    CHARACTER observer;
    PetProcess::PetList pets;
    const auto first = AddPet(pets, owner, 123);
    const auto other = AddPet(pets, observer, 123);
    const auto second = AddPet(pets, owner, 80);
    GameLogic::Pets::RemovePetObjects(pets, &owner, -1, true);
    CHECK(first->GetObject() == nullptr);
    CHECK(second->GetObject() == nullptr);
    REQUIRE(pets.size() == 1);
    CHECK(pets.front() == other);
    CHECK(other->GetObject() != nullptr);
}

TEST_CASE("Recreating a character clears its old mount before resetting helper identity [transform_appearance]")
{
    AppearanceFixture fixture;
    auto& c = fixture.character;
    c.Helper.Type = MODEL_HORN_OF_UNIRIA;
    Mounts[0].Live = true;
    Mounts[0].Owner = &c.Object;
    Mounts[1].Live = true;
    Mounts[1].Owner = &fixture.observer.Object;
    CreateCharacterPointer(&c, MODEL_PLAYER, 0, 0);
    CHECK(c.Helper.Type == -1);
    CHECK_FALSE(Mounts[0].Live);
    CHECK(Mounts[1].Live);
}

TEST_CASE("Single and typed companion deletion keep their original scope [transform_appearance]")
{
    CHARACTER owner;
    PetProcess::PetList pets;
    const auto first = AddPet(pets, owner, 123);
    const auto second = AddPet(pets, owner, 123);
    const auto third = AddPet(pets, owner, 80);
    GameLogic::Pets::RemovePetObjects(pets, &owner, -1, false);
    CHECK(first->GetObject() == nullptr);
    CHECK(pets.size() == 2);
    GameLogic::Pets::RemovePetObjects(pets, &owner, 123, true);
    CHECK(second->GetObject() == nullptr);
    CHECK(pets.size() == 1);
    CHECK(third->GetObject() != nullptr);
    GameLogic::Pets::RemovePetObjects(pets, nullptr, -1, true);
    CHECK(pets.size() == 1);
}
