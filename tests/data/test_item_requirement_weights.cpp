#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"
#include "LevelBasisScrollProfile.h"

#include "UI/ItemTools/ItemRequirementTable.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"

#include <filesystem>
#include <string>

using namespace MuEditor::Items;
using namespace Data::Items;

// What Item Requirements and Item Basics compute from the shipped item data.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int KrisType = MakeItemType(0, 0);
constexpr int SmallAxeType = MakeItemType(1, 0);
constexpr int IncreaseBlockParchmentType = MakeItemType(15, 36);
constexpr int JewelOfBlessType = MakeItemType(14, 13);
constexpr int HornOfFenrirType = MakeItemType(13, 37);

class ShippedItems
{
public:
    ShippedItems() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

private:
    ClientDataScope m_client;
};

const ItemDefinition& Shipped(int itemType)
{
    const ItemDefinition* item = g_ItemDatabase.Find(itemType);
    REQUIRE(item != nullptr);
    return *item;
}
// Whether `weight` gives `value` at +0, by the requirement table.
bool Gives(ItemDefinition item, ItemRequirement requirement, int weight, int value)
{
    WORD* const bases[] = {&item.requirements.level,  &item.requirements.strength, &item.requirements.dexterity,
                           &item.requirements.energy, &item.requirements.vitality, &item.requirements.leadership};
    *bases[static_cast<size_t>(requirement)] = static_cast<WORD>(weight);
    return ComputeRequirementTable(item, {})[0][static_cast<size_t>(requirement)] == value;
}
} // namespace

TEST_CASE("The item tools name the stat profile storing an item gives it [data][items][editor]")
{
    ShippedItems items;
    using Reason = StatProfileChoice::Reason;

    const StatProfileChoice kris = FindStatProfileChoice(Shipped(KrisType));
    CHECK(kris.name == "weapon");
    CHECK(kris.reason == Reason::ByRules);
    CHECK(g_ItemDatabase.GetStatProfiles().GetRuleProfile(kris.rule) == "weapon");

    const StatProfileChoice horn = FindStatProfileChoice(Shipped(HornOfFenrirType));
    CHECK(horn.name == "default");
    CHECK(horn.reason == Reason::Picked);
    CHECK(horn.given == "helper");

    const StatProfileChoice jewel = FindStatProfileChoice(Shipped(JewelOfBlessType));
    CHECK(jewel.name == "default");
    CHECK(jewel.reason == Reason::ByRules);
    CHECK(jewel.rule == NoStatProfileRule);

    // A draft that follows the rules, and one with a pick.
    ItemDefinition draft = Shipped(KrisType);
    draft.statProfile = "";
    CHECK(FindStatProfileChoice(draft).name == "weapon");
    CHECK(FindStatProfileChoice(draft).reason == Reason::ByRules);
    draft.statProfile = "armor";
    CHECK(FindStatProfileChoice(draft).name == "armor");
    CHECK(FindStatProfileChoice(draft).reason == Reason::Picked);
    CHECK(FindStatProfileChoice(draft).given == "weapon");
    CHECK(&FindStatProfile(draft) ==
          &g_ItemDatabase.GetStatProfiles().Get(g_ItemDatabase.GetStatProfiles().Find("armor")));
}

