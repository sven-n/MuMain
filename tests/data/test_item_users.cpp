#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ItemModelDatabaseScope.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/ItemTools/ItemUsers.h"

#include <vector>

using namespace Data::Items;
using namespace MuEditor::Items;

namespace
{
ItemModelDefinition MakeModel(int number)
{
    ItemModelDefinition model;
    model.number = number;
    model.file = "Data/Item/Sword01.bmd";
    return model;
}

ItemDefinition MakeItem(int number, const char* profile)
{
    ItemDefinition item;
    item.number = number;
    item.names = Data::LocalizedString::Parse("Item");
    item.statProfile = profile;
    return item;
}

// Fills the global item database for a test and empties it again.
class ItemsScope
{
public:
    explicit ItemsScope(const std::vector<ItemDefinition>& items)
    {
        g_ItemDatabase.Build(items);
    }
    ~ItemsScope()
    {
        g_ItemDatabase.Build({});
    }

    ItemsScope(const ItemsScope&) = delete;
    ItemsScope& operator=(const ItemsScope&) = delete;
};
} // namespace

TEST_CASE("The item tools find the items that use a shared model, a look or a glow color [editor][items]")
{
    ItemModelDatabaseScope restoreModels;
    std::vector<ItemModelDefinition> models = {MakeModel(0), MakeModel(1), MakeModel(2)};
    models[0].model = "parchment";
    models[1].model = "parchment";
    models[0].renderStyle = "chrome";
    models[2].renderStyle = "chrome";
    models[1].itemEffect = "sparkle";
    models[2].glow.shineColor = "red";
    g_ItemModelDatabase.Build(models, Data::Effects::GlowColorList{});

    CHECK(FindSharedModelUsers("parchment") == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 1)});
    CHECK(FindRenderStyleUsers("chrome") == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 2)});
    CHECK(FindItemEffectUsers("sparkle") == std::vector<int>{MakeItemType(0, 1)});
    CHECK(FindGlowColorUsers("red") == std::vector<int>{MakeItemType(0, 2)});
    // The default glow color counts where an item leaves it out.
    CHECK(FindGlowColorUsers(ItemGlow::DefaultColor).size() == 3);
    CHECK(FindSharedModelUsers("noSuchModel").empty());
}

TEST_CASE("The item tools find the items of a stat profile, the default by either name [editor][items]")
{
    ItemsScope items({MakeItem(0, ""), MakeItem(1, "default"), MakeItem(2, "weapon")});
    CHECK(FindStatProfileUsers("weapon") == std::vector<int>{MakeItemType(0, 2)});
    CHECK(FindStatProfileUsers("") == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 1)});
    CHECK(FindStatProfileUsers("default") == FindStatProfileUsers(""));
}

TEST_CASE("The stat profile counts are those of the users lists and follow changes [editor][items]")
{
    ItemsScope items({MakeItem(0, ""), MakeItem(1, "default"), MakeItem(2, "weapon")});
    ItemStatProfileUseCounts counts;
    CHECK(counts.Items("weapon") == 1);
    CHECK(counts.Items("") == 2);
    CHECK(counts.Items("default") == 2);
    CHECK(counts.Items("armor") == 0);
    CHECK(counts.Rules("weapon") == 0);

    g_ItemDatabase.Set(MakeItem(3, "weapon"));
    CHECK(counts.Items("weapon") == 2);
    CHECK(static_cast<size_t>(counts.Items("weapon")) == FindStatProfileUsers("weapon").size());
}

#endif // _EDITOR
