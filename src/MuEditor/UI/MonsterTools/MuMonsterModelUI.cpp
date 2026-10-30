#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterModelUI.h"

#include "MonsterAnimationFacts.h"
#include "MonsterEdits.h"
#include "MonsterModelValues.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"
#include "MuMonsterSoundsUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemModelFiles.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;

// Pixel sizes before the editor UI scale.
constexpr float SearchWidth = 220.0f;
constexpr float FileListHeight = 160.0f;
constexpr float SpeedWidth = 90.0f;
// How many users the slot line lists before "and N more".
constexpr int ShownUsers = 12;
constexpr float SpeedStep = 0.005f;
constexpr float MaxSpeed = 5.0f;
constexpr ImVec4 WarningColor{1.0f, 0.75f, 0.3f, 1.0f};

void RenderWarning(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

std::string StepLabelOf(const MonsterModelDefinition& model, const std::string& what)
{
    return StepLabel::Model(model.slot, what);
}

// The model and part files of the model list, whose folders the file
// picker lists.
std::vector<std::string> ListedFiles()
{
    std::vector<std::string> files;
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        files.push_back(model->load.file);
        for (const Data::Monsters::MonsterModelPart& part : model->parts)
        {
            files.push_back(part.load.file);
        }
    }
    return files;
}

} // namespace

const MonsterModelDefinition* MuMonsterModelUI::GetShownDraft(int slot) const
{
    // Only the draft of this frame: a hidden window keeps an old one.
    if (m_shownDraft == nullptr || m_shownSlot != slot || m_shownFrame != ImGui::GetFrameCount())
    {
        return nullptr;
    }
    return m_shownDraft;
}

MuMonsterModelUI& MuMonsterModelUI::GetInstance()
{
    static MuMonsterModelUI instance;
    return instance;
}

MuMonsterModelUI::MuMonsterModelUI()
    : m_draft([](int slot) { return g_MonsterDataHandler.ReadModel(slot); },
              [](const std::string& label, const MonsterModelDefinition& model)
              { g_MonsterEdits.SetModel(label, model, g_MonsterSelection.GetNumber()); })
{
}

void MuMonsterModelUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterModel, WindowIds::MonsterModel, open))
    {
        const int number = g_MonsterSelection.GetNumber();
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        const std::optional<int> slot = monster != nullptr ? ShownModel::FindShownSlot(*monster) : std::nullopt;
        MonsterModelDefinition* model = m_draft.Begin(slot.value_or(-1), ToolWindow::GetDataVersion());
        ShownModel::RenderSelectedMonster(number, monster);
        if (monster != nullptr && (!slot || model == nullptr))
        {
            ShownModel::RenderWhyNoModel(*monster);
        }
        else if (model != nullptr)
        {
            ImGui::PushID(*slot);
            RenderModel(*model, *slot);
            ImGui::PopID();
        }
        m_draft.End();
        // What the preview drawn after this window plays, also while a speed
        // is being dragged.
        if (model != nullptr && slot)
        {
            m_shownDraft = model;
            m_shownSlot = *slot;
            m_shownFrame = ImGui::GetFrameCount();
        }
    }
    ToolWindow::End();
}

void MuMonsterModelUI::RenderModel(MonsterModelDefinition& model, int slot)
{
    RenderSlot(slot);
    RenderUsers(slot);
    if (ImGui::CollapsingHeader(I18N::Editor::ModelFile, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderFile(model);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::TextureFolders, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderTextureFolders(model);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::Animations, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderAnimations(model, slot);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::Sounds, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderSounds(slot);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::Cues, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderCues(model);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::MoreOfTheModelEntry))
    {
        RenderMore(model);
    }
}

void MuMonsterModelUI::RenderSlot(int slot)
{
    const std::string symbol(Data::Monsters::GetMonsterModelSymbol(slot));
    ImGui::Text("%s (%d)", symbol.c_str(), slot);
    ImGui::SameLine();
    const bool monsterRange = AnimationFacts::IsMonsterRange(slot);
    ImGui::TextDisabled("%s", monsterRange ? I18N::Editor::MonsterRange : I18N::Editor::NpcRange);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", monsterRange ? I18N::Editor::MonsterRangeNote : I18N::Editor::NpcRangeNote);
    }
}

void MuMonsterModelUI::RenderUsers(int slot)
{
    if (slot != m_usersSlot || ToolWindow::GetDataVersion() != m_usersVersion)
    {
        m_users.clear();
        for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
        {
            const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
            if (monster != nullptr && Data::Monsters::FindMonsterModelReference(monster->model) == slot)
            {
                m_users.push_back(number);
            }
        }
        m_usersSlot = slot;
        m_usersVersion = ToolWindow::GetDataVersion();
    }
    const std::vector<int>& users = m_users;
    ImGui::TextUnformatted(I18N::Editor::MonstersWithTheModel);
    for (size_t index = 0; index < users.size() && index < static_cast<size_t>(ShownUsers); ++index)
    {
        const int number = users[index];
        const std::string label =
            std::to_string(number) + " " + g_MonsterDatabase.Find(number)->names.Get(I18N::GetCurrentLocale());
        ImGui::SameLine();
        ImGui::PushID(number);
        // The selected monster is the one the tool shows.
        if (ImGui::SmallButton(label.c_str()))
        {
            g_MonsterSelection.Select(number);
        }
        ImGui::PopID();
    }
    if (users.size() > static_cast<size_t>(ShownUsers))
    {
        ImGui::SameLine();
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::And0More, {std::to_string(users.size() - static_cast<size_t>(ShownUsers))})
                      .c_str());
    }
}

