#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"

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

#endif // _EDITOR
