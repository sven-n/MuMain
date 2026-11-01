#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemTooltipUI.h"

#include "ItemEdits.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Colors/GroundLabelWidgets.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ItemDropModel.h"
#include "Engine/Object/ItemTooltipSnapshot.h"
#include "GameLogic/Items/CSItemOption.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <optional>
#include <string>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;
using Data::Items::ItemGroundLabel;
namespace GroundLabelWidgets = MuEditor::Colors::GroundLabelWidgets;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float LevelSliderWidth = 160.0f;
constexpr float LevelFieldWidth = 40.0f;
constexpr float SetComboWidth = 200.0f;
constexpr int MaxLevel = Data::Items::ItemLevelCount - 1;
// Every excellent option: the label only asks whether the item has one.
constexpr BYTE AllExcellentOptions = 0x3F;
// The option level the preview gives: the label only asks whether the item has one.
constexpr BYTE PreviewOptionLevel = 1;

std::optional<ItemDefinition> LoadItem(int itemType)
{
    const ItemDefinition* item = g_ItemDatabase.Find(itemType);
    return item != nullptr ? std::optional<ItemDefinition>(*item) : std::nullopt;
}

void StoreItem(const std::string& label, const ItemDefinition& item)
{
    g_ItemEdits.SetItem(label, item);
}

// A grey line that wraps at the window's edge.
void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

// The set's name as the label shows it in front of the item's name, without the space after it; "" for none.
std::string SetName(int itemType, int set)
{
    wchar_t name[64]{};
    if (!g_csItemOption.GetSetItemName(name, itemType, set))
    {
        return {};
    }
    std::string text = Core::Text::ToUtf8(name);
    while (!text.empty() && text.back() == ' ')
    {
        text.pop_back();
    }
    return text;
}

bool LevelField(const char* label, int& level)
{
    ImGui::SetNextItemWidth(LevelFieldWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::InputInt(label, &level, 0))
    {
        return false;
    }
    level = std::clamp(level, 0, MaxLevel);
    return true;
}
} // namespace

CMuItemTooltipUI& CMuItemTooltipUI::GetInstance()
{
    static CMuItemTooltipUI instance;
    return instance;
}

CMuItemTooltipUI::CMuItemTooltipUI() : m_draft(&LoadItem, &StoreItem) {}

void CMuItemTooltipUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemTooltip, MuEditor::WindowIds::ItemTooltip, open))
    {
        const int itemType = g_ItemSelection.GetItemType();
        ItemDefinition* item = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ToolWindow::RenderSelectedItem(itemType);
        if (item != nullptr)
        {
            // Every field belongs to this item: an edit left open when the selection changes is not stored to the
            // next item.
            ImGui::PushID(itemType);
            RenderPreview(*item, itemType);
            RenderRules(*item);
            ImGui::PopID();
        }
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemTooltipUI::RenderPreview(const ItemDefinition& item, int itemType)
{
    ImGui::SeparatorText(I18N::Editor::Preview);
    ImGui::SetNextItemWidth(LevelSliderWidth * g_MuEditorCore.GetUIScale());
    ImGui::SliderInt(I18N::Editor::Level, &m_level, 0, MaxLevel);
    ImGui::Checkbox(I18N::Editor::Skill, &m_skill);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Luck, &m_luck);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Option, &m_option);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Excellent, &m_excellent);
    RenderSetCombo(itemType);

    const GroundItemLabel label = DescribePreviewLabel(itemType);
    GroundLabelWidgets::RenderLabel(Core::Text::ToUtf8(label.text.c_str()).c_str(),
                                    GroundLabelWidgets::FromGameColor(label.textColor),
                                    GroundLabelWidgets::FromGameColor(label.backgroundColor), label.bold);
    // Which rule gives the label at the level: the item's own (the first whose levels hold) or the standard rules.
    const auto own =
        std::ranges::find_if(item.groundLabels, [this](const ItemGroundLabel& rule) { return rule.HoldsAt(m_level); });
    const std::string source =
        own != item.groundLabels.end()
            ? I18N::Format(I18N::Editor::ItsOwnRule0, {std::to_string(own - item.groundLabels.begin() + 1)})
            : std::string(I18N::Editor::TheStandardRules);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", source.c_str());
}