void MuMonsterModelUI::RenderFile(MonsterModelDefinition& model)
{
    ImGui::TextUnformatted(model.load.file.c_str());
    if (!Data::Monsters::IsInClientFolder(model.load.file))
    {
        RenderWarning(I18N::Editor::TheFileIsNotInTheClientFolderTheModelHasNoBody);
    }
    RenderFilePicker(model);
}

void MuMonsterModelUI::RenderFilePicker(MonsterModelDefinition& model)
{
    const float scale = g_MuEditorCore.GetUIScale();
    ImGui::SetNextItemWidth(SearchWidth * scale);
    Items::InputStringWithHint("##FileSearch", I18N::Editor::SearchModelFiles, m_fileSearch);
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Refresh))
    {
        m_files.Refresh();
    }
    if (g_MonsterModelDatabase.GetVersion() != m_listedVersion)
    {
        m_listedFiles = ListedFiles();
        m_listedVersion = g_MonsterModelDatabase.GetVersion();
    }
    const std::vector<std::string>& listed = m_listedFiles;
    if (!ImGui::BeginChild("##Files", ImVec2(0.0f, FileListHeight * scale), ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        return;
    }
    std::optional<std::string> picked;
    for (const std::string_view file : m_files.Find(listed, m_fileSearch))
    {
        const std::string text(file);
        if (ImGui::Selectable(text.c_str(), text == model.load.file))
        {
            picked = text;
        }
    }
    ImGui::EndChild();
    if (picked && *picked != model.load.file)
    {
        model.load.textureFolders =
            Items::ItemModelFiles::GetTextureFoldersForFile(model.load.textureFolders, model.load.file, *picked);
        model.load.file = *picked;
        m_draft.AfterChange(true, StepLabelOf(model, I18N::Editor::StepWhatFile));
    }
}

void MuMonsterModelUI::RenderTextureFolders(MonsterModelDefinition& model)
{
    ImGui::TextDisabled("%s", I18N::Editor::TextureFoldersNote);
    std::optional<size_t> removed;
    for (size_t index = 0; index < model.load.textureFolders.size(); ++index)
    {
        const std::string& folder = model.load.textureFolders[index];
        ImGui::PushID(static_cast<int>(index));
        // The model needs a folder: the last one stays.
        ImGui::BeginDisabled(model.load.textureFolders.size() == 1);
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = index;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextUnformatted(folder.c_str());
        if (!Data::Monsters::IsInClientFolder("Data/" + folder, true))
        {
            ImGui::SameLine();
            RenderWarning(I18N::Editor::NotInTheClientFolder);
        }
        ImGui::PopID();
    }
    if (removed)
    {
        model.load.textureFolders.erase(model.load.textureFolders.begin() + static_cast<std::ptrdiff_t>(*removed));
        m_draft.AfterChange(true, StepLabelOf(model, I18N::Editor::StepWhatTextureFolders));
    }
    ImGui::SetNextItemWidth(SearchWidth * g_MuEditorCore.GetUIScale());
    Items::InputStringWithHint("##NewFolder", I18N::Editor::AddAFolder, m_newFolder);
    ImGui::SameLine();
    ImGui::BeginDisabled(m_newFolder.empty());
    if (ImGui::Button(I18N::Editor::Add))
    {
        model.load.textureFolders.push_back(m_newFolder);
        m_newFolder.clear();
        m_draft.AfterChange(true, StepLabelOf(model, I18N::Editor::StepWhatTextureFolders));
    }
    ImGui::EndDisabled();
}

