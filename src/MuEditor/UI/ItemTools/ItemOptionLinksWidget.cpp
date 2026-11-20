#include "stdafx.h"

#ifdef _EDITOR

#include "ItemOptionLinksWidget.h"

#include "ItemFieldWidgets.h"
#include "ItemOptionFileChange.h"
#include "ItemToolWidgets.h"
#include "MuItemOptionsUI.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/OptionRule.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <string>
#include <vector>

using Data::Items::ItemDefinition;
using Data::Items::ItemOptionDatabase;
using Data::Items::ItemOptionDefinition;

namespace MuEditor::Items
{
namespace
{
using ToolWidgets::ErrorColor;
using ToolWidgets::WarningColor;
constexpr float PickerWidth = 260.0f;

std::string JoinNames(std::span<const std::string> names)
{
    std::string text;
    for (const std::string& name : names)
    {
        text += (text.empty() ? "" : ", ") + name;
    }
    return text.empty() ? std::string("-") : text;
}

void StoreList(ItemEditDraft<ItemDefinition>& draft, ItemDefinition& item, std::vector<std::string> names)
{
    g_ItemDatabase.GetOptions().OrderLinks(names);
    if (names == item.options)
    {
        return;
    }
    item.options = std::move(names);
    draft.AfterChange(true, I18N::Editor::OptionLinks);
}
} // namespace

void RenderOptionLinks(ItemEditDraft<ItemDefinition>& draft, ItemDefinition& item)
{
    const ItemOptionDatabase& options = g_ItemDatabase.GetOptions();
    std::vector<std::string> remove;
    for (const std::string& name : item.options)
    {
        ImGui::PushID(name.c_str());
        const int index = options.Find(name);
        if (ImGui::SmallButton("x"))
        {
            remove.push_back(name);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::RemoveTheLink);
        }
        ImGui::SameLine();
        if (index == ItemOptionDatabase::NoDefinition)
        {
            ImGui::TextColored(ErrorColor, "%s", I18N::Format(I18N::Editor::NoDefinition0, {name}).c_str());
        }
        else
        {
            const ItemOptionDefinition& definition = options.Get(static_cast<Data::Items::ItemOptionIndex>(index));
            if (ImGui::SmallButton(name.c_str()))
            {
                OpenItemOptions(name);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::OpenInItemOptions);
            }
            ImGui::SameLine();
            const char* type = Data::Items::FindEnumName(definition.type);
            ImGui::TextDisabled("%s, %s", type != nullptr ? type : "?", definition.displayName.Get("en").c_str());
        }
        ImGui::PopID();
    }
    if (!remove.empty())
    {
        std::vector<std::string> names = item.options;
        std::erase_if(names, [&](const std::string& name)
                      { return std::ranges::find(remove, name) != remove.end(); });
        StoreList(draft, item, std::move(names));
    }

    ImGui::SetNextItemWidth(Fields::ScaledWidth(PickerWidth));
    if (ImGui::BeginCombo(I18N::Editor::LinkAnOptionDefinition, I18N::Editor::Pick))
    {
        for (const ItemOptionDefinition& definition : options.GetDefinitions())
        {
            if (!IsUsefulOptionLink(item, definition, options))
            {
                continue;
            }
            const char* type = Data::Items::FindEnumName(definition.type);
            const std::string label = definition.name + " (" + (type != nullptr ? type : "?") + ")";
            if (ImGui::Selectable(label.c_str()))
            {
                std::vector<std::string> names = item.options;
                names.push_back(definition.name);
                StoreList(draft, item, std::move(names));
            }
        }
        // Wrapped at the picker's width, so the long note does not widen the list.
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + Fields::ScaledWidth(PickerWidth));
        ImGui::TextDisabled("%s", I18N::Editor::LinksThatWouldDoNothingAreLeftOut);
        ImGui::PopTextWrapPos();
        ImGui::EndCombo();
    }

    const std::vector<std::string> given = Data::Items::DeriveItemOptions(options.GetRules(), item);
    if (IsSameOptionList(item.options, given))
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheOptionRulesGiveThisList);
        return;
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(WarningColor, "%s", I18N::Format(I18N::Editor::TheOptionRulesGive0, {JoinNames(given)}).c_str());
    ImGui::PopTextWrapPos();
    if (ImGui::SmallButton(I18N::Editor::UseTheRules))
    {
        StoreList(draft, item, given);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::AListThatDiffersFromTheRulesStaysUntilChanged);
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
