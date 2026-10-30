#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterSoundsUI.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterSoundPreview.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorToolWindows.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterRandomVariant;
using Data::Monsters::MonsterSound;
using Data::Monsters::MonsterSoundSet;

// Pixel sizes before the editor UI scale.
constexpr float SetComboWidth = 220.0f;
constexpr float ListWidth = 230.0f;
constexpr float ListHeight = 320.0f;
constexpr float FileWidth = 260.0f;
constexpr float IdWidth = 200.0f;
constexpr float ChannelsWidth = 70.0f;
// The users a set lists by name; the others are counted.
constexpr size_t ShownUsers = 6;
constexpr int MaxSoundChannels = 4;

// A name for a new set, unused.
std::string FreeName(const std::string& base)
{
    std::string name = base;
    for (int number = 2; g_MonsterSoundSetDatabase.Find(name) != nullptr; ++number)
    {
        name = base + " " + std::to_string(number);
    }
    return name;
}

bool NamesFile(const MonsterSoundSet& set, const std::string& file)
{
    return std::any_of(set.sounds.begin(), set.sounds.end(),
                       [&](const MonsterSound& sound) { return sound.file == file; });
}

// Whether an event of the set plays the sound.
bool IsPlayed(const MonsterSoundSet& set, const MonsterSound& sound)
{
    const std::string& name = Data::Monsters::SoundNameOf(sound);
    return std::any_of(set.soundEvents.begin(), set.soundEvents.end(),
                       [&](const Data::Monsters::MonsterSoundEvent& event)
                       { return std::find(event.sounds.begin(), event.sounds.end(), name) != event.sounds.end(); });
}

// How many monsters have the model of the slot, for the cue rows.
int CountMonstersWithModel(int slot)
{
    int count = 0;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster != nullptr && Data::Monsters::FindMonsterModelReference(monster->model) == slot)
        {
            ++count;
        }
    }
    return count;
}
} // namespace

MuMonsterSoundsUI& MuMonsterSoundsUI::GetInstance()
{
    static MuMonsterSoundsUI instance;
    return instance;
}

MuMonsterSoundsUI::MuMonsterSoundsUI()
    : m_setDraft(
          [this](int id)
          {
              return id >= 0 && static_cast<size_t>(id) < m_idNames.size()
                         ? g_MonsterDataHandler.ReadSoundSet(m_idNames[static_cast<size_t>(id)])
                         : std::nullopt;
          },
          [](const std::string& label, const MonsterSoundSet& set)
          { g_MonsterEdits.SetSoundSet(label, set, g_MonsterSelection.GetNumber()); }),
      m_modelDraft([](int slot) { return g_MonsterDataHandler.ReadModel(slot); },
                   [](const std::string& label, const MonsterModelDefinition& model)
                   { g_MonsterEdits.SetModel(label, model, g_MonsterSelection.GetNumber()); })
{
}

int MuMonsterSoundsUI::IdOf(const std::string& name)
{
    const auto found = std::find(m_idNames.begin(), m_idNames.end(), name);
    if (found != m_idNames.end())
    {
        return static_cast<int>(found - m_idNames.begin());
    }
    m_idNames.push_back(name);
    return static_cast<int>(m_idNames.size() - 1);
}

void MuMonsterSoundsUI::ShowEvent(const std::string& set, std::optional<size_t> event)
{
    m_shown = set;
    m_focusEvent = event;
    g_MuEditorCore.GetToolWindows().monsterSounds = true;
    ImGui::SetWindowFocus(WindowIds::MonsterSounds);
}

const MonsterModelDefinition* MuMonsterSoundsUI::GetShownModelDraft(int slot) const
{
    // Only the draft of this frame: a hidden window keeps an old one.
    if (slot != m_shownSlot || m_shownFrame != ImGui::GetFrameCount())
    {
        return nullptr;
    }
    return m_shownModel;
}

const std::map<std::string, std::vector<int>, std::less<>>& MuMonsterSoundsUI::GetUsers()
{
    const long long version = ToolWindow::GetDataVersion();
    if (version == m_usersVersion)
    {
        return m_users;
    }
    m_users.clear();
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr)
        {
            continue;
        }
        if (!monster->soundSet.empty())
        {
            m_users[monster->soundSet].push_back(number);
        }
        for (const MonsterRandomVariant& variant : monster->randomVariants)
        {
            if (variant.soundSet && *variant.soundSet != monster->soundSet)
            {
                m_users[*variant.soundSet].push_back(number);
            }
        }
    }
    m_usersVersion = version;
    return m_users;
}

