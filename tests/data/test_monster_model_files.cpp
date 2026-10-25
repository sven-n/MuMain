#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/EffectData/EffectDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterModelDefinition;

namespace
{
const std::filesystem::path ClientData = MU_TEST_DATA_DIR;

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

// The shipped list, read as the game reads it (from the client folder, after
// the effect catalogue).
Data::Monsters::MonsterModelsLoadResult LoadShipped()
{
    ClientDataScope scope(ClientData.parent_path());
    std::string error;
    REQUIRE(Data::Effects::LoadEffectTypes(error));
    return Data::Monsters::LoadMonsterModelsFile(Data::Monsters::GetMonsterModelsFile());
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

// The entry of `folder` named `name` in any case, or an empty path.
std::filesystem::path FindIgnoringCase(const std::filesystem::path& folder, const std::string& name)
{
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(folder, error))
    {
        if (Lower(entry.path().filename().string()) == Lower(name))
        {
            return entry.path();
        }
    }
    return {};
}

// Whether `path` (below the client folder, '/' between its parts) is there,
// spelled exactly as on disk.
bool IsSpelledAsOnDisk(const std::string& path)
{
    std::filesystem::path folder = ClientData.parent_path();
    size_t start = 0;
    while (start <= path.size())
    {
        const size_t end = std::min(path.find('/', start), path.size());
        const std::string part = path.substr(start, end - start);
        const std::filesystem::path found = FindIgnoringCase(folder, part);
        if (found.empty() || found.filename().string() != part)
        {
            return false;
        }
        folder = found;
        start = end + 1;
    }
    return true;
}

std::vector<std::string> AllFolders(const MonsterModelDefinition& model)
{
    std::vector<std::string> folders = model.load.textureFolders;
    for (const auto& world : model.worldTextureFolders)
    {
        folders.insert(folders.end(), world.folders.begin(), world.folders.end());
    }
    return folders;
}

// The texture names of the model file, with the names that replace them by
// mesh, that no folder of the list has.
std::vector<std::string> TexturesNotInFolders(const std::string& file, const std::vector<std::string>& folders,
                                              const std::vector<std::pair<int, std::string>>& replacedNames = {})
{
    const std::filesystem::path path = ClientData.parent_path() / file;
    BMD model;
    const std::wstring folder = path.parent_path().wstring() + L"/";
    if (!model.Open2(folder.c_str(), path.filename().wstring().c_str()))
    {
        return {"(the file does not open)"};
    }
    std::vector<std::string> missing;
    for (int i = 0; i < model.NumMeshs; ++i)
    {
        std::string name = model.Textures[i].FileName;
        for (const auto& [mesh, replaced] : replacedNames)
        {
            if (mesh == i)
            {
                name = replaced;
            }
        }
        const std::optional<std::string> stored = Data::Items::GetStoredTextureFileName(name);
        if (Data::Items::IsHiddenTexture(name) || !stored)
        {
            continue;
        }
        const bool found = std::any_of(folders.begin(), folders.end(), [&](const std::string& textureFolder)
                                       { return !FindIgnoringCase(ClientData / textureFolder, *stored).empty(); });
        if (!found)
        {
            missing.push_back(name);
        }
    }
    return missing;
}
} // namespace

TEST_CASE("Monster models: the shipped model list loads; its only warning is Molt's missing file [data][monsters]")
{
    const Data::Monsters::MonsterModelsLoadResult result = LoadShipped();
    INFO(Messages(result.issues));
    CHECK_FALSE(HasErrors(result.issues));
    REQUIRE(result.issues.size() == 1);
    CHECK(result.issues[0].severity == ItemDataIssueSeverity::Warning);
    CHECK(result.issues[0].field == "model MONSTER_MODEL_MOLT file");
    CHECK(result.models.size() == 295);
}

TEST_CASE("Monster models: the shipped model list is written as its writer writes it [data][monsters]")
{
    const std::optional<std::string> text = Data::ReadTextFile(ClientData / "Monsters" / "Models.json");
    REQUIRE(text);
    std::vector<MonsterModelDefinition> models;
    std::vector<ItemDataIssue> issues;
    Data::Monsters::ReadMonsterModelsJson(*text, "Models.json", models, issues);
    REQUIRE_FALSE(HasErrors(issues));
    CHECK(Data::Monsters::WriteMonsterModelsJson(models) == *text);
}

TEST_CASE("Monster models: the files and texture folders of the shipped list are spelled as on disk [data][monsters]")
{
    const Data::Monsters::MonsterModelsLoadResult result = LoadShipped();
    for (const MonsterModelDefinition& model : result.models)
    {
        INFO(model.slot);
        // Molt's file is missing, here and in the original client (P1).
        if (model.slot != "MONSTER_MODEL_MOLT")
        {
            CHECK(IsSpelledAsOnDisk(model.load.file));
        }
        for (const std::string& folder : AllFolders(model))
        {
            CHECK(IsSpelledAsOnDisk("Data/" + folder));
        }
        for (const auto& part : model.parts)
        {
            CHECK(IsSpelledAsOnDisk(part.load.file));
            for (const std::string& folder : part.load.textureFolders)
            {
                CHECK(IsSpelledAsOnDisk("Data/" + folder));
            }
        }
    }
}

