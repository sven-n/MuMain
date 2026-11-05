#include "stdafx.h"

#ifdef _EDITOR

#include "SoundEditorEdits.h"
#include "SoundFolderTree.h"

#include "../MuEditor/UI/MapEditor/MapSoundsTab.h"
#include "../MuEditor/UI/MonsterTools/MonsterEdits.h"
#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterSoundJsonStorage.h"
#include "Data/DataHandler/SoundData/SoundEventStorage.h"
#include "Data/DataHandler/SoundData/SoundFileStorage.h"
#include "Data/DataHandler/SoundData/SoundFolderStorage.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "I18N/All.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace MuEditor::SoundEditor
{
namespace
{
using Data::Sounds::SoundFile;

bool SameLoad(const SoundFile& left, const SoundFile& right)
{
    return left.file == right.file && left.channels == right.channels && left.spatial == right.spatial;
}

// A code sound that is loaded loads again when its file, channels or 3D
// changed, so the next time it plays it is heard as it is now.
void ReloadChanged(const std::vector<SoundFile>& before, const std::vector<SoundFile>& after)
{
    for (const SoundFile& sound : after)
    {
        const auto old = std::find_if(before.begin(), before.end(),
                                      [&sound](const SoundFile& other) { return other.id == sound.id; });
        const bool changed = old == before.end() || !SameLoad(*old, sound);
        if (changed && !GetLoadedSoundFile(static_cast<ESound>(sound.number)).empty())
        {
            LoadGameSound(static_cast<ESound>(sound.number));
        }
    }
}

std::string StepLabel(const SoundRow& row)
{
    return I18N::Format(I18N::Editor::SoundEditorStep0, {ShownName(row)});
}

// Where the entry a step left is now: at its index, or elsewhere when
// entries before it came or went; none when it changed since.
template <typename Entry>
std::optional<size_t> FindAsLeft(const std::vector<Entry>& entries, size_t index, const Entry& left)
{
    if (index < entries.size() && entries[index] == left)
    {
        return index;
    }
    const auto found = std::find(entries.begin(), entries.end(), left);
    if (found == entries.end())
    {
        return std::nullopt;
    }
    return static_cast<size_t>(found - entries.begin());
}

// The set with the step's sound turned from one side to the other, and the
// events that named it renamed with it; none when the sound is not as `from`.
std::optional<Data::Monsters::MonsterSoundSet> TurnSetSound(const Data::Monsters::MonsterSoundSet& set,
                                                            const SetSoundStep& step, bool forward)
{
    const Data::Monsters::MonsterSound& from = forward ? step.before : step.after;
    const Data::Monsters::MonsterSound& to = forward ? step.after : step.before;
    const std::optional<size_t> at = FindAsLeft(set.sounds, step.index, from);
    if (!at)
    {
        return std::nullopt;
    }
    Data::Monsters::MonsterSoundSet turned = set;
    turned.sounds[*at] = to;
    const std::string fromName = Data::Monsters::SoundNameOf(from);
    const std::string toName = Data::Monsters::SoundNameOf(to);
    for (const auto& [event, sound] : step.eventNames)
    {
        if (event < turned.soundEvents.size() && sound < turned.soundEvents[event].sounds.size() &&
            turned.soundEvents[event].sounds[sound] == fromName)
        {
            turned.soundEvents[event].sounds[sound] = toName;
        }
    }
    return turned;
}

// The set `name` as the file has it (`fileSets`, under `fileName`), under
// its name now; one the file lacks goes. False when the monster tools refused
// it.
bool PutSetBack(const std::string& name, const std::string& fileName,
                const std::vector<Data::Monsters::MonsterSoundSet>& fileSets)
{
    const auto saved =
        std::find_if(fileSets.begin(), fileSets.end(),
                     [&fileName](const Data::Monsters::MonsterSoundSet& set) { return set.name == fileName; });
    const Data::Monsters::MonsterSoundSet* now = g_MonsterSoundSetDatabase.Find(name);
    if (saved == fileSets.end())
    {
        return now == nullptr || g_MonsterEdits.RemoveSoundSet(I18N::Editor::Revert, name, -1);
    }
    // A set renamed in another window since keeps its new name.
    Data::Monsters::MonsterSoundSet put = *saved;
    put.name = name;
    return (now != nullptr && *now == put) || g_MonsterEdits.SetSoundSet(I18N::Editor::Revert, put, -1);
}

// The set's sounds this window turned (`turns`), written into the file's set
// (`fileSet`, nullptr when the file has none): each sound as this window left
// it where the file has it as it was, and the events of the file's set that
// named it by its old name follow. A sound the table (`now`) has otherwise
// was changed in another window since, which writes it. The turns not
// written are handed back, each with a problem: the set is not in the file
// yet, or the file has the sound otherwise (changed in the monster tools and
// not saved); their Save writes them.
std::vector<SetSoundTurn> WriteTurns(const std::string& name, const Data::Monsters::MonsterSoundSet* now,
                                     Data::Monsters::MonsterSoundSet* fileSet, const std::vector<SetSoundTurn>& turns,
                                     std::vector<Data::Items::ItemDataIssue>& issues)
{
    std::vector<SetSoundTurn> kept;
    for (const SetSoundTurn& turn : turns)
    {
        if (now == nullptr || !FindAsLeft(now->sounds, turn.index, turn.to))
        {
            continue;
        }
        if (fileSet != nullptr)
        {
            if (const std::optional<size_t> at = FindAsLeft(fileSet->sounds, turn.index, turn.from))
            {
                fileSet->sounds[*at] = turn.to;
                const std::string fromName = Data::Monsters::SoundNameOf(turn.from);
                const std::string toName = Data::Monsters::SoundNameOf(turn.to);
                for (Data::Monsters::MonsterSoundEvent& event : fileSet->soundEvents)
                {
                    std::replace(event.sounds.begin(), event.sounds.end(), fromName, toName);
                }
                continue;
            }
            if (FindAsLeft(fileSet->sounds, turn.index, turn.to))
            {
                continue;
            }
        }
        kept.push_back(turn);
        const std::string problem = fileSet == nullptr ? "is not in the file yet: the monster tools' Save writes it"
                                                       : "has the sound " + Data::Monsters::SoundNameOf(turn.to) +
                                                             " changed in the monster tools too: their Save writes it";
        issues.push_back({Data::Items::ItemDataIssueSeverity::Fatal,
                          Data::Monsters::GetMonsterSoundsFile().generic_string(), Data::Items::ItemDataIssue::NoItem,
                          Data::Items::ItemDataIssue::NoItem, "", "the set \"" + name + "\" " + problem});
    }
    return kept;
}

// The versions of the tables the keys come from (SoundKeyFollow).
std::pair<int, int> KeyedVersions()
{
    return {g_MonsterSoundSetDatabase.GetVersion(), g_MapObjectSoundDatabase.GetVersion()};
}

// `names` (a set's name now to its name elsewhere) after the sets of
// `renamed` (old name to new) took their new names, all at once: a name one
// set gives up another may take.
void FollowNames(std::map<std::string, std::string>& names, const std::map<std::string, std::string>& renamed)
{
    std::map<std::string, std::string> moved;
    for (const auto& [from, to] : renamed)
    {
        const auto found = names.find(from);
        moved.emplace(to, found != names.end() ? found->second : from);
    }
    for (const auto& entry : renamed)
    {
        names.erase(entry.first);
    }
    for (auto& [to, elsewhere] : moved)
    {
        if (elsewhere != to)
        {
            names[to] = std::move(elsewhere);
        }
        else
        {
            names.erase(to);
        }
    }
}

std::string NameIn(const std::map<std::string, std::string>& names, const std::string& name)
{
    const auto found = names.find(name);
    return found != names.end() ? found->second : name;
}

// The names in `to` of the sets that `from` names otherwise, by their names
// in `from`; both map a set's name now to its name there.
std::map<std::string, std::string> NamesBetween(const std::map<std::string, std::string>& from,
                                                const std::map<std::string, std::string>& to)
{
    std::map<std::string, std::string> names;
    for (const std::map<std::string, std::string>* side : {&from, &to})
    {
        for (const auto& entry : *side)
        {
            const std::string there = NameIn(from, entry.first);
            const std::string here = NameIn(to, entry.first);
            if (there != here)
            {
                names[there] = here;
            }
        }
    }
    return names;
}
} // namespace

RowValues ValuesOf(const SoundRow& row)
{
    return RowValues{row.name, row.file, row.channels, row.spatial, row.type};
}

SoundEdits& SoundEdits::GetInstance()
{
    static SoundEdits instance;
    return instance;
}

bool SoundEdits::Change(const SoundRow& row, const RowValues& values)
{
    if (!m_savedCode)
    {
        TakeSaved();
    }
    // What other windows changed first, apart from this change.
    FollowKeyChanges();
    SoundEdit edit;
    edit.label = StepLabel(row);
    if (row.source == RowSource::Code)
    {
        std::vector<SoundFile> after = g_SoundFileDatabase.GetAll();
        const auto sound =
            std::find_if(after.begin(), after.end(), [&row](const SoundFile& entry) { return entry.id == row.id; });
        if (sound == after.end())
        {
            return false;
        }
        const SoundFile old = *sound;
        sound->name = values.name;
        sound->file = values.file;
        sound->channels = values.channels;
        sound->spatial = values.spatial;
        sound->type = values.type;
        if (*sound == old)
        {
            return false;
        }
        edit.codeBefore = g_SoundFileDatabase.GetAll();
        edit.codeAfter = std::move(after);
    }
    else if (row.source == RowSource::MonsterSet)
    {
        const Data::Monsters::MonsterSoundSet* found = g_MonsterSoundSetDatabase.Find(row.set);
        if (found == nullptr || row.index >= found->sounds.size())
        {
            return false;
        }
        SetSoundStep step;
        step.set = row.set;
        step.index = row.index;
        step.before = found->sounds[row.index];
        step.after = step.before;
        step.after.name = values.name;
        step.after.file = values.file;
        step.after.channels = values.channels;
        step.after.spatial = values.spatial;
        if (step.after == step.before)
        {
            return false;
        }
        // The events name a sound without an id by its file: they follow.
        const std::string oldName = Data::Monsters::SoundNameOf(step.before);
        if (Data::Monsters::SoundNameOf(step.after) != oldName)
        {
            for (size_t event = 0; event < found->soundEvents.size(); ++event)
            {
                const std::vector<std::string>& named = found->soundEvents[event].sounds;
                for (size_t sound = 0; sound < named.size(); ++sound)
                {
                    if (named[sound] == oldName)
                    {
                        step.eventNames.emplace_back(event, sound);
                    }
                }
            }
        }
        edit.setSound = std::move(step);
    }
    else if (row.source == RowSource::MapObject)
    {
        const std::vector<Data::Maps::MapObjectSound>& sounds = g_MapObjectSoundDatabase.GetAll();
        if (row.index >= sounds.size())
        {
            return false;
        }
        ObjectSoundStep step;
        step.index = row.index;
        step.before = sounds[row.index];
        step.after = step.before;
        step.after.name = values.name;
        step.after.file = values.file;
        step.after.channels = values.channels;
        if (step.after == step.before)
        {
            return false;
        }
        edit.objectSound = std::move(step);
    }
    else
    {
        return false;
    }
    if (!Apply(edit, true))
    {
        return false;
    }
    if (edit.setSound)
    {
        Turn(edit.setSound->set, edit.setSound->index, edit.setSound->before, edit.setSound->after);
    }
    m_undo.push_back(std::move(edit));
    m_redo.clear();
    // A set's or a map object sound's key has its file in it (SoundRow::key).
    FollowKeyChanges();
    return true;
}

const std::vector<Data::Sounds::SoundFolder>& SoundEdits::Folders()
{
    if (!m_folders)
    {
        ReadFolders();
    }
    return *m_folders;
}

void SoundEdits::ReadFolders()
{
    // The tables as they are now (m_keyed), the sets' renames since followed.
    FollowKeyChanges();
    Data::Sounds::SoundFoldersLoadResult read = Data::Sounds::LoadSoundFoldersFile(Data::Sounds::GetSoundFoldersFile());
    m_folders = std::move(read.folders);
    ++m_foldersVersion;
    m_savedFolders = m_folders;
    m_foldersFile = *m_folders;
    m_folderProblems.clear();
    std::copy_if(read.issues.begin(), read.issues.end(), std::back_inserter(m_folderProblems),
                 [](const Data::Items::ItemDataIssue& issue)
                 { return issue.severity == Data::Items::ItemDataIssueSeverity::Fatal; });
    // The file names the sounds by the keys of its tables, the window by the
    // keys of now: the ones that changed since keep their folders.
    if (m_folderProblems.empty())
    {
        const std::map<std::string, std::string> renamed =
            FollowedKeys(*m_foldersFileTables, *m_keyed, NamesBetween(m_folderNames, {}));
        RenameSounds(*m_folders, renamed);
        RenameSounds(*m_savedFolders, renamed);
    }
    CheckFoldersFile();
}

void SoundEdits::FollowKeyChanges()
{
    if (!m_keyed || KeyedVersions() != m_keyedVersions)
    {
        FollowTables();
    }
    // After the renames of a revert: the monster tools wrote or read their
    // files as a whole, so the sets' names are the file's.
    if (g_MonsterDataHandler.GetFilesVersion() != m_filesVersion)
    {
        m_filesVersion = g_MonsterDataHandler.GetFilesVersion();
        m_fileNames.clear();
        // The table's sets are the file's: this window's changes of them were
        // saved with them, or read over.
        m_setTurns.clear();
        // The sets' file has the table's sets now.
        m_savedTables->sets = m_keyed->sets;
        CheckFoldersFile();
    }
}

void SoundEdits::FollowTables()
{
    // Only the tables whose versions moved are copied and keyed: a change of
    // the map object sounds touches no set, and the other way round.
    const std::pair<int, int> versions = KeyedVersions();
    const bool sets = !m_keyed || versions.first != m_keyedVersions.first;
    const bool objects = !m_keyed || versions.second != m_keyedVersions.second;
    KeyedTables now;
    if (sets)
    {
        now.sets = CurrentKeyedSets();
    }
    if (objects)
    {
        now.objects = g_MapObjectSoundDatabase.GetAll();
    }
    if (m_keyed)
    {
        // The tables that changed as they were; the others are empty on both
        // sides, so nothing of them is followed.
        KeyedTables before;
        if (sets)
        {
            before.sets = std::move(m_keyed->sets);
            FollowRenamedSets(RenamedSets(before, now));
        }
        if (objects)
        {
            before.objects = std::move(m_keyed->objects);
        }
        // A file that could not be read is never written: its folders stay.
        if (m_folders && m_folderProblems.empty())
        {
            RenameInFolders(FollowedKeys(before, now));
        }
        if (!sets)
        {
            now.sets = std::move(m_keyed->sets);
        }
        if (!objects)
        {
            now.objects = std::move(m_keyed->objects);
        }
    }
    if (!m_foldersFileTables)
    {
        // The files, and the folders' file with them, go with the tables as
        // the editor found them.
        m_savedTables = now;
        m_foldersFileTables = now;
        m_folderNames.clear();
        m_foldersFileStale = false;
    }
    m_keyed = std::move(now);
    m_keyedVersions = versions;
}

void SoundEdits::FollowRenamedSets(const std::map<std::string, std::string>& renamed)
{
    if (renamed.empty())
    {
        return;
    }
    FollowNames(m_fileNames, renamed);
    FollowNames(m_folderNames, renamed);
    std::map<std::string, std::vector<SetSoundTurn>> followed;
    for (auto& [name, turns] : m_setTurns)
    {
        const auto found = renamed.find(name);
        followed.emplace(found != renamed.end() ? found->second : name, std::move(turns));
    }
    m_setTurns = std::move(followed);
    for (std::vector<SoundEdit>* steps : {&m_undo, &m_redo})
    {
        for (SoundEdit& step : *steps)
        {
            const auto found = step.setSound ? renamed.find(step.setSound->set) : renamed.end();
            if (found != renamed.end())
            {
                step.setSound->set = found->second;
            }
        }
    }
}

void SoundEdits::Turn(const std::string& set, size_t index, const Data::Monsters::MonsterSound& from,
                      const Data::Monsters::MonsterSound& to)
{
    std::vector<SetSoundTurn>& turns = m_setTurns[set];
    auto turn = std::find_if(turns.begin(), turns.end(),
                             [&](const SetSoundTurn& entry) { return entry.index == index && entry.to == from; });
    if (turn == turns.end())
    {
        turn =
            std::find_if(turns.begin(), turns.end(), [&from](const SetSoundTurn& entry) { return entry.to == from; });
    }
    if (turn == turns.end())
    {
        turns.push_back({index, from, to});
    }
    else
    {
        turn->to = to;
        if (turn->to == turn->from)
        {
            turns.erase(turn);
        }
    }
    if (turns.empty())
    {
        m_setTurns.erase(set);
    }
}

std::string SoundEdits::FileNameOf(const std::string& name) const
{
    return NameIn(m_fileNames, name);
}

KeyedTables SoundEdits::ReadKeyedTables() const
{
    KeyedTables tables = *m_savedTables;
    const Data::Monsters::MonsterSoundsLoadResult sets =
        Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile());
    if (!Data::Items::HasFatal(sets.issues))
    {
        tables.sets = sets.sets;
    }
    const Data::Maps::MapObjectSoundsLoadResult objects =
        Data::Maps::LoadMapObjectSoundsFile(Data::Maps::GetMapObjectSoundsFile());
    if (!Data::Items::HasFatal(objects.issues))
    {
        tables.objects = objects.sounds;
    }
    return tables;
}

