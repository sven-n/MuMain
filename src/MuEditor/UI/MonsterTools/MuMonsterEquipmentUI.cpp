#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterEquipmentUI.h"

#include "MonsterSetupLists.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/UI/Common/MapPicker.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemFieldWidgets.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterEquipment;
using Data::Monsters::MonsterItem;
using Slot = MuMonsterEquipmentUI::Slot;

constexpr ImVec4 MissingColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr ImVec4 PickedColor{0.26f, 0.59f, 0.98f, 0.80f};
constexpr float SlotWidth = 150.0f;
constexpr float SlotHeight = 46.0f;

struct SlotInfo
{
    const char* const* name;
    // The item groups that fit the slot.
    std::span<const int> groups;
    // A body that is no item, as a part model ("MODEL_BODY_HELM"); empty for
    // a slot that holds items only.
    const char* body;
};

constexpr std::array<int, 7> WeaponGroups{0, 1, 2, 3, 4, 5, 6};
constexpr std::array<int, 1> HelmGroups{7};
constexpr std::array<int, 1> ArmorGroups{8};
constexpr std::array<int, 1> PantsGroups{9};
constexpr std::array<int, 1> GlovesGroups{10};
constexpr std::array<int, 1> BootsGroups{11};
constexpr std::array<int, 2> WingsGroups{12, 13};
constexpr std::array<int, 1> HelperGroups{13};

const std::array<SlotInfo, MuMonsterEquipmentUI::SlotCount> Slots{{
    {&I18N::Editor::Weapon1, WeaponGroups, ""},
    {&I18N::Editor::Weapon2, WeaponGroups, ""},
    {&I18N::Editor::Helm, HelmGroups, "MODEL_BODY_HELM"},
    {&I18N::Editor::Armor, ArmorGroups, "MODEL_BODY_ARMOR"},
    {&I18N::Editor::Pants, PantsGroups, "MODEL_BODY_PANTS"},
    {&I18N::Editor::Gloves, GlovesGroups, "MODEL_BODY_GLOVES"},
    {&I18N::Editor::Boots, BootsGroups, "MODEL_BODY_BOOTS"},
    {&I18N::Editor::Wings, WingsGroups, ""},
    {&I18N::Editor::Helper, HelperGroups, ""},
}};

// The character window's arrangement: three rows of four cells.
constexpr int NoSlot = -1;
constexpr std::array<std::array<int, 4>, 3> Doll{{
    {NoSlot, MuMonsterEquipmentUI::Helm, MuMonsterEquipmentUI::Wings, NoSlot},
    {MuMonsterEquipmentUI::Weapon1, MuMonsterEquipmentUI::Armor, MuMonsterEquipmentUI::Weapon2,
     MuMonsterEquipmentUI::Helper},
    {MuMonsterEquipmentUI::Gloves, MuMonsterEquipmentUI::Pants, MuMonsterEquipmentUI::Boots, NoSlot},
}};

template <typename TEquipment> auto& SlotOf(TEquipment& equipment, Slot slot)
{
    switch (slot)
    {
    case MuMonsterEquipmentUI::Weapon2:
        return equipment.weapons[1];
    case MuMonsterEquipmentUI::Helm:
        return equipment.helm;
    case MuMonsterEquipmentUI::Armor:
        return equipment.armor;
    case MuMonsterEquipmentUI::Pants:
        return equipment.pants;
    case MuMonsterEquipmentUI::Gloves:
        return equipment.gloves;
    case MuMonsterEquipmentUI::Boots:
        return equipment.boots;
    case MuMonsterEquipmentUI::Wings:
        return equipment.wings;
    case MuMonsterEquipmentUI::Helper:
        return equipment.helper;
    default:
        return equipment.weapons[0];
    }
}

