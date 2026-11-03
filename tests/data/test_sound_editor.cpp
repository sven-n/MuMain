#include "stdafx.h"

// The Sound Editor (sound editor design, Q-SE1): its rows from the three
// files that name sounds, their problems, the changes with undo, the names
// in the files, the file picker and the window.

#include "doctest.h"

#ifdef _EDITOR
#include "Audio/DSPlaySound.h"
#include "Audio/SoundIds.h"
#include "Audio/SoundLoads.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MapData/MapObjectSoundsJson.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundsJson.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "TestImGuiContext.h"
#include "UI/SoundEditor/MuSoundEditorUI.h"
#include "UI/SoundEditor/SoundEditorEdits.h"
#include "UI/SoundEditor/SoundEditorRows.h"
#include "UI/SoundEditor/SoundFilePicker.h"
#include "UI/SoundEditor/SoundSelection.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace
{
using Data::Sounds::SoundFile;
using MuEditor::SoundEditor::RowProblem;
using MuEditor::SoundEditor::RowSource;
using MuEditor::SoundEditor::SoundRow;

// An audio backend that notes what loads.
class LoadingAudio final : public mu::IPlatformAudio
{
public:
    std::vector<std::tuple<int, std::wstring, int, bool>> loads;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound buffer, const wchar_t* filename, int channels, bool enable3D) override
    {
        loads.emplace_back(static_cast<int>(buffer), filename != nullptr ? filename : L"", channels, enable3D);
    }
    bool PlaySound(ESound, const void*, bool) override
    {
        return true;
    }
    bool PlaySoundAtLevel(ESound, float) override
    {
        return true;
    }
    void StopSound(ESound, bool) override {}
    void AllStopSound() override {}
    void ReleaseSound(ESound) override {}
    void Set3DSoundPosition() override {}
    void SetVolume(ESound, long) override {}
    void SetMasterVolume(long) override {}
    void PlayMusic(const char*, bool) override {}
    void StopMusic(const char*, bool) override {}
    bool IsEndMusic() override
    {
        return true;
    }
    int GetMusicPosition() override
    {
        return 0;
    }
    void SetBGMVolume(float) override {}
    void SetSFXVolume(float) override {}
    float GetBGMVolume() const override
    {
        return 1.0f;
    }
    float GetSFXVolume() const override
    {
        return 1.0f;
    }
};

