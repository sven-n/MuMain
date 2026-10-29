#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemBasicsUI.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"
#include "ItemStatProfileWidgets.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"
#include "ItemUsers.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

using namespace MuEditor::Items;
using Data::Items::ItemDefinition;

namespace
{
constexpr float NameWidth = 260.0f;

// An item, or an entry without a name: its English name makes it an item.
std::optional<ItemDefinition> LoadItem(int itemType)
{
    const ItemDefinition* entry = g_ItemDatabase.FindEntry(itemType);
    return entry != nullptr ? std::optional<ItemDefinition>(*entry) : std::nullopt;
}

// What refuses the English name the draft changed: empty, or with the
// separator the save refuses. An item without an English name (from a bmd
// import) and an entry without a name can still be edited.
const char* RefuseEnglishName(const ItemDefinition& item)
{
    const ItemDefinition* stored = g_ItemDatabase.FindEntry(item.group, item.number);
    const std::string& english = item.names.GetNeutral();
    if (stored != nullptr && stored->names.GetNeutral() == english)
    {
        return nullptr;
    }
    if (english.empty())
    {
        return I18N::Editor::TheEnglishNameCannotBeEmpty;
    }
    return english.find(Data::LocalizedString::Separator) != std::string::npos
               ? I18N::Editor::ItemNamesCannotContainTheSeparator
               : nullptr;
}

// Also for an edit stored without its field being left (ItemEditDraft): a
// refused one is logged, and the draft takes the stored item back.
void StoreItem(const std::string& label, const ItemDefinition& item)
{
    if (const char* refused = RefuseEnglishName(item))
    {
        g_MuEditorConsoleUI.LogEditor(refused);
        return;
    }
    // Refused for a size or slot typed in before the game took the item.
    if (!g_ItemEdits.SetItem(label, item))
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheItemIsWornOrInAnItemWindowItsSizeAndSlotStayAsTheyAre);
    }
}

// The resistance names in the order of ItemDefinition::resistances.
std::array<const char*, MAX_RESISTANCE + 1> ResistanceLabels()
{
    return {I18N::Metadata::IceResistance,   I18N::Metadata::PoisonResistance, I18N::Metadata::LightningResistance,
            I18N::Metadata::FireResistance,  I18N::Metadata::EarthResistance,  I18N::Metadata::WindResistance,
            I18N::Metadata::WaterResistance, I18N::Metadata::Resistance7};
}

// A column of the stat table: a value ComputeItemStats gives, and the stat
// whose formula gives it.
struct StatColumn
{
    int GameLogic::Items::ItemStatValues::* value;
    const char* label;
    Data::Items::ItemStat stat;
    bool maxDamage = false;
};

// Pixel width before the editor UI scale.
constexpr float StatColumnWidth = 90.0f;

std::array<StatColumn, 6> StatColumns()
{
    using Data::Items::ItemStat;
    using GameLogic::Items::ItemStatValues;
    return {{{&ItemStatValues::damageMin, I18N::Metadata::MinDamage, ItemStat::Damage},
             {&ItemStatValues::damageMax, I18N::Metadata::MaxDamage, ItemStat::Damage, true},
             {&ItemStatValues::magicPower, I18N::Metadata::MagicPower, ItemStat::MagicPower},
             {&ItemStatValues::defense, I18N::Metadata::Defense, ItemStat::Defense},
             {&ItemStatValues::magicDefense, I18N::Metadata::MagicDefense, ItemStat::MagicDefense},
             {&ItemStatValues::blockRate, I18N::Metadata::BlockRate, ItemStat::BlockRate}}};
}

} // namespace

CMuItemBasicsUI& CMuItemBasicsUI::GetInstance()
{
    static CMuItemBasicsUI instance;
    return instance;
}

CMuItemBasicsUI::CMuItemBasicsUI() : m_draft(&LoadItem, &StoreItem) {}

void CMuItemBasicsUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::ItemBasics, MuEditor::WindowIds::ItemBasics, open))
    {
        const int itemType = g_ItemSelection.GetItemType();
        ItemDefinition* item = m_draft.Begin(itemType, ToolWindow::GetDataVersion());
        ToolWindow::RenderSelectedItem(itemType);
        if (item != nullptr)
        {
            // Every field belongs to this item: an edit left open when the
            // selection changes is not stored to the next item.
            ImGui::PushID(itemType);
            RenderName(*item);
            RenderShape(*item, itemType);
            RenderStats(*item);
            RenderStatTable(*item);
            RenderPrices(*item);
            RenderResistances(*item);
            ImGui::PopID();
        }
        m_draft.End();
    }
    ToolWindow::End();
}

void CMuItemBasicsUI::RenderName(ItemDefinition& item)
{
    std::string english = item.names.GetNeutral();
    ImGui::SetNextItemWidth(Fields::ScaledWidth(NameWidth));
    if (InputString(I18N::Editor::EnglishName, english))
    {
        item.names.Set(Data::LocalizedString::NeutralLocale, english);
    }
    // StoreItem refuses an empty name or one with the separator.
    m_draft.AfterField(I18N::Editor::EnglishName);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::ItemTranslations))
    {
        g_MuEditorCore.GetToolWindows().itemTranslations = true;
        ImGui::SetWindowFocus(MuEditor::WindowIds::ItemTranslations);
    }
}