void CMuItemTooltipUI::RenderSetCombo(int itemType)
{
    std::string names[MAX_ITEM_SETS_PER_ITEM + 1];
    names[0] = I18N::Editor::NoSet;
    bool hasSet = false;
    for (int set = 1; set <= MAX_ITEM_SETS_PER_ITEM; ++set)
    {
        const std::string name = SetName(itemType, set);
        hasSet |= !name.empty();
        names[set] = name.empty() ? std::string() : I18N::Format(I18N::Editor::Set01, {std::to_string(set), name});
    }
    if (m_set > 0 && names[m_set].empty())
    {
        m_set = 0;
    }
    ImGui::BeginDisabled(!hasSet);
    ImGui::SetNextItemWidth(SetComboWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo("##Set", names[m_set].c_str()))
    {
        for (int set = 0; set <= MAX_ITEM_SETS_PER_ITEM; ++set)
        {
            if (!names[set].empty() && ImGui::Selectable(names[set].c_str(), set == m_set))
            {
                m_set = set;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}

void CMuItemTooltipUI::RenderRules(ItemDefinition& item)
{
    ImGui::SeparatorText(I18N::Editor::GroundLabel);
    RenderNote(item.groundLabels.empty() ? I18N::Editor::NoRulesOfItsOwn
                                         : I18N::Editor::TheFirstRuleWhoseLevelsHoldGivesTheStyle);
    const std::string step = I18N::Editor::ChangeTheGroundLabelRules;
    std::vector<ItemGroundLabel>& rules = item.groundLabels;
    std::optional<size_t> removed;
    std::optional<std::pair<size_t, size_t>> swapped;
    for (size_t index = 0; index < rules.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        ItemGroundLabel& rule = rules[index];
        m_draft.AfterChange(GroundLabelWidgets::RenderStyleCombo("##Style", rule.style, m_counts), step);
        ImGui::SameLine();
        if (RenderRuleLevels(rule))
        {
            m_draft.AfterChange(true, step);
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(index == 0);
        if (ImGui::ArrowButton("##Up", ImGuiDir_Up))
        {
            swapped = {index - 1, index};
        }
        ImGui::SetItemTooltip("%s", I18N::Editor::MoveUp);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(index + 1 == rules.size());
        if (ImGui::ArrowButton("##Down", ImGuiDir_Down))
        {
            swapped = {index, index + 1};
        }
        ImGui::SetItemTooltip("%s", I18N::Editor::MoveDown);
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = index;
        }
        ImGui::PopID();
    }
    if (swapped)
    {
        std::swap(rules[swapped->first], rules[swapped->second]);
        m_draft.AfterChange(true, step);
    }
    else if (removed)
    {
        rules.erase(rules.begin() + static_cast<std::ptrdiff_t>(*removed));
        m_draft.AfterChange(true, step);
    }
    if (ImGui::Button(I18N::Editor::AddRule))
    {
        AddRule(item);
        m_draft.AfterChange(true, step);
    }
}

bool CMuItemTooltipUI::RenderRuleLevels(ItemGroundLabel& rule)
{
    bool allLevels = !rule.hasLevels;
    if (ImGui::Checkbox(I18N::Editor::AllLevels, &allLevels))
    {
        rule.hasLevels = !allLevels;
        rule.minLevel = 0;
        rule.maxLevel = MaxLevel;
        return true;
    }
    if (!rule.hasLevels)
    {
        return false;
    }
    ImGui::SameLine();
    // A level typed past the other one moves it along, so the levels never run backwards.
    if (LevelField(I18N::Editor::From, rule.minLevel))
    {
        rule.maxLevel = std::max(rule.maxLevel, rule.minLevel);
    }
    m_draft.AfterField(I18N::Editor::ChangeTheGroundLabelRules);
    ImGui::SameLine();
    if (LevelField(I18N::Editor::To, rule.maxLevel))
    {
        rule.minLevel = std::min(rule.minLevel, rule.maxLevel);
    }
    m_draft.AfterField(I18N::Editor::ChangeTheGroundLabelRules);
    return false;
}

// A rule for the previewed level with the look the label has there: its own rule's style, else a style that looks like
// the label the standard rules give. It goes before the first rule for every level: a rule after that one would never
// be used.
void CMuItemTooltipUI::AddRule(ItemDefinition& item) const
{
    ItemGroundLabel rule;
    rule.hasLevels = true;
    rule.minLevel = m_level;
    rule.maxLevel = m_level;
    const auto own = std::ranges::find_if(item.groundLabels, [this](const ItemGroundLabel& existing)
                                          { return existing.HoldsAt(m_level); });
    const Data::Items::GroundLabelFile& labels = g_ItemDatabase.GetGroundLabels();
    if (own != item.groundLabels.end())
    {
        rule.style = own->style;
    }
    else
    {
        const GroundItemLabel label = DescribePreviewLabel(Data::Items::MakeItemType(item.group, item.number));
        const auto looksLike = [&label](const Data::Items::GroundLabelStyle& style)
        {
            return style.color == GroundLabelWidgets::FromGameColor(label.textColor) &&
                   style.background == GroundLabelWidgets::FromGameColor(label.backgroundColor) &&
                   style.bold == label.bold;
        };
        const auto same = std::ranges::find_if(labels.styles, looksLike);
        if (same != labels.styles.end())
        {
            rule.style = same->name;
        }
        else if (!labels.styles.empty())
        {
            rule.style = labels.styles.front().name;
        }
    }
    const auto everyLevel =
        std::ranges::find_if(item.groundLabels, [](const ItemGroundLabel& existing) { return !existing.hasLevels; });
    item.groundLabels.insert(everyLevel, rule);
}

GroundItemLabel CMuItemTooltipUI::DescribePreviewLabel(int itemType) const
{
    ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(
        Data::Items::GetItemGroup(itemType), Data::Items::GetItemNumber(itemType));
    params.Level = static_cast<BYTE>(m_level);
    params.WithSkill = m_skill;
    params.WithLuck = m_luck;
    params.WithOption = m_option;
    params.OptionLevel = m_option ? PreviewOptionLevel : 0;
    params.HasExcellentOption = m_excellent;
    params.ExcellentFlags = m_excellent ? AllExcellentOptions : 0;
    params.IsAncient = m_set > 0;
    params.AncientDiscriminator = static_cast<BYTE>(m_set);
    ITEM item;
    FillItemFromParameters(item, params);
    // Drawn with the model a drop of it has: a tag condition reads the drawn model's item (the Weapon of Archangel at
    // +0 to +2 is drawn as a divine archangel weapon).
    OBJECT object{};
    object.Type = Engine::Items::Drops::DrawnModel(item.Type, item.Level);
    return DescribeGroundItemLabel(&object, &item);
}

#endif // _EDITOR