// The three tables, the backend, the load record, the edits and two sound
// files in the working folder for the scope; all put back after.
class EditorScope
{
public:
    EditorScope()
        : m_previousAudio(g_platformAudio), m_code(g_SoundFileDatabase.GetAll()),
          m_objects(g_MapObjectSoundDatabase.GetAll())
    {
        for (const Data::Monsters::MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
        {
            m_sets.push_back(*set);
        }
        g_platformAudio = &audio;
        ForgetLoadedSounds();
        Audio::SoundLoads::Clear();
        g_SoundEdits.Clear();
        // A working folder of its own: the build folders' Data/Sound links
        // the shared client folder, which no test may write into.
        m_previousFolder = std::filesystem::current_path();
        m_folder = std::filesystem::temp_directory_path() / "mu_sound_editor_test";
        std::error_code error;
        std::filesystem::remove_all(m_folder, error);
        std::filesystem::create_directories(m_folder);
        std::filesystem::current_path(m_folder);
        std::filesystem::create_directories(Folder);
        for (const char* file : {"Data/Sound/EditorTest/ok.wav", "Data/Sound/EditorTest/Other.wav"})
        {
            std::ofstream(file, std::ios::binary) << "RIFF";
        }
    }
    ~EditorScope()
    {
        g_platformAudio = m_previousAudio;
        g_SoundFileDatabase.Build(m_code);
        g_MonsterSoundSetDatabase.Build(m_sets);
        g_MapObjectSoundDatabase.Build(m_objects);
        ForgetLoadedSounds();
        Audio::SoundLoads::Clear();
        g_SoundEdits.Clear();
        std::filesystem::current_path(m_previousFolder);
        std::error_code error;
        std::filesystem::remove_all(m_folder, error);
    }
    EditorScope(const EditorScope&) = delete;
    EditorScope& operator=(const EditorScope&) = delete;

    static constexpr const char* Folder = "Data/Sound/EditorTest";
    LoadingAudio audio;

private:
    mu::IPlatformAudio* m_previousAudio;
    std::filesystem::path m_previousFolder;
    std::filesystem::path m_folder;
    std::vector<SoundFile> m_code;
    std::vector<Data::Monsters::MonsterSoundSet> m_sets;
    std::vector<Data::Maps::MapObjectSound> m_objects;
};

SoundFile Code(const char* id, ESound number, const char* file, const char* name = "")
{
    SoundFile sound;
    sound.id = id;
    sound.number = static_cast<int>(number);
    sound.file = file;
    sound.name = name;
    return sound;
}

Data::Monsters::MonsterSoundSet DeathTree()
{
    Data::Monsters::MonsterSoundSet set;
    set.name = "Death Tree";
    Data::Monsters::MonsterSound idle;
    idle.file = "Data/Sound/EditorTest/ok.wav";
    idle.channels = 2;
    idle.spatial = true;
    Data::Monsters::MonsterSound attack;
    attack.file = "Data/Sound/EditorTest/attack.wav";
    attack.name = "Death Tree attack";
    set.sounds = {idle, attack};
    Data::Monsters::MonsterSoundEvent event;
    event.animations = {0};
    event.sounds = {"Data/Sound/EditorTest/ok.wav"};
    set.soundEvents = {event};
    return set;
}

Data::Maps::MapObjectSound Waterfall()
{
    Data::Maps::MapObjectSound sound;
    sound.maps = {{24}};
    sound.objects = {37, 38};
    sound.file = "Data/Sound/EditorTest/ok.wav";
    sound.channels = 3;
    return sound;
}

const SoundRow* FindRow(const std::vector<SoundRow>& rows, const std::string& key)
{
    const auto found = std::find_if(rows.begin(), rows.end(), [&key](const SoundRow& row) { return row.key == key; });
    return found != rows.end() ? &*found : nullptr;
}
} // namespace

