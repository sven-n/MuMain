#include "stdafx.h"

#ifdef _EDITOR

#include "RequirementWeightPopup.h"

#include "ItemRequirementTable.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>

namespace MuEditor::Items
{
namespace
{
constexpr const char* PopupId = "###RequirementWeight";
// The rows the list shows before it scrolls.
constexpr int VisibleRows = 10;

int WeightOf(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement)
{
    switch (requirement)
    {
    case Data::Items::ItemRequirement::Strength:
        return item.requirements.strength;
    case Data::Items::ItemRequirement::Dexterity:
        return item.requirements.dexterity;
    case Data::Items::ItemRequirement::Energy:
        return item.requirements.energy;
    case Data::Items::ItemRequirement::Vitality:
        return item.requirements.vitality;
    case Data::Items::ItemRequirement::Leadership:
        return item.requirements.leadership;
    default:
        return item.requirements.level;
    }
}

std::string DescribeReachable(const ReachableValue& reachable)
{
    const WeightRange& weights = reachable.weights;
    if (weights.GetCount() == 1)
    {
        return I18N::Format(I18N::Editor::_0Weight1, {std::to_string(reachable.value), std::to_string(weights.first)});
    }
    return I18N::Format(I18N::Editor::_0Weights1To2,
                        {std::to_string(reachable.value), std::to_string(weights.first), std::to_string(weights.last)});
}
} // namespace

void CRequirementWeightPopup::Open(int itemType, Data::Items::ItemRequirement requirement, std::string label, int value)
{
    m_itemType = itemType;
    m_requirement = requirement;
    m_label = std::move(label);
    m_value = value;
    m_selected = -1;
    m_opening = true;
}

void CRequirementWeightPopup::Select(int value, int weight)
{
    m_value = value;
    m_selected = weight;
    m_scrollToSelected = true;
}

std::optional<int> CRequirementWeightPopup::Render(const Data::Items::ItemDefinition& item)
{
    const int itemType = Data::Items::MakeItemType(item.group, item.number);
    if (m_opening && itemType == m_itemType)
    {
        ImGui::OpenPopup(PopupId);
        m_opening = false;
    }
    const std::string title = std::string(I18N::Editor::PickAWeight) + PopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return std::nullopt;
    }
    if (itemType != m_itemType)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return std::nullopt;
    }

    std::optional<int> taken;
    const WeightRange weights = FindWeightsGiving(item, m_requirement, m_value);
    if (!weights.IsEmpty())
    {
        const int currentWeight = WeightOf(item, m_requirement);
        if (!weights.Contains(m_selected))
        {
            // The item's weight when it gives the value, else the smallest.
            Select(m_value, weights.Contains(currentWeight) ? currentWeight : weights.first);
        }
        ImGui::TextUnformatted(m_label.c_str());
        ImGui::TextUnformatted(
            I18N::Format(I18N::Editor::TheWeights0To1AllGive2At0,
                         {std::to_string(weights.first), std::to_string(weights.last), std::to_string(m_value)})
                .c_str());
        ImGui::TextDisabled("%s", I18N::Editor::TheyDifferAtHigherItemLevelsAndForExcellentItems);

        const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
        const float height = rowHeight * static_cast<float>(std::min(weights.GetCount(), VisibleRows) + 1);
        if (ImGui::BeginTable("##Weights", 4,
                              ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit,
                              ImVec2(0.0f, height)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(I18N::Editor::Weight);
            ImGui::TableSetupColumn("+15");
            ImGui::TableSetupColumn(I18N::Editor::Excellent0);
            ImGui::TableSetupColumn(I18N::Editor::Excellent15);
            ImGui::TableHeadersRow();
            ImGuiListClipper clipper;
            clipper.Begin(weights.GetCount());
            if (m_scrollToSelected)
            {
                clipper.IncludeItemByIndex(m_selected - weights.first);
            }
            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
                {
                    const int weight = weights.first + row;
                    const WeightValues values = ComputeWeightValues(item, m_requirement, weight);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    const std::string name = weight == currentWeight
                                                 ? I18N::Format(I18N::Editor::_0Current, {std::to_string(weight)})
                                                 : std::to_string(weight);
                    ImGui::PushID(weight);
                    if (ImGui::Selectable(name.c_str(), weight == m_selected,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        m_selected = weight;
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            taken = weight;
                        }
                    }
                    ImGui::PopID();
                    if (m_scrollToSelected && weight == m_selected)
                    {
                        ImGui::SetScrollHereY();
                        m_scrollToSelected = false;
                    }
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", values.plus15);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", values.excellent);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", values.excellentPlus15);
                }
            }
            ImGui::EndTable();
        }
        if (ImGui::Button(I18N::Editor::TakeThisWeight))
        {
            taken = m_selected;
        }
        ImGui::SameLine();
    }
    else
    {
        ImGui::TextUnformatted(m_label.c_str());
        ImGui::TextUnformatted(
            I18N::Format(I18N::Editor::NoWeightGives0At0TheNearestValues, {std::to_string(m_value)}).c_str());
        const NearestZeroValues nearest = FindNearestZeroValues(item, m_requirement, m_value);
        for (const std::optional<ReachableValue>& reachable : {nearest.below, nearest.above})
        {
            if (!reachable)
            {
                continue;
            }
            ImGui::PushID(reachable->value);
            if (ImGui::Button(DescribeReachable(*reachable).c_str()))
            {
                if (reachable->weights.GetCount() == 1)
                {
                    taken = reachable->weights.first;
                }
                else
                {
                    Select(reachable->value, reachable->weights.first);
                }
            }
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }
    if (ImGui::Button(I18N::Editor::Cancel) || taken)
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
    return taken;
}
} // namespace MuEditor::Items

#endif // _EDITOR
