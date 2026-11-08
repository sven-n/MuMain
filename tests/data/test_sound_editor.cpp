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
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterSoundJsonStorage.h"
#include "Data/DataHandler/SoundData/SoundFolderStorage.h"
#include "Data/GameData/MapData/MapObjectSoundsJson.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "Data/GameData/SoundData/SoundFilesJson.h"
#include "Data/GameData/SoundData/SoundFolders.h"
#include "I18N/All.h"
#include "TestImGuiContext.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/MapEditor/MapSoundsTab.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/SoundEditor/MuSoundEditorUI.h"
#include "UI/SoundEditor/SoundEditorEdits.h"
#include "UI/SoundEditor/SoundEditorRows.h"
#include "UI/SoundEditor/SoundEventEditing.h"
#include "UI/SoundEditor/SoundFilePicker.h"
#include "UI/SoundEditor/SoundFolderTree.h"
#include "UI/SoundEditor/SoundSelection.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <sstream>
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
        // The rows check files against the list of this folder.
        MuEditor::SoundEditor::RefreshClientSoundFiles();
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
        MuEditor::SoundEditor::RefreshClientSoundFiles();
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
    // Played by its id: the set's events and the folders name it so.
    attack.id = "SOUND_MONSTER_MAGICSKULL1";
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
    CHECK(rows[5].key == "objects:Data/Sound/EditorTest/ok.wav");
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
    CHECK(rows[4].key == "set:Death Tree:SOUND_MONSTER_MAGICSKULL1");
    CHECK(MuEditor::SoundEditor::ShownName(rows[4]) == "Death Tree attack");
    CHECK(MuEditor::SoundEditor::IdText(rows[4]) == "Death Tree, SOUND_MONSTER_MAGICSKULL1");
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

// The Sounds tab builds its rows each frame: their files are looked up in the
// list of the client folder, which is read once, not on the disk per row.
TEST_CASE("Sound Editor: the rows find a missing file in the client folder's list, which is read once and again "
          "on request [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_WIND01", SOUND_WIND01, "Data/Sound/EditorTest/new.wav")});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    const auto problem = []
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, "SOUND_WIND01");
        REQUIRE(row != nullptr);
        return row->problem;
    };
    CHECK(problem() == RowProblem::MissingFile);

    // A file copied in while the editor runs is found once the list is read again.
    std::ofstream("Data/Sound/EditorTest/new.wav", std::ios::binary) << "RIFF";
    CHECK(problem() == RowProblem::MissingFile);
    MuEditor::SoundEditor::RefreshClientSoundFiles();
    CHECK(problem() == RowProblem::None);

    // Spelled as the game finds it: any case, either slash.
    CHECK(MuEditor::SoundEditor::IsClientSoundFile("data\\sound\\EditorTest\\NEW.wav"));
    CHECK_FALSE(MuEditor::SoundEditor::IsClientSoundFile("Data/Sound/EditorTest/gone.wav"));
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

TEST_CASE("Sound Editor: undo of a set's sound puts back only that sound and keeps what the Monster Sounds window "
          "changed since, and a sound changed there is not undone [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    REQUIRE(rows.size() == 2);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(rows[0]);
    values.file = "Data/Sound/EditorTest/Other.wav";
    REQUIRE(g_SoundEdits.Change(rows[0], values));
    // The Monster Sounds window: another sound and another event, and a new
    // sound in front.
    Data::Monsters::MonsterSoundSet other = *g_MonsterSoundSetDatabase.Find("Death Tree");
    other.sounds[1].channels = 3;
    Data::Monsters::MonsterSoundEvent attackEvent;
    attackEvent.animations = {1};
    attackEvent.sounds = {"SOUND_MONSTER_MAGICSKULL1"};
    other.soundEvents.push_back(attackEvent);
    Data::Monsters::MonsterSound added;
    added.file = "Data/Sound/EditorTest/added.wav";
    other.sounds.insert(other.sounds.begin(), added);
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", other, -1));

    REQUIRE(g_SoundEdits.Undo());
    const Data::Monsters::MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find("Death Tree");
    REQUIRE(set != nullptr);
    REQUIRE(set->sounds.size() == 3);
    CHECK(set->sounds[0] == added);
    CHECK(set->sounds[1] == DeathTree().sounds[0]);
    CHECK(set->sounds[2].channels == 3);
    CHECK(set->soundEvents[0].sounds == std::vector<std::string>{"Data/Sound/EditorTest/ok.wav"});
    CHECK(set->soundEvents.size() == 2);
    REQUIRE(g_SoundEdits.Redo());
    CHECK(g_MonsterSoundSetDatabase.Find("Death Tree")->sounds[1].file == "Data/Sound/EditorTest/Other.wav");
    CHECK(g_MonsterSoundSetDatabase.Find("Death Tree")->soundEvents[0].sounds ==
          std::vector<std::string>{"Data/Sound/EditorTest/Other.wav"});

    // The Monster Sounds window changes the step's own sound: the step stays.
    Data::Monsters::MonsterSoundSet changed = *g_MonsterSoundSetDatabase.Find("Death Tree");
    changed.sounds[1].channels = 4;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", changed, -1));
    CHECK_FALSE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.WasLastRefused());
    CHECK(g_SoundEdits.CanUndo());
    CHECK(*g_MonsterSoundSetDatabase.Find("Death Tree") == changed);
}

TEST_CASE("Sound Editor: Revert puts a set it changed back as Data/Monsters/Sounds.json has it, also when the "
          "Monster Sounds window changed it since, and takes out a set the file does not have [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    Data::Monsters::MonsterSoundSet added;
    added.name = "Added";
    added.sounds.resize(1);
    added.sounds[0].file = "Data/Sound/EditorTest/Other.wav";
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree(), added});
    // The file has Death Tree as it is, not the added set.
    const std::vector<Data::Monsters::MonsterSoundSet> saved{DeathTree()};
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(saved);
    g_SoundEdits.TakeSaved();

    // The Monster Sounds window changes Death Tree without saving, then the
    // Sound Editor names a sound of each set.
    Data::Monsters::MonsterSoundSet edited = DeathTree();
    edited.sounds[0].channels = 3;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", edited, -1));
    for (const char* key : {"set:Death Tree:Data/Sound/EditorTest/ok.wav", "set:Added:Data/Sound/EditorTest/Other.wav"})
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, key);
        REQUIRE(row != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
        values.name = "Named";
        REQUIRE(g_SoundEdits.Change(*row, values));
    }

    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Revert(issues));
    INFO(issues.size());
    const Data::Monsters::MonsterSoundSet* deathTree = g_MonsterSoundSetDatabase.Find("Death Tree");
    REQUIRE(deathTree != nullptr);
    CHECK(*deathTree == DeathTree());
    CHECK(g_MonsterSoundSetDatabase.Find("Added") == nullptr);
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a set renamed in Monster Sounds keeps this window's change under its new name, so undo, "
          "redo and Revert act on it and the old name does not come back [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const SoundRow* idle = FindRow(rows, "set:Death Tree:Data/Sound/EditorTest/ok.wav");
    REQUIRE(idle != nullptr);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*idle);
    values.name = "Named";
    REQUIRE(g_SoundEdits.Change(*idle, values));
    REQUIRE(g_MonsterEdits.RenameSoundSet("Rename the set", "Death Tree", "Tree of Death", -1));
    g_SoundEdits.FollowKeyChanges();
    CHECK(g_SoundEdits.IsModified());

    REQUIRE(g_SoundEdits.Undo());
    const Data::Monsters::MonsterSoundSet* renamed = g_MonsterSoundSetDatabase.Find("Tree of Death");
    REQUIRE(renamed != nullptr);
    CHECK(renamed->sounds[0].name.empty());
    CHECK_FALSE(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Redo());
    CHECK(g_MonsterSoundSetDatabase.Find("Tree of Death")->sounds[0].name == "Named");

    // Revert: the file's sounds, under the new name.
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(g_MonsterSoundSetDatabase.Find("Death Tree") == nullptr);
    renamed = g_MonsterSoundSetDatabase.Find("Tree of Death");
    REQUIRE(renamed != nullptr);
    Data::Monsters::MonsterSoundSet expected = DeathTree();
    expected.name = "Tree of Death";
    CHECK(*renamed == expected);
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a set renamed in Monster Sounds before this window changes it is saved into and put back "
          "from the file's set of its old name, and stays in the table under its new one [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    // The editor's frames: one before the renames and one after each; the
    // second rename keeps the file's name too.
    g_SoundEdits.FollowKeyChanges();
    REQUIRE(g_MonsterEdits.RenameSoundSet("Rename the set", "Death Tree", "Dead Tree", -1));
    g_SoundEdits.FollowKeyChanges();
    REQUIRE(g_MonsterEdits.RenameSoundSet("Rename the set", "Dead Tree", "Tree of Death", -1));
    g_SoundEdits.FollowKeyChanges();
    const auto change = [](const char* name)
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* idle = FindRow(rows, "set:Tree of Death:Data/Sound/EditorTest/ok.wav");
        REQUIRE(idle != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*idle);
        values.name = name;
        REQUIRE(g_SoundEdits.Change(*idle, values));
    };
    const auto fileSets = []
    { return Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile()).sets; };

    // Save: the sound into the file's Death Tree; the rename stays the
    // monster tools' to save.
    change("Named");
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    std::vector<Data::Monsters::MonsterSoundSet> file = fileSets();
    REQUIRE(file.size() == 1);
    CHECK(file[0].name == "Death Tree");
    CHECK(file[0].sounds[0].name == "Named");
    CHECK_FALSE(g_SoundEdits.IsModified());

    // Revert: the file's Death Tree, under the new name.
    change("Again");
    issues.clear();
    REQUIRE(g_SoundEdits.Revert(issues));
    const Data::Monsters::MonsterSoundSet* renamed = g_MonsterSoundSetDatabase.Find("Tree of Death");
    REQUIRE(renamed != nullptr);
    CHECK(renamed->sounds[0].name == "Named");
    CHECK(g_MonsterSoundSetDatabase.Find("Death Tree") == nullptr);
    CHECK_FALSE(g_SoundEdits.IsModified());

    // The monster tools write their files: the set has its new name there.
    issues.clear();
    REQUIRE(g_MonsterEdits.Save(issues));
    g_SoundEdits.FollowKeyChanges();
    change("Saved again");
    issues.clear();
    REQUIRE(g_SoundEdits.Save(issues));
    file = fileSets();
    REQUIRE(file.size() == 1);
    CHECK(file[0].name == "Tree of Death");
    CHECK(file[0].sounds[0].name == "Saved again");
}