TEST_CASE("Sound Editor: the rows list the code's sounds, the sets' and the map objects' with their names, ids, "
          "loads and problems, and the sounds played that nothing loads (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav", "Button click"),
                               Code("SOUND_RAIN01", SOUND_RAIN01, "Data/Sound/EditorTest/ok.wav"),
                               Code("SOUND_WIND01", SOUND_WIND01, "Data/Sound/EditorTest/gone.wav")});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall()});
    // A load at start, and a sound played that nothing loads.
    Audio::SoundLoads::NoteLoad(static_cast<int>(SOUND_CLICK01));
    Audio::SoundLoads::NotePlayedUnloaded(static_cast<int>(SOUND_FENRIR_DAMAGE_2));

    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    REQUIRE(rows.size() == 7);
    CHECK(rows[0].key == "SOUND_CLICK01");
    CHECK(rows[3].key == "set:Death Tree:Data/Sound/EditorTest/ok.wav");
    CHECK(rows[5].key == "objects:0");
    CHECK(rows[6].key == "SOUND_FENRIR_DAMAGE_2");

    const SoundRow& click = rows[0];
    CHECK(click.source == RowSource::Code);
    CHECK(click.number == static_cast<int>(SOUND_CLICK01));
    CHECK(MuEditor::SoundEditor::ShownName(click) == "Button click");
    CHECK(MuEditor::SoundEditor::IdText(click) == "SOUND_CLICK01");
    CHECK(click.loads == "at start");
    CHECK(click.livesIn == "Data/Audio/SoundFiles.json");
    // Two code sounds of one file; a file that is not there.
    CHECK(click.problem == RowProblem::SharedFile);
    CHECK(rows[1].problem == RowProblem::SharedFile);
    CHECK(rows[1].loads == "not loaded yet");
    CHECK(MuEditor::SoundEditor::ShownName(rows[1]) == "SOUND_RAIN01");
    CHECK(MuEditor::SoundEditor::IdText(rows[1]).empty());
    CHECK(rows[2].problem == RowProblem::MissingFile);

    const SoundRow& idle = rows[3];
    CHECK(idle.source == RowSource::MonsterSet);
    CHECK(idle.set == "Death Tree");
    CHECK(idle.channels == 2);
    CHECK(idle.spatial);
    CHECK(idle.problem == RowProblem::None);
    CHECK(MuEditor::SoundEditor::ShownName(idle) == "Death Tree: ok.wav");
    CHECK(idle.loads == "with its monster");
    CHECK(idle.livesIn == "Data/Monsters/Sounds.json");
    CHECK(MuEditor::SoundEditor::ShownName(rows[4]) == "Death Tree attack");
    CHECK(MuEditor::SoundEditor::IdText(rows[4]) == "Death Tree");
    CHECK(rows[4].problem == RowProblem::MissingFile);

    const SoundRow& waterfall = rows[5];
    CHECK(waterfall.source == RowSource::MapObject);
    CHECK(waterfall.channels == 3);
    CHECK(waterfall.livesIn == "Data/Maps/ObjectSounds.json");

    const SoundRow& fenrir = rows[6];
    CHECK(fenrir.source == RowSource::NotLoaded);
    CHECK(fenrir.problem == RowProblem::PlayedNotLoaded);
    CHECK(fenrir.number == static_cast<int>(SOUND_FENRIR_DAMAGE_2));

    CHECK(MuEditor::SoundEditor::Matches(click, "BUTTON"));
    CHECK(MuEditor::SoundEditor::Matches(click, "click01"));
    CHECK(MuEditor::SoundEditor::Matches(idle, "death tree"));
    CHECK(MuEditor::SoundEditor::Matches(idle, "ok.wav"));
    CHECK_FALSE(MuEditor::SoundEditor::Matches(click, "rain"));
    CHECK(MuEditor::SoundEditor::Matches(click, ""));

    // The mixer's and the overlay's name of a number.
    CHECK(MuEditor::SoundEditor::NameOfNumber(static_cast<int>(SOUND_CLICK01)) == "Button click");
    CHECK(MuEditor::SoundEditor::NameOfNumber(static_cast<int>(SOUND_RAIN01)).empty());
    CHECK(MuEditor::SoundEditor::NameOfNumber(rows[4].number) == "Death Tree attack");
}

TEST_CASE("Sound Editor: the session notes where LoadGameSound loaded a sound, at start or for the map being "
          "entered, and the sounds PlayBuffer was asked for that nothing loaded (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav"),
                               Code("SOUND_RAIN01", SOUND_RAIN01, "Data/Sound/EditorTest/Other.wav")});
    LoadGameSound(SOUND_CLICK01);
    {
        const Audio::SoundLoads::ScopedLoadWhere crywolf(WD_34CRYWOLF_1ST);
        LoadGameSound(SOUND_RAIN01);
        // A later load keeps the first one's place.
        LoadGameSound(SOUND_CLICK01);
    }
    CHECK(Audio::SoundLoads::WhereLoaded(static_cast<int>(SOUND_CLICK01)) == Audio::SoundLoads::AtStart);
    CHECK(Audio::SoundLoads::WhereLoaded(static_cast<int>(SOUND_RAIN01)) == WD_34CRYWOLF_1ST);
    CHECK_FALSE(Audio::SoundLoads::WhereLoaded(static_cast<int>(SOUND_WIND01)).has_value());

    PlayBuffer(SOUND_FENRIR_DAMAGE_2);
    PlayBuffer(SOUND_CLICK01);
    CHECK(Audio::SoundLoads::PlayedUnloaded() == std::set<int>{static_cast<int>(SOUND_FENRIR_DAMAGE_2)});
    // A load later takes it off the list.
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_FENRIR_DAMAGE_2", SOUND_FENRIR_DAMAGE_2, "Data/Sound/EditorTest/ok.wav")});
    LoadGameSound(SOUND_FENRIR_DAMAGE_2);
    CHECK(Audio::SoundLoads::PlayedUnloaded().empty());
}