void SoundEdits::CheckFoldersFile()
{
    std::vector<Data::Sounds::SoundFolder> moved = m_foldersFile;
    m_foldersFileStale = RenameSounds(
        moved, FollowedKeys(*m_foldersFileTables, *m_savedTables, NamesBetween(m_folderNames, m_fileNames)));
}

void SoundEdits::RenameInFolders(const std::map<std::string, std::string>& renamed)
{
    if (renamed.empty())
    {
        return;
    }
    RenameSounds(*m_folders, renamed);
    ++m_foldersVersion;
    // The folders' file names the same sounds still.
    if (m_savedFolders)
    {
        RenameSounds(*m_savedFolders, renamed);
    }
    for (std::vector<SoundEdit>* steps : {&m_undo, &m_redo})
    {
        for (SoundEdit& step : *steps)
        {
            if (step.foldersBefore)
            {
                RenameSounds(*step.foldersBefore, renamed);
            }
            if (step.foldersAfter)
            {
                RenameSounds(*step.foldersAfter, renamed);
            }
        }
    }
}

const std::vector<Data::Items::ItemDataIssue>& SoundEdits::FolderProblems()
{
    Folders();
    return m_folderProblems;
}

bool SoundEdits::ChangeFolders(std::string label, std::vector<Data::Sounds::SoundFolder> after)
{
    if (!m_savedCode)
    {
        TakeSaved();
    }
    if (after == Folders() || !m_folderProblems.empty())
    {
        return false;
    }
    SoundEdit edit;
    edit.label = std::move(label);
    edit.foldersBefore = Folders();
    edit.foldersAfter = std::move(after);
    Apply(edit, true);
    m_undo.push_back(std::move(edit));
    m_redo.clear();
    return true;
}