void CMuItemBasicsUI::RenderShape(ItemDefinition& item, int itemType)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::SizeAndKind, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    const bool held = IsHeldByCharacter(itemType);
    ImGui::BeginDisabled(held);
    Fields::Byte(m_draft, I18N::Metadata::Width, item.width);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Metadata::Height, item.height);
    Fields::Enum(m_draft, I18N::Metadata::Slot, item.slot, {{Data::Items::ItemSlot::None, I18N::Editor::None}});
    ImGui::EndDisabled();
    if (held)
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheItemIsWornOrInAnItemWindowItsSizeAndSlotStayAsTheyAre);
    }
    Fields::Check(m_draft, I18N::Metadata::TwoHand, item.twoHanded);
    Fields::Enum(m_draft, I18N::Editor::WingTier, item.wingTier, {{Data::Items::WingTier::None, I18N::Editor::None}});
    // "By the group" picks the kind the group gives, which then follows the
    // group, slot and classes again (CItemDataHandler::SetItem); none is a
    // pick of its own.
    const Data::Items::WeaponKind given = Data::Items::GetGroupWeaponKind(item);
    const char* givenName = Data::Items::FindEnumName(given);
    const std::string byGroup =
        I18N::Format(I18N::Editor::ByTheGroup0, {givenName != nullptr ? givenName : I18N::Editor::None});
    Fields::Enum(m_draft, I18N::Editor::WeaponKind, item.weaponKind,
                 {{given, byGroup.c_str()}, {Data::Items::WeaponKind::None, I18N::Editor::None}});
    Fields::Word(m_draft, I18N::Metadata::Skill, item.skill);
    ImGui::SameLine();
    Fields::Word(m_draft, I18N::Metadata::Level, item.level);
    Fields::Byte(m_draft, I18N::Editor::PotionPriority, item.potionPriority);
}

void CMuItemBasicsUI::RenderStats(ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Stats, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    Fields::Byte(m_draft, I18N::Metadata::Durability, item.durability);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Metadata::MagicDurability, item.magicDurability);
    Fields::Word(m_draft, I18N::Metadata::MinDamage, item.damageMin);
    ImGui::SameLine();
    Fields::Word(m_draft, I18N::Metadata::MaxDamage, item.damageMax);
    Fields::Byte(m_draft, I18N::Metadata::AttackSpeed, item.attackSpeed);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Metadata::MoveSpeed, item.walkSpeed);
    Fields::Word(m_draft, I18N::Metadata::MagicPower, item.magicPower);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Metadata::AttackType, item.attackType);
    Fields::Byte(m_draft, I18N::Metadata::Defense, item.defense);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Metadata::MagicDefense, item.magicDefense);
    Fields::Byte(m_draft, I18N::Metadata::BlockRate, item.blockRate);
    ImGui::SameLine();
    Fields::Byte(m_draft, I18N::Editor::ExcellentBonus, item.excellentBonus);
}

void CMuItemBasicsUI::RenderStatTable(const ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::StatsByItemLevel, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::Checkbox(I18N::Editor::Excellent, &m_statVariant.excellent);
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::Ancient, &m_statVariant.ancient);

    const MuEditor::Items::ItemLevelValues values = MuEditor::Items::ComputeItemLevelValues(item, m_statVariant);
    // The stats the item has at some level.
    std::vector<StatColumn> shown;
    for (const StatColumn& column : StatColumns())
    {
        for (const GameLogic::Items::ItemStatValues& level : values)
        {
            if (level.*column.value > 0)
            {
                shown.push_back(column);
                break;
            }
        }
    }
    if (shown.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheItemHasNoDamageMagicPowerDefenseOrBlockRate);
        return;
    }
    const float columnWidth = Fields::ScaledWidth(StatColumnWidth);
    if (!ImGui::BeginTable("##StatTable", static_cast<int>(shown.size()) + 1,
                           ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
    {
        return;
    }
    ImGui::TableSetupColumn("+", ImGuiTableColumnFlags_None, columnWidth * 0.5f);
    for (const StatColumn& column : shown)
    {
        ImGui::TableSetupColumn(column.label, ImGuiTableColumnFlags_None, columnWidth);
    }
    ImGui::TableHeadersRow();
    for (size_t level = 0; level < values.size(); ++level)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("+%d", static_cast<int>(level));
        for (size_t index = 0; index < shown.size(); ++index)
        {
            ImGui::TableSetColumnIndex(static_cast<int>(index) + 1);
            ImGui::Text("%d", values[level].*shown[index].value);
        }
    }
    ImGui::EndTable();

    // Where the values come from, whatever the checkboxes say.
    const MuEditor::Items::ExpandedStatProfile profile = MuEditor::Items::ExpandItemStatProfile(item);
    for (const StatColumn& column : shown)
    {
        MuEditor::Items::RenderFormulaLine(
            column.label, MuEditor::Items::DescribeStatFormula(item, column.stat, column.maxDamage, profile));
    }
}

void CMuItemBasicsUI::RenderPrices(ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::PricesAndProfile, ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    Fields::Byte(m_draft, I18N::Metadata::SellValue, item.sellValue);
    ImGui::SameLine();
    Fields::Int(m_draft, I18N::Metadata::BuyPrice, item.buyPrice);
    MuEditor::Items::RenderStatProfileCombo(m_draft, item, m_profileCounts);
}

void CMuItemBasicsUI::RenderResistances(ItemDefinition& item)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Resistances))
    {
        return;
    }
    const auto labels = ResistanceLabels();
    for (size_t index = 0; index < item.resistances.size(); ++index)
    {
        Fields::Byte(m_draft, labels[index], item.resistances[index]);
        if (index % 2 == 0 && index + 1 < item.resistances.size())
        {
            ImGui::SameLine();
        }
    }
}

#endif // _EDITOR