void MuMonsterSoundsUI::FollowSelection(const MonsterDefinition* monster)
{
    const int variant = monster != nullptr ? ShownModel::GetShownVariant(*monster) : -1;
    if (g_MonsterSelection.GetSerial() == m_shownSerial && variant == m_shownVariant)
    {
        return;
    }
    m_shownSerial = g_MonsterSelection.GetSerial();
    m_shownVariant = variant;
    if (monster == nullptr)
    {
        return;
    }
    const std::optional<int> slot = ShownModel::FindShownSlot(*monster);
    const std::string& name = slot ? Data::Monsters::GetSoundSetWithModel(*monster, *slot) : monster->soundSet;
    if (!name.empty())
    {
        m_shown = name;
    }
}

void MuMonsterSoundsUI::Render(bool* open)
{
    m_shownModel = nullptr;
    m_shownSlot = -1;
    if (!ToolWindow::Begin(I18N::Editor::MonsterSounds, WindowIds::MonsterSounds, open))
    {
        ToolWindow::End();
        return;
    }
    const int number = g_MonsterSelection.GetNumber();
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    FollowSelection(monster);
    ShownModel::RenderSelectedMonster(number, monster);
    if (monster != nullptr)
    {
        RenderMonster(*monster);
    }
    ImGui::Separator();

    // The model of the shown look, whose cues the cue rows change.
    const std::optional<int> slot = monster != nullptr ? ShownModel::FindShownSlot(*monster) : std::nullopt;
    MonsterModelDefinition* model = m_modelDraft.Begin(slot.value_or(-1), ToolWindow::GetDataVersion());
    SoundEventsEditor::Context context;
    if (model != nullptr && slot)
    {
        context.slot = *slot;
        context.model = model;
        context.modelDraft = &m_modelDraft;
        context.modelOpen = ShownModel::IsOpen(*slot);
        context.modelMonsters = CountMonstersWithModel(*slot);
        context.monster = number;
        m_shownModel = model;
        m_shownSlot = *slot;
        m_shownFrame = ImGui::GetFrameCount();
    }

    RenderSetList();
    ImGui::SameLine();
    ImGui::BeginGroup();
    // Begun and ended each frame, also without a set: the draft follows the
    // window being drawn.
    MonsterSoundSet* set = m_setDraft.Begin(IdOf(m_shown), ToolWindow::GetDataVersion());
    if (set == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoSoundSetIsShown);
    }
    else
    {
        context.focusedEvent = m_focusEvent;
        m_focusEvent.reset();
        RenderSet(*set, context);
    }
    m_setDraft.End();
    ImGui::EndGroup();
    m_modelDraft.End();
    ToolWindow::End();
}

bool MuMonsterSoundsUI::SetCombo(const char* id, std::string& value, const char* noneLabel)
{
    bool picked = false;
    ImGui::SetNextItemWidth(SetComboWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo(id, value.empty() ? noneLabel : value.c_str()))
    {
        if (ImGui::Selectable(noneLabel, value.empty()) && !value.empty())
        {
            value.clear();
            picked = true;
        }
        for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
        {
            if (ImGui::Selectable(set->name.c_str(), set->name == value) && set->name != value)
            {
                value = set->name;
                picked = true;
            }
        }
        ImGui::EndCombo();
    }
    return picked;
}

void MuMonsterSoundsUI::RenderMonster(const MonsterDefinition& monster)
{
    ImGui::TextUnformatted(I18N::Editor::SoundSet);
    ImGui::SameLine();
    std::string name = monster.soundSet;
    if (SetCombo("##MonsterSet", name, I18N::Editor::NoSoundSet))
    {
        MonsterDefinition changed = monster;
        changed.soundSet = name;
        g_MonsterEdits.SetMonster(StepLabel::SoundSetOf(changed), changed);
    }
    if (!monster.soundSet.empty())
    {
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Open))
        {
            m_shown = monster.soundSet;
        }
    }
    if (!monster.randomVariants.empty())
    {
        RenderVariants(monster);
    }
}