bool SoundEdits::ChangeEvents(std::string label, std::vector<Data::Sounds::SoundEvent> after)
{
    if (!m_savedCode)
    {
        TakeSaved();
    }
    if (after == g_SoundEventDatabase.GetAll())
    {
        return false;
    }
    SoundEdit edit;
    edit.label = std::move(label);
    edit.eventsBefore = g_SoundEventDatabase.GetAll();
    edit.eventsAfter = std::move(after);
    Apply(edit, true);
    m_undo.push_back(std::move(edit));
    m_redo.clear();
    return true;
}

bool SoundEdits::Apply(const SoundEdit& edit, bool forward)
{
    // The set and the map object sounds are checked before anything changes.
    if (edit.setSound)
    {
        const Data::Monsters::MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find(edit.setSound->set);
        const std::optional<Data::Monsters::MonsterSoundSet> turned =
            set != nullptr ? TurnSetSound(*set, *edit.setSound, forward) : std::nullopt;
        if (!turned || !g_MonsterEdits.SetSoundSet(edit.label, *turned, -1))
        {
            return false;
        }
    }
    if (edit.objectSound)
    {
        std::vector<Data::Maps::MapObjectSound> sounds = g_MapObjectSoundDatabase.GetAll();
        const ObjectSoundStep& step = *edit.objectSound;
        const std::optional<size_t> at = FindAsLeft(sounds, step.index, forward ? step.before : step.after);
        if (!at)
        {
            return false;
        }
        sounds[*at] = forward ? step.after : step.before;
        // The map's object sounds follow the table's version (MapObjectSounds).
        g_MapObjectSoundDatabase.Build(sounds);
    }
    if (edit.codeBefore && edit.codeAfter)
    {
        const std::vector<SoundFile>& from = forward ? *edit.codeBefore : *edit.codeAfter;
        const std::vector<SoundFile>& to = forward ? *edit.codeAfter : *edit.codeBefore;
        g_SoundFileDatabase.Build(to);
        ReloadChanged(from, to);
    }
    if (edit.foldersBefore && edit.foldersAfter)
    {
        m_folders = forward ? *edit.foldersAfter : *edit.foldersBefore;
        ++m_foldersVersion;
    }
    if (edit.eventsBefore && edit.eventsAfter)
    {
        g_SoundEventDatabase.Build(forward ? *edit.eventsAfter : *edit.eventsBefore);
    }
    return true;
}