TEST_CASE("Sound Editor: a Revert that cannot read Data/Monsters/Sounds.json says why and keeps the sets it changed "
          "not saved, and a later one puts them back [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary) << "{ \"formatVersion\": 1, \"sets\": [";
    g_SoundEdits.TakeSaved();
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const SoundRow* idle = FindRow(rows, "set:Death Tree:Data/Sound/EditorTest/ok.wav");
    REQUIRE(idle != nullptr);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*idle);
    values.name = "Named";
    REQUIRE(g_SoundEdits.Change(*idle, values));

    std::vector<Data::Items::ItemDataIssue> issues;
    CHECK_FALSE(g_SoundEdits.Revert(issues));
    CHECK(Data::Items::HasFatal(issues));
    CHECK(g_MonsterSoundSetDatabase.Find("Death Tree")->sounds[0].name == "Named");
    CHECK(g_SoundEdits.IsModified());

    // The file fixed: Revert puts the set back.
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    issues.clear();
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(*g_MonsterSoundSetDatabase.Find("Death Tree") == DeathTree());
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a Revert that cannot read the code sounds', the map object sounds' or the sound events' file "
          "keeps that table's changes not saved, and a later one reads it [sounds][editor]")
{
    EditorScope scope;
    const std::vector<Data::Sounds::SoundEvent> previousEvents = g_SoundEventDatabase.GetAll();
    const std::vector<SoundFile> code{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav")};
    const std::vector<Data::Maps::MapObjectSound> objects{Waterfall()};
    Data::Sounds::SoundEvent click;
    click.event = "characterSelected";
    Data::Sounds::SoundEventRule rule;
    MuEditor::SoundEditor::AddPlay(rule, "SOUND_CLICK01");
    click.rules = {rule};
    const std::vector<Data::Sounds::SoundEvent> events{click};
    g_SoundFileDatabase.Build(code);
    g_MapObjectSoundDatabase.Build(objects);
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_SoundEventDatabase.Build(events);
    std::filesystem::create_directories("Data/Audio");
    std::filesystem::create_directories("Data/Maps");
    g_SoundEdits.TakeSaved();
    const auto changeRow = [](const char* key, const char* name)
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, key);
        REQUIRE(row != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
        values.name = name;
        REQUIRE(g_SoundEdits.Change(*row, values));
    };

    struct Case
    {
        const char* file;
        std::string text;
        std::function<void()> change;
        std::function<bool()> changed;
    };
    const std::vector<Case> cases{
        {"Data/Audio/SoundFiles.json", Data::Sounds::WriteSoundFilesJson(code),
         [&] { changeRow("SOUND_CLICK01", "Click"); }, [] { return g_SoundFileDatabase.GetAll()[0].name == "Click"; }},
        {"Data/Maps/ObjectSounds.json", Data::Maps::WriteMapObjectSoundsJson(objects),
         [&] { changeRow("objects:Data/Sound/EditorTest/ok.wav", "Kalima waterfall"); },
         [] { return g_MapObjectSoundDatabase.GetAll()[0].name == "Kalima waterfall"; }},
        {"Data/Audio/SoundEvents.json", Data::Sounds::WriteSoundEventsJson(events),
         [&]
         {
             Data::Sounds::SoundEvent after = click;
             REQUIRE(MuEditor::SoundEditor::AddPlay(after.rules[0], "SOUND_INTERFACE01"));
             REQUIRE(g_SoundEdits.ChangeEvents("Sound event", std::vector<Data::Sounds::SoundEvent>{after}));
         },
         [] { return g_SoundEventDatabase.GetAll()[0].rules[0].plays.size() == 2; }},
    };
    for (const Case& entry : cases)
    {
        CAPTURE(entry.file);
        entry.change();
        std::ofstream(entry.file, std::ios::binary) << "{ \"formatVersion\": 1, \"broken\": [";
        std::vector<Data::Items::ItemDataIssue> issues;
        CHECK_FALSE(g_SoundEdits.Revert(issues));
        CHECK(Data::Items::HasFatal(issues));
        CHECK(entry.changed());
        CHECK(g_SoundEdits.IsModified());

        // The file fixed: Revert reads it.
        std::ofstream(entry.file, std::ios::binary) << entry.text;
        issues.clear();
        REQUIRE(g_SoundEdits.Revert(issues));
        CHECK_FALSE(entry.changed());
        CHECK_FALSE(g_SoundEdits.IsModified());
    }
    g_SoundEventDatabase.Build(previousEvents);
}

TEST_CASE("Sound Editor: Save writes the sets it changed into Data/Monsters/Sounds.json alone, and the monster "
          "tools' other changes stay unsaved [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    Data::Monsters::MonsterSoundSet other;
    other.name = "Other";
    other.sounds.resize(1);
    other.sounds[0].file = "Data/Sound/EditorTest/Other.wav";
    const std::vector<Data::Monsters::MonsterSoundSet> sets{DeathTree(), other};
    g_MonsterSoundSetDatabase.Build(sets);
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(sets);
    g_SoundEdits.TakeSaved();

    // The Monster Sounds window changes the other set without saving, then
    // the Sound Editor names a sound of Death Tree and saves.
    Data::Monsters::MonsterSoundSet edited = other;
    edited.sounds[0].channels = 3;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", edited, -1));
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const SoundRow* idle = FindRow(rows, "set:Death Tree:Data/Sound/EditorTest/ok.wav");
    REQUIRE(idle != nullptr);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*idle);
    values.name = "Death Tree idle";
    REQUIRE(g_SoundEdits.Change(*idle, values));
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK_FALSE(g_SoundEdits.IsModified());

    const Data::Monsters::MonsterSoundsLoadResult file =
        Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile());
    REQUIRE(file.sets.size() == 2);
    CHECK(file.sets[0] == *g_MonsterSoundSetDatabase.Find("Death Tree"));
    CHECK(file.sets[0].sounds[0].name == "Death Tree idle");
    CHECK(file.sets[1] == other);
    // The monster list and the model list are not written, and the monster
    // tools still have their change to save.
    CHECK_FALSE(std::filesystem::exists(Data::Monsters::GetMonstersFile()));
    CHECK_FALSE(std::filesystem::exists(Data::Monsters::GetMonsterModelsFile()));
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
}