void MuMonsterModelUI::RenderAnimations(MonsterModelDefinition& model, int slot)
{
    ShownModel::OpenWhenShown(slot);
    if (!ShownModel::IsOpen(slot))
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::TheModelIsNotOpenItsAnimationsShowWhenItIs);
        ImGui::PopTextWrapPos();
        return;
    }
    const int count = Models[slot].NumActions;
    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##Animations", 5, flags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Animation);
    ImGui::TableSetupColumn(I18N::Editor::Speed);
    ImGui::TableSetupColumn(I18N::Editor::HoldsLastFrame);
    ImGui::TableSetupColumn("##Default");
    ImGui::TableSetupColumn(I18N::Editor::InGame, ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    const float scale = g_MuEditorCore.GetUIScale();
    for (int animation = 0; animation < count; ++animation)
    {
        ImGui::PushID(animation);
        const std::string name = Data::Monsters::GetMonsterAnimationName(animation);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d %s", animation, name.c_str());

        ImGui::TableNextColumn();
        float speed = ModelValues::GetSpeed(model, slot, animation);
        ImGui::SetNextItemWidth(SpeedWidth * scale);
        if (ImGui::DragFloat("##Speed", &speed, SpeedStep, 0.0f, MaxSpeed, "%.3f"))
        {
            ModelValues::SetSpeed(model, slot, animation, std::clamp(speed, 0.0f, MaxSpeed));
        }
        m_draft.AfterField(StepLabelOf(model, I18N::Format(I18N::Editor::StepWhatSpeedOf0, {name})));

        ImGui::TableNextColumn();
        bool hold = ModelValues::HoldsLastFrame(model, slot, animation);
        if (ImGui::Checkbox("##Hold", &hold))
        {
            ModelValues::SetHold(model, slot, count, animation, hold);
            m_draft.AfterChange(true, StepLabelOf(model, I18N::Format(I18N::Editor::StepWhatHoldOf0, {name})));
        }

        ImGui::TableNextColumn();
        if (ModelValues::HasOwnSpeed(model, animation))
        {
            if (ImGui::SmallButton(I18N::Editor::Default))
            {
                ModelValues::SetSpeed(model, slot, animation, ModelValues::GetDefaultSpeed(slot, animation));
                m_draft.AfterChange(true, StepLabelOf(model, I18N::Format(I18N::Editor::StepWhatSpeedOf0, {name})));
            }
        }
        else
        {
            ImGui::TextDisabled("%s", I18N::Editor::RangeDefault);
        }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted(AnimationFacts::DescribeInGame(slot, animation).c_str());
        ImGui::PopID();
    }
    ImGui::EndTable();
}

// The sounds are the monster's sound set's (Q-MN78): the set the selected
// monster plays with this model, and where it is edited.
void MuMonsterModelUI::RenderSounds(int slot)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(g_MonsterSelection.GetNumber());
    const std::string set = monster != nullptr ? Data::Monsters::GetSoundSetWithModel(*monster, slot) : std::string();
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::ModelSoundsNote);
    ImGui::PopTextWrapPos();
    if (set.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheMonsterNamesNoSoundSet);
        return;
    }
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheMonsterPlays0, {set}).c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::OpenInMonsterSounds))
    {
        g_MuMonsterSoundsUI.ShowEvent(set, std::nullopt);
    }
}

// The cues, which sound events play on (Q-MN79); their frames are edited on
// the events in Monster Sounds (Q-MN81).
void MuMonsterModelUI::RenderCues(const MonsterModelDefinition& model)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::CuesNote);
    ImGui::PopTextWrapPos();
    if (model.cues.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheModelHasNoCues);
        return;
    }
    for (const Data::Monsters::MonsterModelCue& cue : model.cues)
    {
        char frames[64];
        std::snprintf(frames, sizeof(frames), "%.2f - %.2f", static_cast<double>(cue.frames.first),
                      static_cast<double>(cue.frames.second));
        ImGui::BulletText("%s: %s %s", cue.name.c_str(), Data::Monsters::GetMonsterAnimationName(cue.animation).c_str(),
                          frames);
    }
}

void MuMonsterModelUI::RenderMore(const MonsterModelDefinition& model)
{
    ImGui::TextDisabled("%s", I18N::Editor::TheseValuesAreEditedInTheFile);
    const auto line = [](const char* label, const std::string& value)
    { ImGui::BulletText("%s: %s", label, value.c_str()); };
    const auto number = [](const std::optional<int>& value)
    { return value ? std::to_string(*value) : std::string(I18N::Editor::AsTheFileHasIt); };
    line(I18N::Editor::HeadBone, number(model.headBone));
    line(I18N::Editor::StreamMesh, number(model.streamMesh));
    std::string meshes;
    for (const int mesh : model.noneBlendMeshes)
    {
        meshes += (meshes.empty() ? "" : ", ") + std::to_string(mesh);
    }
    line(I18N::Editor::MeshesWithoutBlending, meshes.empty() ? std::string("-") : meshes);
    line(I18N::Editor::PartModels, std::to_string(model.parts.size()));
    line(I18N::Editor::FixedBitmaps, std::to_string(model.bitmaps.size()));
    line(I18N::Editor::TextureNamesByMesh, std::to_string(model.textureNames.size()));
    line(I18N::Editor::TextureFoldersOnMaps, std::to_string(model.mapTextureFolders.size()));
    const auto vector = [](const std::optional<std::array<float, 3>>& value)
    {
        if (!value)
        {
            return std::string(I18N::Editor::Default);
        }
        char text[64];
        std::snprintf(text, sizeof(text), "%g, %g, %g", (*value)[0], (*value)[1], (*value)[2]);
        return std::string(text);
    };
    line(I18N::Editor::BoundingBoxMin, vector(model.body.boxMin));
    line(I18N::Editor::BoundingBoxMax, vector(model.body.boxMax));
    std::string bones = model.body.linkBones ? std::string() : std::string(I18N::Editor::Default);
    if (model.body.linkBones)
    {
        for (const int bone : *model.body.linkBones)
        {
            bones += (bones.empty() ? "" : ", ") + std::to_string(bone);
        }
    }
    line(I18N::Editor::WeaponBones, bones.empty() ? std::string("-") : bones);
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
