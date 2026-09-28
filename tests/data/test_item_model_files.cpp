#include "stdafx.h"

#include "doctest.h"

#include "TestFiles.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelGlowJson.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Items/ItemGlow.h"
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

using GlowLevels = std::array<int, ItemGlow::ItemLevelCount>;

GlowLevels SameGlowLevel(int level)
{
    GlowLevels levels{};
    levels.fill(level);
    return levels;
}

const GlowColorsLoadResult& ShippedGlowColors()
{
    static const GlowColorsLoadResult result = LoadGlowColorsFile(DataDirectory / "Effects" / "GlowColors.json");
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

std::unique_ptr<BMD> OpenModelFile(const ItemModelDefinition& model)
{
    const std::filesystem::path path = ClientDirectory / model.file;
    const std::wstring folder = path.parent_path().wstring() + L"/";
    auto bmd = std::make_unique<BMD>();
    REQUIRE(bmd->Open2(folder.c_str(), path.filename().wstring().c_str()));
    return bmd;
}

// The texture names of the meshes of the model file.
std::vector<std::string> ReadMeshTextures(const ItemModelDefinition& model)
{
    const std::unique_ptr<BMD> bmd = OpenModelFile(model);
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

TEST_CASE("Shipped item models mark the capes that are worn as cloth [data][items]")
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
// the old drawing code had it (recorded for levels 0 to 15). The old code put
// the Life Stone above level 1 and Rena above level 3 at the top left corner
// of the slot; they now have the anchor of their item there. The Weapon of
// Archangel and the Wizard's Ring are not drawn at their other levels.
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
    const Anchor defaultAnchor{static_cast<float>(ItemInventoryDisplay::DefaultAnchor[0]),
                               static_cast<float>(ItemInventoryDisplay::DefaultAnchor[1])};
    const std::vector<Expected> expected{
        {13, 11, {{{1}, {0.5f, 0.5f}}}, {0.5f, 0.8f}},                                            // Life Stone
        {13, 14, {{{1}, {0.55f, 0.85f}}}, {0.6f, 1.0f}},                                          // Loch's Feather
        {13, 19, {{{0}, {0.5f, 0.5f}}, {{1}, {0.7f, 0.8f}}, {{2}, {0.7f, 0.7f}}}, defaultAnchor}, // Weapon of Archangel
        {13, 20, {{{0}, {0.5f, 0.65f}}, {{1, 2, 3}, {0.5f, 0.8f}}}, defaultAnchor},               // Wizard's Ring
        {14, 9, {{{1}, {0.5f, 0.8f}}}, {0.5f, 0.95f}},                                            // Ale
        {14, 11, {{{3, 13}, {0.5f, 0.5f}}, {{14, 15}, {0.5f, 0.8f}}}, {0.5f, 0.95f}},             // Box of Luck
        {14, 21, {{{1, 2}, {0.4f, 0.8f}}}, {0.5f, 0.5f}},                                         // Rena
        {14, 24, {{{1}, {0.5f, 0.8f}}}, {0.5f, 0.95f}}, // Broken Sword / Dark Stone
    };
    g_ItemModelDatabase.Build(ShippedModels().models, Data::Effects::GlowColorList{});

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
    g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
}

namespace
{
void CheckInventoryDisplay(int modelType, const Render::Items::Display::InventoryDisplay& expected)
{
    const Render::Items::Display::InventoryDisplay display = Render::Items::Display::GetInventoryDisplay(modelType);
    INFO("model " << modelType);
    CHECK(display.offset == expected.offset);
    CHECK(display.rotation == expected.rotation);
    CHECK(display.scale == expected.scale);
    CHECK(display.bodyHeight == expected.bodyHeight);
}

void CheckGroundDisplay(int modelType, const Render::Items::Display::GroundDisplay& expected)
{
    const Render::Items::Display::GroundDisplay display = Render::Items::Display::GetGroundDisplay(modelType);
    INFO("model " << modelType);
    CHECK(display.rotation == expected.rotation);
    CHECK(display.scale == expected.scale);
    CHECK(display.bodyHeight == expected.bodyHeight);
}
} // namespace

// Spot checks of the models that are drawn for items but are not item
// models, with the values of the old drawing code.
TEST_CASE("Models drawn for items keep the look of the old drawing code [data][items]")
{
    using namespace Render::Items::Display;
    g_ItemModelDatabase.Build(ShippedModels().models, Data::Effects::GlowColorList{});

    // The Rage Fighter armor has inventory models of its own, drawn on the
    // character skeleton with the look of the item.
    CHECK(GetInventoryModel(ITEM_SACRED_ARMOR) == MODEL_ARMORINVEN_60);
    CHECK(GetInventoryModel(ITEM_PHOENIX_SOUL_ARMOR) == MODEL_ARMORINVEN_74);
    CHECK(GetInventoryModel(ITEM_KRIS) == MODEL_ITEM + ITEM_KRIS);
    CHECK(IsDrawnOnCharacterSkeleton(MODEL_ARMORINVEN_74));
    CHECK(IsDrawnOnCharacterSkeleton(MODEL_HELM));
    CHECK(IsDrawnOnCharacterSkeleton(MODEL_BOOTS + MAX_ITEM_INDEX - 1));
    CHECK_FALSE(IsDrawnOnCharacterSkeleton(MODEL_BOOTS + MAX_ITEM_INDEX));
    CHECK_FALSE(IsDrawnOnCharacterSkeleton(MODEL_SWORD));
    CheckInventoryDisplay(MODEL_ARMORINVEN_60, {{0.01f, 0.08f, 0.0f}, {0, 0, 0}, 0.0039f, -100.0f});
    CheckInventoryDisplay(MODEL_ARMORINVEN_61, {{0.01f, 0.08f, 0.0f}, {0, 0, 0}, 0.0039f, -100.0f});
    CheckInventoryDisplay(MODEL_ARMORINVEN_62, {{0.01f, 0.08f, 0.0f}, {0, 0, 0}, 0.0039f, -100.0f});
    CheckInventoryDisplay(MODEL_ARMORINVEN_74, {{0.01f, 0.05f, 0.0f}, {90, 0, 0}, 0.0039f, -100.0f});

    // Event models of level variants; MODEL_EVENT + 13 has the default look.
    CheckInventoryDisplay(MODEL_EVENT, {{}, {180, 0, 0}, 0.0025f, 0.0f});
    CheckInventoryDisplay(MODEL_EVENT + 6, {{}, {270, 90, 0}, 0.0039f, 0.0f});
    CheckInventoryDisplay(MODEL_EVENT + 10, {{}, {270, -10, 0}, 0.001f, 0.0f});
    CheckInventoryDisplay(MODEL_EVENT + 13, {{}, {270, -10, 0}, 0.0025f, 0.0f});
    CheckInventoryDisplay(MODEL_EVENT + 21, {{0.0f, 0.08f, 0.0f}, {0, -10, 0}, 0.002f, 0.0f});
    CheckGroundDisplay(MODEL_EVENT + 4, {{90, 0, -45}, std::nullopt, 0.0f});
    CheckGroundDisplay(MODEL_EVENT + 6, {{0, 0, -45}, std::nullopt, 0.0f});
    CheckGroundDisplay(MODEL_EVENT + 12, {{160, -183, 198}, 0.38f, 0.0f});
    CheckGroundDisplay(MODEL_EVENT + 13, {{160, -183, 198}, 0.54f, 0.0f});

    // (14,12) is drawn with event models at level 0 and 2.
    CHECK(GetDrawnModel(MODEL_POTION + 12, 0) == MODEL_EVENT);
    CHECK(GetDrawnModel(MODEL_POTION + 12, 1) == MODEL_POTION + 12);
    CHECK(GetDrawnModel(MODEL_POTION + 12, 2) == MODEL_EVENT + 1);
    CHECK(GetDrawnModel(MODEL_SWORD, 0) == MODEL_SWORD);

    // The Weapon of Archangel draws the archangel weapons smaller, with
    // level -1.
    CHECK(GetSmallArchangelWeaponScale(MODEL_DIVINE_STAFF_OF_ARCHANGEL, -1) == 0.001f);
    CHECK(GetSmallArchangelWeaponScale(MODEL_DIVINE_SWORD_OF_ARCHANGEL, -1) == 0.001f);
    CHECK(GetSmallArchangelWeaponScale(MODEL_DIVINE_CB_OF_ARCHANGEL, -1) == 0.0015f);
    CHECK_FALSE(GetSmallArchangelWeaponScale(MODEL_DIVINE_STAFF_OF_ARCHANGEL, 0).has_value());
    CHECK_FALSE(GetSmallArchangelWeaponScale(MODEL_EVENT + 12, -1).has_value());

    g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
}

TEST_CASE("Shipped item models keep the glow of the old drawing code [data][items]")
{
    const std::vector<ItemModelDefinition>& models = ShippedModels().models;
    const auto glowOf = [&](int group, int number) -> const ItemGlow&
    {
        const ItemModelDefinition* model = FindModel(models, group, number);
        REQUIRE(model != nullptr);
        return model->glow;
    };

    CHECK(glowOf(0, 0) == ItemGlow{});

    const ItemGlow& lightningSword = glowOf(0, 14);
    CHECK(lightningSword.color == "blue");
    CHECK(lightningSword.shineColor == "blue");

    // The glow leaves out one mesh, or is on some meshes only.
    CHECK(glowOf(2, 7).meshes.hidden == 2);
    // The glow leaves out the mesh these draw as an effect of their own.
    CHECK(glowOf(0, 31).meshes.hidden == 2);
    CHECK(glowOf(3, 10).meshes.hidden == 1);
    CHECK(glowOf(6, 16).meshes.hidden == 2);
    CHECK(glowOf(3, 11).meshes.only == std::vector<int>{0, 1});

    // Armor sets: the ancient shine is gold for some, the shine plain white.
    CHECK(glowOf(7, 3).ancientColor == "gold");
    CHECK(glowOf(7, 21).shineWhite);
    CHECK(glowOf(7, 59).excellentMesh == 1);
    CHECK(glowOf(7, 39).excellentMeshWithoutSkin == 2);

    // Jewels glow like +8; wings and capes like +0 and without the excellent
    // glow.
    CHECK(glowOf(14, 13).levels == SameGlowLevel(8));
    CHECK(glowOf(12, 0).levels == SameGlowLevel(0));
    // +1 arrows glow like +3; from +8 they would glow like +17 and up, which
    // is no glow at all.
    CHECK(glowOf(4, 15).levels == GlowLevels{0, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31});
    // The Devil's Square items glow like half their square, the seventh
    // square like +13.
    CHECK(glowOf(14, 17).levels == GlowLevels{0, 0, 1, 1, 2, 2, 3, 13, 13, 13, 13, 13, 13, 13, 13, 13});
    CHECK_FALSE(glowOf(12, 0).excellent);
    CHECK_FALSE(glowOf(13, 30).excellent);
}

// A mesh the model does not have would leave the glow out without a message
// (loading only warns about it).
TEST_CASE("The glow of shipped item models is on meshes the models have [data][items]")
{
    for (const ItemModelDefinition& model : ShippedModels().models)
    {
        std::vector<std::pair<std::string, int>> meshes;
        GlowJson::ForEachMesh(model.glow,
                              [&](const std::string& field, int mesh) { meshes.emplace_back(field, mesh); });
        if (meshes.empty())
        {
            continue;
        }

        const std::unique_ptr<BMD> bmd = OpenModelFile(model);
        for (const auto& [field, mesh] : meshes)
        {
            INFO("(" << model.group << "," << model.number << ") " << field << " " << mesh);
            CHECK(mesh < bmd->NumMeshs);
        }
    }
}

TEST_CASE("Shipped item model glow colors are in the glow color list [data][items]")
{
    CHECK(ShippedGlowColors().issues.empty());
    std::vector<ItemDataIssue> issues;
    ValidateItemModelGlowColors(ShippedModels().models, ShippedGlowColors().colors, issues);
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
}

TEST_CASE("Models drawn for items glow like the old drawing code [data][items]")
{
    using namespace Render::Items::Glow;
    g_GlowColors.Build(ShippedGlowColors().colors);
    g_ItemModelDatabase.Build(ShippedModels().models, g_GlowColors);

    CHECK(GetLevel(MODEL_ARROWS, 0) == 0);
    CHECK(GetLevel(MODEL_ARROWS, 3) == 7);
    CHECK(GetLevel(MODEL_DEVILS_EYE, 5) == 2);
    CHECK(GetLevel(MODEL_DEVILS_EYE, 7) == 13);
    // The event models of level variants stay in code.
    CHECK(GetLevel(MODEL_EVENT + 14, 2) == 9);
    CHECK(GetLevel(MODEL_ITEM + MakeItemType(14, 13), 0) == 8);
    CHECK(GetLevel(MODEL_ITEM + MakeItemType(0, 0), 5) == 5);

    // The inventory models of the Rage Fighter armor and the second models of
    // the Rage Fighter gloves have the colors of their item; their meshes are
    // their own.
    const Color copper{0.8f, 0.46f, 0.25f};
    CHECK(GetColors(MODEL_ITEM + ITEM_SACRED_ARMOR).color == copper);
    CHECK(GetColors(MODEL_ARMORINVEN_60).color == copper);
    CHECK(GetColors(MODEL_SWORD_32_LEFT).color == copper);
    CHECK(Get(MODEL_ARMORINVEN_60) == ItemGlow{});
    // Models that are not items keep the colors of the drawing code, whatever
    // the list says.
    CHECK(GetColors(MODEL_PLAYER).color == Color{1.0f, 0.5f, 0.0f});
    CHECK(GetColors(MODEL_PLAYER).ancientColor == Color{0.1f, 0.6f, 1.0f});
    CHECK(Get(MODEL_PLAYER) == ItemGlow{});
    CHECK(HasExcellentGlow(MODEL_PLAYER));
    CHECK_FALSE(HasExcellentGlow(MODEL_WING));

    g_GlowColors.Build({});
    g_ItemModelDatabase.Build({}, g_GlowColors);
}