TEST_CASE("Sound Editor: Save writes only the sounds it changed into a set, not the Monster Sounds window's unsaved "
          "changes of the same set, and leaves a sound changed there too to that window [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    std::filesystem::create_directories("Data/Monsters");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    const auto change = [](const char* key, const char* file)
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, key);
        REQUIRE(row != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
        values.file = file;
        REQUIRE(g_SoundEdits.Change(*row, values));
    };
    const auto fileSet = []
    {
        const std::vector<Data::Monsters::MonsterSoundSet> sets =
            Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile()).sets;
        REQUIRE(sets.size() == 1);
        return sets[0];
    };

    // Monster Sounds changes the event and the attack's channels, then this
    // window gives the idle sound another file.
    Data::Monsters::MonsterSoundSet edited = DeathTree();
    edited.soundEvents[0].chance = Data::Monsters::MonsterSoundChance{4};
    edited.sounds[1].channels = 3;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", edited, -1));
    change("set:Death Tree:Data/Sound/EditorTest/ok.wav", "Data/Sound/EditorTest/Other.wav");
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    Data::Monsters::MonsterSoundSet expected = DeathTree();
    expected.sounds[0].file = "Data/Sound/EditorTest/Other.wav";
    expected.soundEvents[0].sounds = {"Data/Sound/EditorTest/Other.wav"};
    CHECK(fileSet() == expected);
    CHECK_FALSE(g_SoundEdits.IsModified());
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());

    // The attack, which Monster Sounds changed and has not saved: Save leaves
    // it to that window and says so; the window still says Not saved yet.
    change("set:Death Tree:SOUND_MONSTER_MAGICSKULL1", "Data/Sound/EditorTest/ok.wav");
    issues.clear();
    CHECK_FALSE(g_SoundEdits.Save(issues));
    CHECK(Data::Items::HasFatal(issues));
    CHECK(fileSet() == expected);
    CHECK(g_SoundEdits.IsModified());

    // The monster tools save: the attack is in the file with this window's
    // file, and nothing is left to save here.
    issues.clear();
    REQUIRE(g_MonsterEdits.Save(issues));
    g_SoundEdits.FollowKeyChanges();
    CHECK(fileSet().sounds[1].file == "Data/Sound/EditorTest/ok.wav");
    CHECK_FALSE(g_SoundEdits.IsModified());

    // Monster Sounds changes a sound this window changed since: that window's
    // to save, so this window writes nothing of it.
    change("set:Death Tree:Data/Sound/EditorTest/Other.wav", "Data/Sound/EditorTest/ok.wav");
    Data::Monsters::MonsterSoundSet over = *g_MonsterSoundSetDatabase.Find("Death Tree");
    over.sounds[0].channels = 1;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", over, -1));
    const Data::Monsters::MonsterSoundSet before = fileSet();
    issues.clear();
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK(fileSet() == before);
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: undo of a map object sound puts back only that entry and keeps what the Map Editor's Sounds "
          "tab changed since, and an entry changed there is not undone [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    Data::Maps::MapObjectSound river = Waterfall();
    river.objects = {40};
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall(), river});
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const auto second = std::find_if(rows.begin(), rows.end(), [](const SoundRow& row) { return row.index == 1; });
    REQUIRE(second != rows.end());
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*second);
    values.channels = 1;
    REQUIRE(g_SoundEdits.Change(*second, values));
    // The Sounds tab: another entry changes, the first goes, one is added.
    std::vector<Data::Maps::MapObjectSound> sounds = g_MapObjectSoundDatabase.GetAll();
    sounds[0].channels = 2;
    Data::Maps::MapObjectSound added = Waterfall();
    added.objects = {50};
    sounds.push_back(added);
    sounds.erase(sounds.begin());
    g_MapObjectSoundDatabase.Build(sounds);

    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_MapObjectSoundDatabase.GetAll() == std::vector<Data::Maps::MapObjectSound>{river, added});
    REQUIRE(g_SoundEdits.Redo());
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].channels == 1);

    sounds = g_MapObjectSoundDatabase.GetAll();
    sounds[0].file = "Data/Sound/EditorTest/ok.wav";
    sounds[0].channels = 4;
    g_MapObjectSoundDatabase.Build(sounds);
    CHECK_FALSE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.WasLastRefused());
    CHECK(g_MapObjectSoundDatabase.GetAll() == sounds);
}