bool SoundEdits::Undo()
{
    m_lastRefused = false;
    if (m_undo.empty())
    {
        return false;
    }
    FollowKeyChanges();
    if (!Apply(m_undo.back(), false))
    {
        m_lastRefused = true;
        return false;
    }
    if (const std::optional<SetSoundStep>& step = m_undo.back().setSound)
    {
        Turn(step->set, step->index, step->after, step->before);
    }
    m_redo.push_back(std::move(m_undo.back()));
    m_undo.pop_back();
    FollowKeyChanges();
    return true;
}

bool SoundEdits::Redo()
{
    m_lastRefused = false;
    if (m_redo.empty())
    {
        return false;
    }
    FollowKeyChanges();
    if (!Apply(m_redo.back(), true))
    {
        m_lastRefused = true;
        return false;
    }
    if (const std::optional<SetSoundStep>& step = m_redo.back().setSound)
    {
        Turn(step->set, step->index, step->before, step->after);
    }
    m_undo.push_back(std::move(m_redo.back()));
    m_redo.pop_back();
    FollowKeyChanges();
    return true;
}

bool SoundEdits::IsModified() const
{
    const bool code = m_savedCode && *m_savedCode != g_SoundFileDatabase.GetAll();
    const bool objects = m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll();
    const bool folders = m_folders && m_savedFolders && (m_foldersFileStale || *m_folders != *m_savedFolders);
    const bool events = m_savedEvents && *m_savedEvents != g_SoundEventDatabase.GetAll();
    return code || objects || folders || events || !m_setTurns.empty();
}

