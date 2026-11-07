#include "stdafx.h"
#include <doctest.h>
#include "ClientFolderScope.h"
#include "ExecutableConfigFile.h"
#include "TestModelSlots.h"
#include "Core/Text/Utf8.h"
#include "Data/GameConfig/GameConfig.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Data/DataHandler/LoadData.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>

namespace
{
const std::filesystem::path ClientFolder = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

struct BodyPartFile
{
    int firstSlot;
    const char* prefix;
};

const std::array<BodyPartFile, 5> BodyPartFiles{{
    {MODEL_BODY_HELM, "HelmClass"},
    {MODEL_BODY_ARMOR, "ArmorClass"},
    {MODEL_BODY_PANTS, "PantClass"},
    {MODEL_BODY_GLOVES, "GloveClass"},
    {MODEL_BODY_BOOTS, "BootClass"},
}};

// The forms OpenPlayers opens: the Magic Gladiator, Dark Lord and Rage Fighter have no second form.
bool HasForm(int stage, int baseClass)
{
    return stage != 1 || (baseClass != CLASS_DARK && baseClass != CLASS_DARK_LORD && baseClass != CLASS_RAGEFIGHTER);
}

// "HelmClass201.bmd" for the second form of the Dark Wizard.
std::string BodyFile(const BodyPartFile& part, int stage, int baseClass)
{
    const int number = baseClass + 1;
    std::string name = part.prefix;
    if (stage > 0)
        name += std::to_string(stage + 1);
    return name + (number < 10 ? "0" : "") + std::to_string(number) + ".bmd";
}

int BodySlot(const BodyPartFile& part, int stage, int baseClass)
{
    return part.firstSlot + stage * MAX_CLASS + baseClass;
}

std::string OpenedFile(int slot)
{
    return Core::Text::ToUtf8(gLoadData.GetModelFile(slot).c_str());
}

std::string FirstTextureName(const BMD& model)
{
    const char* name = model.Textures[0].FileName;
    return std::string(name, strnlen(name, sizeof(model.Textures[0].FileName)));
}

// A client folder for one test, with a copy of the Dark Knight's original body parts.
struct KnightClientFolder
{
    KnightClientFolder()
        : path(std::filesystem::temp_directory_path() /
               ("mu_test_player_bodies_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::create_directories(path / "Data" / "Player");
        for (const BodyPartFile& part : BodyPartFiles)
        {
            const std::string file = BodyFile(part, 0, CLASS_KNIGHT);
            std::filesystem::copy_file(std::filesystem::path(MU_TEST_DATA_DIR) / "Player" / file,
                                       path / "Data" / "Player" / file);
        }
    }
    ~KnightClientFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
    KnightClientFolder(const KnightClientFolder&) = delete;
    KnightClientFolder& operator=(const KnightClientFolder&) = delete;

    std::filesystem::path path;
};
} // namespace

TEST_CASE("S6 selector defaults and explicit choices survive save and restart [player-selector]")
{
    const ExecutableConfigFile file;
    GameConfig& config = GameConfig::GetInstance();
    file.Write("[Render]\n");
    config.Load();
    CHECK(config.GetUseS6PlayerModels() == true);
    for (bool useOriginal : {false, true})
    {
        config.SetUseS6PlayerModels(useOriginal);
        config.Save();
        config.SetUseS6PlayerModels(!useOriginal);
        config.Load();
        CHECK(config.GetUseS6PlayerModels() == useOriginal);
    }
}

TEST_CASE("Without the S6 models each body part comes from the Updated folder when it has the file [player-selector]")
{
    const std::filesystem::path updatedFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player" / "Updated";
    ClientFolderScope folder(ClientFolder);
    int improved = 0;
    for (bool useS6 : {true, false})
    {
        // A slot that holds a model is not opened again, so each choice starts from empty slots.
        TestModelSlots slots;
        for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
        {
            for (int baseClass = 0; baseClass < MAX_CLASS; ++baseClass)
            {
                if (!HasForm(stage, baseClass))
                    continue;
                Engine::Object::OpenPlayerBodyParts(stage, baseClass, useS6);
                for (const BodyPartFile& part : BodyPartFiles)
                {
                    const std::string file = BodyFile(part, stage, baseClass);
                    INFO(file, " useS6=", useS6);
                    const int slot = BodySlot(part, stage, baseClass);
                    const bool fromUpdated = !useS6 && std::filesystem::exists(updatedFolder / file);
                    CHECK(OpenedFile(slot) == (fromUpdated ? "Data\\Player\\Updated\\" : "Data\\Player\\") + file);
                    CHECK(Models[slot].NumMeshs > 0);
                    improved += fromUpdated;
                }
            }
        }
    }
    // The packaged improved set: 13 class forms of five parts each (docs/player-base-models.md).
    CHECK(improved == 65);
}

TEST_CASE("Both packaged player model choices open through the native loader [player-selector]")
{
    ClientFolderScope folder(ClientFolder);
    const int slot = MODEL_BODY_HELM + static_cast<int>(CLASS_KNIGHT);
    for (bool useS6 : {true, false})
    {
        TestModelSlots slots;
        Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, useS6);
        REQUIRE(Models[slot].NumMeshs > 0);
        CHECK(FirstTextureName(Models[slot]) == (useS6 ? "skin_wizard_01.jpg" : "skin_hq_class02.jpg"));
    }
}

TEST_CASE("A missing Updated folder leaves the original body parts [player-selector]")
{
    const KnightClientFolder client;
    TestModelSlots slots;
    ClientFolderScope folder(client.path);
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    for (const BodyPartFile& part : BodyPartFiles)
    {
        const std::string file = BodyFile(part, 0, CLASS_KNIGHT);
        INFO(file);
        CHECK(OpenedFile(BodySlot(part, 0, CLASS_KNIGHT)) == "Data\\Player\\" + file);
        CHECK(Models[BodySlot(part, 0, CLASS_KNIGHT)].NumMeshs > 0);
    }
}

TEST_CASE("An Updated body part that fails to load leaves the original part [player-selector]")
{
    const KnightClientFolder client;
    const std::string helm = BodyFile(BodyPartFiles[0], 0, CLASS_KNIGHT);
    std::filesystem::create_directories(client.path / "Data" / "Player" / "Updated");
    std::ofstream(client.path / "Data" / "Player" / "Updated" / helm, std::ios::binary) << "BM";
    TestModelSlots slots;
    ClientFolderScope folder(client.path);
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    const int slot = BodySlot(BodyPartFiles[0], 0, CLASS_KNIGHT);
    CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
    CHECK(Models[slot].NumMeshs > 0);
}