TEST_CASE("Monster models: the shipped models' textures are in their folders [data][monsters]")
{
    // Textures that no folder of their model has; the game takes them from an
    // effect that loaded a texture of that name before, else it shows an
    // error. The original client has the same files.
    const std::set<std::string> knownMissing = {
        "MONSTER_MODEL_LIFE_STONE TowerGateplane_R.JPG",      "MONSTER_MODEL_SELUPAN blast2_r.jpg",
        "MODEL_UNITEDMARKETPLACE_RAUL flareRed.jpg",          "MODEL_UNITEDMARKETPLACE_RAUL youmuk01aeff.jpg",
        "MODEL_UNITEDMARKETPLACE_RAUL sulnalbagiteam.jpg",    "MODEL_UNITEDMARKETPLACE_RAUL puckitemz.jpg",
        "MODEL_UNITEDMARKETPLACE_RAUL sulnalbagiteampeg.jpg", "MODEL_UNITEDMARKETPLACE_RAUL motion_blur_r3.JPG",
        "MODEL_UNITEDMARKETPLACE_RAUL xmaseffitemx.jpg",
    };
    const Data::Monsters::MonsterModelsLoadResult result = LoadShipped();
    for (const MonsterModelDefinition& model : result.models)
    {
        if (model.slot == "MONSTER_MODEL_MOLT")
        {
            continue;
        }
        for (const std::string& texture : TexturesNotInFolders(model.load.file, AllFolders(model), model.textureNames))
        {
            INFO(model.slot << " " << texture);
            CHECK(knownMissing.contains(model.slot + " " + texture));
        }
        for (const auto& part : model.parts)
        {
            for (const std::string& texture : TexturesNotInFolders(part.load.file, part.load.textureFolders))
            {
                INFO(model.slot << " part " << part.load.file << " " << texture);
                CHECK(false);
            }
        }
    }
}

TEST_CASE("Monster models: a SOUND_ id that a sound slot names is loaded by a model of the shipped list "
          "[data][monsters]")
{
    // A slot plays the sound of its number, whichever model loaded it: the
    // Giant Ogre's attack plays the Dark Skull Soldier's attack sound in
    // Kalima, Vepar's the Stone Golem's death sound. A number that no model
    // loads plays nothing; the slots that named one in the original client
    // name their models' sounds now (P9, Q-MN20).
    const Data::Monsters::MonsterModelsLoadResult result = LoadShipped();
    std::set<std::string> loaded;
    for (const MonsterModelDefinition& model : result.models)
    {
        for (const Data::Monsters::MonsterModelSound& sound : model.sounds)
        {
            if (!sound.id.empty())
            {
                loaded.insert(sound.id);
            }
        }
    }
    std::set<std::string> notLoaded;
    for (const MonsterModelDefinition& model : result.models)
    {
        if (!model.soundSlots)
        {
            continue;
        }
        std::vector<std::string> values(model.soundSlots->first.begin(), model.soundSlots->first.end());
        if (model.soundSlots->second)
        {
            values.insert(values.end(), model.soundSlots->second->begin(), model.soundSlots->second->end());
        }
        for (const std::string& value : values)
        {
            if (value.starts_with("SOUND_") && !loaded.contains(value))
            {
                notLoaded.insert(value);
            }
        }
    }
    std::string listed;
    for (const std::string& id : notLoaded)
    {
        listed += id + " ";
    }
    INFO("named by slots, loaded by no model: " << listed);
    CHECK(notLoaded.empty());
}

TEST_CASE("Monster models: the game's load fills the model table [data][monsters]")
{
    ClientDataScope scope(ClientData.parent_path());
    std::string error;
    REQUIRE(Data::Effects::LoadEffectTypes(error));
    REQUIRE(g_MonsterDataHandler.LoadModels(error));
    const MonsterModelDefinition* hound = g_MonsterModelDatabase.Find(MODEL_HOUND);
    REQUIRE(hound != nullptr);
    CHECK(hound->load.file == "Data/Monster/Monster02.bmd");
    CHECK(g_MonsterModelDatabase.Find(MODEL_SMITH) != nullptr);
    CHECK(g_MonsterModelDatabase.Find(MODEL_PLAYER) == nullptr);
}

// A sound that several models list is loaded once (MonsterModelLoader skips
// one already loaded from the same file): they must list the same file,
// channels and 3D setting, or the later model loads it again with its own.
TEST_CASE("Monster model files: models that share a sound id load it alike [data][monsters]")
{
    const Data::Monsters::MonsterModelsLoadResult loaded = LoadShipped();
    REQUIRE_FALSE(Data::Items::HasErrors(loaded.issues));
    std::map<std::string, std::set<std::string>> loads;
    for (const MonsterModelDefinition& model : loaded.models)
    {
        for (const Data::Monsters::MonsterModelSound& sound : model.sounds)
        {
            if (!sound.id.empty())
            {
                loads[sound.id].insert(Lower(sound.file) + " channels " + std::to_string(sound.channels) +
                                       (sound.spatial ? " 3d" : ""));
            }
        }
    }
    std::vector<std::string> differing;
    for (const auto& [id, ways] : loads)
    {
        if (ways.size() > 1)
        {
            differing.push_back(id);
        }
    }
    // As in the original client: the Beam Knight loads death1.wav with two
    // channels in 3D, the Iron Knight (and its dark twin) with one.
    CHECK(differing == std::vector<std::string>{"SOUND_MONSTER_DEATH1"});
}