// The looks the game draws for the monster, each with its set (Q-MN83).
void MuMonsterSoundsUI::RenderVariants(const MonsterDefinition& monster)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::RandomVariants, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::RandomVariantsNote);
    ImGui::PopTextWrapPos();
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Variants", 3, flags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Variant);
    ImGui::TableSetupColumn(I18N::Editor::Model);
    ImGui::TableSetupColumn(I18N::Editor::SoundSet);
    ImGui::TableHeadersRow();
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(I18N::Editor::TheEntry);
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(monster.model.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(monster.soundSet.empty() ? I18N::Editor::NoSoundSet : monster.soundSet.c_str());
    for (size_t index = 0; index < monster.randomVariants.size(); ++index)
    {
        const MonsterRandomVariant& variant = monster.randomVariants[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::Variant0, {std::to_string(index + 1)}).c_str());
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(variant.model.c_str());
        ImGui::TableNextColumn();
        std::string name = variant.soundSet.value_or(std::string());
        if (SetCombo("##VariantSet", name, I18N::Editor::TheEntrySet))
        {
            MonsterDefinition changed = monster;
            std::optional<std::string>& value = changed.randomVariants[index].soundSet;
            value = name.empty() ? std::nullopt : std::optional<std::string>(name);
            g_MonsterEdits.SetMonster(StepLabel::SoundSetOf(changed), changed);
        }
        if (variant.soundSet)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(I18N::Editor::Open))
            {
                m_shown = *variant.soundSet;
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void MuMonsterSoundsUI::RenderSetList()
{
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::BeginGroup();
    const int monster = g_MonsterSelection.GetNumber();
    if (ImGui::Button(I18N::Editor::AddASoundSet))
    {
        MonsterSoundSet set;
        set.name = FreeName(I18N::Editor::NewSoundSet);
        if (g_MonsterEdits.SetSoundSet(StepLabel::SoundSet(set.name, I18N::Editor::StepWhatAdd), set, monster))
        {
            m_shown = set.name;
        }
    }
    ImGui::SameLine();
    const MonsterSoundSet* shown = m_shown.empty() ? nullptr : g_MonsterSoundSetDatabase.Find(m_shown);
    ImGui::BeginDisabled(shown == nullptr);
    if (ImGui::Button(I18N::Editor::Copy) && shown != nullptr)
    {
        MonsterSoundSet copy = *shown;
        copy.name = FreeName(I18N::Format(I18N::Editor::SoundSetCopyOf0, {shown->name}));
        if (g_MonsterEdits.SetSoundSet(StepLabel::SoundSet(copy.name, I18N::Editor::StepWhatAdd), copy, monster))
        {
            m_shown = copy.name;
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    const auto& users = GetUsers();
    const auto shownUsers = shown != nullptr ? users.find(shown->name) : users.end();
    // A set that monsters name stays until they name another.
    const bool used = shownUsers != users.end() && !shownUsers->second.empty();
    ImGui::BeginDisabled(shown == nullptr || used);
    if (ImGui::Button(I18N::Editor::Remove) && shown != nullptr)
    {
        const std::string name = shown->name;
        if (g_MonsterEdits.RemoveSoundSet(StepLabel::SoundSet(name, I18N::Editor::StepWhatRemove), name, monster))
        {
            m_shown.clear();
        }
    }
    ImGui::EndDisabled();
    if (used && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::SoundSetInUseNote);
    }
    if (ImGui::BeginChild("##Sets", ImVec2(ListWidth * scale, ListHeight * scale),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
    {
        for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
        {
            const auto found = users.find(set->name);
            const size_t count = found != users.end() ? found->second.size() : 0;
            const std::string label = set->name + " (" + std::to_string(count) + ")##" + set->name;
            if (ImGui::Selectable(label.c_str(), set->name == m_shown))
            {
                m_shown = set->name;
            }
        }
    }
    ImGui::EndChild();
    ImGui::EndGroup();
}

void MuMonsterSoundsUI::RenderSet(MonsterSoundSet& set, const SoundEventsEditor::Context& context)
{
    RenderName(set);
    RenderUsers(set);
    if (ImGui::CollapsingHeader(I18N::Format(I18N::Editor::SoundFiles0, {std::to_string(set.sounds.size())}).c_str(),
                                ImGuiTreeNodeFlags_DefaultOpen))
    {
        SoundPreview::RenderVolume();
        RenderFiles(set);
        RenderAddFile(set);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::SoundEvents, ImGuiTreeNodeFlags_DefaultOpen))
    {
        SoundEventsEditor::Render(set, m_setDraft, context);
    }
}

// The name, renamed when the field is left: the monsters that name the set
// get the new name in the same undo step.
void MuMonsterSoundsUI::RenderName(const MonsterSoundSet& set)
{
    if (m_nameEditOf != set.name && !m_nameActive)
    {
        m_nameEdit = set.name;
        m_nameEditOf = set.name;
    }
    ImGui::TextUnformatted(I18N::Editor::SoundSetName);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(SetComboWidth * g_MuEditorCore.GetUIScale());
    Items::InputString("##SetName", m_nameEdit);
    m_nameActive = ImGui::IsItemActive();
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        const std::string from = set.name;
        const std::string to = m_nameEdit;
        if (g_MonsterEdits.RenameSoundSet(StepLabel::SoundSet(from, I18N::Editor::StepWhatName), from, to,
                                          g_MonsterSelection.GetNumber()))
        {
            m_shown = to;
        }
        m_nameEditOf.clear();
    }
}

void MuMonsterSoundsUI::RenderUsers(const MonsterSoundSet& set)
{
    const auto& users = GetUsers();
    const auto found = users.find(set.name);
    ImGui::TextUnformatted(I18N::Editor::PlayedBy);
    if (found == users.end() || found->second.empty())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Editor::NoMonsterPlaysTheSet);
        return;
    }
    const std::vector<int>& numbers = found->second;
    for (size_t index = 0; index < numbers.size() && index < ShownUsers; ++index)
    {
        const int number = numbers[index];
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        const std::string label = std::to_string(number) + " " +
                                  (monster != nullptr ? monster->names.Get(I18N::GetCurrentLocale()) : std::string());
        ImGui::SameLine();
        ImGui::PushID(number);
        if (ImGui::SmallButton(label.c_str()))
        {
            g_MonsterSelection.Select(number);
        }
        ImGui::PopID();
    }
    if (numbers.size() > ShownUsers)
    {
        ImGui::SameLine();
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::And0More, {std::to_string(numbers.size() - ShownUsers)}).c_str());
    }
}

