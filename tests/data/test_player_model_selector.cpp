#include "stdafx.h"
#include <doctest.h>
#include "TestModelSlots.h"
#include "Data/GameConfig/GameConfig.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Data/DataHandler/LoadData.h"
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
// The selector test executable has its own config, never the game runtime's config.
struct ConfigScope
{
    GameConfig& config = GameConfig::GetInstance();
    std::filesystem::path path = config.GetConfigPath();
    bool existed = std::filesystem::exists(path);
    std::string saved;
    ConfigScope()
    {
        if (existed)
        {
            std::ifstream in(path, std::ios::binary);
            saved.assign(std::istreambuf_iterator<char>(in), {});
        }
    }
    ~ConfigScope()
    {
        if (existed)
            std::ofstream(path, std::ios::binary) << saved;
        else
            std::filesystem::remove(path);
        config.Load();
    }
};
} // namespace

TEST_CASE("S6 selector defaults and explicit choices survive save and restart [player-selector]")
{
    ConfigScope scope;
    std::ofstream(scope.path) << "[Render]\n";
    scope.config.Load();
    CHECK(scope.config.GetUseS6PlayerModels() == true);
    for (bool useOriginal : {false, true})
    {
        scope.config.SetUseS6PlayerModels(useOriginal);
        scope.config.Save();
        scope.config.SetUseS6PlayerModels(!useOriginal);
        scope.config.Load();
        CHECK(scope.config.GetUseS6PlayerModels() == useOriginal);
    }
}

TEST_CASE("S6 selector changes exactly 13 body family directories [player-selector]")
{
    using Engine::Object::PlayerBodyModelDirectory;
    int updated = 0;
    for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
        for (int base = 0; base < MAX_CLASS; ++base)
        {
            const int offset = stage * MAX_CLASS + base;
            CHECK(std::wstring(PlayerBodyModelDirectory(offset, true)) == L"Data\\Player\\");
            const bool selected = base <= CLASS_DARK_LORD && (stage != 1 || base <= CLASS_ELF);
            CHECK(std::wstring(PlayerBodyModelDirectory(offset, false)) ==
                  (selected ? L"Data\\Player\\Updated\\" : L"Data\\Player\\"));
            updated += selected;
        }
    CHECK(updated == 13);
    CHECK(std::wstring(PlayerBodyModelDirectory(-1, false)) == L"Data\\Player\\");
    CHECK(std::wstring(PlayerBodyModelDirectory(MAX_CLASS * MAX_CLASS_STAGES, false)) == L"Data\\Player\\");
}

TEST_CASE("both packaged player model choices open through native loader [player-selector]")
{
    ConfigScope config;
    const auto previous = std::filesystem::current_path();
    struct RestoreFolder
    {
        std::filesystem::path path;
        ~RestoreFolder()
        {
            std::filesystem::current_path(path);
        }
    } restore{previous};
    std::filesystem::current_path(std::filesystem::path(MU_TEST_DATA_DIR).parent_path());
    for (bool useOriginal : {true, false})
    {
        TestModelSlots slots;
        config.config.SetUseS6PlayerModels(useOriginal);
        const int slot = MODEL_BODY_HELM + static_cast<int>(CLASS_KNIGHT);
        REQUIRE(gLoadData.AccessModel(
            slot, Engine::Object::PlayerBodyModelDirectory(CLASS_KNIGHT, config.config.GetUseS6PlayerModels()),
            L"HelmClass", CLASS_KNIGHT + 1));
        REQUIRE(Models[slot].NumMeshs > 0);
        CHECK(std::string(Models[slot].Textures[0].FileName) ==
              (useOriginal ? "skin_wizard_01.jpg" : "skin_hq_class02.jpg"));
    }
}