bool SoundEdits::Save(std::vector<Data::Items::ItemDataIssue>& issues)
{
    FollowKeyChanges();
    bool saved = true;
    if (m_savedCode && *m_savedCode != g_SoundFileDatabase.GetAll())
    {
        if (Data::Sounds::SaveSoundFiles(issues))
        {
            m_savedCode = g_SoundFileDatabase.GetAll();
        }
        else
        {
            saved = false;
        }
    }
    if (!m_setTurns.empty())
    {
        // Only the sounds this window changed, into the file's sets under
        // their names there: not the monster tools' other unsaved changes, a
        // rename among them.
        std::map<std::string, std::vector<SetSoundTurn>> kept;
        const auto change = [this, &kept, &issues](std::vector<Data::Monsters::MonsterSoundSet>& fileSets)
        {
            for (const auto& [name, turns] : m_setTurns)
            {
                const std::string fileName = FileNameOf(name);
                const auto fileSet = std::find_if(fileSets.begin(), fileSets.end(),
                                                  [&fileName](const Data::Monsters::MonsterSoundSet& set)
                                                  { return set.name == fileName; });
                std::vector<SetSoundTurn> left =
                    WriteTurns(name, g_MonsterSoundSetDatabase.Find(name),
                               fileSet != fileSets.end() ? &*fileSet : nullptr, turns, issues);
                if (!left.empty())
                {
                    kept.emplace(name, std::move(left));
                }
            }
            return true;
        };
        if (g_MonsterDataHandler.SaveSoundSets(change, issues))
        {
            m_setTurns = std::move(kept);
            saved = saved && m_setTurns.empty();
        }
        else
        {
            saved = false;
        }
    }
    if (m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll())
    {
        if (Data::Maps::SaveMapObjectSounds(issues))
        {
            m_savedObjects = g_MapObjectSoundDatabase.GetAll();
            g_MapSoundsTab.TakeSaved();
        }
        else
        {
            saved = false;
        }
    }
    // The keys the files give the sounds now: a sound whose file was saved
    // has another key there, so the folders' file is written too.
    if (m_folders)
    {
        m_savedTables = ReadKeyedTables();
        CheckFoldersFile();
    }
    if (m_folders && m_savedFolders && (m_foldersFileStale || *m_folders != *m_savedFolders))
    {
        // Under those keys: a set another window renamed or changed and has
        // not saved keeps its old keys there, as in the sets' file.
        std::vector<Data::Sounds::SoundFolder> written = *m_folders;
        RenameSounds(written, FollowedKeys(*m_keyed, *m_savedTables, m_fileNames));
        if (Data::Sounds::SaveSoundFolders(Data::Sounds::GetSoundFoldersFile(), written, issues))
        {
            m_savedFolders = m_folders;
            m_foldersFile = std::move(written);
            m_foldersFileTables = m_savedTables;
            m_folderNames = m_fileNames;
            m_foldersFileStale = false;
        }
        else
        {
            saved = false;
        }
    }
    if (m_savedEvents && *m_savedEvents != g_SoundEventDatabase.GetAll())
    {
        if (Data::Sounds::SaveSoundEvents(issues))
        {
            m_savedEvents = g_SoundEventDatabase.GetAll();
        }
        else
        {
            saved = false;
        }
    }
    return saved;
}

