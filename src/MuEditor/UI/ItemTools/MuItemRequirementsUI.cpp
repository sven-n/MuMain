#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemRequirementsUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <array>
#include <cstdio>
#include <optional>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;

namespace
{
constexpr float LevelValueWidth = 46.0f;
// The item levels shown in one row of a requirement given by level.
constexpr size_t LevelsPerRow = 8;

std::optional<ItemDefinition> LoadItem(int itemType)
{
    const ItemDefinition* item = g_ItemDatabase.Find(itemType);
    return item != nullptr ? std::optional<ItemDefinition>(*item) : std::nullopt;
}

void StoreItem(const std::string& label, const ItemDefinition& item)
{
    g_ItemEdits.SetItem(label, item);
}

struct RequirementField
{
    ItemRequirement requirement;
    WORD Data::Items::ItemRequirements::* value;
    const char* label;
};

std::array<RequirementField, Data::Items::ItemRequirementCount> RequirementFields()
{
    using Data::Items::ItemRequirements;
    return {{{ItemRequirement::Level, &ItemRequirements::level, I18N::Metadata::ReqLevel},
             {ItemRequirement::Strength, &ItemRequirements::strength, I18N::Metadata::ReqStrength},
             {ItemRequirement::Dexterity, &ItemRequirements::dexterity, I18N::Metadata::ReqDexterity},
             {ItemRequirement::Energy, &ItemRequirements::energy, I18N::Metadata::ReqEnergy},
             {ItemRequirement::Vitality, &ItemRequirements::vitality, I18N::Metadata::ReqVitality},
             {ItemRequirement::Leadership, &ItemRequirements::leadership, I18N::Metadata::ReqLeadership}}};
}

// The class names in the order of ItemDefinition::classRequirements.
std::array<const char*, MAX_CLASS> ClassLabels()
{
    return {I18N::Metadata::DWSM, I18N::Metadata::DKBK,  I18N::Metadata::ELFME, I18N::Metadata::MGDM,
            I18N::Metadata::DLLE, I18N::Metadata::SUMBS, I18N::Metadata::RFFM};
}
} // namespace

CMuItemRequirementsUI& CMuItemRequirementsUI::GetInstance()
{
    static CMuItemRequirementsUI instance;
    return instance;
}

CMuItemRequirementsUI::CMuItemRequirementsUI() : m_draft(&LoadItem, &StoreItem) {}

void CMuItemRequirementsUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemRequirements, MuEditor::WindowIds::ItemRequirements, open))
    {
        ToolWindow::RenderUndoRedo();
        const int itemType = g_ItemSelection.GetItemType();
        ItemDefinition* item = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ImGui::SameLine();
        ToolWindow::RenderSelectedItem(itemType);
        if (item != nullptr)
        {
            ImGui::PushID(itemType);
            RenderRequirements(*item);
            RenderClasses(*item);
            Fields::Check(m_draft, I18N::Editor::ExcellentOptionsRaiseTheLevelRequirement, item->excellentRaisesLevel);
            ImGui::PopID();
        }
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemRequirementsUI::RenderRequirements(ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Requirements, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    for (const RequirementField& field : RequirementFields())
    {
        ImGui::PushID(static_cast<int>(field.requirement));
        const bool byLevel = item.requirementsByLevel.Find(field.requirement) != nullptr;
        // A requirement given by level does not use its base value.
        ImGui::BeginDisabled(byLevel);
        Fields::Word(m_draft, field.label, item.requirements.*field.value);
        ImGui::EndDisabled();
        ImGui::SameLine();
        RenderByLevel(item, field.requirement, field.label, item.requirements.*field.value);
        ImGui::PopID();
    }
}

void CMuItemRequirementsUI::RenderByLevel(ItemDefinition& item, ItemRequirement requirement, const char* label,
                                          WORD& baseValue)
{
    bool byLevel = item.requirementsByLevel.Find(requirement) != nullptr;
    if (ImGui::Checkbox(I18N::Editor::ByItemLevel, &byLevel))
    {
        if (byLevel)
        {
            Data::Items::RequirementByLevel values{};
            // Every level starts at the base value it replaces, which is
            // then not used (the checks warn about one that is kept).
            values.fill(baseValue);
            item.requirementsByLevel.Set(requirement, values);
            baseValue = 0;
        }
        else
        {
            // The base value comes back from the first level.
            baseValue = item.requirementsByLevel.Find(requirement)->front();
            item.requirementsByLevel.Remove(requirement);
        }
        m_draft.AfterChange(true, label);
    }
    const Data::Items::RequirementByLevel* found = item.requirementsByLevel.Find(requirement);
    if (found == nullptr)
    {
        return;
    }
    Data::Items::RequirementByLevel values = *found;
    for (size_t level = 0; level < values.size(); ++level)
    {
        if (level % LevelsPerRow != 0)
        {
            ImGui::SameLine();
        }
        char levelLabel[16];
        std::snprintf(levelLabel, sizeof(levelLabel), "+%zu", level);
        ImGui::SetNextItemWidth(Fields::ScaledWidth(LevelValueWidth));
        if (ImGui::InputScalar(levelLabel, ImGuiDataType_U16, &values[level]))
        {
            item.requirementsByLevel.Set(requirement, values);
        }
        m_draft.AfterField(label);
    }
}

void CMuItemRequirementsUI::RenderClasses(ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Classes, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::TextDisabled("%s", I18N::Editor::AClassWith0CannotUseTheItemOtherwiseItNeedsThatClassStep);
    const auto labels = ClassLabels();
    for (size_t characterClass = 0; characterClass < item.classRequirements.size(); ++characterClass)
    {
        Fields::Byte(m_draft, labels[characterClass], item.classRequirements[characterClass]);
        if (characterClass % 2 == 0 && characterClass + 1 < item.classRequirements.size())
        {
            ImGui::SameLine();
        }
    }
}

#endif // _EDITOR
