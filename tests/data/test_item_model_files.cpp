#include "stdafx.h"

#include "doctest.h"

#include "TestFiles.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace Data::Items;

// These tests read the item models shipped in src/bin/Data, so broken model
// data is caught before it is merged.
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ClientDirectory = DataDirectory.parent_path();
const std::filesystem::path ModelDirectory = DataDirectory / "Items" / "Models";

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

std::string ToLower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return text;
}

// The file the game reads for a texture of a model, in lower case. Empty for
// textures the game does not load: hidden ones and other file types.
std::string GetStoredTextureName(const std::string& texture)
{
    if (IsHiddenTexture(texture))
    {
        return {};
    }
    return ToLower(GetStoredTextureFileName(texture).value_or(""));
}

// The files of a folder below Data/ in lower case, because the game finds
// textures regardless of upper and lower case.
const std::set<std::string>& GetFolderFiles(const std::string& folder)
{
    static std::map<std::string, std::set<std::string>> cache;
    auto [entry, inserted] = cache.try_emplace(folder);
    if (inserted)
    {
        for (const auto& file : std::filesystem::directory_iterator(DataDirectory / folder))
        {
            entry->second.insert(ToLower(file.path().filename().string()));
        }
    }
    return entry->second;
}

bool IsInFolders(const std::string& storedName, const std::vector<std::string>& folders)
{
    return std::any_of(folders.begin(), folders.end(),
                       [&](const std::string& folder) { return GetFolderFiles(folder).contains(storedName); });
}

// The texture names of the meshes of the model file.
std::vector<std::string> ReadMeshTextures(const ItemModelDefinition& model)
{
    const std::filesystem::path path = ClientDirectory / model.file;
    const std::wstring folder = path.parent_path().wstring() + L"/";
    auto bmd = std::make_unique<BMD>();
    REQUIRE(bmd->Open2(folder.c_str(), path.filename().wstring().c_str()));

    std::vector<std::string> textures;
    for (int mesh = 0; mesh < bmd->NumMeshs; ++mesh)
    {
        textures.emplace_back(bmd->Textures[mesh].FileName);
    }
    return textures;
}

// The textures of the model that none of its texture folders has.
std::vector<std::string> FindTexturesOutsideFolders(const ItemModelDefinition& model)
{
    const std::vector<std::string> textures = ReadMeshTextures(model);
    std::vector<std::string> missing;
    for (size_t mesh = 0; mesh < textures.size(); ++mesh)
    {
        const std::string& texture = textures[mesh];
        const std::string storedName = GetStoredTextureName(texture);
        if (!storedName.empty() && !IsInFolders(storedName, model.textureFolders))
        {
            missing.push_back("mesh " + std::to_string(mesh) + ": " + texture);
        }
    }
    return missing;
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

// Nothing in the client saves model files yet; this checks that the shipped
// files are in the format the writer produces (sorted, fixed field order).
TEST_CASE("Shipped item model files are in the written format [data][items]")
{
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        const std::string fileName = GetItemGroupFileName(group);
        INFO(fileName);
        CHECK(WriteItemModelGroupJson(group, ShippedModels().models) ==
              TestFiles::ReadWholeFile(ModelDirectory / fileName));
    }
}

// The paths have the case of the files, so they also load on file systems
// that tell upper and lower case apart.
TEST_CASE("Shipped item model files and texture folders exist [data][items]")
{
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        INFO("(" << model.group << "," << model.number << ") " << model.file);
        CHECK(std::filesystem::is_regular_file(ClientDirectory / model.file));
        for (const std::string& folder : model.textureFolders)
        {
            INFO(folder);
            CHECK(std::filesystem::is_directory(DataDirectory / folder));
        }
    }
}

// The game also uses a texture that another model loaded before, so a wrong
// texture folder would only work as long as that other model loads first.
TEST_CASE("Every texture of a shipped item model is in one of its texture folders [data][items]")
{
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        for (const std::string& missing : FindTexturesOutsideFolders(model))
        {
            INFO("(" << model.group << "," << model.number << ") " << model.file << " " << missing);
            CHECK(false);
        }
    }
}