// What a slot holds, as the doll shows it; `missing` for an item the item
// list does not have (D60).
std::string Describe(const std::optional<MonsterItem>& item, bool& missing)
{
    missing = false;
    if (!item)
        return I18N::Editor::Empty;
    if (!item->part.empty())
        return I18N::Format(I18N::Editor::Body0, {item->part});
    const int type = Data::Items::MakeItemType(item->group, item->number);
    if (!Data::Items::IsValidItemType(type) || g_ItemDatabase.FindEntry(type) == nullptr)
    {
        missing = true;
        return I18N::Format(I18N::Editor::NoItemAt01, {std::to_string(item->group), std::to_string(item->number)});
    }
    std::string name = g_ItemDatabase.GetLogName(type);
    if (item->level > 0)
        name += " +" + std::to_string(item->level);
    return name;
}

float Width(float numbers)
{
    return Items::Fields::ScaledWidth(Items::Fields::NumberWidth * numbers);
}
} // namespace

MuMonsterEquipmentUI& MuMonsterEquipmentUI::GetInstance()
{
    static MuMonsterEquipmentUI instance;
    return instance;
}

MuMonsterEquipmentUI::MuMonsterEquipmentUI()
    : m_draft([](int key) { return ShownValues::Load(key); },
              [](const std::string& label, const ShownValues::ShownMonster& shown)
              { ShownValues::Store(label, shown); })
{
}

void MuMonsterEquipmentUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterEquipment, WindowIds::MonsterEquipment, open))
    {
        const int number = g_MonsterSelection.GetNumber();
        const int key = ShownValues::GetSelectedKey();
        if (key != m_scopeKey)
        {
            m_scopeKey = key;
            m_mapVariant = NoVariant;
            m_stateVariant = NoVariant;
            m_look = NoVariant;
        }
        ShownValues::ShownMonster* shown = m_draft.Begin(key, ToolWindow::GetDataVersion());
        MonsterDefinition* monster = shown != nullptr ? &shown->monster : nullptr;
        ShownModel::RenderSelectedMonster(number, monster, shown != nullptr && shown->wholeMonster);
        if (monster != nullptr)
        {
            m_shown = shown;
            ImGui::PushID(key);
            RenderEquipment(*monster);
            ImGui::PopID();
            m_shown = nullptr;
        }
        m_draft.End();
    }
    ToolWindow::End();
}

std::string MuMonsterEquipmentUI::EquipmentLabel() const
{
    return m_shown != nullptr ? ShownValues::LogName(*m_shown) + ": " + I18N::Editor::Equipment : std::string();
}

const MonsterEquipment& MuMonsterEquipmentUI::ParentEquipment(const MonsterDefinition& monster) const
{
    // A map variant's state variant wears the map variant's equipment, when
    // it has one, else the monster's.
    if (m_stateVariant != NoVariant && m_mapVariant != NoVariant)
    {
        const MonsterEquipment& map = SetupLists::SetupOfMap(monster, m_mapVariant).equipment;
        if (!map.IsEmpty())
            return map;
    }
    return monster.setup.equipment;
}

OwnValues::State MuMonsterEquipmentUI::EditState() const
{
    using Data::Monsters::MonsterValueField;
    if (m_look != NoVariant)
        return OwnValues::GetState(*m_shown, MonsterValueField::RandomVariants);
    if (m_mapVariant != NoVariant)
        return OwnValues::GetState(*m_shown, MonsterValueField::MapVariants);
    const std::string_view key = m_stateVariant != NoVariant ? "stateVariants" : "equipment";
    for (size_t index = 0; index < Data::Monsters::GetMonsterSetupFields().size(); ++index)
    {
        if (Data::Monsters::GetMonsterSetupFields()[index].key == key)
            return OwnValues::GetState(*m_shown, Data::Monsters::GetSetupValueField(index));
    }
    return OwnValues::State::Plain;
}