TEST_CASE("Sound Editor: a code sound's stored type is set and cleared in its row and undone, and other rows have "
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

TEST_CASE("Sound Editor: a map object sound's name, file and channels change its table, which the map follows, and it "
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

TEST_CASE("Sound Editor: names are read and written in the sets' sounds and the map object sounds, and without one the "
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

// Linux: the game opens "Data/Sound/..." in a client folder spelled
// "data/SOUND" (MuResolvePath); the list and the rows' check find it too.
TEST_CASE("Sound Editor: the client folder's list reads Data/Sound spelled in another case, as the game does "
          "[sounds][editor]")
{
    const std::filesystem::path previous = std::filesystem::current_path();
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_sound_editor_case";
    std::error_code error;
    std::filesystem::remove_all(folder, error);
    std::filesystem::create_directories(folder / "data" / "SOUND" / "Case");
    std::ofstream(folder / "data" / "SOUND" / "Case" / "Wave.wav", std::ios::binary) << "RIFF";
    std::filesystem::current_path(folder);
    const std::vector<std::string> files = MuEditor::SoundEditor::ListSoundFiles("Data/Sound");
    MuEditor::SoundEditor::RefreshClientSoundFiles();
    const bool found = MuEditor::SoundEditor::IsClientSoundFile("Data/Sound/Case/Wave.wav");
    std::filesystem::current_path(previous);
    MuEditor::SoundEditor::RefreshClientSoundFiles();
    std::filesystem::remove_all(folder, error);
    CHECK(files == std::vector<std::string>{"Data/Sound/Case/Wave.wav"});
    CHECK(found);
}

namespace
{
using Data::Sounds::SoundFolder;
using MuEditor::SoundEditor::FolderPath;

SoundFolder Folder(std::string name, std::vector<std::string> sounds = {}, std::vector<SoundFolder> folders = {})
{
    return SoundFolder{std::move(name), std::move(folders), std::move(sounds)};
}

std::vector<SoundFolder> ReadFolders(const std::string& text, std::vector<Data::Items::ItemDataIssue>& issues)
{
    std::vector<SoundFolder> folders;
    Data::Sounds::ReadSoundFoldersJson(text, "SoundFolders.json", folders, issues);
    return folders;
}

bool HasIssue(const std::vector<Data::Items::ItemDataIssue>& issues, Data::Items::ItemDataIssueSeverity severity,
              const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(), [&](const Data::Items::ItemDataIssue& issue)
                       { return issue.severity == severity && issue.field == field; });
}

std::string FileText(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string text = buffer.str();
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    return text;
}
} // namespace

TEST_CASE("Sound Editor: the folders' file reads folders in folders with their sounds and writes them back, and a name "
          "twice among siblings is fatal, a sound twice stays in its first folder (Q-SE4) [sounds][editor]")
{
    const std::vector<SoundFolder> folders{
        Folder("Monsters", {}, {Folder("Death Tree", {"set:Death Tree:Data/Sound/w34/dt_idle1.wav"})}),
        Folder("Interface", {"SOUND_CLICK01", "SOUND_INTERFACE01"})};
    const std::string text = Data::Sounds::WriteSoundFoldersJson(folders);
    std::vector<Data::Items::ItemDataIssue> issues;
    CHECK(ReadFolders(text, issues) == folders);
    CHECK(issues.empty());
    // No empty lists are written.
    CHECK(text.find("\"folders\": []") == std::string::npos);
    CHECK(text.find("\"sounds\": []") == std::string::npos);
    CHECK(*Data::Sounds::FindSoundFolder(folders, "set:Death Tree:Data/Sound/w34/dt_idle1.wav") ==
          std::vector<std::string>{"Monsters", "Death Tree"});
    CHECK(*Data::Sounds::FindSoundFolder(folders, "SOUND_INTERFACE01") == std::vector<std::string>{"Interface"});
    CHECK_FALSE(Data::Sounds::FindSoundFolder(folders, "SOUND_RAIN01").has_value());

    issues.clear();
    CHECK(ReadFolders(R"({"formatVersion": 1, "folders": [{"name": "A"}, {"name": "A"}]})", issues).empty());
    CHECK(HasIssue(issues, Data::Items::ItemDataIssueSeverity::Fatal, "folders[1].name"));
    issues.clear();
    CHECK(ReadFolders(R"({"formatVersion": 1, "folders": [{"name": ""}]})", issues).empty());
    CHECK(HasIssue(issues, Data::Items::ItemDataIssueSeverity::Fatal, "folders[0].name"));
    issues.clear();
    const std::vector<SoundFolder> twice =
        ReadFolders(R"({"formatVersion": 1, "folders": [{"name": "A", "sounds": ["SOUND_CLICK01"]},)"
                    R"( {"name": "B", "sounds": ["SOUND_CLICK01", "SOUND_RAIN01"], "colour": 1}]})",
                    issues);
    REQUIRE(twice.size() == 2);
    CHECK(twice[0].sounds == std::vector<std::string>{"SOUND_CLICK01"});
    CHECK(twice[1].sounds == std::vector<std::string>{"SOUND_RAIN01"});
    CHECK(HasIssue(issues, Data::Items::ItemDataIssueSeverity::Warning, "folders[1].sounds[0]"));
    CHECK(HasIssue(issues, Data::Items::ItemDataIssueSeverity::Warning, "folders[1].colour"));
    CHECK_FALSE(Data::Items::HasFatal(issues));
}

TEST_CASE("Sound Editor: folders are added with a free name, renamed, removed with their contents moving up, moved "
          "among their siblings, and sounds moved between them (Q-SE4) [sounds][editor]")
{
    using namespace MuEditor::SoundEditor;
    std::vector<SoundFolder> folders{Folder("Monsters", {"SOUND_A"}, {Folder("Aida", {"SOUND_B"}), Folder("Old")}),
                                     Folder("Old", {"SOUND_C"})};
    CHECK(FindFolder(folders, {"Monsters", "Aida"})->sounds == std::vector<std::string>{"SOUND_B"});
    CHECK(FindFolder(folders, {"Monsters", "None"}) == nullptr);
    CHECK(FindFolder(folders, {}) == nullptr);

    CHECK(AddFolder(folders, {}, "New folder") == FolderPath{"New folder"});
    CHECK(AddFolder(folders, {}, "New folder") == FolderPath{"New folder 2"});
    CHECK(FreeFolderName(folders, "New folder 2") == "New folder 3");
    CHECK(AddFolder(folders, {"Monsters"}, "Old") == FolderPath{"Monsters", "Old 2"});
    CHECK(AddFolder(folders, {"Nowhere"}, "X").empty());

    CHECK(RenameFolder(folders, {"New folder 2"}, "Maps"));
    CHECK(FindFolder(folders, {"Maps"}) != nullptr);
    CHECK_FALSE(RenameFolder(folders, {"Maps"}, "Monsters"));
    CHECK_FALSE(RenameFolder(folders, {"Maps"}, ""));
    CHECK_FALSE(RenameFolder(folders, {"Nowhere"}, "X"));

    // Removing Monsters: its folders take its place, a name its new siblings
    // have gets a free one; its sounds are not sorted then (it was at the top).
    REQUIRE(RemoveFolder(folders, {"Monsters"}));
    REQUIRE(folders.size() == 6);
    CHECK(folders[0].name == "Aida");
    CHECK(folders[1].name == "Old 2");
    CHECK(folders[2].name == "Old 3");
    CHECK(folders[3].name == "Old");
    CHECK_FALSE(Data::Sounds::FindSoundFolder(folders, "SOUND_A").has_value());
    // A folder below the top: its sounds go to its parent.
    CHECK(MoveSound(folders, "SOUND_D", {"Maps"}));
    AddFolder(folders, {"Maps"}, "Kalima");
    CHECK(MoveSound(folders, "SOUND_D", {"Maps", "Kalima"}));
    REQUIRE(RemoveFolder(folders, {"Maps", "Kalima"}));
    CHECK(FindFolder(folders, {"Maps"})->sounds == std::vector<std::string>{"SOUND_D"});

    CHECK(MoveFolder(folders, {"Aida"}, 1));
    CHECK(folders[1].name == "Aida");
    CHECK_FALSE(MoveFolder(folders, {"Aida"}, -2));
    CHECK_FALSE(MoveFolder(folders, {folders.back().name}, 1));

    CHECK(MoveSound(folders, "SOUND_B", {"Old"}));
    CHECK(*Data::Sounds::FindSoundFolder(folders, "SOUND_B") == std::vector<std::string>{"Old"});
    CHECK(FindFolder(folders, {"Aida"})->sounds.empty());
    CHECK_FALSE(MoveSound(folders, "SOUND_B", {"Old"}));
    CHECK_FALSE(MoveSound(folders, "SOUND_B", {"Nowhere"}));
    CHECK(MoveSound(folders, "SOUND_B", {}));
    CHECK_FALSE(Data::Sounds::FindSoundFolder(folders, "SOUND_B").has_value());
    CHECK_FALSE(MoveSound(folders, "SOUND_B", {}));

    CHECK(DropStaleSounds(folders, {"SOUND_C"}) == 1);
    CHECK(*Data::Sounds::FindSoundFolder(folders, "SOUND_C") == std::vector<std::string>{"Old"});
    CHECK_FALSE(Data::Sounds::FindSoundFolder(folders, "SOUND_D").has_value());
}

TEST_CASE("Sound Editor: a change of the folders is a step of the history, Save writes their file and Revert reads "
          "it again (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    // No file yet: no folders.
    CHECK(g_SoundEdits.Folders().empty());
    g_SoundEdits.TakeSaved();
    std::vector<SoundFolder> after{Folder("Interface", {"SOUND_CLICK01"})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", after));
    CHECK_FALSE(g_SoundEdits.ChangeFolders("Sound folders", after));
    CHECK(g_SoundEdits.Folders() == after);
    CHECK(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.Folders().empty());
    CHECK_FALSE(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Redo());
    std::filesystem::create_directories("Data/Audio");
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK_FALSE(g_SoundEdits.IsModified());
    const Data::Sounds::SoundFoldersLoadResult saved =
        Data::Sounds::LoadSoundFoldersFile(Data::Sounds::GetSoundFoldersFile());
    CHECK(saved.folders == after);
    CHECK(saved.issues.empty());
    // Changed, then read again from the file.
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", std::vector<SoundFolder>{}));
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(g_SoundEdits.Folders() == after);
    CHECK_FALSE(g_SoundEdits.CanUndo());
}

TEST_CASE("Sound Editor: two rows with the same key, map object sounds of one file, get keys of their own, so a "
          "folder holds each and the window tells them apart (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    Data::Maps::MapObjectSound other = Waterfall();
    other.maps = {{30}};
    Data::Maps::MapObjectSound third = Waterfall();
    third.maps = {{31}};
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall(), other, third});
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    REQUIRE(rows.size() == 3);
    CHECK(rows[0].key == "objects:Data/Sound/EditorTest/ok.wav");
    CHECK(rows[1].key == "objects:Data/Sound/EditorTest/ok.wav#2");
    CHECK(rows[2].key == "objects:Data/Sound/EditorTest/ok.wav#3");
    CHECK(rows[2].index == 2);

    // The first one's file changes: the others' keys move up, and the
    // folders follow every one.
    const std::vector<SoundFolder> folders{Folder("Kalima", {rows[0].key}), Folder("Map 30", {rows[1].key}),
                                           Folder("Map 31", {rows[2].key})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", folders));
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(rows[0]);
    values.file = "Data/Sound/EditorTest/Other.wav";
    REQUIRE(g_SoundEdits.Change(rows[0], values));
    CHECK(g_SoundEdits.Folders() ==
          std::vector<SoundFolder>{Folder("Kalima", {"objects:Data/Sound/EditorTest/Other.wav"}),
                                   Folder("Map 30", {"objects:Data/Sound/EditorTest/ok.wav"}),
                                   Folder("Map 31", {"objects:Data/Sound/EditorTest/ok.wav#2"})});

    TestImGuiContext context;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(900.0f, 400.0f));
    bool open = true;
    g_MuSoundEditorUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

TEST_CASE("Sound Editor: a sound whose file changes stays in its folder under its new key, and undo and redo take "
          "the key back and forth (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall()});
    g_SoundEdits.TakeSaved();
    const std::vector<SoundFolder> folders{Folder("Death Tree", {"set:Death Tree:Data/Sound/EditorTest/ok.wav"}),
                                           Folder("Map objects", {"objects:Data/Sound/EditorTest/ok.wav"})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", folders));
    for (const char* key : {"set:Death Tree:Data/Sound/EditorTest/ok.wav", "objects:Data/Sound/EditorTest/ok.wav"})
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, key);
        REQUIRE(row != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
        values.file = "Data/Sound/EditorTest/Other.wav";
        REQUIRE(g_SoundEdits.Change(*row, values));
    }
    const std::vector<SoundFolder> moved{Folder("Death Tree", {"set:Death Tree:Data/Sound/EditorTest/Other.wav"}),
                                         Folder("Map objects", {"objects:Data/Sound/EditorTest/Other.wav"})};
    CHECK(g_SoundEdits.Folders() == moved);
    // A set's sound with an id keeps its key.
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const SoundRow* attack = FindRow(rows, "set:Death Tree:SOUND_MONSTER_MAGICSKULL1");
    REQUIRE(attack != nullptr);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*attack);
    values.file = "Data/Sound/EditorTest/Other.wav";
    REQUIRE(g_SoundEdits.Change(*attack, values));
    CHECK(g_SoundEdits.Folders() == moved);
    REQUIRE(g_SoundEdits.Undo());
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.Folders() ==
          std::vector<SoundFolder>{Folder("Death Tree", {"set:Death Tree:Data/Sound/EditorTest/Other.wav"}),
                                   Folder("Map objects", {"objects:Data/Sound/EditorTest/ok.wav"})});
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.Folders() == folders);
    REQUIRE(g_SoundEdits.Redo());
    REQUIRE(g_SoundEdits.Redo());
    CHECK(g_SoundEdits.Folders() == moved);
}

TEST_CASE("Sound Editor: the keys follow a set renamed among others in the file's order and a sound changed in its "
          "place, and a known name finds a set renamed and changed (Q-SE4) [sounds][editor]")
{
    using MuEditor::SoundEditor::KeyedTables;
    const auto set = [](const char* name, const char* file)
    {
        Data::Monsters::MonsterSoundSet made;
        made.name = name;
        made.sounds.resize(1);
        made.sounds[0].file = file;
        return made;
    };
    const KeyedTables before{{set("B", "b.wav"), set("A", "a.wav"), set("D", "d.wav")}, {}};
    KeyedTables after{{set("D", "d.wav"), set("C", "a.wav"), set("B", "b2.wav")}, {}};
    CHECK(MuEditor::SoundEditor::RenamedSets(before, after) == std::map<std::string, std::string>{{"A", "C"}});
    CHECK(MuEditor::SoundEditor::FollowedKeys(before, after) ==
          std::map<std::string, std::string>{{"set:A:a.wav", "set:C:a.wav"}, {"set:B:b.wav", "set:B:b2.wav"}});

    // Renamed and changed: alike no longer, found by its known name.
    after.sets[1].sounds[0].channels = 2;
    CHECK(MuEditor::SoundEditor::RenamedSets(before, after).empty());
    CHECK(MuEditor::SoundEditor::FollowedKeys(before, after, {{"A", "C"}}) ==
          std::map<std::string, std::string>{{"set:A:a.wav", "set:C:a.wav"}, {"set:B:b.wav", "set:B:b2.wav"}});
}