TEST_CASE("Sound Editor: a code sound's change reaches its table at once, a loaded one loads again, and undo and "
          "redo take it back and forth (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav")});
    LoadGameSound(SOUND_CLICK01);
    g_SoundEdits.TakeSaved();
    REQUIRE(scope.audio.loads.size() == 1);
    CHECK_FALSE(g_SoundEdits.IsModified());

    const SoundRow row = MuEditor::SoundEditor::BuildRows().front();
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
    // Nothing differs: nothing changes.
    CHECK_FALSE(g_SoundEdits.Change(row, values));
    values.file = "Data/Sound/EditorTest/Other.wav";
    values.channels = 2;
    values.spatial = true;
    REQUIRE(g_SoundEdits.Change(row, values));
    const SoundFile* changed = g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01));
    REQUIRE(changed != nullptr);
    CHECK(changed->file == "Data/Sound/EditorTest/Other.wav");
    CHECK(g_SoundEdits.IsModified());
    REQUIRE(scope.audio.loads.size() == 2);
    CHECK(std::get<1>(scope.audio.loads[1]) == L"Data\\Sound\\EditorTest\\Other.wav");
    CHECK(std::get<2>(scope.audio.loads[1]) == 2);
    CHECK(std::get<3>(scope.audio.loads[1]));

    // A name only: no load.
    const SoundRow renamed = MuEditor::SoundEditor::BuildRows().front();
    values = MuEditor::SoundEditor::ValuesOf(renamed);
    values.name = "Button click";
    REQUIRE(g_SoundEdits.Change(renamed, values));
    CHECK(scope.audio.loads.size() == 2);
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01))->name == "Button click");

    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01))->name.empty());
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01))->file == "Data/Sound/EditorTest/ok.wav");
    REQUIRE(scope.audio.loads.size() == 3);
    CHECK(std::get<1>(scope.audio.loads[2]) == L"Data\\Sound\\EditorTest\\ok.wav");
    CHECK_FALSE(g_SoundEdits.IsModified());
    CHECK_FALSE(g_SoundEdits.Undo());
    REQUIRE(g_SoundEdits.Redo());
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_CLICK01))->file == "Data/Sound/EditorTest/Other.wav");
    CHECK(g_SoundEdits.CanRedo());
    // A new change drops what could be redone.
    values = MuEditor::SoundEditor::ValuesOf(MuEditor::SoundEditor::BuildRows().front());
    values.channels = 4;
    REQUIRE(g_SoundEdits.Change(MuEditor::SoundEditor::BuildRows().front(), values));
    CHECK_FALSE(g_SoundEdits.CanRedo());
}