void MuMonsterEquipmentUI::RenderRows(MonsterDefinition& monster)
{
    if (m_mapVariant >= static_cast<int>(monster.mapVariants.size()))
        m_mapVariant = NoVariant;
    if (m_stateVariant >= static_cast<int>(SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants.size()))
        m_stateVariant = NoVariant;
    if (m_look >= static_cast<int>(monster.randomVariants.size()))
        m_look = NoVariant;
    const auto none = [this]
    {
        m_mapVariant = NoVariant;
        m_stateVariant = NoVariant;
        m_look = NoVariant;
    };
    ImGui::PushID("Rows");
    if (!monster.mapVariants.empty())
    {
        ImGui::TextUnformatted(I18N::Editor::Map);
        ImGui::SameLine();
        if (ImGui::RadioButton(I18N::Editor::AllMaps, m_mapVariant == NoVariant))
            none();
        for (size_t index = 0; index < monster.mapVariants.size(); ++index)
        {
            ImGui::SameLine();
            const std::string name = MapPicker::Describe(monster.mapVariants[index].maps, I18N::Editor::NoMapYet) +
                                     "##Map" + std::to_string(index);
            if (ImGui::RadioButton(name.c_str(), m_mapVariant == static_cast<int>(index)))
            {
                none();
                m_mapVariant = static_cast<int>(index);
            }
        }
    }
    // The state variants of the picked map variant, or the monster's; a
    // state keeps the map picked.
    const std::vector<Data::Monsters::MonsterStateVariant>& states =
        SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants;
    if (!states.empty())
    {
        ImGui::TextUnformatted(I18N::Editor::State);
        ImGui::SameLine();
        if (ImGui::RadioButton(I18N::Editor::Always, m_stateVariant == NoVariant))
        {
            m_stateVariant = NoVariant;
            m_look = NoVariant;
        }
        for (size_t index = 0; index < states.size(); ++index)
        {
            ImGui::SameLine();
            const std::string name = states[index].state + "##State" + std::to_string(index);
            if (ImGui::RadioButton(name.c_str(), m_stateVariant == static_cast<int>(index)))
            {
                m_look = NoVariant;
                m_stateVariant = static_cast<int>(index);
            }
        }
    }
    if (const std::string inherited = SetupLists::MonsterStatesOnMap(monster, m_mapVariant); !inherited.empty())
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::MapVariantMonsterStates0, {inherited}).c_str());
    if (!monster.randomVariants.empty())
    {
        ImGui::TextUnformatted(I18N::Editor::Look);
        ImGui::SameLine();
        if (ImGui::RadioButton((monster.model + "##Own").c_str(), m_look == NoVariant))
            none();
        for (size_t index = 0; index < monster.randomVariants.size(); ++index)
        {
            ImGui::SameLine();
            const std::string name = monster.randomVariants[index].model + "##Look" + std::to_string(index);
            if (ImGui::RadioButton(name.c_str(), m_look == static_cast<int>(index)))
            {
                none();
                m_look = static_cast<int>(index);
            }
        }
    }
    ImGui::PopID();
}

MonsterEquipment* MuMonsterEquipmentUI::EditedEquipment(MonsterDefinition& monster) const
{
    if (m_look != NoVariant)
    {
        std::optional<MonsterEquipment>& equipment = monster.randomVariants[static_cast<size_t>(m_look)].equipment;
        return equipment ? &*equipment : nullptr;
    }
    MonsterEquipment* variant = nullptr;
    if (m_stateVariant != NoVariant)
        variant = &SetupLists::SetupOfMap(monster, m_mapVariant)
                       .stateVariants[static_cast<size_t>(m_stateVariant)]
                       .setup.equipment;
    else if (m_mapVariant != NoVariant)
        variant = &monster.mapVariants[static_cast<size_t>(m_mapVariant)].setup.equipment;
    else
        return &monster.setup.equipment;
    // A variant without equipment wears the one it falls back to; while that
    // has none either, the variant's empty one is edited.
    return variant->IsEmpty() && !ParentEquipment(monster).IsEmpty() ? nullptr : variant;
}