// The editor calls FollowKeyChanges each frame, after the other windows.
TEST_CASE("Sound Editor: the folders keep the sounds whose keys another window changes, a set renamed in Monster "
          "Sounds, an entry removed or given another file in the Map Editor's Sounds tab, also in the history and "
          "after Revert (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    Data::Maps::MapObjectSound other = Waterfall();
    other.maps = {{30}};
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall(), other});
    g_SoundEdits.TakeSaved();
    const std::string idle = "set:Death Tree:Data/Sound/EditorTest/ok.wav";
    const std::string attack = "set:Death Tree:SOUND_MONSTER_MAGICSKULL1";
    const std::string first = "objects:Data/Sound/EditorTest/ok.wav";
    const std::string picked = "objects:Data/Sound/EditorTest/Other.wav";
    const std::vector<SoundFolder> folders{Folder("Death Tree", {idle, attack}), Folder("Kalima", {first}),
                                           Folder("Map 30", {first + "#2"})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", folders));
    std::filesystem::create_directories("Data/Audio");
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));

    // Monster Sounds renames the set: its sounds keep their folder; the
    // folders' file names them as the sets' file does, so nothing is to be
    // saved. Revert reads the file's folders and follows it too.
    REQUIRE(g_MonsterEdits.RenameSoundSet("Rename the set", "Death Tree", "Tree of Death", -1));
    g_SoundEdits.FollowKeyChanges();
    const std::vector<SoundFolder> renamed{Folder("Death Tree", {"set:Tree of Death:Data/Sound/EditorTest/ok.wav",
                                                                 "set:Tree of Death:SOUND_MONSTER_MAGICSKULL1"}),
                                           Folder("Kalima", {first}), Folder("Map 30", {first + "#2"})};
    CHECK(g_SoundEdits.Folders() == renamed);
    CHECK_FALSE(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(g_SoundEdits.Folders() == renamed);
    // Its undo there takes the keys back.
    REQUIRE(g_MonsterEdits.Undo());
    g_SoundEdits.FollowKeyChanges();
    CHECK(g_SoundEdits.Folders() == folders);
    CHECK_FALSE(g_SoundEdits.IsModified());

    // The Sounds tab removes the first entry: the second takes its key ("#2"
    // moves), and the first's entry goes.
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{other});
    g_SoundEdits.FollowKeyChanges();
    CHECK(g_SoundEdits.Folders() ==
          std::vector<SoundFolder>{Folder("Death Tree", {idle, attack}), Folder("Kalima"), Folder("Map 30", {first})});

    // A file that a gone entry names (here in Kalima) is picked in the Sounds
    // tab: the sound stays in its own folder only, also in the history.
    std::vector<SoundFolder> withGone = g_SoundEdits.Folders();
    withGone[1].sounds = {picked};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", withGone));
    other.file = "Data/Sound/EditorTest/Other.wav";
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{other});
    g_SoundEdits.FollowKeyChanges();
    const std::vector<SoundFolder> followed{Folder("Death Tree", {idle, attack}), Folder("Kalima"),
                                            Folder("Map 30", {picked})};
    CHECK(g_SoundEdits.Folders() == followed);
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.Folders() == followed);
}

TEST_CASE("Sound Editor: Save writes the folders with the keys the saved files give the sounds, so a set renamed in "
          "Monster Sounds keeps its folders in the files until that window saves it (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    std::filesystem::create_directories("Data/Monsters");
    std::filesystem::create_directories("Data/Audio");
    std::ofstream(Data::Monsters::GetMonsterSoundsFile(), std::ios::binary)
        << Data::Monsters::WriteMonsterSoundsJson(std::vector<Data::Monsters::MonsterSoundSet>{DeathTree()});
    g_SoundEdits.TakeSaved();
    g_SoundEdits.FollowKeyChanges();
    const std::string idle = "set:Death Tree:Data/Sound/EditorTest/ok.wav";
    const std::string attack = "set:Death Tree:SOUND_MONSTER_MAGICSKULL1";
    const std::vector<SoundFolder> saved{Folder("Death Tree", {idle, attack})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", saved));
    const auto folderFile = []
    { return Data::Sounds::LoadSoundFoldersFile(Data::Sounds::GetSoundFoldersFile()).folders; };

    // Monster Sounds renames the set and changes it, and saves neither.
    REQUIRE(g_MonsterEdits.RenameSoundSet("Rename the set", "Death Tree", "Tree of Death", -1));
    g_SoundEdits.FollowKeyChanges();
    Data::Monsters::MonsterSoundSet edited = *g_MonsterSoundSetDatabase.Find("Tree of Death");
    edited.sounds[0].channels = 4;
    REQUIRE(g_MonsterEdits.SetSoundSet("Monster Sounds", edited, -1));
    g_SoundEdits.FollowKeyChanges();
    const std::vector<SoundFolder> renamed{Folder("Death Tree", {"set:Tree of Death:Data/Sound/EditorTest/ok.wav",
                                                                 "set:Tree of Death:SOUND_MONSTER_MAGICSKULL1"})};
    CHECK(g_SoundEdits.Folders() == renamed);

    // The folders' file names the sounds as Sounds.json does, which the
    // editor reads when it starts again; the window keeps the names of now,
    // also after Revert.
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK(folderFile() == saved);
    std::vector<std::string> fileKeys;
    for (const SoundRow& row : MuEditor::SoundEditor::KeyRows(
             Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile()).sets, {}))
    {
        fileKeys.push_back(row.key);
    }
    CHECK(fileKeys == std::vector<std::string>{idle, attack});
    CHECK(g_SoundEdits.Folders() == renamed);
    CHECK_FALSE(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(g_SoundEdits.Folders() == renamed);
    CHECK_FALSE(g_SoundEdits.IsModified());

    // The monster tools save the set under its new name: the folders' file is
    // not saved until it has the new keys too.
    issues.clear();
    REQUIRE(g_MonsterEdits.Save(issues));
    g_SoundEdits.FollowKeyChanges();
    CHECK(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK(folderFile() == renamed);
    CHECK_FALSE(g_SoundEdits.IsModified());

    // This window gives a sound another file: Save writes the sets' file and
    // the folders' file with the sound's new key.
    const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
    const SoundRow* row = FindRow(rows, "set:Tree of Death:Data/Sound/EditorTest/ok.wav");
    REQUIRE(row != nullptr);
    MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
    values.file = "Data/Sound/EditorTest/Other.wav";
    REQUIRE(g_SoundEdits.Change(*row, values));
    issues.clear();
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK(folderFile() ==
          std::vector<SoundFolder>{Folder("Death Tree", {"set:Tree of Death:Data/Sound/EditorTest/Other.wav",
                                                         "set:Tree of Death:SOUND_MONSTER_MAGICSKULL1"})});
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a Save or Revert in the Map Editor's Sounds tab takes the map object sounds as saved here "
          "too, and a file saved there with a sound's new key makes the folders' file not saved (Q-SE4) "
          "[sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall()});
    std::filesystem::create_directories("Data/Maps");
    std::filesystem::create_directories("Data/Audio");
    g_SoundEdits.TakeSaved();
    g_MapSoundsTab.TakeSaved();
    const std::string first = "objects:Data/Sound/EditorTest/ok.wav";
    const auto rename = [&first](const char* name)
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, first);
        REQUIRE(row != nullptr);
        MuEditor::SoundEditor::RowValues values = MuEditor::SoundEditor::ValuesOf(*row);
        values.name = name;
        REQUIRE(g_SoundEdits.Change(*row, values));
    };

    rename("Kalima waterfall");
    CHECK(g_SoundEdits.IsModified());
    CHECK(g_MapSoundsTab.IsModified());
    g_MapSoundsTab.Save();
    CHECK_FALSE(g_MapSoundsTab.IsModified());
    CHECK_FALSE(g_SoundEdits.IsModified());

    rename("Other");
    g_MapSoundsTab.Revert();
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].name == "Kalima waterfall");
    CHECK_FALSE(g_SoundEdits.IsModified());
    // The file changed by hand: the tab's Revert reads it, and the Sound
    // Editor takes it as saved too.
    std::vector<Data::Maps::MapObjectSound> byHand = g_MapObjectSoundDatabase.GetAll();
    byHand[0].name = "By hand";
    std::ofstream("Data/Maps/ObjectSounds.json", std::ios::binary) << Data::Maps::WriteMapObjectSoundsJson(byHand);
    g_MapSoundsTab.Revert();
    CHECK(g_MapObjectSoundDatabase.GetAll()[0].name == "By hand");
    CHECK_FALSE(g_SoundEdits.IsModified());

    // A folder holds the entry; the tab gives it another file and saves.
    const std::vector<SoundFolder> folders{Folder("Kalima", {first})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", folders));
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    std::vector<Data::Maps::MapObjectSound> picked = g_MapObjectSoundDatabase.GetAll();
    picked[0].file = "Data/Sound/EditorTest/Other.wav";
    g_MapObjectSoundDatabase.Build(picked);
    g_SoundEdits.FollowKeyChanges();
    // The table differs from its file, here too; once the tab saved it, the
    // folders' file still has the entry's old key.
    CHECK(g_SoundEdits.IsModified());
    g_MapSoundsTab.Save();
    CHECK(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK(Data::Sounds::LoadSoundFoldersFile(Data::Sounds::GetSoundFoldersFile()).folders ==
          std::vector<SoundFolder>{Folder("Kalima", {"objects:Data/Sound/EditorTest/Other.wav"})});
    CHECK_FALSE(g_SoundEdits.IsModified());
}

TEST_CASE("Sound Editor: a folders' file that cannot be read keeps the folders as they are, so Save leaves the file "
          "alone, and Revert reads it again once it is fixed (Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    std::filesystem::create_directories("Data/Audio");
    const std::string broken = "{ \"formatVersion\": 1, \"folders\": [ { \"name\": \"Interface\", \"sounds\": [";
    std::ofstream(Data::Sounds::GetSoundFoldersFile(), std::ios::binary) << broken;
    CHECK(g_SoundEdits.Folders().empty());
    CHECK_FALSE(g_SoundEdits.FolderProblems().empty());
    CHECK_FALSE(g_SoundEdits.ChangeFolders("Sound folders", {Folder("Interface", {"SOUND_CLICK01"})}));
    CHECK_FALSE(g_SoundEdits.IsModified());
    std::vector<Data::Items::ItemDataIssue> issues;
    CHECK(g_SoundEdits.Save(issues));
    std::stringstream text;
    text << std::ifstream(Data::Sounds::GetSoundFoldersFile(), std::ios::binary).rdbuf();
    CHECK(text.str() == broken);
    // The window says so.
    {
        TestImGuiContext context;
        ImGui::NewFrame();
        bool open = true;
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    }

    const std::vector<SoundFolder> fixed{Folder("Interface", {"SOUND_CLICK01"})};
    REQUIRE(Data::Sounds::SaveSoundFolders(Data::Sounds::GetSoundFoldersFile(), fixed, issues));
    REQUIRE(g_SoundEdits.Revert(issues));
    CHECK(g_SoundEdits.FolderProblems().empty());
    CHECK(g_SoundEdits.Folders() == fixed);
    CHECK(g_SoundEdits.ChangeFolders("Sound folders", std::vector<SoundFolder>{}));
}

TEST_CASE("Sound Editor: the shipped folders hold every sound of the shipped files once, by their keys, and write "
          "back the same (Q-SE4) [sounds][editor]")
{
    const std::string dir = MU_TEST_DATA_DIR;
    const std::string text = FileText(dir + "/Audio/SoundFolders.json");
    std::vector<Data::Items::ItemDataIssue> issues;
    const std::vector<SoundFolder> folders = ReadFolders(text, issues);
    CHECK(issues.empty());
    CHECK(Data::Sounds::WriteSoundFoldersJson(folders) == text);

    std::set<std::string> keys;
    std::vector<Data::Sounds::SoundFile> code;
    Data::Sounds::ReadSoundFilesJson(FileText(dir + "/Audio/SoundFiles.json"), "SoundFiles.json", code, issues);
    for (const Data::Sounds::SoundFile& sound : code)
    {
        keys.insert(sound.id);
    }
    std::vector<Data::Monsters::MonsterSoundSet> sets;
    Data::Monsters::ReadMonsterSoundsJson(FileText(dir + "/Monsters/Sounds.json"), "Sounds.json", sets, issues);
    for (const Data::Monsters::MonsterSoundSet& set : sets)
    {
        for (const Data::Monsters::MonsterSound& sound : set.sounds)
        {
            keys.insert("set:" + set.name + ":" + Data::Monsters::SoundNameOf(sound));
        }
    }
    std::vector<Data::Maps::MapObjectSound> objects;
    Data::Maps::ReadMapObjectSoundsJson(FileText(dir + "/Maps/ObjectSounds.json"), "ObjectSounds.json", objects,
                                        issues);
    for (const Data::Maps::MapObjectSound& sound : objects)
    {
        keys.insert("objects:" + sound.file);
    }
    CHECK_FALSE(Data::Items::HasFatal(issues));
    REQUIRE(keys.size() > 900);
    size_t held = 0;
    for (const std::string& key : keys)
    {
        INFO(key);
        CHECK(Data::Sounds::FindSoundFolder(folders, key).has_value());
        held += Data::Sounds::FindSoundFolder(folders, key) ? 1 : 0;
    }
    CHECK(held == keys.size());
    // Death Tree's sounds in its own folder, by their ids.
    CHECK(Data::Sounds::FindSoundFolder(folders, "set:Death Tree:SOUND_AIDA_DEATHTREE_MOVE1") ==
          std::vector<std::string>{"Monsters", "Death Tree"});
}

TEST_CASE("Sound Editor: the window draws the folders as a tree, the sounds no folder holds under Not sorted, and "
          "opens the folders of the selected sound (Q-SE3, Q-SE4) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav", "Button click"),
                               Code("SOUND_RAIN01", SOUND_RAIN01, "Data/Sound/EditorTest/ok.wav")});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    REQUIRE(g_SoundEdits.ChangeFolders(
        "Sound folders", std::vector<SoundFolder>{Folder("Game", {}, {Folder("Interface", {"SOUND_CLICK01"})})}));
    MuEditor::SoundSelection::Set(static_cast<int>(SOUND_CLICK01));
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
    // Both folders on the selected sound's path are open.
    const ImGuiWindow* table = nullptr;
    for (ImGuiWindow* window : ImGui::GetCurrentContext()->Windows)
    {
        if (std::string(window->Name).find("##Sounds") != std::string::npos)
        {
            table = window;
        }
    }
    REQUIRE(table != nullptr);
    const ImGuiStorage& storage = table->DC.StateStorage != nullptr ? *table->DC.StateStorage : table->StateStorage;
    size_t open = 0;
    for (const ImGuiStoragePair& pair : storage.Data)
    {
        open += pair.val_i != 0 ? 1 : 0;
    }
    CHECK(open >= 2);
    MuEditor::SoundSelection::Set(-1);
}

