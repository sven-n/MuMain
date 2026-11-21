#include "stdafx.h"

// The per-map file lists: the files the game reads at start-up, for each map and for each monster, recorded by running
// the game's loaders on the shipped client folder through a content storage that names every file read. The lists in
// Data/Maps/Files must match; MU_WRITE_MAP_FILE_LISTS=1 rewrites them.

#include "../core/ContentMemoryStorage.h"
#include "MonsterCreationScope.h"
#include "MonsterModelRecorder.h"

#include "Core/Content/ContentStore.h"
#include "Core/Platform/Quit.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GM_Kanturu_3rd.h"
#include "World/MapInfra/MapManager.h"

#include "json.hpp"

#include <doctest.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

extern BYTE m_OccupationState;

namespace
{
namespace fs = std::filesystem;
using Files = std::set<std::string>;
using MonsterRecorder::CreationScope;

// Folders shipped whole: what a run without a window does not open (interface documents and their images, fonts),
// every language, not only the one the run loads, and the renderer's look file.
constexpr std::string_view WholeFolders[] = {"Data/Interface", "Data/Local", "Data/Render", "fonts"};

// The event states that make a map read other files, or spawn other models. LoadWorld reads some (the siege, Crywolf's
// occupation) and resets the others, which the game then sets while the map is loaded.
struct MapState
{
    int map;
    bool afterLoad;
    std::function<void(CreationScope&, bool)> apply;
};
const MapState MapStates[] = {
    {WD_30BATTLECASTLE, false,
     [](CreationScope& scope, bool on)
     {
         battleCastle::SetBattleCastleStart(on);
         scope.SetSpawnState("castleSiegeRunning", on);
     }},
    {WD_34CRYWOLF_1ST, false,
     [](CreationScope&, bool on) { m_OccupationState = on ? CRYWOLF_OCCUPATION_STATE_OCCUPIED : 0; }},
    {WD_34CRYWOLF_1ST, false,
     [](CreationScope&, bool on) { m_OccupationState = on ? CRYWOLF_OCCUPATION_STATE_WAR : 0; }},
    {WD_34CRYWOLF_1ST, true, [](CreationScope& scope, bool on) { scope.SetSpawnState("crywolfReady", on); }},
    {WD_39KANTURU_3RD, true,
     [](CreationScope&, bool on)
     {
         if (on)
             M39Kanturu3rd::CheckSuccessBattle(KANTURU_STATE_TOWER, KANTURU_TOWER_REVITALIXATION);
         else
             M39Kanturu3rd::Kanturu3rdInit();
     }},
    {57, true, [](CreationScope& scope, bool on) { scope.SetSpawnState("raklionBossWaiting", on); }},
    {58, true, [](CreationScope& scope, bool on) { scope.SetSpawnState("raklionBossWaiting", on); }},
};

// The monster models a map's flying creatures open on a later frame (MoveBoids), which a load without frames misses.
struct FrameModel
{
    bool (*onMap)();
    EMonsterModelType model;
};
// The dragons of Icarus are the event dragons of the shared set.
const FrameModel FrameModels[] = {
    {[] { return gMapManager.InHellas(); }, MONSTER_MODEL_BAHAMUT},
};

// The worlds the login and character scenes load (LoginScene.cpp, CharacterScene.cpp), in the shared set, and the
// older scene worlds no scene loads: none of them is a map the server sends.
constexpr int SceneWorlds[] = {WD_73NEW_LOGIN_SCENE, WD_74NEW_CHARACTER_SCENE, WD_54CHARACTERSCENE,
                               WD_55LOGINSCENE,      WD_77NEW_LOGIN_SCENE,     WD_78NEW_CHARACTER_SCENE};

fs::path ClientFolder()
{
    return MonsterRecorder::ClientFolder();
}

fs::path ListsFolder()
{
    return ClientFolder() / "Data" / "Maps" / "Files";
}

std::string Lower(std::string text)
{
    std::ranges::transform(text, text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

// The client's files by their names in lower case, for names the game spells another way (the sound table).
const std::map<std::string, std::string>& StoredNames()
{
    static const std::map<std::string, std::string> names = []
    {
        std::map<std::string, std::string> found;
        // Other test binaries may write logs into the folder meanwhile: an entry that vanishes is skipped.
        std::error_code error;
        for (fs::recursive_directory_iterator entry(ClientFolder(), error), end; !error && entry != end;
             entry.increment(error))
        {
            std::error_code fileError;
            if (!entry->is_regular_file(fileError))
                continue;
            const std::u8string name = entry->path().lexically_relative(ClientFolder()).generic_u8string();
            const std::string stored(name.begin(), name.end());
            // Not content: logs and the hidden files a desktop leaves (the repository ignores both).
            if (stored.ends_with(".log") || entry->path().filename().u8string().starts_with(u8"."))
                continue;
            found.emplace(Lower(stored), stored);
        }
        REQUIRE_FALSE(error);
        return found;
    }();
    return names;
}

std::optional<std::string> StoredName(const std::wstring& gameName)
{
    std::string name;
    for (const wchar_t c : gameName)
        name.push_back(c == L'\\' ? '/' : static_cast<char>(c));
    const auto found = StoredNames().find(Lower(name));
    return found == StoredNames().end() ? std::nullopt : std::optional<std::string>(found->second);
}

// The images the interface documents and style sheets name outside their own folder (the login logos, the siege map),
// as the game stores them: a .jpg as .OZJ, a .tga as .OZT.
Files InterfaceReferences()
{
    static const std::regex image(R"([^"'\s()]+\.(jpg|tga|png))", std::regex::icase);
    Files files;
    for (const auto& [lower, stored] : StoredNames())
    {
        if (!lower.starts_with("data/interface/") || !(lower.ends_with(".rml") || lower.ends_with(".rcss")))
            continue;
        const std::string text = Content::ReadFile(ClientFolder() / stored).value_or("");
        const fs::path folder = fs::path(stored).parent_path();
        for (auto match = std::sregex_iterator(text.begin(), text.end(), image); match != std::sregex_iterator();
             ++match)
        {
            const std::string reference = match->str();
            const std::string extension = Lower(reference.substr(reference.size() - 4));
            const std::string storedExtension = extension == ".jpg" ? ".ozj" : extension == ".tga" ? ".ozt" : extension;
            const std::string named = reference.substr(0, reference.size() - 4) + storedExtension;
            // Relative to the document; a path that climbs out of the client folder is found from Data/ as the
            // game finds it (siege_warfare.rml's World31 map).
            std::string past = named;
            while (past.starts_with("../"))
                past.erase(0, 3);
            for (const std::string& name : {(folder / named).lexically_normal().generic_string(), "Data/" + past, past})
            {
                const auto found = StoredNames().find(Lower(name));
                if (found != StoredNames().end())
                {
                    if (!found->first.starts_with("data/interface/"))
                        files.insert(found->second);
                    break;
                }
            }
        }
    }
    return files;
}

bool IsLoaded(const BMD& model)
{
    return model.NumMeshs > 0 || model.NumActions > 0;
}

// What WinMain sets up before the start-up data.
struct GameGlobals
{
    std::vector<GATE_ATTRIBUTE> gates = std::vector<GATE_ATTRIBUTE>(MAX_GATES);
    std::vector<SKILL_ATTRIBUTE> skills = std::vector<SKILL_ATTRIBUTE>(MAX_SKILLS);
    std::vector<ITEM_ATTRIBUTE> items = std::vector<ITEM_ATTRIBUTE>(MAX_ITEM);
    std::unique_ptr<CHARACTER_MACHINE> machine = std::make_unique<CHARACTER_MACHINE>();

    GameGlobals()
    {
        GateAttribute = gates.data();
        SkillAttribute = skills.data();
        ItemAttribute = items.data();
        CharacterMachine = machine.get();
        CharacterAttribute = &machine->Character;
        machine->Init();
        // The language of config.ini.template; the other languages' files ship whole (WholeFolders).
        g_strSelectedML = L"Eng";
    }
    ~GameGlobals()
    {
        GateAttribute = nullptr;
        SkillAttribute = nullptr;
        ItemAttribute = nullptr;
        CharacterMachine = nullptr;
        CharacterAttribute = nullptr;
    }
};

struct Lists
{
    Files shared;
    std::map<int, Files> maps;
    std::map<int, Files> monsters;
    Files firstBloodCastleAgain;
};

// Runs the game's loads on the shipped client folder and names the files each part read.
class Recorder
{
public:
    Recorder() : m_scope(0)
    {
        Random::SeedRand(1);
        Content::Open(ContentMemoryStorage::OpenFolder(ClientFolder(), m_reads));
    }
    ~Recorder()
    {
        Content::Close();
    }

    // The files `load` read, the textures it queued for decoding included.
    Files Record(const std::function<void()>& load)
    {
        m_reads->Take();
        {
            // Its end waits for the decodes the load queued.
            const TextureLoadBatch batch;
            load();
        }
        return m_reads->Take();
    }

    // Takes what is loaded now as the start-up state each later load starts from.
    void KeepAsShared()
    {
        m_sharedTextures.clear();
        for (const GLuint index : Bitmaps.LoadedIndices())
            m_sharedTextures[index] = Bitmaps.FindTexture(index)->FileName;
        m_sharedModels.clear();
        for (int slot = 0; slot < MAX_MODELS; ++slot)
        {
            if (IsLoaded(Models[slot]))
                m_sharedModels[slot] = gLoadData.GetModelFile(slot);
        }
    }

    // Record, from the start-up state: as a port that has only the shared files and enters the map or meets the
    // monster first. A texture or model another load left behind would hide the reads of this one.
    Files RecordFromShared(const std::function<void()>& load)
    {
        for (int slot = 0; slot < MAX_MODELS; ++slot)
        {
            const auto shared = m_sharedModels.find(slot);
            if (IsLoaded(Models[slot]) &&
                (shared == m_sharedModels.end() || shared->second != gLoadData.GetModelFile(slot)))
                Models[slot].Release();
        }
        for (const GLuint index : Bitmaps.LoadedIndices())
        {
            const auto shared = m_sharedTextures.find(index);
            if (shared == m_sharedTextures.end() || shared->second != Bitmaps.FindTexture(index)->FileName)
                Bitmaps.UnloadImage(index, true);
        }
        Random::SeedRand(1);
        return Record(load);
    }

    CreationScope& Scope()
    {
        return m_scope;
    }

private:
    GameGlobals m_globals;
    CreationScope m_scope;
    std::map<GLuint, std::wstring> m_sharedTextures;
    std::map<int, std::wstring> m_sharedModels;
    std::shared_ptr<ContentMemoryStorage::ReadLog> m_reads = std::make_shared<ContentMemoryStorage::ReadLog>();
};

std::vector<int> MonsterNumbers()
{
    std::vector<int> numbers;
    for (const auto& monster :
         Data::Monsters::LoadMonstersFile(ClientFolder() / "Data" / "Monsters" / "Monsters.json").monsters)
        numbers.push_back(monster.number);
    return numbers;
}

// The spawn file of the server: map number -> the monsters it spawns.
std::map<int, std::vector<int>> Spawns()
{
    const fs::path file = fs::path(MU_TEST_SOURCE_DIR) / "tools" / "MapFileLists" / "openmu-spawns.json";
    const std::optional<std::string> text = Content::ReadFile(file);
    REQUIRE(text);
    const nlohmann::json document = nlohmann::json::parse(*text);
    std::map<int, std::vector<int>> spawns;
    for (const auto& [map, monsters] : document.at("maps").items())
        spawns[std::stoi(map)] = monsters.get<std::vector<int>>();
    REQUIRE_FALSE(spawns.empty());
    return spawns;
}

// The server maps the client has a terrain for, the scenes' worlds aside.
std::vector<int> ClientMaps()
{
    std::vector<int> maps;
    for (int map = 0; map < 100; ++map)
    {
        const int folder = gMapManager.GetFilesFolder(map);
        const fs::path terrain = ClientFolder() / "Data" / ("World" + std::to_string(folder)) /
                                 ("EncTerrain" + std::to_string(folder) + ".map");
        // A folder without its height map (World6) holds no map the client can show.
        const std::wstring height = L"Data/World" + std::to_wstring(folder) + L"/TerrainHeight.OZB";
        if (std::ranges::find(SceneWorlds, map) == std::end(SceneWorlds) && fs::exists(terrain) && StoredName(height))
            maps.push_back(map);
    }
    return maps;
}

// A few creations of each monster with other random draws: some take one of two models at random (the Chaos Castle
// fighters); the fixed seeds keep the lists the same on every run.
void Spawn(CreationScope& scope, const std::vector<int>& monsters)
{
    for (const std::uint32_t seed : {1u, 2u, 3u, 4u})
    {
        for (const int monster : monsters)
        {
            Random::SeedRand(seed);
            scope.CreatedModel(monster);
        }
    }
}

// What the map reads in each of its states, with the monsters the server spawns there, less the shared files.
Files RecordMap(Recorder& recorder, int map, const std::vector<int>& monsters, const Files& shared)
{
    CreationScope& scope = recorder.Scope();
    const auto enter = [&](const MapState* state)
    {
        if (state != nullptr && !state->afterLoad)
            state->apply(scope, true);
        gMapManager.LoadWorld(map);
        if (state != nullptr && state->afterLoad)
            state->apply(scope, true);
        for (const FrameModel& frameModel : FrameModels)
        {
            if (frameModel.onMap())
                OpenMonsterModel(frameModel.model);
        }
        Spawn(scope, monsters);
    };
    Files files = recorder.RecordFromShared([&] { enter(nullptr); });
    for (const MapState& state : MapStates)
    {
        if (state.map != map)
            continue;
        files.merge(recorder.RecordFromShared([&] { enter(&state); }));
        state.apply(scope, false);
    }
    // The minimap document loads the map's picture itself (mini_map.rml).
    const std::wstring picture = L"Data/World" + std::to_wstring(gMapManager.GetFilesFolder(map)) + L"/mini_map.ozt";
    if (const std::optional<std::string> name = StoredName(picture))
        files.insert(*name);
    std::erase_if(files, [&](const std::string& file) { return shared.contains(file); });
    return files;
}

Lists RecordLists()
{
    Recorder recorder;
    Lists lists;

    lists.shared = recorder.Record(
        []
        {
            for (const auto& step : OpenBasicDataSteps())
                step();
            OpenLogoSceneData();
            gMapManager.LoadWorld(WD_73NEW_LOGIN_SCENE);
            OpenCharacterSceneData();
            gMapManager.LoadWorld(WD_74NEW_CHARACTER_SCENE);
            // Both player-model choices, whichever the saved settings hold: the option applies at the next start.
            for (const bool s6 : {false, true})
            {
                for (int playerClass = 0; playerClass < MAX_CLASS; ++playerClass)
                    Engine::Object::OpenPlayerClassBodyParts(playerClass, s6);
                for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
                {
                    for (int playerClass = 0; playerClass < MAX_CLASS; ++playerClass)
                    {
                        if (Engine::Object::HasPlayerBodyForm(stage, playerClass))
                            Engine::Object::OpenPlayerBodyTextures(stage, playerClass);
                    }
                }
            }
            // The dragons MoveBoids sends over any map while the server runs an event.
            OpenMonsterModel(MONSTER_MODEL_DRAGON);
        });
    recorder.KeepAsShared();
    for (const std::string_view folder : WholeFolders)
    {
        const std::string prefix = Lower(std::string(folder) + "/");
        for (const auto& [lower, stored] : StoredNames())
        {
            if (lower.starts_with(prefix))
                lists.shared.insert(stored);
        }
    }
    lists.shared.merge(InterfaceReferences());

    const std::map<int, std::vector<int>> spawns = Spawns();
    const auto spawnsOf = [&](int map)
    {
        const auto found = spawns.find(map);
        return found == spawns.end() ? std::vector<int>{} : found->second;
    };
    for (const int map : ClientMaps())
        lists.maps[map] = RecordMap(recorder, map, spawnsOf(map), lists.shared);
    // Blood Castle 1 again, after every other map: the same list when no earlier load hides a read.
    lists.firstBloodCastleAgain = RecordMap(recorder, WD_11BLOODCASTLE1, spawnsOf(WD_11BLOODCASTLE1), lists.shared);

    // Each monster on its own, as a server may spawn it on any map (created on Lorencia).
    gMapManager.SetServerMap(0);
    for (const int monster : MonsterNumbers())
    {
        Files files = recorder.RecordFromShared([&] { Spawn(recorder.Scope(), {monster}); });
        std::erase_if(files, [&](const std::string& file) { return lists.shared.contains(file); });
        lists.monsters[monster] = std::move(files);
    }
    return lists;
}

std::string Quoted(const std::string& text)
{
    return "\"" + text + "\"";
}

// {"files": [...]}, a file per line.
std::string FileList(const Files& files)
{
    std::string text = "{\n \"files\": [";
    bool first = true;
    for (const std::string& file : files)
    {
        text += (first ? "\n  " : ",\n  ") + Quoted(file);
        first = false;
    }
    return text + (files.empty() ? "]\n}\n" : "\n ]\n}\n");
}

// {"<monster>": [...], ...}, a monster per line.
std::string MonsterLists(const std::map<int, Files>& monsters)
{
    std::string text = "{";
    bool first = true;
    for (const auto& [monster, files] : monsters)
    {
        text += (first ? "\n " : ",\n ") + Quoted(std::to_string(monster)) + ": [";
        bool firstFile = true;
        for (const std::string& file : files)
        {
            text += (firstFile ? "" : ", ") + Quoted(file);
            firstFile = false;
        }
        text += "]";
        first = false;
    }
    return text + "\n}\n";
}

// The files of the lists folder by name, as they should read.
std::map<std::string, std::string> ListFiles(const Lists& lists)
{
    std::map<std::string, std::string> files;
    Files shared = lists.shared;
    // The shared pack serves both client builds. Only the editor loads the option rules at startup.
    shared.insert(Data::Items::GetOptionRulesFile().generic_string());
    files["Shared.json"] = FileList(shared);
    files["Monsters.json"] = MonsterLists(lists.monsters);
    for (const auto& [map, mapFiles] : lists.maps)
        files["Map" + std::to_string(map) + ".json"] = FileList(mapFiles);
    return files;
}

// Report just the changed entries, rather than dumping thousands of shared asset paths.
void ReportListDifference(const std::string& stored, const std::string& expected)
{
    const auto lines = [](const std::string& text)
    {
        Files result;
        std::istringstream input(text);
        for (std::string line; std::getline(input, line);)
            result.insert(std::move(line));
        return result;
    };
    const Files storedLines = lines(stored);
    const Files expectedLines = lines(expected);
    for (const std::string& line : expectedLines)
    {
        if (!storedLines.contains(line))
            MESSAGE("missing entry: " << line);
    }
    for (const std::string& line : storedLines)
    {
        if (!expectedLines.contains(line))
            MESSAGE("unexpected entry: " << line);
    }
}

// The file's text with plain line ends: a Windows checkout may turn them into CRLF.
std::string ReadText(const fs::path& file)
{
    std::ifstream in(file, std::ios::binary);
    std::string text(std::istreambuf_iterator<char>(in), {});
    std::erase(text, '\r');
    return text;
}
} // namespace

TEST_CASE("the per-map file lists name what the game reads at start-up, on each map and for each monster "
          "[content][maps]")
{
    const Lists lists = RecordLists();
    // A load that failed stops the game: the lists of a run that did would miss what came after.
    REQUIRE_FALSE(Core::Platform::QuitRequested());

    // Loaded from another map's folder at start-up: the shared set has it, no map list does.
    CHECK(lists.shared.contains("Data/Object31/flag.OZT"));
    CHECK(lists.maps.at(0).contains("Data/World1/EncTerrain1.map"));
    CHECK_FALSE(lists.maps.at(0).contains("Data/Object31/flag.OZT"));
    // Named by a document of every map's interface (siege_warfare.rml).
    CHECK(lists.shared.contains("Data/World31/map1.OZJ"));
    // The monsters the server spawns on the map: Lorencia's Spider.
    CHECK(lists.maps.at(0).contains("Data/Monster/Monster10.bmd"));
    // A monster's files are listed wherever it spawns: the Golden Budge Dragon of the invasions (43).
    CHECK_FALSE(lists.monsters.at(43).empty());
    // Each event state adds its own terrain: the siege, Crywolf occupied and at war, Kanturu's tower won.
    CHECK(lists.maps.at(WD_30BATTLECASTLE).contains("Data/World31/EncTerrain312.att"));
    CHECK(lists.maps.at(WD_34CRYWOLF_1ST).contains("Data/World35/EncTerrain351.att"));
    CHECK(lists.maps.at(WD_34CRYWOLF_1ST).contains("Data/World35/EncTerrain352.att"));
    CHECK(lists.maps.at(WD_39KANTURU_3RD).contains("Data/World40/EncTerrain401.att"));
    // The flying creatures a later frame opens: the event dragons over any map, Kalima's Bahamut.
    CHECK(lists.shared.contains("Data/Monster/Monster32.bmd"));
    CHECK(lists.maps.at(WD_24HELLAS).contains("Data/Monster/Monster34.bmd"));
    // Every improved player model, whichever player models the saved settings choose.
    const auto improved = [](const std::string& file) { return file.starts_with("Data/Player/Updated/"); };
    CHECK(std::ranges::count_if(lists.shared, improved) ==
          std::ranges::count_if(StoredNames(), [&](const auto& name) { return improved(name.second); }));
    CHECK(std::ranges::count_if(lists.shared, improved) > 0);
    // Every Blood Castle reads its shared folder's files, whichever was loaded before.
    CHECK(lists.firstBloodCastleAgain == lists.maps.at(WD_11BLOODCASTLE1));
    CHECK(lists.maps.at(WD_11BLOODCASTLE1 + 1).contains("Data/World12/leaf02.OZJ"));

    const std::map<std::string, std::string> expected = ListFiles(lists);
    if (const char* write = std::getenv("MU_WRITE_MAP_FILE_LISTS"); write != nullptr && *write == '1')
    {
        fs::create_directories(ListsFolder());
        for (const fs::directory_entry& entry : fs::directory_iterator(ListsFolder()))
        {
            if (!expected.contains(entry.path().filename().string()))
                fs::remove(entry.path());
        }
        for (const auto& [name, text] : expected)
            std::ofstream(ListsFolder() / name, std::ios::binary) << text;
        MESSAGE("wrote " << expected.size() << " lists to " << ListsFolder().string());
        return;
    }

    std::set<std::string> onDisk;
    if (fs::exists(ListsFolder()))
    {
        for (const fs::directory_entry& entry : fs::directory_iterator(ListsFolder()))
            onDisk.insert(entry.path().filename().string());
    }
    for (const auto& [name, text] : expected)
    {
        INFO(name << " is out of date: run map_file_lists_tests with MU_WRITE_MAP_FILE_LISTS=1 to rewrite the lists");
        const std::string stored = ReadText(ListsFolder() / name);
        const bool upToDate = stored == text;
        if (!upToDate)
            ReportListDifference(stored, text);
        CHECK(upToDate);
        onDisk.erase(name);
    }
    INFO("lists of maps the client no longer has: run map_file_lists_tests with MU_WRITE_MAP_FILE_LISTS=1");
    CHECK(onDisk.empty());
}
