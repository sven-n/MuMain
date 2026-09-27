#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace Data::Items;

// These tests read the item models shipped in src/bin/Data, so broken model
// data is caught before it is merged.
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ModelDirectory = DataDirectory / "Items" / "Models";

std::string ReadWholeFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

const ItemModelDefinition* FindModel(const std::vector<ItemModelDefinition>& models, int group, int number)
{
    const auto found = std::find_if(models.begin(), models.end(), [&](const ItemModelDefinition& model) {
        return model.group == group && model.number == number;
    });
    return found != models.end() ? &*found : nullptr;
}

const ItemModelDataLoadResult& ShippedModels()
{
    static const ItemModelDataLoadResult result = LoadItemModelDataDirectory(ModelDirectory);
    return result;
}
} // namespace

TEST_CASE("Shipped item models load without problems [data][items]")
{
    const ItemModelDataLoadResult& result = ShippedModels();

    for (const ItemDataIssue& issue : result.issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    CHECK(result.models.size() > 850);
}

TEST_CASE("Shipped item model files are written the way the client writes them [data][items]")
{
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        const std::string fileName = GetItemGroupFileName(group);
        INFO(fileName);
        CHECK(WriteItemModelGroupJson(group, ShippedModels().models) == ReadWholeFile(ModelDirectory / fileName));
    }
}

// The paths have the case of the files, so they also load on file systems
// that tell upper and lower case apart.
TEST_CASE("Shipped item model files and texture folders exist [data][items]")
{
    const std::filesystem::path clientDirectory = DataDirectory.parent_path();
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        INFO("(" << model.group << "," << model.number << ") " << model.file);
        CHECK(std::filesystem::is_regular_file(clientDirectory / model.file));
        for (const std::string& folder : model.textureFolders)
        {
            INFO(folder);
            CHECK(std::filesystem::is_directory(DataDirectory / folder));
        }
    }
}

TEST_CASE("Shipped item models keep the models of the old loading code [data][items]")
{
    const std::vector<ItemModelDefinition>& models = ShippedModels().models;

    const ItemModelDefinition* kris = FindModel(models, 0, 0);
    REQUIRE(kris != nullptr);
    CHECK(kris->file == "Data/Item/Sword01.bmd");
    CHECK(kris->textureFolders == std::vector<std::string>{"Item"});

    // Two of its textures are armor textures in Data/Player.
    const ItemModelDefinition* stormHardGlove = FindModel(models, 0, 33);
    REQUIRE(stormHardGlove != nullptr);
    CHECK(stormHardGlove->textureFolders == std::vector<std::string>{"Item", "Player"});

    const ItemModelDefinition* darkHorse = FindModel(models, 13, 4);
    REQUIRE(darkHorse != nullptr);
    CHECK(darkHorse->textureFolders == std::vector<std::string>{"Item", "Skill"});

    const ItemModelDefinition* spear = FindModel(models, 3, 0);
    REQUIRE(spear != nullptr);
    CHECK(spear->noneBlendMeshes == std::vector<int>{1});

    // Armor items use the player models.
    const ItemModelDefinition* bronzeHelm = FindModel(models, 7, 0);
    REQUIRE(bronzeHelm != nullptr);
    CHECK(bronzeHelm->file == "Data/Player/HelmMale01.bmd");
}
