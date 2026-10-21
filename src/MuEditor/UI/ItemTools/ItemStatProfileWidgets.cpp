#include "stdafx.h"

#ifdef _EDITOR

#include "ItemStatProfileWidgets.h"

#include "ItemFieldWidgets.h"

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

// The profile with how many items use it: "weapon (186)".
std::string ProfileLabel(const std::string& name, ItemStatProfileUseCounts& counts)
{
    const std::string shown = name.empty() ? I18N::Editor::None : name;
    return shown + " (" + std::to_string(counts.Items(name)) + ")";
}
} // namespace

void RenderStatProfileCombo(ItemEditDraft<Data::Items::ItemDefinition>& draft, Data::Items::ItemDefinition& item,
                            ItemStatProfileUseCounts& counts)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ProfileListWidth));
    const std::string shown = item.statProfile.empty() ? I18N::Editor::None : item.statProfile;
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
            if (ImGui::Selectable(ProfileLabel(name, counts).c_str(), name == item.statProfile) &&
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
}
} // namespace MuEditor::Items

#endif // _EDITOR
