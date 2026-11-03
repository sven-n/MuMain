#include "stdafx.h"

#ifdef _EDITOR

#include "SoundEditorEdits.h"

#include "../MuEditor/UI/MapEditor/MapSoundsTab.h"
#include "../MuEditor/UI/MonsterTools/MonsterEdits.h"
#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/DataHandler/SoundData/SoundFileStorage.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
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
        Data::Monsters::MonsterSoundSet after = *found;
        Data::Monsters::MonsterSound& sound = after.sounds[row.index];
        const std::string oldName = Data::Monsters::SoundNameOf(sound);
        sound.name = values.name;
        sound.file = values.file;
        sound.channels = values.channels;
        sound.spatial = values.spatial;
        // The events name a sound without an id by its file: they follow.
        const std::string newName = Data::Monsters::SoundNameOf(sound);
        if (newName != oldName)
        {
            for (Data::Monsters::MonsterSoundEvent& event : after.soundEvents)
            {
                std::replace(event.sounds.begin(), event.sounds.end(), oldName, newName);
            }
        }
        if (after == *found)
        {
            return false;
        }
        edit.setBefore = *found;
        edit.setAfter = std::move(after);
        if (!g_MonsterEdits.SetSoundSet(edit.label, *edit.setAfter, -1))
        {
            return false;
        }
        m_savedSets.try_emplace(row.set, *edit.setBefore);
        m_undo.push_back(std::move(edit));
        m_redo.clear();
        return true;
    }
    else if (row.source == RowSource::MapObject)
    {
        std::vector<Data::Maps::MapObjectSound> after = g_MapObjectSoundDatabase.GetAll();
        if (row.index >= after.size())
        {
            return false;
        }
        Data::Maps::MapObjectSound& sound = after[row.index];
        const Data::Maps::MapObjectSound old = sound;
        sound.name = values.name;
        sound.file = values.file;
        sound.channels = values.channels;
        if (sound == old)
        {
            return false;
        }
        edit.objectsBefore = g_MapObjectSoundDatabase.GetAll();
        edit.objectsAfter = std::move(after);
    }
    else
    {
        return false;
    }
    Apply(edit, true);
    m_undo.push_back(std::move(edit));
    m_redo.clear();
    return true;
}

void SoundEdits::Apply(const SoundEdit& edit, bool forward)
{
    if (edit.codeBefore && edit.codeAfter)
    {
        const std::vector<SoundFile>& from = forward ? *edit.codeBefore : *edit.codeAfter;
        const std::vector<SoundFile>& to = forward ? *edit.codeAfter : *edit.codeBefore;
        g_SoundFileDatabase.Build(to);
        ReloadChanged(from, to);
    }
    if (edit.setBefore && edit.setAfter)
    {
        g_MonsterEdits.SetSoundSet(edit.label, forward ? *edit.setAfter : *edit.setBefore, -1);
    }
    if (edit.objectsBefore && edit.objectsAfter)
    {
        // The map's object sounds follow the table's version (MapObjectSounds).
        g_MapObjectSoundDatabase.Build(forward ? *edit.objectsAfter : *edit.objectsBefore);
    }
}

bool SoundEdits::Undo()
{
    if (m_undo.empty())
    {
        return false;
    }
    SoundEdit edit = std::move(m_undo.back());
    m_undo.pop_back();
    Apply(edit, false);
    m_redo.push_back(std::move(edit));
    return true;
}

bool SoundEdits::Redo()
{
    if (m_redo.empty())
    {
        return false;
    }
    SoundEdit edit = std::move(m_redo.back());
    m_redo.pop_back();
    Apply(edit, true);
    m_undo.push_back(std::move(edit));
    return true;
}

bool SoundEdits::IsModified() const
{
    const bool code = m_savedCode && *m_savedCode != g_SoundFileDatabase.GetAll();
    const bool objects = m_savedObjects && *m_savedObjects != g_MapObjectSoundDatabase.GetAll();
    return code || objects || SetsDiffer(m_savedSets);
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
    return saved;
}

bool SoundEdits::Revert(std::vector<Data::Items::ItemDataIssue>& issues)
{
    bool reverted = true;
    const std::vector<SoundFile> before = g_SoundFileDatabase.GetAll();
    if (Data::Sounds::RevertSoundFiles(issues))
    {
        ReloadChanged(before, g_SoundFileDatabase.GetAll());
    }
    else
    {
        reverted = false;
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
}

void SoundEdits::Clear()
{
    m_undo.clear();
    m_redo.clear();
    m_savedCode.reset();
    m_savedSets.clear();
    m_savedObjects.reset();
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