TEST_CASE("Sound Editor: a set's sound changes through the monster tools, the events that name its file follow, "
          "and undo takes both back (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    REQUIRE(rows.size() == 2);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(rows[0]);
    values.name = "Death Tree idle";
    values.file = "Data/Sound/EditorTest/Other.wav";
    REQUIRE(g_SoundEdits.Change(rows[0], values));
    const Data::Monsters::MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find("Death Tree");
    REQUIRE(set != nullptr);
    CHECK(set->sounds[0].file == "Data/Sound/EditorTest/Other.wav");
    CHECK(set->sounds[0].name == "Death Tree idle");
    CHECK(set->sounds[0].channels == 2);
    // The event named the sound by its file.
    CHECK(set->soundEvents[0].sounds == std::vector<std::string>{"Data/Sound/EditorTest/Other.wav"});
    CHECK(g_SoundEdits.IsModified());
    CHECK(MuEditor::SoundEditor::BuildRows()[0].key == "set:Death Tree:Data/Sound/EditorTest/Other.wav");
    REQUIRE(g_SoundEdits.Undo());
    CHECK(*g_MonsterSoundSetDatabase.Find("Death Tree") == DeathTree());
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a code sound's stored type is set and cleared in its row and undone; other rows have "
          "none (Q-SE2) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_LEVEL_UP", SOUND_LEVEL_UP, "Data/Sound/EditorTest/ok.wav")});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    SoundRow row = MuEditor::SoundEditor::BuildRows().front();
    CHECK_FALSE(row.type.has_value());
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
    values.type = Audio::SoundKind::Hero;
    REQUIRE(g_SoundEdits.Change(row, values));
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_LEVEL_UP))->type == Audio::SoundKind::Hero);
    row = MuEditor::SoundEditor::BuildRows().front();
    CHECK(row.type == Audio::SoundKind::Hero);
    // A rename keeps the type.
    values = MuEditor::SoundEditor::ValuesOf(row);
    values.name = "Level up";
    REQUIRE(g_SoundEdits.Change(row, values));
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_LEVEL_UP))->type == Audio::SoundKind::Hero);
    row = MuEditor::SoundEditor::BuildRows().front();
    values = MuEditor::SoundEditor::ValuesOf(row);
    values.type.reset();
    REQUIRE(g_SoundEdits.Change(row, values));
    CHECK_FALSE(g_SoundFileDatabase.Find(static_cast<int>(SOUND_LEVEL_UP))->type.has_value());
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundFileDatabase.Find(static_cast<int>(SOUND_LEVEL_UP))->type == Audio::SoundKind::Hero);
}

TEST_CASE("Sound Editor: the window draws the selected sound's row, also when it was selected while the window "
          "was closed (Q-SE3) [sounds][editor]")
{
    EditorScope scope;
    std::vector<SoundFile> many;
    for (int i = 0; i < 300; ++i)
    {
        const auto number = static_cast<ESound>(static_cast<int>(SOUND_WIND01) + i);
        const std::string_view name = Audio::SoundIds::NameOf(number);
        if (!name.empty())
        {
            many.push_back(Code(std::string(name).c_str(), number, "Data/Sound/EditorTest/ok.wav"));
        }
    }
    g_SoundFileDatabase.Build(many);
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    // Selected far down, before the window draws.
    MuEditor::SoundSelection::Set(many.back().number);
    TestImGuiContext context;
    for (int frame = 0; frame < 3; ++frame)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(900.0f, 400.0f));
        bool open = true;
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    }
    // The table scrolled down to it.
    const ImGuiWindow* table = nullptr;
    for (ImGuiWindow* window : ImGui::GetCurrentContext()->Windows)
    {
        if (std::string(window->Name).find("##Sounds") != std::string::npos)
        {
            table = window;
        }
    }
    REQUIRE(table != nullptr);
    CHECK(table->Scroll.y > 0.0f);
    MuEditor::SoundSelection::Set(-1);
}

TEST_CASE("Sound Editor: a map object sound's name, file and channels change its table, which the map follows; it "
          "has no 3D (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall()});
    const int version = g_MapObjectSoundDatabase.GetVersion();
    const SoundRow row = MuEditor::SoundEditor::BuildRows().front();
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(row);
    values.name = "Kalima waterfall";
    values.file = "Data/Sound/EditorTest/Other.wav";
    values.channels = 1;
    values.spatial = true;
    REQUIRE(g_SoundEdits.Change(row, values));
    const Data::Maps::MapObjectSound& sound = g_MapObjectSoundDatabase.GetAll().front();
    CHECK(sound.name == "Kalima waterfall");
    CHECK(sound.file == "Data/Sound/EditorTest/Other.wav");
    CHECK(sound.channels == 1);
    CHECK(sound.objects == std::vector<int>{37, 38});
    CHECK(g_MapObjectSoundDatabase.GetVersion() != version);
    CHECK_FALSE(MuEditor::SoundEditor::BuildRows().front().spatial);
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_MapObjectSoundDatabase.GetAll().front() == Waterfall());
}