// The Sounds tab's buttons: Data/Sound read again for a file copied in, and the
// folders' entries of sounds that are gone (removed in another window, or by
// hand in the file) dropped as a step that Undo puts back.
TEST_CASE("Sound Editor: the Sounds tab reads Data/Sound again on its button, and drops the folders' entries of "
          "sounds that are gone as a step of the history [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/new.wav", "Click")});
    g_MonsterSoundSetDatabase.Build(std::vector<Data::Monsters::MonsterSoundSet>{});
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    const std::string gone = "objects:Data/Sound/EditorTest/gone.wav";
    const std::vector<SoundFolder> withGone{Folder("Game", {"SOUND_CLICK01", gone})};
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", withGone));
    TestImGuiContext context;
    const auto frame = []
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(1400.0f, 600.0f));
        bool open = true;
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::SoundEditor);
    REQUIRE(window != nullptr);
    const ImGuiID tabs = ImGui::GetIDWithSeed("##SoundEditorTabs", nullptr, window->ID);
    const ImGuiID sounds = ImGui::GetIDWithSeed(I18N::Editor::SoundEditorTabSounds, nullptr, tabs);
    const auto press = [&](const char* label)
    {
        ImGui::ActivateItemByID(ImGui::GetIDWithSeed(label, nullptr, sounds));
        frame();
        frame();
    };

    press(I18N::Editor::SoundEditorDropGoneSounds);
    CHECK(g_SoundEdits.Folders() == std::vector<SoundFolder>{Folder("Game", {"SOUND_CLICK01"})});
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEdits.Folders() == withGone);
    // The count follows the folders' undo: the button is there again.
    press(I18N::Editor::SoundEditorDropGoneSounds);
    CHECK(g_SoundEdits.Folders() == std::vector<SoundFolder>{Folder("Game", {"SOUND_CLICK01"})});
    REQUIRE(g_SoundEdits.Undo());
    // And the rows: a sound of the gone entry's key comes, so none is gone.
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{Waterfall()});
    std::vector<Data::Maps::MapObjectSound> back = g_MapObjectSoundDatabase.GetAll();
    back[0].file = "Data/Sound/EditorTest/gone.wav";
    g_MapObjectSoundDatabase.Build(back);
    frame();
    press(I18N::Editor::SoundEditorDropGoneSounds);
    CHECK(g_SoundEdits.Folders() == withGone);
    g_MapObjectSoundDatabase.Build(std::vector<Data::Maps::MapObjectSound>{});
    // Counted with none gone, then an entry of a gone sound comes: the count
    // follows the folders, and the button is there.
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", std::vector<SoundFolder>{Folder("Game", {"SOUND_CLICK01"})}));
    frame();
    REQUIRE(g_SoundEdits.ChangeFolders("Sound folders", withGone));
    frame();
    press(I18N::Editor::SoundEditorDropGoneSounds);
    CHECK(g_SoundEdits.Folders() == std::vector<SoundFolder>{Folder("Game", {"SOUND_CLICK01"})});

    const auto missing = []
    {
        const std::vector<SoundRow> rows = MuEditor::SoundEditor::BuildRows();
        const SoundRow* row = FindRow(rows, "SOUND_CLICK01");
        REQUIRE(row != nullptr);
        return row->problem == RowProblem::MissingFile;
    };
    std::ofstream("Data/Sound/EditorTest/new.wav", std::ios::binary) << "RIFF";
    CHECK(missing());
    press(I18N::Editor::SoundEditorReadFilesAgain);
    CHECK_FALSE(missing());
}

TEST_CASE("Sound Editor: a row plays when it has a file that is in the client folder [sounds][editor]")
{
    using MuEditor::SoundEditor::CanPlay;
    MuEditor::SoundEditor::SoundRow row;
    CHECK_FALSE(CanPlay(row));
    row.file = "Data/Sound/eMeteorite.wav";
    CHECK(CanPlay(row));
    row.problem = MuEditor::SoundEditor::RowProblem::SharedFile;
    CHECK(CanPlay(row));
    row.problem = MuEditor::SoundEditor::RowProblem::MissingFile;
    CHECK_FALSE(CanPlay(row));
}

