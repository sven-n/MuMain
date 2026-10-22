#include "stdafx.h"

#ifdef _EDITOR

#include "ItemStatProfileWidgets.h"

#include "ItemFieldWidgets.h"
#include "ItemRequirementTable.h"
#include "MuItemFormulasUI.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string>
#include <vector>

namespace MuEditor::Items
{
namespace
{
// Before the editor UI scale.
constexpr float ProfileListWidth = 260.0f;

// The profile with how many items use it: "weapon (186)"; "" is what the
// rules give.
std::string ProfileLabel(const std::string& name, const StatProfileChoice& choice, ItemStatProfileUseCounts& counts)
{
    if (name.empty())
    {
        return I18N::Format(I18N::Editor::ByTheRules0, {choice.given});
    }
    return name + " (" + std::to_string(counts.Items(name)) + ")";
}

std::string DescribeChoice(const StatProfileChoice& choice)
{
    if (choice.reason == StatProfileChoice::Reason::Picked)
    {
        return I18N::Format(I18N::Editor::PickedForTheItemTheRulesGive0, {choice.given});
    }
    if (choice.rule == Data::Items::NoStatProfileRule)
    {
        return I18N::Editor::NoRuleMatchesTheDefault;
    }
    return I18N::Format(I18N::Editor::GivenByRule0, {std::to_string(choice.rule + 1)});
}
} // namespace

void RenderStatProfileCombo(ItemEditDraft<Data::Items::ItemDefinition>& draft, Data::Items::ItemDefinition& item,
                            ItemStatProfileUseCounts& counts)
{
    const StatProfileChoice choice = FindStatProfileChoice(item);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ProfileListWidth));
    const std::string shown = item.statProfile.empty() ? ProfileLabel("", choice, counts) : item.statProfile;
    if (ImGui::BeginCombo(I18N::Editor::StatProfile, shown.c_str()))
    {
        // The default by its two names: "" follows the rules, "default" keeps
        // the default also where the rules give a profile.
        std::vector<std::string> names = {"", Data::Items::StatProfileNames::Default};
        for (const std::string& name : g_ItemDatabase.GetStatProfiles().GetProfileNames())
        {
            names.push_back(name);
        }
        for (const std::string& name : names)
        {
            if (ImGui::Selectable(ProfileLabel(name, choice, counts).c_str(), name == item.statProfile) &&
                name != item.statProfile)
            {
                item.statProfile = name;
                draft.AfterChange(true, I18N::Editor::StatProfile);
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::TheProfilesAreInTheItemStatFileTheNumberIsHowManyItemsUseOne);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", DescribeChoice(choice).c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::ItemFormulas))
    {
        const bool isDefault = choice.name == Data::Items::StatProfileNames::Default;
        OpenItemFormulas({isDefault ? CMuItemFormulasUI::Entry::Kind::Default : CMuItemFormulasUI::Entry::Kind::Profile,
                          isDefault ? std::string() : choice.name});
    }
}

ExpandedStatProfile ExpandItemStatProfile(const Data::Items::ItemDefinition& item)
{
    return ExpandStatProfile(g_ItemDataHandler.GetStatFile(), FindStatProfileChoice(item).name);
}

void RenderFormulaLine(const char* label, const FormulaLine& line)
{
    ImGui::PushTextWrapPos(0.0f);
    if (label != nullptr)
    {
        ImGui::TextDisabled("%s: %s", label, line.text.c_str());
    }
    else
    {
        ImGui::TextDisabled("%s", line.text.c_str());
    }
    ImGui::PopTextWrapPos();
    if (!line.tooltip.empty() && ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", line.tooltip.c_str());
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
