#include "stdafx.h"

#include "doctest.h"

#include "TestFiles.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
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
