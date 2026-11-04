#include "stdafx.h"

#ifdef _EDITOR

#include "SoundEditorEdits.h"

#include "../MuEditor/UI/MapEditor/MapSoundsTab.h"
#include "../MuEditor/UI/MonsterTools/MonsterEdits.h"
#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/DataHandler/SoundData/SoundEventStorage.h"
#include "Data/DataHandler/SoundData/SoundFileStorage.h"
#include "Data/DataHandler/SoundData/SoundFolderStorage.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/SoundData/SoundEventDatabase.h"
#include "Data/GameData/SoundData/SoundFileDatabase.h"
#include "I18N/All.h"

#include <algorithm>

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

bool SetsDiffer(const std::map<std::string, Data::Monsters::MonsterSoundSet>& saved)
{
    return std::any_of(saved.begin(), saved.end(),
                       [](const auto& entry)
                       {
                           const Data::Monsters::MonsterSoundSet* now = g_MonsterSoundSetDatabase.Find(entry.first);
                           return now == nullptr || *now != entry.second;
                       });
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
    SoundEdit edit;
    edit.label = StepLabel(row);
    std::optional<Data::Monsters::MonsterSoundSet> savedSet;
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
        savedSet = *found;
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
    if (savedSet)
    {
        m_savedSets.try_emplace(row.set, *savedSet);
    }
    m_undo.push_back(std::move(edit));
    m_redo.clear();
    return true;
}

const std::vector<Data::Sounds::SoundFolder>& SoundEdits::Folders()
{
    if (!m_folders)
    {
        m_folders = Data::Sounds::LoadSoundFoldersFile(Data::Sounds::GetSoundFoldersFile()).folders;
        m_savedFolders = m_folders;
    }
    return *m_folders;
}

bool SoundEdits::ChangeFolders(std::string label, std::vector<Data::Sounds::SoundFolder> after)
{
    if (!m_savedCode)
    {
        TakeSaved();
    }
    if (after == Folders())
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
    if (!Apply(m_undo.back(), false))
    {
        m_lastRefused = true;
        return false;
    }
    m_redo.push_back(std::move(m_undo.back()));
    m_undo.pop_back();
    return true;
}

bool SoundEdits::Redo()
{
    m_lastRefused = false;
    if (m_redo.empty())
    {
        return false;
    }
    if (!Apply(m_redo.back(), true))
    {
        m_lastRefused = true;
        return false;
    }
    m_undo.push_back(std::move(m_redo.back()));
    m_redo.pop_back();
    return true;
}

bool SoundEdits::IsModified() const
{
    const bool code = m_savedCode && *m_savedCode != g_SoundFileDatabase.GetAll();
    const bool objects = m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll();
    const bool folders = m_folders && m_savedFolders && *m_folders != *m_savedFolders;
    const bool events = m_savedEvents && *m_savedEvents != g_SoundEventDatabase.GetAll();
    return code || objects || folders || events || SetsDiffer(m_savedSets);
}

bool SoundEdits::Save(std::vector<Data::Items::ItemDataIssue>& issues)
{
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
    if (SetsDiffer(m_savedSets))
    {
        if (g_MonsterEdits.Save(issues))
        {
            m_savedSets.clear();
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
    if (m_folders && m_savedFolders && *m_folders != *m_savedFolders)
    {
        if (Data::Sounds::SaveSoundFolders(Data::Sounds::GetSoundFoldersFile(), *m_folders, issues))
        {
            m_savedFolders = m_folders;
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
    bool reverted = true;
    // Only the files whose tables changed are read again.
    const std::vector<SoundFile> before = g_SoundFileDatabase.GetAll();
    if (m_savedCode && *m_savedCode != before)
    {
        if (Data::Sounds::RevertSoundFiles(issues))
        {
            ReloadChanged(before, g_SoundFileDatabase.GetAll());
        }
        else
        {
            reverted = false;
        }
    }
    for (const auto& [name, set] : m_savedSets)
    {
        const Data::Monsters::MonsterSoundSet* now = g_MonsterSoundSetDatabase.Find(name);
        if (now == nullptr || *now != set)
        {
            g_MonsterEdits.SetSoundSet(I18N::Editor::Revert, set, -1);
        }
    }
    if (m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll())
    {
        if (Data::Maps::RevertMapObjectSounds(issues))
        {
            g_MapSoundsTab.TakeSaved();
        }
        else
        {
            reverted = false;
        }
    }
    if (m_savedEvents && *m_savedEvents != g_SoundEventDatabase.GetAll() && !Data::Sounds::RevertSoundEvents(issues))
    {
        reverted = false;
    }
    // The folders as the file has them.
    m_folders.reset();
    Folders();
    m_undo.clear();
    m_redo.clear();
    TakeSaved();
    return reverted;
}

void SoundEdits::TakeSaved()
{
    m_savedCode = g_SoundFileDatabase.GetAll();
    m_savedSets.clear();
    m_savedObjects = g_MapObjectSoundDatabase.GetAll();
    Folders();
    m_savedFolders = m_folders;
    m_savedEvents = g_SoundEventDatabase.GetAll();
}

void SoundEdits::Clear()
{
    m_undo.clear();
    m_redo.clear();
    m_savedCode.reset();
    m_savedSets.clear();
    m_savedObjects.reset();
    m_folders.reset();
    m_savedFolders.reset();
    m_savedEvents.reset();
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
