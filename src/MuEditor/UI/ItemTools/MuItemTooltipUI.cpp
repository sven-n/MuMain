#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemTooltipUI.h"

#include "ItemEdits.h"
#include "ItemLayoutChangeNotes.h"
#include "ItemSelection.h"
#include "ItemToolWindow.h"
#include "TooltipLineLooks.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Colors/GroundLabelWidgets.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
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
constexpr float LayoutComboWidth = 240.0f;
constexpr int MaxLevel = Data::Items::ItemLevelCount - 1;
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

// The style a label took. With no rule holding the label keeps the game's own look: a style that looks like it, else
// the first style.
std::string StyleOfLabel(const GroundItemLabel& label)
{
    if (!label.style.empty())
    {
        return label.style;
    }
    const std::vector<Data::Items::GroundLabelStyle>& styles = g_ItemDatabase.GetGroundLabels().styles;
    const auto looksLike = [&label](const Data::Items::GroundLabelStyle& style)
    {
        return style.color == GroundLabelWidgets::FromGameColor(label.textColor) &&
               style.background == GroundLabelWidgets::FromGameColor(label.backgroundColor) &&
               style.backgroundAlpha == GroundLabelWidgets::AlphaOfGameColor(label.backgroundColor) &&
               style.bold == label.bold;
    };
    const auto same = std::ranges::find_if(styles, looksLike);
    if (same != styles.end())
    {
        return same->name;
    }
    return styles.empty() ? std::string() : styles.front().name;
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
            RenderLayout(*item);
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

    // The two places the item's name shows, each a row with its caption in front and where its look comes from after.
    const ITEM preview = MakePreviewItem(itemType);
    const float valueX =
        std::max(ImGui::CalcTextSize(I18N::Editor::InTheTooltip).x, ImGui::CalcTextSize(I18N::Editor::OnTheGround).x) +
        2.0f * ImGui::GetStyle().ItemSpacing.x;
    const auto caption = [valueX](const char* text)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(text);
        ImGui::SameLine(valueX);
    };

    // In the tooltip: the name row as the tooltip writes it, always bold, in the active theme's tooltip colors.
    const ItemTooltipNameRow name = DescribeItemTooltipName(preview);
    const MuEditor::Items::TooltipLooks& looks = MuEditor::Items::GetTooltipLooks();
    const MuEditor::Items::TooltipLineLook& look = MuEditor::Items::GetTooltipLineLook(looks, name.color);
    // Its bar, or the tooltip's box where it has none.
    const bool bar = look.backgroundAlpha != 0;
    caption(I18N::Editor::InTheTooltip);
    GroundLabelWidgets::RenderLabel(Core::Text::ToUtf8(name.text.c_str()).c_str(), look.text,
                                    bar ? look.background : looks.box, true,
                                    bar ? look.backgroundAlpha : looks.boxAlpha);
    const std::string nameSource = name.nameLine
                                       ? I18N::Format(I18N::Editor::NameLine0, {std::to_string(*name.nameLine + 1)})
                                       : std::string(I18N::Editor::StandardName);
    ImGui::SameLine();
    ImGui::TextDisabled("%s", nameSource.c_str());

    // On the ground: the label, and which rule gives it at the level: the item's own (the first whose levels hold) or
    // the standard rules.
    const GroundItemLabel label = DescribePreviewLabel(preview);
    caption(I18N::Editor::OnTheGround);
    GroundLabelWidgets::RenderLabel(Core::Text::ToUtf8(label.text.c_str()).c_str(),
                                    GroundLabelWidgets::FromGameColor(label.textColor),
                                    GroundLabelWidgets::FromGameColor(label.backgroundColor), label.bold,
                                    GroundLabelWidgets::AlphaOfGameColor(label.backgroundColor));
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
        // A stored change takes the item back as stored, into new storage of the rules: each widget takes its rule
        // from the list again.
        m_draft.AfterChange(GroundLabelWidgets::RenderStyleCombo("##Style", rules[index].style, m_counts), step);
        ImGui::SameLine();
        if (RenderRuleLevels(rules, index))
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