TEST_CASE("Sound Editor: names are read and written in the sets' sounds and the map object sounds; without one the "
          "line stays as it was (Q-SE1) [sounds][editor]")
{
    std::vector<Data::Monsters::MonsterSoundSet> sets;
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Monsters::ReadMonsterSoundsJson(Data::Monsters::WriteMonsterSoundsJson(std::vector{DeathTree()}),
                                          "Sounds.json", sets, issues);
    REQUIRE(sets.size() == 1);
    CHECK(issues.empty());
    CHECK(sets[0].sounds[1].name == "Death Tree attack");
    CHECK(sets[0].sounds[0].name.empty());
    const std::string setText = Data::Monsters::WriteMonsterSoundsJson(sets);
    CHECK(setText.find(R"("file": "Data/Sound/EditorTest/ok.wav", "channels": 2, "spatial": true})") !=
          std::string::npos);
    CHECK(setText.find(R"("spatial": false, "name": "Death Tree attack"})") != std::string::npos);

    Data::Maps::MapObjectSound named = Waterfall();
    named.name = "Kalima waterfall";
    std::vector<Data::Maps::MapObjectSound> objects;
    const std::string objectText = Data::Maps::WriteMapObjectSoundsJson(std::vector{named, Waterfall()});
    CHECK(objectText.find(R"("channels": 3, "name": "Kalima waterfall"})") != std::string::npos);
    Data::Maps::ReadMapObjectSoundsJson(objectText, "ObjectSounds.json", objects, issues);
    REQUIRE(objects.size() == 2);
    CHECK(issues.empty());
    CHECK(objects[0].name == "Kalima waterfall");
    CHECK(objects[1].name.empty());
}

TEST_CASE("Sound Editor: the picker lists a folder's sound files sorted, finds them by any part, and previews a "
          "file under its own number (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    std::ofstream("Data/Sound/EditorTest/notes.txt") << "no sound";
    std::filesystem::create_directories("Data/Sound/EditorTest/w39");
    std::ofstream("Data/Sound/EditorTest/w39/Night.WAV", std::ios::binary) << "RIFF";
    const std::vector<std::string> files = MuEditor::SoundEditor::ListSoundFiles("Data/Sound/EditorTest");
    CHECK(files == std::vector<std::string>{"Data/Sound/EditorTest/ok.wav", "Data/Sound/EditorTest/Other.wav",
                                            "Data/Sound/EditorTest/w39/Night.WAV"});
    CHECK(MuEditor::SoundEditor::FilterFiles(files, "NIGHT") ==
          std::vector<std::string>{"Data/Sound/EditorTest/w39/Night.WAV"});
    CHECK(MuEditor::SoundEditor::FilterFiles(files, "").size() == 3);
    MuEditor::SoundEditor::PreviewFile("Data/Sound/EditorTest/ok.wav", 2, false);
    REQUIRE(scope.audio.loads.size() == 1);
    CHECK(std::get<1>(scope.audio.loads[0]) == L"Data\\Sound\\EditorTest\\ok.wav");
    // Loaded once: a second preview plays it again.
    MuEditor::SoundEditor::PreviewFile("Data/Sound/EditorTest/ok.wav", 2, false);
    CHECK(scope.audio.loads.size() == 1);
}

TEST_CASE("Sound Editor: the window draws the rows, filtered and with only the problems (Q-SE1) "
          "[sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav", "Button click"),
                               Code("SOUND_WIND01", SOUND_WIND01, "Data/Sound/EditorTest/gone.wav")});
    TestImGuiContext context;
    for (int frame = 0; frame < 3; ++frame)
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    }
    CHECK(ImGui::FindWindowByName("Sound Editor###SoundEditor") != nullptr);
}
#endif // _EDITOR
