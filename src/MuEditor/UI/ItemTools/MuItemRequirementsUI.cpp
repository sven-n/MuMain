#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemRequirementsUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemRequirementTable.h"
#include "ItemSelection.h"
#include "ItemStatProfileWidgets.h"
#include "ItemToolWindow.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <array>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;

namespace
{
constexpr float LevelValueWidth = 46.0f;
// The item levels shown in one row of a requirement given by level.
constexpr size_t LevelsPerRow = 8;
// The option levels an item can have (+4 to +28 damage or defense).
constexpr int MaxOptionLevel = 7;
constexpr float OptionLevelWidth = 120.0f;
constexpr float TableColumnWidth = 70.0f;
constexpr int TableRows = 16;

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
        const int itemType = g_ItemSelection.GetItemType();
        ItemDefinition* item = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ToolWindow::RenderSelectedItem(itemType);
        if (item != nullptr)
        {
            ImGui::PushID(itemType);
            RenderRequirements(*item);
            RenderLevelTable(*item);
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
    // The profile gives the formulas the requirements grow by.
    RenderStatProfileCombo(m_draft, item, m_profileCounts);
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
            // Every level starts at the requirement the base value gives a
            // normal item there, so nothing changes for it, except that a stat
            // whose formula takes the level requirement (a scroll's energy)
            // then follows the level of each + level. The base value is not
            // used then (the checks warn about one that is kept).
            const RequirementTable table = ComputeRequirementTable(item, {});
            Data::Items::RequirementByLevel values{};
            for (size_t level = 0; level < values.size(); ++level)
            {
                values[level] = static_cast<WORD>(table[level][static_cast<size_t>(requirement)]);
            }
            item.requirementsByLevel.Set(requirement, values);
            baseValue = 0;
        }
        else
        {
            // The base value that gives these values back.
            baseValue = FindRequirementBase(item, requirement, *item.requirementsByLevel.Find(requirement));
            item.requirementsByLevel.Remove(requirement);
        }
        m_draft.AfterChange(true, label);
    }
    if (ImGui::IsItemHovered())
    {
        // What the game still adds to the values by level (ItemStats).
        const char* stillAdded = requirement == ItemRequirement::Level
                                     ? I18N::Editor::ExcellentOptionsStillRaiseItIfTheyRaiseTheLevelRequirement
                                     : I18N::Editor::ExcellentAndAncientItemsGetNoHigherRequirementFromIt;
        if (requirement == ItemRequirement::Strength && OptionLevelAddsStrength(item))
        {
            ImGui::SetTooltip("%s\n%s\n%s", I18N::Editor::StartsAtWhatANormalItemNeedsAtEachLevelAndReplacesTheFormula,
                              stillAdded, I18N::Editor::TheOptionLevelStillAddsToIt);
        }
        else
        {
            ImGui::SetTooltip("%s\n%s", I18N::Editor::StartsAtWhatANormalItemNeedsAtEachLevelAndReplacesTheFormula,
                              stillAdded);
        }
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

// What the game asks of the item at each + level, from the values above as
// they are typed (the /<item name> window shows the normal variant).
void CMuItemRequirementsUI::RenderLevelTable(const ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::RequirementsByItemLevel, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::Checkbox(I18N::Editor::Excellent, &m_variant.excellent);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Ancient, &m_variant.ancient);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Fields::ScaledWidth(OptionLevelWidth));
    ImGui::SliderInt(I18N::Editor::OptionLevel, &m_variant.optionLevel, 0, MaxOptionLevel, "%d",
                     ImGuiSliderFlags_AlwaysClamp);

    const RequirementTable table = ComputeRequirementTable(item, m_variant);
    // The requirements the item has at some level, as the game's window shows them.
    std::vector<RequirementField> shown;
    for (const RequirementField& field : RequirementFields())
    {
        const size_t column = static_cast<size_t>(field.requirement);
        for (const RequirementRow& row : table)
        {
            if (row[column] > 0)
            {
                shown.push_back(field);
                break;
            }
        }
    }
    if (shown.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheItemHasNoRequirements);
        return;
    }
    const float columnWidth = Fields::ScaledWidth(TableColumnWidth);
    if (!ImGui::BeginTable("##LevelTable", static_cast<int>(shown.size()) + 1,
                           ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
    {
        return;
    }
    ImGui::TableSetupColumn("+", ImGuiTableColumnFlags_None, columnWidth * 0.5f);
    for (const RequirementField& field : shown)
    {
        ImGui::TableSetupColumn(field.label, ImGuiTableColumnFlags_None, columnWidth);
    }
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableSetColumnIndex(0);
    ImGui::TableHeader("+");
    for (size_t index = 0; index < shown.size(); ++index)
    {
        ImGui::TableSetColumnIndex(static_cast<int>(index) + 1);
        const bool byLevel = item.requirementsByLevel.Find(shown[index].requirement) != nullptr;
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableHeader(byLevel ? (std::string(shown[index].label) + " *").c_str() : shown[index].label);
        ImGui::PopID();
        if (byLevel && ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::GivenByLevel);
        }
    }
    for (int level = 0; level < TableRows; ++level)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("+%d", level);
        for (size_t index = 0; index < shown.size(); ++index)
        {
            ImGui::TableSetColumnIndex(static_cast<int>(index) + 1);
            ImGui::Text("%d", table[static_cast<size_t>(level)][static_cast<size_t>(shown[index].requirement)]);
        }
    }
    ImGui::EndTable();
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