void MuMonsterEquipmentUI::RenderEquipment(MonsterDefinition& monster)
{
    RenderRows(monster);
    const bool variant = m_look != NoVariant || m_mapVariant != NoVariant || m_stateVariant != NoVariant;
    MonsterEquipment* edited = EditedEquipment(monster);
    const OwnValues::State state = EditState();
    bool changed = false;
    OwnValues::BeginField(state);
    if (edited == nullptr)
    {
        ImGui::TextDisabled("%s", I18N::Editor::EquipmentInheritedNote);
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Set))
        {
            if (m_look != NoVariant)
                monster.randomVariants[static_cast<size_t>(m_look)].equipment = monster.setup.equipment;
            else if (m_stateVariant != NoVariant)
                SetupLists::SetupOfMap(monster, m_mapVariant)
                    .stateVariants[static_cast<size_t>(m_stateVariant)]
                    .setup.equipment = ParentEquipment(monster);
            else
                monster.mapVariants[static_cast<size_t>(m_mapVariant)].setup.equipment = monster.setup.equipment;
            changed = true;
        }
    }
    else if (variant && (m_look != NoVariant || !edited->IsEmpty()))
    {
        if (ImGui::SmallButton(I18N::Editor::UseTheMonsterValue))
        {
            if (m_look != NoVariant)
                monster.randomVariants[static_cast<size_t>(m_look)].equipment.reset();
            else
                *edited = MonsterEquipment{};
            changed = true;
        }
    }
    OwnValues::EndField(state);
    if (changed)
    {
        m_draft.AfterChange(true, EquipmentLabel());
        return;
    }
    // A monster model hangs an item on its link bones in the weapon slots
    // only; its other slots take a body (the merchants' part models) or
    // nothing (the game's RenderCharacter draws the body parts of any model).
    m_monsterModel = !IsPlayerShaped(monster);
    if (m_monsterModel)
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", I18N::Editor::MonsterModelItemsNote);
        ImGui::PopTextWrapPos();
    }
    RenderDoll(edited != nullptr ? *edited : ParentEquipment(monster), edited == nullptr);
    if (edited == nullptr)
        return;
    ImGui::Separator();
    OwnValues::BeginField(state);
    RenderSlotEditor(SlotOf(*edited, m_slot), m_slot);
    OwnValues::EndField(state);
}

bool MuMonsterEquipmentUI::IsPlayerShaped(const MonsterDefinition& monster) const
{
    // The model of what is shown: a random look's, a map variant's, else the
    // monster's.
    const std::string* model = &monster.model;
    if (m_look != NoVariant)
        model = &monster.randomVariants[static_cast<size_t>(m_look)].model;
    else if (m_mapVariant != NoVariant && monster.mapVariants[static_cast<size_t>(m_mapVariant)].model)
        model = &*monster.mapVariants[static_cast<size_t>(m_mapVariant)].model;
    return Data::Monsters::FindMonsterModelReference(*model) == MODEL_PLAYER;
}

void MuMonsterEquipmentUI::RenderDoll(const MonsterEquipment& equipment, bool inherited)
{
    const ImVec2 size(Items::Fields::ScaledWidth(SlotWidth), Items::Fields::ScaledWidth(SlotHeight));
    ImGui::PushID("Doll");
    for (const std::array<int, 4>& row : Doll)
    {
        for (size_t column = 0; column < row.size(); ++column)
        {
            if (column > 0)
                ImGui::SameLine();
            if (row[column] == NoSlot)
            {
                ImGui::Dummy(size);
                continue;
            }
            const Slot slot = static_cast<Slot>(row[column]);
            bool missing = false;
            const std::string text = Describe(SlotOf(equipment, slot), missing);
            const std::string label = std::string(*Slots[slot].name) + "\n" + text + "###Slot" + std::to_string(slot);
            const bool picked = slot == m_slot;
            if (picked)
                ImGui::PushStyleColor(ImGuiCol_Button, PickedColor);
            if (missing)
                ImGui::PushStyleColor(ImGuiCol_Text, MissingColor);
            else if (inherited)
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            if (ImGui::Button(label.c_str(), size))
                m_slot = slot;
            ImGui::PopStyleColor((picked ? 1 : 0) + (missing || inherited ? 1 : 0));
        }
    }
    ImGui::PopID();
}

