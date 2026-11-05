#include "stdafx.h"

#ifdef _EDITOR

#include "ItemAddDialog.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemGroupNames.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"

#include "../MuEditor/UI/ItemEditor/ItemListModel.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>

namespace MuEditor::Items
{
namespace
{
constexpr char PopupId[] = "###ItemAddDialog";
constexpr float FieldWidth = 220.0f;
constexpr ImVec4 NotFreeColor{1.0f, 0.4f, 0.4f, 1.0f};
// A new item's size within the inventory's 8 columns.
constexpr int MaxItemSize = 8;

using Data::Items::ItemDefinition;
using Data::Items::ItemModelDefinition;
bool IsAsciiDigit(char character)
{
    return character >= '0' && character <= '9';
}

bool IsAsciiLetter(char character)
{
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

// Only A to Z: the client's C locale follows the system's code page.
char ToAsciiCase(char character, bool upper)
{
    if (upper && character >= 'a' && character <= 'z')
    {
        return static_cast<char>(character - 'a' + 'A');
    }
    if (!upper && character >= 'A' && character <= 'Z')
    {
        return static_cast<char>(character - 'A' + 'a');
    }
    return character;
}

// A shared model name of letters and digits from the item's English name
// ("Scroll of Wizard" -> "scrollOfWizard"), unused so far.
std::string MakeSharedModelName(const std::string& itemName)
{
    std::string name;
    bool newWord = false;
    for (const char character : itemName)
    {
        if (!IsAsciiLetter(character) && !IsAsciiDigit(character))
        {
            newWord = !name.empty();
            continue;
        }
        // The first letter small, the first of each further word capital.
        name += name.empty() ? ToAsciiCase(character, false) : newWord ? ToAsciiCase(character, true) : character;
        newWord = false;
    }
    if (name.empty() || IsAsciiDigit(name.front()))
    {
        name = "sharedModel" + name;
    }
    std::string unique = name;
    for (int suffix = 2; g_ItemModelDatabase.FindSharedModel(unique) != nullptr; ++suffix)
    {
        unique = name + std::to_string(suffix);
    }
    return unique;
}
} // namespace

void CItemAddDialog::OpenAdd(int templateItemType)
{
    m_mode = Mode::Add;
    m_sourceItemType = g_ItemDatabase.Find(templateItemType) != nullptr ? templateItemType : -1;
    m_copySource = m_sourceItemType >= 0;
    m_group = m_sourceItemType >= 0 ? Data::Items::GetItemGroup(m_sourceItemType) : 0;
    m_number = GetNextFreeItemNumber(m_group, 0);
    m_name.clear();
    m_width = 1;
    m_height = 1;
    m_slot = Data::Items::ItemSlot::None;
    m_numberHolder = {};
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void CItemAddDialog::OpenDuplicate(int sourceItemType)
{
    const ItemDefinition* source = g_ItemDatabase.Find(sourceItemType);
    if (source == nullptr)
    {
        return;
    }
    m_mode = Mode::Duplicate;
    m_sourceItemType = sourceItemType;
    m_copySource = true;
    m_group = Data::Items::GetItemGroup(sourceItemType);
    m_number = GetNextFreeItemNumber(m_group, Data::Items::GetItemNumber(sourceItemType));
    m_name = source->names.GetNeutral();
    m_shareModel = true;
    m_numberHolder = {};
    m_open = true;
    ImGui::OpenPopup(PopupId);
}

void CItemAddDialog::Render()
{
    const char* title = m_mode == Mode::Add ? I18N::Editor::AddItem : I18N::Editor::DuplicateItem;
    char fullTitle[128];
    std::snprintf(fullTitle, sizeof(fullTitle), "%s%s", title, PopupId);
    if (!ImGui::BeginPopupModal(fullTitle, &m_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    RenderNumber();
    ImGui::SetNextItemWidth(FieldWidth);
    InputString(I18N::Editor::EnglishName, m_name);
    RenderValues();
    RenderModelChoice();

    ImGui::Separator();
    ImGui::BeginDisabled(!CanConfirm());
    if (ImGui::Button(m_mode == Mode::Add ? I18N::Editor::Add : I18N::Editor::Duplicate))
    {
        Confirm();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CItemAddDialog::RenderNumber()
{
    ImGui::SetNextItemWidth(FieldWidth);
    if (ImGui::BeginCombo(I18N::Editor::Group, GetItemGroupLabel(m_group).c_str()))
    {
        for (int group = 0; group < MAX_ITEM_TYPE; ++group)
        {
            if (ImGui::Selectable(GetItemGroupLabel(group).c_str(), group == m_group))
            {
                m_group = group;
                m_number = GetNextFreeItemNumber(group, 0);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::InputInt(I18N::Editor::Number, &m_number);
    m_number = std::clamp(m_number, 0, MAX_ITEM_INDEX - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::NextFree))
    {
        m_number = GetNextFreeItemNumber(m_group, m_number + 1);
    }
    const int itemType = Data::Items::MakeItemType(m_group, m_number);
    if (!Data::Items::IsFreeItemType(itemType))
    {
        ImGui::TextColored(NotFreeColor, "%s", I18N::Editor::ThisNumberIsNotFree);
        m_numberHolder.Render(itemType);
    }
}

void CItemAddDialog::RenderValues()
{
    if (m_mode == Mode::Duplicate)
    {
        return;
    }
    if (m_sourceItemType >= 0)
    {
        ImGui::Checkbox(I18N::Editor::CopyTheSelectedItem, &m_copySource);
        ImGui::SameLine();
        ImGui::TextDisabled("%s", g_ItemDatabase.GetLogName(m_sourceItemType).c_str());
    }
    if (m_copySource)
    {
        return;
    }
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::SliderInt(I18N::Metadata::Width, &m_width, 1, MaxItemSize);
    ImGui::SetNextItemWidth(FieldWidth);
    ImGui::SliderInt(I18N::Metadata::Height, &m_height, 1, MaxItemSize);
    using Data::Items::ItemSlot;
    const auto slots = Fields::EnumEntries<ItemSlot>({{ItemSlot::None, I18N::Editor::None}});
    const auto current = std::ranges::find(slots, m_slot, &Data::Items::EnumName<ItemSlot>::value);
    ImGui::SetNextItemWidth(FieldWidth);
    if (ImGui::BeginCombo(I18N::Metadata::Slot, current != slots.end() ? current->name : "?"))
    {
        for (const auto& slot : slots)
        {
            if (ImGui::Selectable(slot.name, slot.value == m_slot))
            {
                m_slot = slot.value;
            }
        }
        ImGui::EndCombo();
    }
}

void CItemAddDialog::RenderModelChoice()
{
    if (m_mode != Mode::Duplicate)
    {
        return;
    }
    const ItemModelDefinition* model = g_ItemModelDatabase.Find(m_sourceItemType);
    if (model == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::NoModel);
        return;
    }
    if (!model->model.empty())
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::UsesTheSharedModel0Too, {model->model}).c_str());
        return;
    }
    ImGui::Checkbox(I18N::Editor::ShareTheModel, &m_shareModel);
    ImGui::TextDisabled("%s", m_shareModel ? I18N::Editor::ItsModelFileBecomesASharedModelThatBothItemsUse
                                           : I18N::Editor::TheNewItemHasNoModelYet);
}

bool CItemAddDialog::CanConfirm() const
{
    return !m_name.empty() && m_name.find(Data::LocalizedString::Separator) == std::string::npos &&
           Data::Items::IsFreeItemType(Data::Items::MakeItemType(m_group, m_number));
}

void CItemAddDialog::Confirm()
{
    if (m_mode == Mode::Add)
    {
        AddNew();
    }
    else
    {
        Duplicate();
    }
    g_ItemSelection.Select(Data::Items::MakeItemType(m_group, m_number));
}

void CItemAddDialog::AddNew()
{
    ItemDefinition item;
    const ItemDefinition* source = m_copySource ? g_ItemDatabase.Find(m_sourceItemType) : nullptr;
    if (source != nullptr)
    {
        item = *source;
    }
    else
    {
        item.width = static_cast<BYTE>(m_width);
        item.height = static_cast<BYTE>(m_height);
        item.slot = m_slot;
        // An item that can be worn starts usable by every class; one without
        // a slot by none.
        if (item.slot != Data::Items::ItemSlot::None)
        {
            item.classRequirements.fill(1);
        }
    }
    item.group = m_group;
    item.number = m_number;
    // Only the English name: the translations of a copied item name that item.
    item.names = Data::LocalizedString::Parse(m_name);
    g_ItemEdits.AddItem(I18N::Editor::AddItem, item, nullptr, source);
}

void CItemAddDialog::Duplicate()
{
    const ItemDefinition* source = g_ItemDatabase.Find(m_sourceItemType);
    if (source == nullptr)
    {
        return;
    }
    // The source as it is now: making its model shared below changes the
    // model database only.
    const ItemDefinition copiedFrom = *source;
    ItemDefinition item = copiedFrom;
    item.group = m_group;
    item.number = m_number;
    item.names = Data::LocalizedString::Parse(m_name);

    const ItemModelDefinition* sourceModel = g_ItemModelDatabase.Find(m_sourceItemType);
    if (sourceModel == nullptr || (sourceModel->model.empty() && !m_shareModel))
    {
        g_ItemEdits.AddItem(I18N::Editor::DuplicateItem, item, nullptr, &copiedFrom);
        return;
    }
    ItemModelDefinition model = *sourceModel;
    g_ItemEdits.BeginGroup(I18N::Editor::DuplicateItem);
    if (model.model.empty())
    {
        // The file goes into a shared model that both items name.
        const Data::Items::SharedItemModel shared{MakeSharedModelName(source->names.GetNeutral()), model.file,
                                                  model.textureFolders, model.noneBlendMeshes};
        g_ItemEdits.SetSharedModel(I18N::Editor::DuplicateItem, shared);
        ItemModelDefinition sourceShared = *sourceModel;
        sourceShared.model = shared.name;
        g_ItemEdits.SetItemModel(I18N::Editor::DuplicateItem, sourceShared);
        model.model = shared.name;
    }
    g_ItemEdits.AddItem(I18N::Editor::DuplicateItem, item, &model, &copiedFrom);
    g_ItemEdits.EndGroup();
}
} // namespace MuEditor::Items

#endif // _EDITOR
