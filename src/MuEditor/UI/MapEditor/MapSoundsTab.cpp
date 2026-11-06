#include "stdafx.h"

#ifdef _EDITOR

#include "MapSoundsTab.h"

#include "MapObjectPlace.h"

#include "../MuEditor/UI/Common/MapPicker.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "../MuEditor/UI/SoundEditor/SoundEditorEdits.h"
#include "Audio/DSPlaySound.h"
#include "Audio/SoundNumbers.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/MapData/MapObjectSoundStorage.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>

namespace
{
using Data::Maps::MapObjectSound;

constexpr float FileWidth = 260.0f;
constexpr float ChannelsWidth = 80.0f;

std::string ObjectsText(const std::vector<int>& objects)
{
    std::string text;
    for (const int type : objects)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(type);
    }
    return text;
}

// Plays the file as the map would, loaded under its number first.
void PreviewSound(const MapObjectSound& sound)
{
    const std::wstring file = Data::Paths::ToLoaderPath(sound.file);
    const ESound number = Audio::SoundNumbers::ForFile(file);
    if (number == Audio::SoundNumbers::NoNumber)
    {
        return;
    }
    if (!IsSoundLoaded(number, file.c_str(), sound.channels, false))
    {
        LoadWaveFile(number, file.c_str(), sound.channels, false);
    }
    PlayBufferAtLevel(number, 1.0f);
}
} // namespace

CMapSoundsTab& CMapSoundsTab::GetInstance()
{
    static CMapSoundsTab instance;
    return instance;
}

bool CMapSoundsTab::IsModified() const
{
    return m_saved && *m_saved != g_MapObjectSoundDatabase.GetAll();
}

void CMapSoundsTab::Render(int serverMap, int stage, int world)
{
    if (!m_saved)
    {
        m_saved = g_MapObjectSoundDatabase.GetAll();
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::MapSoundsNote);
    ImGui::PopTextWrapPos();
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::MapSoundsOf01,
                                        {MuEditor::MapPicker::NameOf({serverMap, stage}), std::to_string(serverMap)})
                               .c_str());
    RenderStatus();
    if (ImGui::Button(I18N::Editor::AddASoundToAnObjectType))
    {
        Add(serverMap, stage);
    }
    RenderEntries(serverMap, stage);
    if (m_selected && *m_selected < g_MapObjectSoundDatabase.GetAll().size())
    {
        RenderEntry(*m_selected, world);
    }
}

void CMapSoundsTab::RenderStatus()
{
    ImGui::BeginDisabled(!IsModified());
    if (ImGui::Button(I18N::Editor::Save))
    {
        Save();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Revert))
    {
        Revert();
    }
    if (IsModified())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::NotSavedYet);
    }
    if (!m_status.empty())
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", m_status.c_str());
        ImGui::PopTextWrapPos();
    }
}