void MuMonsterEquipmentUI::RenderSlotEditor(std::optional<MonsterItem>& item, Slot slot)
{
    const SlotInfo& info = Slots[slot];
    const std::string label = EquipmentLabel();
    ImGui::PushID("Slot");
    ImGui::TextUnformatted(*info.name);
    bool changed = false;
    // Nothing, an item, or a body that is no item.
    const int mode = !item ? 0 : (item->part.empty() ? 1 : 2);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::Empty, mode == 0) && mode != 0)
    {
        item.reset();
        changed = true;
    }
    ImGui::SameLine();
    // A monster model takes items in the weapon slots only; one already there
    // stays shown, to be changed or removed.
    const bool itemFits = !m_monsterModel || slot == Weapon1 || slot == Weapon2;
    ImGui::BeginDisabled(!itemFits && mode != 1);
    if (ImGui::RadioButton(I18N::Editor::Item, mode == 1) && mode != 1)
    {
        item = MonsterItem{info.groups.front(), 0};
        changed = true;
    }
    ImGui::EndDisabled();
    if (info.body[0] != '\0')
    {
        ImGui::SameLine();
        if (ImGui::RadioButton(I18N::Editor::Body, mode == 2) && mode != 2)
        {
            item = MonsterItem{};
            item->part = info.body;
            changed = true;
        }
    }
    if (changed)
    {
        m_draft.AfterChange(true, label);
        ImGui::PopID();
        return;
    }
    if (mode == 1 && !itemFits)
        ImGui::TextColored(MissingColor, "%s", I18N::Editor::MonsterModelItemOffWeapons);
    if (mode == 1)
    {
        const int type = Data::Items::MakeItemType(item->group, item->number);
        bool missing = false;
        const std::string shown = Describe(item, missing);
        ImGui::SetNextItemWidth(Width(3.0f));
        if (ImGui::BeginCombo("##Item", shown.c_str(), ImGuiComboFlags_HeightLargest))
        {
            if (ImGui::IsWindowAppearing())
            {
                m_search[0] = '\0';
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##Search", I18N::Editor::FindAnItem, m_search, sizeof(m_search));
            const std::string needle = m_search;
            for (const int group : info.groups)
            {
                for (int number = 0; number < MAX_ITEM_INDEX; ++number)
                {
                    const int candidate = Data::Items::MakeItemType(group, number);
                    if (g_ItemDatabase.FindEntry(candidate) == nullptr)
                        continue;
                    const std::string name = g_ItemDatabase.GetLogName(candidate);
                    if (!needle.empty() && name.find(needle) == std::string::npos)
                        continue;
                    ImGui::PushID(candidate);
                    if (ImGui::Selectable(name.c_str(), candidate == type) && candidate != type)
                    {
                        item->group = group;
                        item->number = number;
                        changed = true;
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SetNextItemWidth(Width(1.5f));
        ImGui::SliderInt(I18N::Editor::Level, &item->level, 0, Data::Monsters::MaxItemLevel);
        m_draft.AfterField(label);
        ImGui::SetNextItemWidth(Width(0.8f));
        if (ImGui::InputInt(I18N::Editor::ExcellentFlags, &item->excellent, 0))
            item->excellent = std::clamp(item->excellent, 0, Data::Monsters::MaxExcellentFlags);
        m_draft.AfterField(label);
        if (slot == Helper)
        {
            int look = item->mountLook.empty() ? 0 : 1;
            ImGui::SetNextItemWidth(Width(1.5f));
            if (ImGui::Combo(I18N::Editor::HorseLook, &look, "normal\0doppelganger\0"))
            {
                item->mountLook = look == 0 ? std::string() : std::string("doppelganger");
                changed = true;
            }
        }
    }
    else if (mode == 2)
    {
        ImGui::SetNextItemWidth(Width(2.5f));
        Items::InputString("##Part", item->part);
        m_draft.AfterField(label);
        if (!Data::Monsters::FindMonsterBodyPartSlot(item->part))
        {
            ImGui::SameLine();
            ImGui::TextColored(MissingColor, "%s", I18N::Format(I18N::Editor::UnknownName, {item->part}).c_str());
        }
    }
    m_draft.AfterChange(changed, label);
    ImGui::PopID();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