void MuMonsterSoundsUI::RenderFiles(MonsterSoundSet& set)
{
    if (set.sounds.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheSetHasNoSounds);
        return;
    }
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Sounds", 5, flags))
    {
        return;
    }
    ImGui::TableSetupColumn("##Play");
    ImGui::TableSetupColumn(I18N::Editor::SoundFile);
    ImGui::TableSetupColumn(I18N::Editor::SoundId);
    ImGui::TableSetupColumn(I18N::Editor::SoundChannels);
    ImGui::TableSetupColumn("##Remove");
    ImGui::TableHeadersRow();
    std::optional<size_t> removed;
    for (size_t index = 0; index < set.sounds.size(); ++index)
    {
        const MonsterSound& sound = set.sounds[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::SmallButton(I18N::Editor::PlayThisSound))
        {
            SoundPreview::PlaySoundOfSet(set, Data::Monsters::SoundNameOf(sound));
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(sound.file.c_str());
        ImGui::TableNextColumn();
        if (sound.id.empty())
        {
            ImGui::TextDisabled("%s", I18N::Editor::NumberOfItsFile);
        }
        else
        {
            ImGui::TextUnformatted(sound.id.c_str());
        }
        ImGui::TableNextColumn();
        ImGui::Text("%d%s", sound.channels, sound.spatial ? ", 3D" : "");
        ImGui::TableNextColumn();
        // An event that plays the sound would name a sound the set lacks.
        const bool played = IsPlayed(set, sound);
        ImGui::BeginDisabled(played);
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = index;
        }
        ImGui::EndDisabled();
        if (played && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("%s", I18N::Editor::SoundFileInUseNote);
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
    if (removed)
    {
        set.sounds.erase(set.sounds.begin() + static_cast<std::ptrdiff_t>(*removed));
        m_setDraft.AfterChange(true, StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundFiles));
    }
}

void MuMonsterSoundsUI::RenderAddFile(MonsterSoundSet& set)
{
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(FileWidth * scale);
    Items::InputStringWithHint("##NewFile", "Data/Sound/mBull1.wav", m_newFile);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(IdWidth * scale);
    Items::InputStringWithHint("##NewId", I18N::Editor::SoundIdHint, m_newId);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ChannelsWidth * scale);
    ImGui::InputInt("##NewChannels", &m_newChannels, 1);
    m_newChannels = std::clamp(m_newChannels, 1, MaxSoundChannels);
    ImGui::SameLine();
    ImGui::Checkbox("3D", &m_newSpatial);
    ImGui::SameLine();
    const std::string problem =
        m_newFile.empty() ? std::string(I18N::Editor::SoundFileMissing) : Data::Paths::FindPathProblem(m_newFile);
    const bool twice = NamesFile(set, m_newFile);
    ImGui::BeginDisabled(!problem.empty() || twice);
    if (ImGui::Button(I18N::Editor::AddAFile))
    {
        MonsterSound sound;
        sound.file = m_newFile;
        sound.id = m_newId;
        sound.channels = m_newChannels;
        sound.spatial = m_newSpatial;
        set.sounds.push_back(sound);
        m_setDraft.AfterChange(true, StepLabel::SoundSet(set.name, I18N::Editor::StepWhatSoundFiles));
        m_newFile.clear();
        m_newId.clear();
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && (!problem.empty() || twice))
    {
        ImGui::SetTooltip("%s", twice ? I18N::Editor::SoundFileTwice : problem.c_str());
    }
    if (!m_newFile.empty() && problem.empty() && !Data::Monsters::IsInClientFolder(m_newFile))
    {
        ImGui::TextDisabled("%s", I18N::Editor::SoundFileNotInClientFolder);
    }
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