void CMapSoundsTab::RenderEntries(int serverMap, int stage)
{
    const std::vector<MapObjectSound>& sounds = g_MapObjectSoundDatabase.GetAll();
    std::vector<size_t> shown;
    for (size_t index = 0; index < sounds.size(); ++index)
    {
        if (Data::Maps::Names(sounds[index].maps, serverMap, stage))
        {
            shown.push_back(index);
        }
    }
    if (shown.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoObjectOfThisMapMakesASound);
        return;
    }
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##ObjectSounds", 5, flags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::ObjectTypes);
    ImGui::TableSetupColumn(I18N::Editor::SoundFile);
    ImGui::TableSetupColumn(I18N::Editor::SoundChannels);
    ImGui::TableSetupColumn(I18N::Editor::ObjectSoundPlays);
    ImGui::TableSetupColumn(I18N::Editor::ObjectSoundMaps);
    ImGui::TableHeadersRow();
    for (const size_t index : shown)
    {
        const MapObjectSound& sound = sounds[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const std::string objects = sound.anyObject         ? std::string(I18N::Editor::AnyObject)
                                    : sound.objects.empty() ? std::string("-")
                                                            : ObjectsText(sound.objects);
        if (ImGui::Selectable(objects.c_str(), m_selected == index, ImGuiSelectableFlags_SpanAllColumns))
        {
            m_selected = index;
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(sound.file.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d", sound.channels);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(I18N::Editor::WhileInView);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(MuEditor::MapPicker::Describe(sound.maps, I18N::Editor::NoMap).c_str());
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void CMapSoundsTab::RenderEntry(size_t index, int world)
{
    MapObjectSound sound = g_MapObjectSoundDatabase.GetAll()[index];
    bool changed = false;
    ImGui::PushID(static_cast<int>(index));
    ImGui::SeparatorText(sound.file.empty() ? I18N::Editor::AddASoundToAnObjectType : sound.file.c_str());
    RenderObjects(sound, world, changed);
    RenderFile(sound, index, changed);

    ImGui::TextUnformatted(I18N::Editor::SoundChannels);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ChannelsWidth);
    int channels = sound.channels;
    if (ImGui::InputInt("##Channels", &channels, 1))
    {
        channels = std::clamp(channels, 1, Data::Maps::MaxMapSoundChannels);
        changed = changed || channels != sound.channels;
        sound.channels = channels;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(I18N::Editor::ObjectSoundPlays);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(FileWidth * 0.5f);
    if (ImGui::BeginCombo("##Plays", I18N::Editor::WhileInView))
    {
        ImGui::Selectable(I18N::Editor::WhileInView, true);
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::WhileInViewNote);
    }

    ImGui::TextUnformatted(I18N::Editor::ObjectSoundMaps);
    ImGui::SameLine();
    if (MuEditor::MapPicker::Render("ObjectSoundMaps", sound.maps, I18N::Editor::NoMap))
    {
        changed = true;
    }
    if (ImGui::Button(I18N::Editor::Remove))
    {
        m_picking = false;
        m_selected.reset();
        m_fileEditOf.reset();
        Store(index, std::nullopt);
        ImGui::PopID();
        return;
    }
    if (changed)
    {
        Store(index, sound);
    }
    ImGui::PopID();
}

void CMapSoundsTab::RenderObjects(MapObjectSound& sound, int world, bool& changed)
{
    ImGui::TextUnformatted(I18N::Editor::ObjectTypes);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::AnyObject, sound.anyObject) && !sound.anyObject)
    {
        sound.anyObject = true;
        sound.objects.clear();
        m_picking = false;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::AnyObjectNote);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::ObjectSoundTypes, !sound.anyObject) && sound.anyObject)
    {
        sound.anyObject = false;
        changed = true;
    }
    if (sound.anyObject)
    {
        return;
    }
    std::optional<int> removed;
    for (const int type : sound.objects)
    {
        ImGui::SameLine();
        ImGui::PushID(type);
        if (ImGui::SmallButton((std::to_string(type) + " x").c_str()))
        {
            removed = type;
        }
        ImGui::PopID();
    }
    if (removed)
    {
        std::erase(sound.objects, *removed);
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(FileWidth * 0.6f);
    if (ImGui::BeginCombo("##AddType", I18N::Editor::AddAnObjectType))
    {
        for (const Editor::ObjectPlace::ModelEntry& model : Editor::ObjectPlace::EnumerateModels(world))
        {
            if (std::find(sound.objects.begin(), sound.objects.end(), model.type) != sound.objects.end())
            {
                continue;
            }
            const std::string label = std::to_string(model.type) + " " + Core::Text::ToUtf8(model.file.c_str());
            if (ImGui::Selectable(label.c_str()))
            {
                sound.objects.push_back(model.type);
                std::sort(sound.objects.begin(), sound.objects.end());
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button(m_picking ? I18N::Editor::ClickAnObjectInTheWorld : I18N::Editor::PickAnObjectInTheWorld))
    {
        m_picking = !m_picking;
    }
}

void CMapSoundsTab::RenderFile(MapObjectSound& sound, size_t index, bool& changed)
{
    if (m_fileEditOf != index && !m_fileActive)
    {
        m_fileEdit = sound.file;
        m_fileEditOf = index;
    }
    ImGui::TextUnformatted(I18N::Editor::SoundFile);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(FileWidth);
    MuEditor::Items::InputStringWithHint("##File", "Data/Sound/aKalimaWaterFall.wav", m_fileEdit);
    m_fileActive = ImGui::IsItemActive();
    if (ImGui::IsItemDeactivatedAfterEdit() && m_fileEdit != sound.file)
    {
        sound.file = m_fileEdit;
        changed = true;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(sound.file.empty());
    if (ImGui::SmallButton(I18N::Editor::PlayThisSound))
    {
        PreviewSound(sound);
    }
    ImGui::EndDisabled();
    if (const std::string problem = sound.file.empty() ? std::string() : Data::Paths::FindPathProblem(sound.file);
        !problem.empty())
    {
        ImGui::TextDisabled("%s", problem.c_str());
    }
}

void CMapSoundsTab::Add(int serverMap, int stage)
{
    MapObjectSound sound;
    sound.maps = {{serverMap, stage}};
    const size_t index = g_MapObjectSoundDatabase.GetAll().size();
    Store(index, sound);
    m_selected = index;
}

void CMapSoundsTab::TakeSaved()
{
    m_saved = g_MapObjectSoundDatabase.GetAll();
}

void CMapSoundsTab::Picked(int objectType)
{
    m_picking = false;
    if (!m_selected || *m_selected >= g_MapObjectSoundDatabase.GetAll().size() || objectType < 0 ||
        objectType > Data::Maps::MaxMapObjectType)
    {
        return;
    }
    MapObjectSound sound = g_MapObjectSoundDatabase.GetAll()[*m_selected];
    if (std::find(sound.objects.begin(), sound.objects.end(), objectType) != sound.objects.end())
    {
        return;
    }
    // A picked object makes an entry of any object one of types.
    sound.anyObject = false;
    sound.objects.push_back(objectType);
    std::sort(sound.objects.begin(), sound.objects.end());
    Store(*m_selected, sound);
}

void CMapSoundsTab::Store(size_t index, const std::optional<MapObjectSound>& sound)
{
    std::vector<MapObjectSound> sounds = g_MapObjectSoundDatabase.GetAll();
    if (sound && index >= sounds.size())
    {
        sounds.push_back(*sound);
    }
    else if (sound)
    {
        sounds[index] = *sound;
    }
    else if (index < sounds.size())
    {
        sounds.erase(sounds.begin() + static_cast<std::ptrdiff_t>(index));
    }
    g_MapObjectSoundDatabase.Build(sounds);
}

void CMapSoundsTab::Save()
{
    std::vector<Data::Items::ItemDataIssue> issues;
    if (Data::Maps::SaveMapObjectSounds(issues))
    {
        m_saved = g_MapObjectSoundDatabase.GetAll();
        g_SoundEdits.TakeSavedObjects();
        m_status = I18N::Format(I18N::Editor::SavedTo0, {Data::Maps::GetMapObjectSoundsFile().generic_string()});
        return;
    }
    const auto fatal = std::find_if(issues.begin(), issues.end(), [](const Data::Items::ItemDataIssue& issue)
                                    { return issue.severity == Data::Items::ItemDataIssueSeverity::Fatal; });
    m_status = I18N::Format(I18N::Editor::NotSaved0, {fatal != issues.end() ? fatal->ToString() : std::string()});
}

void CMapSoundsTab::Revert()
{
    std::vector<Data::Items::ItemDataIssue> issues;
    m_picking = false;
    m_selected.reset();
    m_fileEditOf.reset();
    if (Data::Maps::RevertMapObjectSounds(issues))
    {
        m_saved = g_MapObjectSoundDatabase.GetAll();
        g_SoundEdits.TakeSavedObjects();
        m_status.clear();
        return;
    }
    const auto fatal = std::find_if(issues.begin(), issues.end(), [](const Data::Items::ItemDataIssue& issue)
                                    { return issue.severity == Data::Items::ItemDataIssueSeverity::Fatal; });
    m_status = I18N::Format(I18N::Editor::NotReadAgain0, {fatal != issues.end() ? fatal->ToString() : std::string()});
}

#endif // _EDITOR