TEST_CASE("Sound Editor: the Events tab writes numbers as ranges and reads them back, names the events and a rule's "
          "sounds, and adds and takes out sounds (Q-SE1) [sounds][editor]")
{
    using namespace MuEditor::SoundEditor;
    CHECK(NumbersText({7, 24, 25, 26, 27, 28, 29, 36, 67}) == "7, 24-29, 36, 67");
    CHECK(NumbersText({0, 1, 2, 4, 5, 6, 7, 8, 9}) == "0-2, 4-9");
    CHECK(NumbersText({3, 1, 2, 1}) == "1-3");
    CHECK(NumbersText({4, 5}) == "4, 5");
    CHECK(NumbersText({}).empty());
    CHECK(ParseNumbers("7, 24-29, 36, 67", 255) == std::vector<int>{7, 24, 25, 26, 27, 28, 29, 36, 67});
    CHECK(ParseNumbers(" 9,1 , 1 ", 255) == std::vector<int>{1, 9});
    CHECK(ParseNumbers("", 255) == std::vector<int>{});
    CHECK_FALSE(ParseNumbers("a", 255).has_value());
    CHECK_FALSE(ParseNumbers("5-2", 255).has_value());
    CHECK_FALSE(ParseNumbers("256", 255).has_value());
    CHECK_FALSE(ParseNumbers("1,,2", 255).has_value());
    CHECK(EventTitle("heroFootstep") == "Your footsteps (each step of a walk, not while flying)");
    CHECK(EventTitle("someEvent") == "someEvent");

    EditorScope scope;
    g_SoundFileDatabase.Build(std::vector<SoundFile>{
        Code("SOUND_BRANDISH_SWORD01", SOUND_BRANDISH_SWORD01, "Data/Sound/EditorTest/ok.wav", "Swing 1")});
    Data::Sounds::SoundEventRule rule;
    REQUIRE(AddPlay(rule, "SOUND_BRANDISH_SWORD01"));
    CHECK_FALSE(AddPlay(rule, "SOUND_BRANDISH_SWORD01"));
    CHECK_FALSE(AddPlay(rule, "SOUND_NONE"));
    REQUIRE(AddPlay(rule, "SOUND_BRANDISH_SWORD02"));
    CHECK(rule.numbers ==
          std::vector<int>{static_cast<int>(SOUND_BRANDISH_SWORD01), static_cast<int>(SOUND_BRANDISH_SWORD02)});
    CHECK(PlaysText(rule) == "Swing 1, SOUND_BRANDISH_SWORD02");
    REQUIRE(RemovePlay(rule, 0));
    CHECK(rule.plays == std::vector<std::string>{"SOUND_BRANDISH_SWORD02"});
    CHECK(rule.numbers == std::vector<int>{static_cast<int>(SOUND_BRANDISH_SWORD02)});
    // A rule plays one sound at least.
    CHECK_FALSE(RemovePlay(rule, 0));
    CHECK_FALSE(RemovePlay(rule, 3));

    // An event keeps one rule at least, which "Add rule" copies.
    Data::Sounds::SoundEvent event;
    event.event = "playerAttackSwing";
    Data::Sounds::SoundEventRule other = rule;
    other.holds = Data::Sounds::HeldWeapon::Bow;
    event.rules = {other, rule};
    CHECK_FALSE(RemoveRule(event, 2));
    REQUIRE(RemoveRule(event, 0));
    CHECK(event.rules == std::vector<Data::Sounds::SoundEventRule>{rule});
    CHECK_FALSE(RemoveRule(event, 0));
    CHECK_FALSE(RemoveRule(event, 1));
    CHECK(event.rules.size() == 1);
}

TEST_CASE("Sound Editor: the Events tab's worlds and tiles go up to the limits the file's reader takes "
          "[sounds][editor]")
{
    using Data::Sounds::MaxEventTile;
    using Data::Sounds::MaxEventWorld;
    using MuEditor::SoundEditor::ParseNumbers;
    CHECK(ParseNumbers(std::to_string(MaxEventWorld), MaxEventWorld) == std::vector<int>{MaxEventWorld});
    CHECK_FALSE(ParseNumbers(std::to_string(MaxEventWorld + 1), MaxEventWorld).has_value());
    CHECK_FALSE(ParseNumbers(std::to_string(MaxEventTile + 1), MaxEventTile).has_value());

    const auto read = [](int world, int tile)
    {
        std::vector<Data::Sounds::SoundEvent> events;
        std::vector<Data::Items::ItemDataIssue> issues;
        Data::Sounds::ReadSoundEventsJson(R"({"formatVersion": 1, "events": [{"event": "e", "rules": [{"worlds": [)" +
                                              std::to_string(world) + R"(], "tiles": [)" + std::to_string(tile) +
                                              R"(], "plays": ["SOUND_MAGIC"]}]}]})",
                                          "SoundEvents.json", events, issues);
        return Data::Items::HasFatal(issues);
    };
    CHECK_FALSE(read(MaxEventWorld, MaxEventTile));
    CHECK(read(MaxEventWorld + 1, 0));
    CHECK(read(0, MaxEventTile + 1));
}

TEST_CASE("Sound Editor: the Events tab writes and reads click kinds and shows each event's own fields (Q-UI1) "
          "[sounds][editor]")
{
    using namespace MuEditor::SoundEditor;
    CHECK(KindsText({"confirm", "cancel"}) == "confirm, cancel");
    CHECK(KindsText({}).empty());
    CHECK(ParseKinds(" menu ,confirm, menu") == std::vector<std::string>{"menu", "confirm"});
    CHECK(ParseKinds("") == std::vector<std::string>{});
    CHECK(ParseKinds("tab-2") == std::vector<std::string>{"tab-2"});
    CHECK_FALSE(ParseKinds("Confirm").has_value());
    CHECK_FALSE(ParseKinds("ok cancel").has_value());
    CHECK_FALSE(ParseKinds("a,,b").has_value());
    CHECK(EventTitle("interfaceClick") == "A click on a button, by the click kind its theme gives it (click-sound)");
    CHECK(EventTitle("characterSelected") == "A click on a character in the character select scene");

    Data::Sounds::SoundEvent event;
    event.event = "heroFootstep";
    CHECK(FieldsOf(event) == EventFields{true, true, true, false, false});
    event.event = "playerAttackSwing";
    CHECK(FieldsOf(event) == EventFields{false, false, false, true, false});
    event.event = "interfaceClick";
    CHECK(FieldsOf(event) == EventFields{false, false, false, false, true});
    event.event = "characterSelected";
    CHECK(FieldsOf(event) == EventFields{});
    event.event = "someEvent";
    CHECK(FieldsOf(event) == EventFields{true, true, true, true, true});
    // A field a rule uses shows, whatever the event.
    event.event = "characterSelected";
    Data::Sounds::SoundEventRule rule;
    rule.worlds = {0};
    rule.holds = Data::Sounds::HeldWeapon::Bow;
    event.rules = {rule};
    CHECK(FieldsOf(event) == EventFields{true, false, false, true, false});
    event.rules[0] = {};
    event.rules[0].tiles = {1};
    event.rules[0].safeZone = true;
    event.rules[0].kinds = {"menu"};
    CHECK(FieldsOf(event) == EventFields{false, true, true, false, true});
}

TEST_CASE("Sound Editor: a change of the sound events reaches the game's table at once, is a step of the history and "
          "is what Save writes (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    const std::vector<Data::Sounds::SoundEvent> previous = g_SoundEventDatabase.GetAll();
    Data::Sounds::SoundEvent footsteps;
    footsteps.event = "heroFootstep";
    Data::Sounds::SoundEventRule ground;
    MuEditor::SoundEditor::AddPlay(ground, "SOUND_HUMAN_WALK_GROUND");
    footsteps.rules = {ground};
    g_SoundEventDatabase.Build(std::vector<Data::Sounds::SoundEvent>{footsteps});
    g_SoundEdits.TakeSaved();
    Data::Sounds::SoundEvent after = footsteps;
    Data::Sounds::SoundEventRule grass;
    grass.worlds = {0};
    grass.tiles = {0};
    MuEditor::SoundEditor::AddPlay(grass, "SOUND_HUMAN_WALK_GRASS");
    after.rules.insert(after.rules.begin(), grass);
    REQUIRE(g_SoundEdits.ChangeEvents("Sound event", std::vector<Data::Sounds::SoundEvent>{after}));
    CHECK_FALSE(g_SoundEdits.ChangeEvents("Sound event", std::vector<Data::Sounds::SoundEvent>{after}));
    REQUIRE(g_SoundEventDatabase.Find("heroFootstep") != nullptr);
    CHECK(g_SoundEventDatabase.Find("heroFootstep")->rules.size() == 2);
    CHECK(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Undo());
    CHECK(g_SoundEventDatabase.Find("heroFootstep")->rules.size() == 1);
    CHECK_FALSE(g_SoundEdits.IsModified());
    REQUIRE(g_SoundEdits.Redo());
    std::filesystem::create_directories("Data/Audio");
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_SoundEdits.Save(issues));
    CHECK_FALSE(g_SoundEdits.IsModified());
    std::vector<Data::Sounds::SoundEvent> written;
    Data::Sounds::ReadSoundEventsJson(FileText("Data/Audio/SoundEvents.json"), "SoundEvents.json", written, issues);
    CHECK(written == std::vector<Data::Sounds::SoundEvent>{after});
    g_SoundEventDatabase.Build(previous);
}