TEST_CASE("Item Requirements finds the weights that give a +0 requirement [data][items][editor]")
{
    ShippedItems items;
    const ItemDefinition& kris = Shipped(KrisType);
    CHECK(HasWeight(kris, ItemRequirement::Strength));
    CHECK_FALSE(HasWeight(kris, ItemRequirement::Level));
    CHECK_FALSE(HasWeight(kris, ItemRequirement::Leadership));
    CHECK(ComputeZeroValue(kris, ItemRequirement::Strength) == 27);

    const WeightRange weights = FindWeightsGiving(kris, ItemRequirement::Strength, 27);
    CHECK(weights == WeightRange{39, 44});
    for (int weight = 38; weight <= 45; ++weight)
    {
        CAPTURE(weight);
        CHECK(weights.Contains(weight) == Gives(kris, ItemRequirement::Strength, weight, 27));
    }
    CHECK(FindWeightsGiving(kris, ItemRequirement::Strength, 0) == WeightRange{0, 0});
    CHECK(FindWeightsGiving(kris, ItemRequirement::Strength, 20) == WeightRange{1, 5});
    CHECK(FindWeightsGiving(kris, ItemRequirement::Strength, 28) == WeightRange{45, 49});

    const ItemDefinition& smallAxe = Shipped(SmallAxeType);
    CHECK(ComputeZeroValue(smallAxe, ItemRequirement::Strength) == 21);
    CHECK(FindWeightsGiving(smallAxe, ItemRequirement::Strength, 21) == WeightRange{34, 66});
    CHECK(FindWeightsGiving(smallAxe, ItemRequirement::Strength, 20) == WeightRange{1, 33});

    // What each weight gives at +15 and for excellent items.
    CHECK(ComputeWeightValues(kris, ItemRequirement::Strength, 40) == WeightValues{40, 81, 57, 111});
    for (int weight = 39; weight <= 44; ++weight)
    {
        CAPTURE(weight);
        ItemDefinition withWeight = kris;
        withWeight.requirements.strength = static_cast<WORD>(weight);
        const RequirementTable normal = ComputeRequirementTable(withWeight, {});
        const RequirementTable excellent = ComputeRequirementTable(withWeight, {true, false, 0});
        const size_t strength = static_cast<size_t>(ItemRequirement::Strength);
        CHECK(ComputeWeightValues(kris, ItemRequirement::Strength, weight) ==
              WeightValues{weight, normal[15][strength], excellent[0][strength], excellent[15][strength]});
    }

    // At drop level 0 every weight gives the formula's add.
    ItemDefinition dropLevelZero = kris;
    dropLevelZero.level = 0;
    CHECK(FindWeightsGiving(dropLevelZero, ItemRequirement::Strength, 20) == WeightRange{1, 65535});
}

TEST_CASE("Typing a +0 requirement takes its weight, lists several or offers the nearest values "
          "[data][items][editor]")
{
    ShippedItems items;
    using Kind = WeightChoice::Kind;
    const ItemDefinition& kris = Shipped(KrisType);

    CHECK(ChooseWeight(kris, ItemRequirement::Strength, 27, 40).kind == Kind::Unchanged);
    const WeightChoice pick = ChooseWeight(kris, ItemRequirement::Strength, 28, 40);
    CHECK(pick.kind == Kind::Pick);
    CHECK(pick.weights == WeightRange{45, 49});
    CHECK(pick.weight == 45);

    // A scroll keeps its energy as stored: it has no weight.
    CHECK_FALSE(HasWeight(Shipped(IncreaseBlockParchmentType), ItemRequirement::Energy));
    CHECK(ComputeZeroValue(Shipped(IncreaseBlockParchmentType), ItemRequirement::Energy) == 80);

    // The "levelRequirement" basis, which no shipped profile uses any more: 20 + weight * 50 * 4
    // / 100 for the parchment's level requirement of 50.
    const LevelBasisScrollProfile levelBasis(ClientDirectory);
    ItemDefinition parchment = Shipped(IncreaseBlockParchmentType);
    parchment.requirements.energy = 30;
    REQUIRE(HasWeight(parchment, ItemRequirement::Energy));
    CHECK(ComputeZeroValue(parchment, ItemRequirement::Energy) == 80);
    const WeightChoice take = ChooseWeight(parchment, ItemRequirement::Energy, 82, 30);
    CHECK(take.kind == Kind::Take);
    CHECK(take.weight == 31);
    const WeightChoice odd = ChooseWeight(parchment, ItemRequirement::Energy, 81, 30);
    CHECK(odd.kind == Kind::Unreachable);
    CHECK(odd.weight == 30);
    REQUIRE(odd.nearest.below);
    REQUIRE(odd.nearest.above);
    CHECK(*odd.nearest.below == ReachableValue{80, {30, 30}});
    CHECK(*odd.nearest.above == ReachableValue{82, {31, 31}});

    // Below the formula's add, only no requirement (weight 0) is lower.
    const NearestZeroValues low = FindNearestZeroValues(kris, ItemRequirement::Strength, 10);
    REQUIRE(low.below);
    REQUIRE(low.above);
    CHECK(*low.below == ReachableValue{0, {0, 0}});
    CHECK(*low.above == ReachableValue{20, {1, 5}});
    // Above what the highest weight gives.
    const NearestZeroValues high = FindNearestZeroValues(kris, ItemRequirement::Strength, 20000);
    REQUIRE(high.below);
    CHECK_FALSE(high.above);
    CHECK(*high.below == ReachableValue{11816, {65534, 65535}});
    CHECK(ChooseWeight(kris, ItemRequirement::Strength, 582, 40).kind == Kind::Pick);
}

#endif // _EDITOR