bool SoundEdits::Revert(std::vector<Data::Items::ItemDataIssue>& issues)
{
    // The sets renamed in other windows since are found by their new names.
    FollowKeyChanges();
    // What each file that is not read again stays not saved against.
    const auto savedCode = m_savedCode;
    const auto savedObjects = m_savedObjects;
    const auto savedEvents = m_savedEvents;
    bool codeKept = false;
    bool objectsKept = false;
    bool eventsKept = false;
    // Only the files whose tables changed are read again.
    const std::vector<SoundFile> before = g_SoundFileDatabase.GetAll();
    if (m_savedCode && *m_savedCode != before)
    {
        codeKept = !Data::Sounds::RevertSoundFiles(issues);
        if (!codeKept)
        {
            ReloadChanged(before, g_SoundFileDatabase.GetAll());
        }
    }
    std::map<std::string, std::vector<SetSoundTurn>> keptTurns;
    const bool setsKept = !m_setTurns.empty() && !RevertSets(issues, keptTurns);
    if (m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll())
    {
        objectsKept = !Data::Maps::RevertMapObjectSounds(issues);
        if (!objectsKept)
        {
            g_MapSoundsTab.TakeSaved();
        }
    }
    if (m_savedEvents && *m_savedEvents != g_SoundEventDatabase.GetAll())
    {
        eventsKept = !Data::Sounds::RevertSoundEvents(issues);
    }
    // The folders as the file has them.
    m_folders.reset();
    Folders();
    m_undo.clear();
    m_redo.clear();
    TakeSaved();
    // A file that was not read again still differs from its table: not saved
    // yet, so Save writes it and a later Revert reads it.
    if (codeKept)
    {
        m_savedCode = savedCode;
    }
    m_setTurns = std::move(keptTurns);
    if (objectsKept)
    {
        m_savedObjects = savedObjects;
    }
    if (eventsKept)
    {
        m_savedEvents = savedEvents;
    }
    return !codeKept && !setsKept && !objectsKept && !eventsKept;
}

