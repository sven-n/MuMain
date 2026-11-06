#include "stdafx.h"

#ifdef _EDITOR

#include "SoundEditorRows.h"
#include "SoundFilePicker.h"

#include "../MuEditor/UI/Common/MapPicker.h"
#include "Audio/DSPlaySound.h"
#include "Audio/SoundIds.h"
#include "Audio/SoundLoads.h"
#include "Audio/SoundNumbers.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>

namespace MuEditor::SoundEditor
{
namespace
{
constexpr const char* SoundFilesFile = "Data/Audio/SoundFiles.json";
constexpr const char* SoundSetsFile = "Data/Monsters/Sounds.json";
constexpr const char* ObjectSoundsFile = "Data/Maps/ObjectSounds.json";

std::string Lowered(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string FileName(const std::string& file)
{
    const size_t slash = file.find_last_of("/\\");
    return slash == std::string::npos ? file : file.substr(slash + 1);
}

int NumberOfFile(const std::string& file)
{
    const ESound number = Audio::SoundNumbers::ForFile(Data::Paths::ToLoaderPath(file));
    return number == Audio::SoundNumbers::NoNumber ? -1 : static_cast<int>(number);
}

std::string LoadsText(int number)
{
    const std::optional<int> where = Audio::SoundLoads::WhereLoaded(number);
    if (!where)
    {
        return I18N::Editor::SoundEditorNotLoadedYet;
    }
    if (*where == Audio::SoundLoads::AtStart)
    {
        return I18N::Editor::SoundEditorAtStart;
    }
    const wchar_t* name = gMapManager.GetMapName(*where);
    return name != nullptr && name[0] != L'\0' ? Core::Text::ToUtf8(name) : std::to_string(*where);
}

void AddCodeRows(std::vector<SoundRow>& rows)
{
    for (const Data::Sounds::SoundFile& sound : g_SoundFileDatabase.GetAll())
    {
        SoundRow row;
        row.source = RowSource::Code;
        row.key = sound.id;
        row.id = sound.id;
        row.name = sound.name;
        row.file = sound.file;
        row.channels = sound.channels;
        row.spatial = sound.spatial;
        row.type = sound.type;
        row.number = sound.number;
        row.loads = LoadsText(sound.number);
        row.livesIn = SoundFilesFile;
        rows.push_back(std::move(row));
    }
}

// The name the set's events give the sound: its id, else its file.
std::string SetSoundKey(const std::string& set, const Data::Monsters::MonsterSound& sound)
{
    return "set:" + set + ":" + Data::Monsters::SoundNameOf(sound);
}

std::string ObjectSoundKey(const Data::Maps::MapObjectSound& sound)
{
    return "objects:" + sound.file;
}

void AddSetRows(std::vector<SoundRow>& rows)
{
    for (const Data::Monsters::MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
    {
        for (size_t index = 0; index < set->sounds.size(); ++index)
        {
            const Data::Monsters::MonsterSound& sound = set->sounds[index];
            SoundRow row;
            row.source = RowSource::MonsterSet;
            row.key = SetSoundKey(set->name, sound);
            row.id = sound.id;
            row.set = set->name;
            row.index = index;
            row.name = sound.name;
            row.file = sound.file;
            row.channels = sound.channels;
            row.spatial = sound.spatial;
            const std::optional<ESound> id = Audio::SoundIds::Find(sound.id);
            row.number = id ? static_cast<int>(*id) : NumberOfFile(sound.file);
            row.loads = I18N::Editor::SoundEditorWithItsMonster;
            row.livesIn = SoundSetsFile;
            rows.push_back(std::move(row));
        }
    }
}

void AddObjectRows(std::vector<SoundRow>& rows)
{
    const std::vector<Data::Maps::MapObjectSound>& sounds = g_MapObjectSoundDatabase.GetAll();
    for (size_t index = 0; index < sounds.size(); ++index)
    {
        const Data::Maps::MapObjectSound& sound = sounds[index];
        SoundRow row;
        row.source = RowSource::MapObject;
        row.key = ObjectSoundKey(sound);
        row.index = index;
        row.name = sound.name;
        row.file = sound.file;
        row.channels = sound.channels;
        row.number = NumberOfFile(sound.file);
        row.loads = MapPicker::Describe(sound.maps, "");
        row.livesIn = ObjectSoundsFile;
        rows.push_back(std::move(row));
    }
}

void AddNotLoadedRows(std::vector<SoundRow>& rows)
{
    for (const int number : Audio::SoundLoads::PlayedUnloaded())
    {
        const bool listed =
            std::any_of(rows.begin(), rows.end(), [number](const SoundRow& row) { return row.number == number; });
        if (listed || !GetLoadedSoundFile(static_cast<ESound>(number)).empty())
        {
            continue;
        }
        SoundRow row;
        row.source = RowSource::NotLoaded;
        row.id = std::string(Audio::SoundIds::NameOf(static_cast<ESound>(number)));
        row.key = row.id.empty() ? "#" + std::to_string(number) : row.id;
        row.number = number;
        row.loads = I18N::Editor::SoundEditorNothingLoadsIt;
        row.problem = RowProblem::PlayedNotLoaded;
        rows.push_back(std::move(row));
    }
}

// The rows' keys name one row each, as the folders and the window's ids need
// them: the second row with a key gets "#2" after it, the third "#3" (a key
// that is free).
void MakeKeysUnique(std::vector<SoundRow>& rows)
{
    std::set<std::string> taken;
    for (const SoundRow& row : rows)
    {
        taken.insert(row.key);
    }
    std::set<std::string> seen;
    for (SoundRow& row : rows)
    {
        if (seen.insert(row.key).second)
        {
            continue;
        }
        const std::string base = row.key;
        int count = 2;
        while (taken.contains(base + "#" + std::to_string(count)))
        {
            ++count;
        }
        row.key = base + "#" + std::to_string(count);
        taken.insert(row.key);
        seen.insert(row.key);
    }
}

void FindProblems(std::vector<SoundRow>& rows)
{
    std::map<std::string, int> codeFiles;
    for (const SoundRow& row : rows)
    {
        if (row.source == RowSource::Code)
        {
            ++codeFiles[Lowered(row.file)];
        }
    }
    for (SoundRow& row : rows)
    {
        if (row.source == RowSource::NotLoaded)
        {
            continue;
        }
        if (!IsClientSoundFile(row.file))
        {
            row.problem = RowProblem::MissingFile;
        }
        else if (row.source == RowSource::Code && codeFiles[Lowered(row.file)] > 1)
        {
            row.problem = RowProblem::SharedFile;
        }
    }
}
} // namespace

std::vector<SoundRow> BuildRows()
{
    std::vector<SoundRow> rows;
    AddCodeRows(rows);
    AddSetRows(rows);
    AddObjectRows(rows);
    AddNotLoadedRows(rows);
    MakeKeysUnique(rows);
    FindProblems(rows);
    return rows;
}

std::vector<SoundRow> KeyRows(const std::vector<Data::Monsters::MonsterSoundSet>& sets,
                              const std::vector<Data::Maps::MapObjectSound>& objects)
{
    std::vector<SoundRow> rows;
    for (const Data::Monsters::MonsterSoundSet& set : sets)
    {
        for (size_t index = 0; index < set.sounds.size(); ++index)
        {
            SoundRow row;
            row.source = RowSource::MonsterSet;
            row.set = set.name;
            row.index = index;
            row.key = SetSoundKey(set.name, set.sounds[index]);
            rows.push_back(std::move(row));
        }
    }
    for (size_t index = 0; index < objects.size(); ++index)
    {
        SoundRow row;
        row.source = RowSource::MapObject;
        row.index = index;
        row.key = ObjectSoundKey(objects[index]);
        rows.push_back(std::move(row));
    }
    // The other rows' keys never start with "set:" or "objects:", so these
    // come out of MakeKeysUnique as in BuildRows.
    MakeKeysUnique(rows);
    return rows;
}

std::string ShownName(const SoundRow& row)
{
    if (!row.name.empty())
    {
        return row.name;
    }
    if (!row.id.empty())
    {
        return row.id;
    }
    if (!row.set.empty())
    {
        return row.set + ": " + FileName(row.file);
    }
    return row.file.empty() ? row.key : FileName(row.file);
}

std::string IdText(const SoundRow& row)
{
    if (row.name.empty())
    {
        // The id or set is the shown name already; a set's sound with an id
        // still names its set.
        return row.source == RowSource::MonsterSet && !row.id.empty() ? row.set : "";
    }
    if (!row.id.empty())
    {
        return row.set.empty() ? row.id : row.set + ", " + row.id;
    }
    return row.set;
}

bool CanPlay(const SoundRow& row)
{
    return !row.file.empty() && row.problem != RowProblem::MissingFile;
}

bool Matches(const SoundRow& row, std::string_view text)
{
    if (text.empty())
    {
        return true;
    }
    const std::string wanted = Lowered(std::string(text));
    for (const std::string* field : {&row.name, &row.id, &row.set, &row.file})
    {
        if (Lowered(*field).find(wanted) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::string NameOfNumber(int number)
{
    static std::tuple<int, int, int> versions{-1, -1, -1};
    static std::unordered_map<int, std::string> names;
    const std::tuple<int, int, int> now{g_SoundFileDatabase.GetVersion(), g_MonsterSoundSetDatabase.GetVersion(),
                                        g_MapObjectSoundDatabase.GetVersion()};
    if (now != versions)
    {
        versions = now;
        names.clear();
        std::vector<SoundRow> rows;
        AddCodeRows(rows);
        AddSetRows(rows);
        AddObjectRows(rows);
        for (const SoundRow& row : rows)
        {
            if (!row.name.empty() && row.number >= 0)
            {
                names.try_emplace(row.number, row.name);
            }
        }
    }
    const auto found = names.find(number);
    return found != names.end() ? found->second : std::string();
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