// The game only loads .jpg and .tga textures; any other type is an error at
// the start of the game.
TEST_CASE("Every texture of a shipped item model is a .jpg or .tga texture or hidden [data][items]")
{
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        for (const std::string& texture : ReadMeshTextures(model))
        {
            INFO("(" << model.group << "," << model.number << ") " << model.file << " " << texture);
            CHECK((IsHiddenTexture(texture) || GetStoredTextureFileName(texture).has_value()));
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

TEST_CASE("Shipped item models keep the display of the old drawing code [data][items]")
{
    const std::vector<ItemModelDefinition>& models = ShippedModels().models;

    const ItemModelDefinition* kris = FindModel(models, 0, 0);
    REQUIRE(kris != nullptr);
    CHECK(kris->inventory.anchor == std::array<double, 2>{0.8, 0.85});
    CHECK(kris->inventory.offset == std::array<double, 3>{-0.02, 0.03, 0.0});
    CHECK(kris->inventory.rotation == std::array<double, 3>{180, 270, 15});
    CHECK(kris->inventory.scale == ItemInventoryDisplay::DefaultScale);
    CHECK(kris->ground.rotation == std::array<double, 3>{60, 0, -45});
    CHECK_FALSE(kris->ground.scale.has_value());

    // The only item that is also moved in depth.
    const ItemModelDefinition* lowerRefiningStone = FindModel(models, 14, 43);
    REQUIRE(lowerRefiningStone != nullptr);
    CHECK(lowerRefiningStone->inventory.offset[2] == 0.02);

    // Armor is drawn on the character skeleton, helms 160 units down.
    const ItemModelDefinition* bronzeHelm = FindModel(models, 7, 0);
    REQUIRE(bronzeHelm != nullptr);
    CHECK(bronzeHelm->inventory.bodyHeight == -160);
    CHECK(bronzeHelm->ground.bodyHeight == -160);
}

TEST_CASE("Shipped item models mark the capes that are drawn as cloth [data][items]")
{
    std::vector<std::pair<int, int>> cloth;
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        if (model.cloth)
        {
            cloth.emplace_back(model.group, model.number);
        }
    }
    std::sort(cloth.begin(), cloth.end());

    // Wing of Ruin, Cape of Emperor, Cape of Fighter, Cape of Overrule, Small
    // Cape of Lord, Little Warrior's Cloak and the Cape of Lord.
    const std::vector<std::pair<int, int>> capes{{12, 39},  {12, 40},  {12, 49}, {12, 50},
                                                 {12, 130}, {12, 135}, {13, 30}};
    CHECK(cloth == capes);
}

// The place in the slot of the items whose place depends on their level, as
// the old drawing code had it (recorded for levels 0 to 15).
TEST_CASE("Level variants keep their place in the inventory slot [data][items]")
{
    struct Expected
    {
        int group;
        int number;
        std::vector<std::pair<std::vector<int>, Render::Items::Display::Anchor>> anchors;
        Render::Items::Display::Anchor otherLevels;
    };
    using Anchor = Render::Items::Display::Anchor;
    const std::vector<Expected> expected{
        {13, 11, {{{0}, {0.5f, 0.8f}}, {{1}, {0.5f, 0.5f}}}, {}},                     // Life Stone
        {13, 14, {{{1}, {0.55f, 0.85f}}}, {0.6f, 1.0f}},                              // Loch's Feather
        {13, 19, {{{0}, {0.5f, 0.5f}}, {{1}, {0.7f, 0.8f}}, {{2}, {0.7f, 0.7f}}}, {}}, // Weapon of Archangel
        {13, 20, {{{0}, {0.5f, 0.65f}}, {{1, 2, 3}, {0.5f, 0.8f}}}, {}},              // Wizard's Ring
        {14, 9, {{{1}, {0.5f, 0.8f}}}, {0.5f, 0.95f}},                                // Ale
        {14, 11, {{{3, 13}, {0.5f, 0.5f}}, {{14, 15}, {0.5f, 0.8f}}}, {0.5f, 0.95f}}, // Box of Luck
        {14, 21, {{{0, 3}, {0.5f, 0.5f}}, {{1, 2}, {0.4f, 0.8f}}}, {}},               // Rena
        {14, 24, {{{1}, {0.5f, 0.8f}}}, {0.5f, 0.95f}},                               // Broken Sword / Dark Stone
    };
    g_ItemModelDatabase.Build(ShippedModels().models);

    for (const Expected& item : expected)
    {
        for (int level = 0; level <= 15; ++level)
        {
            Anchor want = item.otherLevels;
            for (const auto& [levels, anchor] : item.anchors)
            {
                if (std::find(levels.begin(), levels.end(), level) != levels.end())
                {
                    want = anchor;
                }
            }
            const Anchor got = Render::Items::Display::GetInventoryAnchor(MakeItemType(item.group, item.number), level);
            INFO("(" << item.group << "," << item.number << ") level " << level);
            CHECK(got.x == want.x);
            CHECK(got.y == want.y);
        }
    }
    g_ItemModelDatabase.Build({});
}
