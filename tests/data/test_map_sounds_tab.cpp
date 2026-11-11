#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "UI/MapEditor/MapSoundsTab.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using Data::Maps::MapObjectSound;

namespace
{
// A working folder of its own with a Data/Maps folder, as the client's, and
// the table as it was.
class FolderScope
{
public:
    FolderScope()
        : m_folder(std::filesystem::temp_directory_path() / "mu-map-sounds-tab"),
          m_previous(std::filesystem::current_path()), m_table(g_MapObjectSoundDatabase.GetAll())
    {
        std::filesystem::remove_all(m_folder);
        std::filesystem::create_directories(m_folder / "Data" / "Maps");
        std::filesystem::current_path(m_folder);
    }
    ~FolderScope()
    {
        std::filesystem::current_path(m_previous);
        std::filesystem::remove_all(m_folder);
        g_MapObjectSoundDatabase.Build(m_table);
        g_MapSoundsTab.TakeSaved();
    }
    FolderScope(const FolderScope&) = delete;
    FolderScope& operator=(const FolderScope&) = delete;

private:
    std::filesystem::path m_folder;
    std::filesystem::path m_previous;
    std::vector<MapObjectSound> m_table;
};
} // namespace

TEST_CASE("Map Editor Sounds tab: draws, adds an entry for the map, takes picked objects, saves and reverts (MP1) "
          "[editor][maps]")
{
    FolderScope folder;
    g_MapObjectSoundDatabase.Build({});
    g_MapSoundsTab.TakeSaved();
    TestImGuiContext context;
    const auto draw = [&]
    {
        ImGui::NewFrame();
        ImGui::Begin("Map Editor");
        g_MapSoundsTab.Render(24, 0, 25);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    draw();
    CHECK_FALSE(g_MapSoundsTab.IsModified());

    g_MapSoundsTab.Add(24, 0);
    draw();
    REQUIRE(g_MapObjectSoundDatabase.GetAll().size() == 1);
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].maps == std::vector<Data::Maps::MapRef>{{24, 0}});
    CHECK(g_MapSoundsTab.IsModified());

    // Two clicks on waterfalls in the world, one of them twice.
    g_MapSoundsTab.Picked(38);
    g_MapSoundsTab.Picked(37);
    g_MapSoundsTab.Picked(38);
    draw();
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].objects == std::vector<int>{37, 38});
    CHECK_FALSE(g_MapSoundsTab.IsPicking());

    // Without a file the entry cannot be saved: nothing is written.
    std::vector<Data::Items::ItemDataIssue> issues;
    CHECK_FALSE(Data::Maps::SaveMapObjectSounds(issues));
    CHECK_FALSE(std::filesystem::exists(Data::Maps::GetMapObjectSoundsFile()));

    MapObjectSound sound = g_MapObjectSoundDatabase.GetAll()[0];
    sound.file = "Data/Sound/aKalimaWaterFall.wav";
    sound.channels = 3;
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{sound});
    issues.clear();
    REQUIRE(Data::Maps::SaveMapObjectSounds(issues));
    const Data::Maps::MapObjectSoundsLoadResult saved =
        Data::Maps::LoadMapObjectSoundsFile(Data::Maps::GetMapObjectSoundsFile());
    CHECK(saved.sounds == std::vector<MapObjectSound>{sound});

    // Revert reads the file again over an edit.
    g_MapObjectSoundDatabase.Build({});
    issues.clear();
    REQUIRE(Data::Maps::RevertMapObjectSounds(issues));
    CHECK(g_MapObjectSoundDatabase.GetAll() == std::vector<MapObjectSound>{sound});
    draw();

    // An entry of any object draws so; a picked object makes it one of types
    // (Q-MP8).
    MapObjectSound hum = sound;
    hum.anyObject = true;
    hum.objects.clear();
    g_MapObjectSoundDatabase.Build(std::vector<MapObjectSound>{hum});
    draw();
    g_MapSoundsTab.Picked(46);
    draw();
    REQUIRE(g_MapObjectSoundDatabase.GetAll().size() == 1);
    CHECK_FALSE(g_MapObjectSoundDatabase.GetAll()[0].anyObject);
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].objects == std::vector<int>{46});
}

#endif // _EDITOR
