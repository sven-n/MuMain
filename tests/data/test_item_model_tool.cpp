#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemLevelModels.h"
#include "UI/ItemTools/ItemModelFiles.h"
#include "UI/ItemTools/ItemNumberList.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace MuEditor::Items;

namespace
{
// A client folder with a few model files and one that is not a model.
class TemporaryModelFolder
{
public:
    TemporaryModelFolder()
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_item_model_tool_" + std::to_string(std::random_device{}())))
    {
        for (const char* file : {"Data/Item/Sword01.bmd", "Data/Item/partCharge1/RING01.BMD", "Data/Player/Armor01.bmd",
                                 "Data/Item/readme.txt"})
        {
            const std::filesystem::path path = m_directory / file;
            std::filesystem::create_directories(path.parent_path());
            std::ofstream(path) << "model";
        }
    }

    ~TemporaryModelFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    const std::filesystem::path& Directory() const
    {
        return m_directory;
    }

private:
    std::filesystem::path m_directory;
};
} // namespace

TEST_CASE("The model tool lists the model files of Data/Item and Data/Player [editor][items]")
{
    TemporaryModelFolder folder;
    ItemModelFiles files(folder.Directory());
    CHECK(files.GetFiles() == std::vector<std::string>{"Data/Item/Sword01.bmd", "Data/Item/partCharge1/RING01.BMD",
                                                       "Data/Player/Armor01.bmd"});
    const std::vector<std::string_view> rings = files.Find("ring");
    REQUIRE(rings.size() == 1);
    CHECK(rings.front() == "Data/Item/partCharge1/RING01.BMD");
    CHECK(files.Find("").size() == 3);
}

TEST_CASE("A new model takes the folder of its file as its texture folder [editor][items]")
{
    CHECK(ItemModelFiles::GetTextureFolder("Data/Item/Sword01.bmd") == "Item");
    CHECK(ItemModelFiles::GetTextureFolder("Data/Item/partCharge1/Ring01.bmd") == "Item/partCharge1");
    CHECK(ItemModelFiles::GetTextureFolder("Sword01.bmd").empty());
}

TEST_CASE("A model that takes another file follows its folder unless the folders were chosen [editor][items]")
{
    using Folders = std::vector<std::string>;
    CHECK(ItemModelFiles::GetTextureFoldersForFile({}, "", "Data/Item/Axe01.bmd") == Folders{"Item"});
    CHECK(ItemModelFiles::GetTextureFoldersForFile({"Player"}, "Data/Player/HelmMale01.bmd", "Data/Item/Axe01.bmd") ==
          Folders{"Item"});
    CHECK(ItemModelFiles::GetTextureFoldersForFile({"Item", "Effect"}, "Data/Item/Axe01.bmd",
                                                   "Data/Player/HelmMale01.bmd") == Folders{"Item", "Effect"});
    CHECK(ItemModelFiles::GetTextureFoldersForFile({"Effect"}, "Data/Item/Axe01.bmd", "Data/Player/HelmMale01.bmd") ==
          Folders{"Effect"});
}

TEST_CASE("The model tool lists the models the game draws an item with by level [editor][items]")
{
    using MuEditor::Items::FindLevelModelRuns;
    using MuEditor::Items::IsDrawnByCodeAtSomeLevel;
    // The Wizard's Ring has no model entry: two event models, then nothing.
    const auto ring = FindLevelModelRuns(ITEM_WIZARDS_RING);
    REQUIRE(ring.size() == 3);
    CHECK(ring[0].lastLevel == 0);
    CHECK(ring[0].modelType == MODEL_EVENT + 15);
    CHECK(ring[1].firstLevel == 1);
    CHECK(ring[1].lastLevel == 3);
    CHECK(ring[1].modelType == MODEL_EVENT + 14);
    CHECK_FALSE(ring[2].modelType.has_value());
    CHECK(ring[2].lastLevel == 15);
    CHECK(IsDrawnByCodeAtSomeLevel(ITEM_WIZARDS_RING));
    // The archangel weapons are drawn small; the event models drawn with level
    // -1 for other items are not.
    CHECK(FindLevelModelRuns(ITEM_WEAPON_OF_ARCHANGEL)[0].small);
    CHECK_FALSE(FindLevelModelRuns(ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)[1].small);
    // The heart of (14,12) at +0 and +2 (GetDrawnModel), the Box of Luck.
    CHECK(IsDrawnByCodeAtSomeLevel(ITEM_POTION + 12));
    CHECK(IsDrawnByCodeAtSomeLevel(ITEM_BOX_OF_LUCK));
    // A plain item: its own model at every level.
    const int kris = Data::Items::MakeItemType(0, 0);
    CHECK(FindLevelModelRuns(kris).size() == 1);
    CHECK_FALSE(IsDrawnByCodeAtSomeLevel(kris));
}

TEST_CASE("The model tool reads and writes mesh numbers as text [editor][items]")
{
    CHECK(NumberList::Format({}) == "");
    CHECK(NumberList::Format({1, 3}) == "1, 3");
    CHECK(NumberList::Parse("1, 3") == std::vector<int>{1, 3});
    CHECK(NumberList::Parse("  2 ,4,") == std::vector<int>{2, 4});
    CHECK(NumberList::Parse("") == std::vector<int>{});
    CHECK_FALSE(NumberList::Parse("1, x").has_value());
    CHECK_FALSE(NumberList::Parse("-1").has_value());
}

#endif // _EDITOR