bool SoundEdits::RevertSets(std::vector<Data::Items::ItemDataIssue>& issues,
                            std::map<std::string, std::vector<SetSoundTurn>>& kept)
{
    const Data::Monsters::MonsterSoundsLoadResult file =
        Data::Monsters::LoadMonsterSoundsFile(Data::Monsters::GetMonsterSoundsFile());
    if (Data::Items::HasFatal(file.issues))
    {
        issues.insert(issues.end(), file.issues.begin(), file.issues.end());
        kept = m_setTurns;
        return false;
    }
    bool all = true;
    for (const auto& entry : m_setTurns)
    {
        if (PutSetBack(entry.first, FileNameOf(entry.first), file.sets))
        {
            continue;
        }
        kept.insert(entry);
        issues.push_back({Data::Items::ItemDataIssueSeverity::Fatal,
                          Data::Monsters::GetMonsterSoundsFile().generic_string(), Data::Items::ItemDataIssue::NoItem,
                          Data::Items::ItemDataIssue::NoItem, "",
                          "the set \"" + entry.first + "\" could not be put back as the file has it"});
        all = false;
    }
    return all;
}

void SoundEdits::TakeSaved()
{
    m_savedCode = g_SoundFileDatabase.GetAll();
    m_setTurns.clear();
    m_savedObjects = g_MapObjectSoundDatabase.GetAll();
    Folders();
    m_savedEvents = g_SoundEventDatabase.GetAll();
}

void SoundEdits::TakeSavedObjects()
{
    FollowKeyChanges();
    if (m_savedObjects)
    {
        m_savedObjects = g_MapObjectSoundDatabase.GetAll();
    }
    // A file picked there has another key in the file now.
    m_savedTables->objects = m_keyed->objects;
    CheckFoldersFile();
}

void SoundEdits::Clear()
{
    m_undo.clear();
    m_redo.clear();
    m_savedCode.reset();
    m_setTurns.clear();
    m_savedObjects.reset();
    m_folders.reset();
    m_savedFolders.reset();
    m_foldersFile.clear();
    m_folderProblems.clear();
    m_keyed.reset();
    m_keyedVersions = {-1, -1};
    m_savedTables.reset();
    m_foldersFileTables.reset();
    m_folderNames.clear();
    m_foldersFileStale = false;
    m_fileNames.clear();
    m_filesVersion = -1;
    m_savedEvents.reset();
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