TEST_CASE("Sound Editor: the Events tab draws each event's rules (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    const std::vector<Data::Sounds::SoundEvent> previous = g_SoundEventDatabase.GetAll();
    Data::Sounds::SoundEvent swing;
    swing.event = "playerAttackSwing";
    Data::Sounds::SoundEventRule bow;
    bow.holds = Data::Sounds::HeldWeapon::Bow;
    MuEditor::SoundEditor::AddPlay(bow, "SOUND_BOW01");
    swing.rules = {bow};
    // A click's rules have a kinds column and no world's (Q-UI1).
    Data::Sounds::SoundEvent click;
    click.event = "interfaceClick";
    Data::Sounds::SoundEventRule confirm;
    confirm.kinds = {"confirm"};
    MuEditor::SoundEditor::AddPlay(confirm, "SOUND_CLICK01");
    click.rules = {confirm};
    Data::Sounds::SoundEvent selected;
    selected.event = "characterSelected";
    Data::Sounds::SoundEventRule any;
    MuEditor::SoundEditor::AddPlay(any, "SOUND_CLICK01");
    selected.rules = {any};
    g_SoundEventDatabase.Build(std::vector<Data::Sounds::SoundEvent>{swing, click, selected});
    TestImGuiContext context;
    for (int frame = 0; frame < 3; ++frame)
    {
        ImGui::NewFrame();
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(900.0f, 400.0f));
        g_MuSoundEditorUI.Render(&open);
        // The second tab, from the second frame on.
        if (frame == 0)
        {
            ImGuiTabBar* bar = nullptr;
            for (int index = 0; index < ImGui::GetCurrentContext()->TabBars.GetMapSize(); ++index)
            {
                if (ImGuiTabBar* found = ImGui::GetCurrentContext()->TabBars.TryGetMapData(index))
                {
                    bar = found;
                }
            }
            if (bar != nullptr && bar->Tabs.Size == 2)
            {
                bar->NextSelectedTabId = bar->Tabs[1].ID;
            }
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    }
    g_SoundEventDatabase.Build(previous);
}

TEST_CASE("Sound Editor: the Events tab takes a field's text that reads and says why it does not take one that "
          "does not (Q-SE1) [sounds][editor]")
{
    EditorScope scope;
    const std::vector<Data::Sounds::SoundEvent> previous = g_SoundEventDatabase.GetAll();
    Data::Sounds::SoundEvent footsteps;
    footsteps.event = "heroFootstep";
    Data::Sounds::SoundEventRule grass;
    grass.worlds = {0};
    MuEditor::SoundEditor::AddPlay(grass, "SOUND_HUMAN_WALK_GRASS");
    footsteps.rules = {grass};
    g_SoundEventDatabase.Build(std::vector<Data::Sounds::SoundEvent>{footsteps});
    g_SoundEdits.TakeSaved();
    TestImGuiContext context;
    bool selectEvents = true;
    const auto frame = [&selectEvents]
    {
        ImGui::NewFrame();
        bool open = true;
        ImGui::SetNextWindowSize(ImVec2(1200.0f, 400.0f));
        g_MuSoundEditorUI.Render(&open);
        // The Events tab, from the next frame on.
        for (int index = 0; selectEvents && index < ImGui::GetCurrentContext()->TabBars.GetMapSize(); ++index)
        {
            ImGuiTabBar* bar = ImGui::GetCurrentContext()->TabBars.TryGetMapData(index);
            if (bar != nullptr && bar->Tabs.Size == 2)
            {
                bar->NextSelectedTabId = bar->Tabs[1].ID;
                selectEvents = false;
            }
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::SoundEditor);
    REQUIRE(window != nullptr);
    const ImGuiID tabs = ImGui::GetIDWithSeed("##SoundEditorTabs", nullptr, window->ID);
    const ImGuiID events = ImGui::GetIDWithSeed(I18N::Editor::SoundEditorTabEvents, nullptr, tabs);
    const ImGuiID rules = ImGui::GetIDWithSeed("##Rules", nullptr, ImGui::GetIDWithSeed(0, events));
    const ImGuiID worlds = ImGui::GetIDWithSeed("##Worlds", nullptr, ImGui::GetIDWithSeed(0, rules));
    // Activation selects the field's text; the typed text replaces it, and
    // Enter leaves the field.
    const auto type = [&](const char* text)
    {
        ImGui::ActivateItemByID(worlds);
        frame();
        ImGui::GetIO().AddInputCharactersUTF8(text);
        frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, true);
        frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Enter, false);
        frame();
    };
    const auto worldsOf = [] { return g_SoundEventDatabase.Find("heroFootstep")->rules[0].worlds; };

    // Past the highest world: not taken, and the status line says why.
    type("7, 300");
    CHECK(worldsOf() == std::vector<int>{0});
    CHECK(g_MuSoundEditorUI.GetStatus() ==
          I18N::Format(I18N::Editor::SoundEventNumbersNotRead01,
                       {I18N::Editor::SoundEventColumnWorlds, std::to_string(Data::Sounds::MaxEventWorld)}));
    // Text that reads is taken, and the status line clears.
    type("7, 24-26");
    CHECK(worldsOf() == std::vector<int>{7, 24, 25, 26});
    CHECK(g_MuSoundEditorUI.GetStatus().empty());
    g_SoundEventDatabase.Build(previous);
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

TEST_CASE("Sound Editor: the play arrow at the start of a row plays its file (Q-SE7) [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav", "Click")});
    // Selected, so that Not sorted opens on it.
    MuEditor::SoundSelection::Set(static_cast<int>(SOUND_CLICK01));
    TestImGuiContext context;
    ImGuiIO& io = ImGui::GetIO();
    const auto frame = []
    {
        ImGui::NewFrame();
        bool open = true;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(1400.0f, 600.0f));
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
    };
    for (int index = 0; index < 4; ++index)
    {
        frame();
    }
    ImGuiTable* table = nullptr;
    ImGuiContext& g = *ImGui::GetCurrentContext();
    for (int index = 0; index < g.Tables.GetMapSize(); ++index)
    {
        if (ImGuiTable* found = g.Tables.TryGetMapData(index); found != nullptr && found->ColumnsCount == 10)
        {
            table = found;
        }
    }
    REQUIRE(table != nullptr);
    const size_t before = scope.audio.loads.size();
    // Down the play column, from below the rows up, until a click plays.
    const float x = (table->Columns[0].MinX + table->Columns[0].MaxX) * 0.5f;
    bool played = false;
    for (float y = table->OuterRect.Min.y + 120.0f; y > table->OuterRect.Min.y && !played; y -= 4.0f)
    {
        io.AddMousePosEvent(x, y);
        frame();
        io.AddMouseButtonEvent(0, true);
        frame();
        io.AddMouseButtonEvent(0, false);
        frame();
        played = scope.audio.loads.size() > before;
    }
    REQUIRE(played);
    CHECK(std::get<1>(scope.audio.loads.back()).find(L"ok.wav") != std::wstring::npos);
    MuEditor::SoundSelection::Set(-1);
}

TEST_CASE("Sound Editor: a drag on the File column's border widens the File column, not its neighbour, and the "
          "names stay while the table scrolls sideways [sounds][editor]")
{
    EditorScope scope;
    g_SoundFileDatabase.Build(
        std::vector<SoundFile>{Code("SOUND_CLICK01", SOUND_CLICK01, "Data/Sound/EditorTest/ok.wav", "Click"),
                               Code("SOUND_WIND01", SOUND_WIND01, "Data/Sound/EditorTest/gone.wav")});
    TestImGuiContext context;
    ImGuiIO& io = ImGui::GetIO();
    const auto frame = []
    {
        ImGui::NewFrame();
        bool open = true;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(1400.0f, 600.0f));
        g_MuSoundEditorUI.Render(&open);
        ImGui::Render();
    };
    const auto soundsTable = []() -> ImGuiTable*
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        for (int index = 0; index < g.Tables.GetMapSize(); ++index)
        {
            if (ImGuiTable* table = g.Tables.TryGetMapData(index); table != nullptr && table->ColumnsCount == 10)
            {
                return table;
            }
        }
        return nullptr;
    };
    for (int index = 0; index < 4; ++index)
    {
        frame();
    }
    ImGuiTable* table = soundsTable();
    REQUIRE(table != nullptr);
    // Play and Name stay when the table scrolls sideways (ImGui counts them as frozen once it does);
    // the folders indent the names (Q-SE7).
    CHECK(table->FreezeColumnsRequest == 2);
    CHECK((table->Columns[0].Flags & ImGuiTableColumnFlags_IndentDisable) != 0);
    CHECK((table->Columns[1].Flags & ImGuiTableColumnFlags_IndentEnable) != 0);
    constexpr int File = 3;
    const float file = table->Columns[File].WidthGiven;
    const float channels = table->Columns[File + 1].WidthGiven;
    // On the header row, at the File column's right border.
    const float x = table->Columns[File].MaxX;
    const float y = table->OuterRect.Min.y + 6.0f;
    io.AddMousePosEvent(x, y);
    frame();
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(x + 120.0f, y);
    frame();
    frame();
    io.AddMouseButtonEvent(0, false);
    frame();
    frame();
    CHECK(table->Columns[File].WidthGiven == doctest::Approx(file + 120.0f).epsilon(0.02));
    CHECK(table->Columns[File + 1].WidthGiven == doctest::Approx(channels));
}

#endif // _EDITOR