void CMuItemTooltipUI::RenderLayout(ItemDefinition& item)
{
    ImGui::SeparatorText(I18N::Editor::TooltipLayout);
    RenderNote(I18N::Editor::TooltipLayoutNote);
    // An item that names no layout uses the default one.
    const std::string current(Data::Items::GetItemTooltipLayoutName(item));
    bool changed = false;
    ImGui::SetNextItemWidth(LayoutComboWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo("##TooltipLayout", current.c_str()))
    {
        const Data::Items::ItemTooltipFile& file = g_ItemDatabase.GetTooltipFile();
        for (const Data::Items::TooltipLayout& layout : file.layouts)
        {
            // A layout the item's own changes don't fit (a line they hide or add after that it doesn't have) can't be
            // picked: the item could not be saved with it.
            const auto problems = Data::Items::CheckTooltipLayoutChanges(file, layout.name, item.tooltipLayoutChanges);
            ImGui::BeginDisabled(!problems.empty());
            if (ImGui::Selectable(layout.name.c_str(), layout.name == current) && layout.name != current)
            {
                item.tooltipLayout = layout.name == Data::Items::DefaultTooltipLayout ? std::string() : layout.name;
                changed = true;
            }
            ImGui::EndDisabled();
            if (!problems.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip(
                    "%s",
                    I18N::Format(I18N::Editor::ItsOwnChangesDoNotFitThisLayout0, {problems.front().second}).c_str());
            }
        }
        ImGui::EndCombo();
    }
    m_draft.AfterChange(changed, I18N::Editor::ChangeTheTooltipLayout);
    // Its own changes (tooltipLayoutChanges), on top of that layout: what it hides and each line it adds.
    for (const ItemLayoutChangeNote& note : DescribeItemLayoutChanges(
             item.tooltipLayoutChanges, g_ItemDatabase.GetTooltipFile(), I18N::GetCurrentLocale()))
    {
        if (note.indented)
        {
            ImGui::Indent();
        }
        RenderNote(note.text.c_str());
        if (note.indented)
        {
            ImGui::Unindent();
        }
    }
}

bool CMuItemTooltipUI::RenderRuleLevels(std::vector<ItemGroundLabel>& rules, size_t index)
{
    // Taken from the list at each use: the From field left after a change is stored, and the To field drawn after it
    // edits the rule as stored.
    const auto rule = [&rules, index]() -> ItemGroundLabel& { return rules[index]; };
    bool allLevels = !rule().hasLevels;
    if (ImGui::Checkbox(I18N::Editor::AllLevels, &allLevels))
    {
        rule().hasLevels = !allLevels;
        rule().minLevel = 0;
        rule().maxLevel = MaxLevel;
        return true;
    }
    if (!rule().hasLevels)
    {
        return false;
    }
    ImGui::SameLine();
    // A level typed past the other one moves it along, so the levels never run backwards.
    if (LevelField(I18N::Editor::From, rule().minLevel))
    {
        rule().maxLevel = std::max(rule().maxLevel, rule().minLevel);
    }
    m_draft.AfterField(I18N::Editor::ChangeTheGroundLabelRules);
    ImGui::SameLine();
    if (LevelField(I18N::Editor::To, rule().maxLevel))
    {
        rule().minLevel = std::min(rule().minLevel, rule().maxLevel);
    }
    m_draft.AfterField(I18N::Editor::ChangeTheGroundLabelRules);
    return false;
}

// A rule for the previewed level with the style the label has there: its own rule's, else the one the standard rules
// give (by its name: other styles may look the same). It goes before the item's rule that holds at the level (the
// first rule that holds gives the style), so it is the one used there.
void CMuItemTooltipUI::AddRule(ItemDefinition& item) const
{
    ItemGroundLabel rule;
    rule.hasLevels = true;
    rule.minLevel = m_level;
    rule.maxLevel = m_level;
    const auto own = std::ranges::find_if(item.groundLabels, [this](const ItemGroundLabel& existing)
                                          { return existing.HoldsAt(m_level); });
    rule.style = own != item.groundLabels.end()
                     ? own->style
                     : StyleOfLabel(DescribePreviewLabel(Data::Items::MakeItemType(item.group, item.number)));
    item.groundLabels.insert(own, rule);
}

ITEM CMuItemTooltipUI::MakePreviewItem(int itemType) const
{
    ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(
        Data::Items::GetItemGroup(itemType), Data::Items::GetItemNumber(itemType));
    params.Level = static_cast<BYTE>(m_level);
    params.WithSkill = m_skill;
    params.WithLuck = m_luck;
    params.WithOption = m_option;
    params.OptionLevel = m_option ? PreviewOptionLevel : 0;
    params.HasExcellentOption = m_excellent;
    params.ExcellentFlags = m_excellent ? Data::Items::AllExcellentOptionFlags : 0;
    params.IsAncient = m_set > 0;
    params.AncientDiscriminator = static_cast<BYTE>(m_set);
    ITEM item;
    FillItemFromParameters(item, params);
    return item;
}

GroundItemLabel CMuItemTooltipUI::DescribePreviewLabel(int itemType) const
{
    return DescribePreviewLabel(MakePreviewItem(itemType));
}

GroundItemLabel CMuItemTooltipUI::DescribePreviewLabel(const ITEM& item) const
{
    // Drawn with the model a drop of it has: a tag condition reads the drawn model's item (the Weapon of Archangel at
    // +0 to +2 is drawn as a divine archangel weapon).
    OBJECT object{};
    object.Type = Engine::Items::Drops::DrawnModel(item.Type, item.Level);
    ITEM drop = item;
    return DescribeGroundItemLabel(&object, &drop);
}

#endif // _EDITOR
