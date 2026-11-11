#include "stdafx.h"

#ifdef _EDITOR

#include "ItemModelForms.h"

#include "ItemEdits.h"
#include "ItemFieldWidgets.h"
#include "ItemListFields.h"
#include "ItemMeshFields.h"
#include "ItemNumberList.h"
#include "ItemSelection.h"
#include "ItemTextInput.h"
#include "ItemUsers.h"

#include "../MuEditor/UI/StyleEditor/MuStyleEditorUI.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/EffectData/Shines.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "I18N/All.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#include "imgui.h"

#include <cstdio>
#include <iterator>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items::ModelForms
{
namespace
{
using Data::Items::ItemModelDefinition;

constexpr float ComboWidth = 220.0f;
constexpr float VectorWidth = 240.0f;
constexpr float UserListLines = 5.0f;
// The highest glow level the model file reader takes (ItemModelGlowJson).
constexpr int MaxGlowLevel = 31;
constexpr size_t GlowLevelsPerRow = 8;
constexpr float GlowLevelWidth = 40.0f;

// The spans of the inventory sliders; every shipped value is inside them.
// The anchor is a share of the slot (a little beyond it, as armor has), the
// offset is in the inventory camera's units, the rotation in degrees and the
// body height in model units (helms -170).
constexpr Fields::SliderRange AnchorRange{-0.25, 1.25, "%.2f"};
constexpr Fields::SliderRange OffsetRange{-0.25, 0.25, "%.3f"};
constexpr Fields::SliderRange RotationRange{-360.0, 360.0, "%.0f"};
// The shipped scales lie between 0.0005 and 0.007; the model file reader
// takes only scales above 0.
constexpr Fields::SliderRange ScaleRange{0.0001, 0.05, "%.5f",
                                         ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_ClampOnInput};
constexpr Fields::SliderRange BodyHeightRange{-300.0, 100.0, "%.0f"};
constexpr const char* AxisNames[] = {"x", "y", "z"};

template <size_t Count> void Doubles(ModelDraft& draft, const char* label, std::array<double, Count>& values)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(VectorWidth));
    ImGui::InputScalarN(label, ImGuiDataType_Double, values.data(), static_cast<int>(Count), nullptr, nullptr, "%.3f");
    draft.AfterField(label);
}

// A slider per value, each on its own line as "<label> x" and so on; a drag
// of any is one undo step under `label`.
template <size_t Count>
void Sliders(ModelDraft& draft, const char* label, std::array<double, Count>& values, const Fields::SliderRange& range)
{
    static_assert(Count <= std::size(AxisNames));
    ImGui::PushID(label);
    for (size_t index = 0; index < Count; ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        const std::string shown = std::string(label) + " " + AxisNames[index];
        Fields::Slider(draft, shown.c_str(), values[index], range, label);
        ImGui::PopID();
    }
    ImGui::PopID();
}

// A list of mesh numbers as text ("1, 3"), stored when it was left and reads as numbers; the number under the
// mouse shows in the preview.
void Numbers(ModelDraft& draft, const char* label, std::vector<int>& numbers)
{
    std::string text = NumberList::Format(numbers);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ComboWidth));
    if (InputString(label, text))
    {
        if (const std::optional<std::vector<int>> parsed = NumberList::Parse(text))
        {
            numbers = *parsed;
        }
    }
    MeshFields::PointAtNumber(text);
    draft.AfterField(label);
}

// A mesh number that may be left out: a checkbox, and the number when it is there, shown in the preview while
// the mouse is on it.
void OptionalInt(ModelDraft& draft, const char* label, std::optional<int>& value)
{
    ImGui::PushID(label);
    bool present = value.has_value();
    if (ImGui::Checkbox("##Present", &present))
    {
        value = present ? std::optional<int>(0) : std::nullopt;
        draft.AfterChange(true, label);
    }
    ImGui::SameLine();
    if (value)
    {
        Fields::Int(draft, label, *value);
        MeshFields::PointAtMesh(*value);
    }
    else
    {
        ImGui::TextDisabled("%s", label);
    }
    ImGui::PopID();
}

// The items that use a look, clickable; folded until opened, and only then
// looked for.
void RenderUsers(const char* label, int count, std::vector<int> (*findUsers)(std::string_view), const std::string& look)
{
    if (count == 0 || !ImGui::TreeNode(label, "%s (%d)", I18N::Editor::UsedBy, count))
    {
        return;
    }
    if (ImGui::BeginListBox("##Users", ImVec2(-FLT_MIN, UserListLines * ImGui::GetTextLineHeightWithSpacing())))
    {
        for (const int user : findUsers(look))
        {
            ImGui::PushID(user);
            if (ImGui::Selectable(g_ItemDatabase.GetLogName(user).c_str(), user == g_ItemSelection.GetItemType()))
            {
                g_ItemSelection.Select(user);
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::TreePop();
}

// A look picked from its names, each with how many models use it and "(code)" when `isCode` says it is still code;
// "" is none.
template <typename TCount, typename TIsCode>
void LookCombo(ModelDraft& draft, const char* label, std::string& value, std::span<const std::string_view> names,
               TCount&& count, TIsCode&& isCode)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ComboWidth));
    const std::string shown = value.empty() ? I18N::Editor::None : value;
    if (!ImGui::BeginCombo(label, shown.c_str()))
    {
        return;
    }
    if (ImGui::Selectable(I18N::Editor::None, value.empty()) && !value.empty())
    {
        value.clear();
        draft.AfterChange(true, label);
    }
    for (const std::string_view name : names)
    {
        std::string entry = std::string(name) + " (" + std::to_string(count(name)) + ")";
        if (isCode(name))
        {
            entry += std::string(" ") + I18N::Editor::CodeMark;
        }
        if (ImGui::Selectable(entry.c_str(), name == value) && name != value)
        {
            value = std::string(name);
            draft.AfterChange(true, label);
        }
    }
    ImGui::EndCombo();
}

// True when its "+" (a new color) is clicked.
bool ColorCombo(ModelDraft& draft, const char* label, std::string& value, ItemLookUseCounts& counts)
{
    const std::span<const Data::Effects::GlowColor> colors = g_GlowColors.GetColors();
    const auto swatch = [](const Data::Effects::GlowColorValue& color, const char* id)
    {
        ImGui::ColorButton(
            id, ImVec4(static_cast<float>(color[0]), static_cast<float>(color[1]), static_cast<float>(color[2]), 1.0f),
            ImGuiColorEditFlags_NoTooltip);
    };
    // Three color combos share one ID scope (glow, shine, ancient).
    ImGui::PushID(label);
    if (const Data::Effects::GlowColorValue* current = g_GlowColors.Find(value))
    {
        swatch(*current, "##Current");
        ImGui::SameLine();
    }
    const bool newColor = ImGui::SmallButton("+##NewColor");
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::NewGlowColor);
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ComboWidth));
    if (!ImGui::BeginCombo(label, value.c_str()))
    {
        return newColor;
    }
    for (const Data::Effects::GlowColor& color : colors)
    {
        ImGui::PushID(color.name.c_str());
        swatch(color.value, "##Swatch");
        ImGui::SameLine();
        const std::string entry = color.name + " (" + std::to_string(counts.GlowColor(color.name)) + ")";
        if (ImGui::Selectable(entry.c_str(), color.name == value) && color.name != value)
        {
            value = color.name;
            draft.AfterChange(true, label);
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
    return newColor;
}

void GlowMeshes(ModelDraft& draft, const char* onlyLabel, const char* hiddenLabel, Data::Items::ItemGlowMeshes& meshes)
{
    Numbers(draft, onlyLabel, meshes.only);
    OptionalInt(draft, hiddenLabel, meshes.hidden);
}

void GlowLevels(ModelDraft& draft, Data::Items::ItemGlow& glow)
{
    bool own = glow.levels.has_value();
    if (ImGui::Checkbox(I18N::Editor::GlowsLikeOtherLevels, &own))
    {
        if (own)
        {
            std::array<int, Data::Items::ItemGlow::ItemLevelCount> levels{};
            for (size_t level = 0; level < levels.size(); ++level)
            {
                levels[level] = static_cast<int>(level);
            }
            glow.levels = levels;
        }
        else
        {
            glow.levels.reset();
        }
        draft.AfterChange(true, I18N::Editor::GlowsLikeOtherLevels);
    }
    if (!glow.levels)
    {
        return;
    }
    for (size_t level = 0; level < glow.levels->size(); ++level)
    {
        if (level % GlowLevelsPerRow != 0)
        {
            ImGui::SameLine();
        }
        char label[16];
        std::snprintf(label, sizeof(label), "+%zu", level);
        ImGui::SetNextItemWidth(Fields::ScaledWidth(GlowLevelWidth));
        ImGui::DragInt(label, &(*glow.levels)[level], 0.1f, 0, MaxGlowLevel);
        draft.AfterField(I18N::Editor::GlowsLikeOtherLevels);
    }
}
} // namespace

void RenderOwnFile(ModelDraft& draft, ItemModelDefinition& model)
{
    ImGui::TextUnformatted(I18N::Editor::TextureFolders);
    // The draft sees a folder being typed in, so a selection that changes
    // meanwhile stores it first.
    const auto afterFolder = [&] { draft.AfterField(I18N::Editor::TextureFolders); };
    if (ListFields::EditFolders("##Folders", model.textureFolders, afterFolder))
    {
        draft.StoreNow(I18N::Editor::TextureFolders);
    }
    Numbers(draft, I18N::Editor::NoneBlendMeshes, model.noneBlendMeshes);
}

void RenderInventory(ModelDraft& draft, Data::Items::ItemInventoryDisplay& inventory)
{
    ImGui::PushID("Inventory");
    Sliders(draft, I18N::Editor::Anchor, inventory.anchor, AnchorRange);
    Sliders(draft, I18N::Editor::Offset, inventory.offset, OffsetRange);
    Sliders(draft, I18N::Editor::Rotation, inventory.rotation, RotationRange);
    Fields::Slider(draft, I18N::Editor::Scale, inventory.scale, ScaleRange);
    Fields::Slider(draft, I18N::Editor::BodyHeight, inventory.bodyHeight, BodyHeightRange);
    ImGui::PopID();
}

void RenderGround(ModelDraft& draft, ItemModelDefinition& model)
{
    if (ImGui::CollapsingHeader(I18N::Editor::OnTheGround))
    {
        ImGui::PushID("Ground");
        Doubles(draft, I18N::Editor::Rotation, model.ground.rotation);
        bool ownScale = model.ground.scale.has_value();
        if (ImGui::Checkbox(I18N::Editor::OwnScale, &ownScale))
        {
            model.ground.scale = ownScale ? std::optional<double>(1.0) : std::nullopt;
            draft.AfterChange(true, I18N::Editor::OwnScale);
        }
        if (model.ground.scale)
        {
            ImGui::SameLine();
            Fields::Double(draft, I18N::Editor::Scale, *model.ground.scale);
        }
        Fields::Double(draft, I18N::Editor::BodyHeight, model.ground.bodyHeight, "%.1f");
        ImGui::PopID();
    }
    Fields::Check(draft, I18N::Editor::WornAsCloth, model.cloth);
    Fields::Double(draft, I18N::Editor::Specular, model.material.specular, "%.2f");
}

// The shine below +3 the glow names: one of the shine list (design L30), or none.
void ShineCombo(ModelDraft& draft, std::string& value)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ComboWidth));
    if (ImGui::BeginCombo(I18N::Editor::ShineBelow3, value.empty() ? I18N::Editor::None : value.c_str()))
    {
        if (ImGui::Selectable(I18N::Editor::None, value.empty()))
        {
            value.clear();
            draft.AfterChange(true, I18N::Editor::ShineBelow3);
        }
        for (const Data::Effects::Shine& shine : g_Shines.GetShines())
        {
            if (ImGui::Selectable(shine.name.c_str(), shine.name == value))
            {
                value = shine.name;
                draft.AfterChange(true, I18N::Editor::ShineBelow3);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SetItemTooltip("%s", I18N::Editor::ShineBelow3Tooltip);
}

void RenderGlow(ModelDraft& draft, ItemModelDefinition& model, ItemLookUseCounts& counts, CGlowColorDialog& newColor)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Glow))
    {
        return;
    }
    Data::Items::ItemGlow& glow = model.glow;
    // The color fields, in the order newColor's target counts them.
    std::string* const colors[] = {&glow.color, &glow.shineColor, &glow.ancientColor};
    const char* const colorLabels[] = {I18N::Editor::GlowColor, I18N::Editor::ShineColor, I18N::Editor::AncientColor};
    const auto colorField = [&](int index)
    {
        if (ColorCombo(draft, colorLabels[index], *colors[index], counts))
        {
            newColor.Open(*colors[index], index);
        }
    };
    ImGui::PushID("Glow");
    GlowLevels(draft, glow);
    colorField(0);
    GlowMeshes(draft, I18N::Editor::GlowMeshes, I18N::Editor::AllMeshesButThisOne, glow.meshes);
    OptionalInt(draft, I18N::Editor::GlowAgainInWhiteOnMesh, glow.glowAgainInWhite);
    ImGui::SetItemTooltip("%s", I18N::Editor::GlowAgainInWhiteTooltip);
    colorField(1);
    Fields::Check(draft, I18N::Editor::ShineWhite, glow.shineWhite);
    ImGui::PushID("Shine");
    GlowMeshes(draft, I18N::Editor::ShineMeshes, I18N::Editor::AllMeshesButThisOne, glow.shineMeshes);
    ImGui::PopID();
    ShineCombo(draft, glow.shineBelowPlus3);
    colorField(2);
    Fields::Check(draft, I18N::Editor::ExcellentGlow, glow.excellent);
    OptionalInt(draft, I18N::Editor::ExcellentMesh, glow.excellentMesh);
    OptionalInt(draft, I18N::Editor::ExcellentMeshWithoutSkin, glow.excellentMeshWithoutSkin);
    if (const std::optional<Data::Effects::GlowColor> created = newColor.Render())
    {
        // The new color and the field that names it, as one undo step.
        const int target = newColor.GetTarget();
        g_ItemEdits.BeginGroup(I18N::Editor::NewGlowColor);
        g_ItemEdits.SetGlowColor(I18N::Editor::NewGlowColor, *created);
        *colors[target] = created->name;
        draft.AfterChange(true, colorLabels[target]);
        g_ItemEdits.EndGroup();
    }
    ImGui::PopID();
}

// The values the item sets for its look (design L12): one field for each value the look lets each item set, written as
// in the model file ("[0.9, 0.1, 0.2]"; null leaves the look's field out); empty: the look's default. Text that is no
// value is not taken.
static void LookValues(ModelDraft& draft, const char* id, const Data::Looks::LookDefinition* look,
                       std::map<std::string, std::string>& values)
{
    if (look == nullptr || !Data::Looks::SetsEachItem(*look))
    {
        return;
    }
    ImGui::PushID(id);
    ImGui::Indent();
    ImGui::TextDisabled("%s", I18N::Editor::ValuesForThisItem);
    ImGui::SetItemTooltip("%s", I18N::Editor::ValuesForThisItemTooltip);
    for (const std::string& name : Data::Looks::EachItemNames(*look))
    {
        ImGui::PushID(name.c_str());
        const auto set = values.find(name);
        const std::string current = set != values.end() ? set->second : "";
        std::string typed;
        ImGui::SetNextItemWidth(Fields::ScaledWidth(ComboWidth));
        if (InputStringOnLeave(name.c_str(), I18N::Editor::TheLookDefault, current, typed))
        {
            if (typed.empty())
            {
                values.erase(name);
                draft.AfterChange(true, I18N::Editor::ValuesForThisItem);
            }
            else if (!Data::Items::Json::OrderedJson::parse(typed, nullptr, false).is_discarded())
            {
                values[name] = typed;
                draft.AfterChange(true, I18N::Editor::ValuesForThisItem);
            }
        }
        if (set != values.end())
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(I18N::Editor::Reset))
            {
                values.erase(name);
                draft.AfterChange(true, I18N::Editor::ValuesForThisItem);
            }
        }
        ImGui::PopID();
    }
    ImGui::Unindent();
    ImGui::PopID();
}

void RenderLooks(ModelDraft& draft, ItemModelDefinition& model, ItemLookUseCounts& counts)
{
    LookCombo(
        draft, I18N::Editor::RenderStyle, model.renderStyle, Render::Items::Styles::GetNames(),
        [&](std::string_view name) { return counts.RenderStyle(name); }, &Render::Items::Styles::ExistsInCode);
    ImGui::SameLine();
    ImGui::BeginDisabled(model.renderStyle.empty() || !Render::Items::Styles::Exists(model.renderStyle));
    if (ImGui::SmallButton(I18N::Editor::OpenInStyleEditor))
    {
        g_MuStyleEditorUI.ShowRenderStyle(model.renderStyle);
    }
    ImGui::EndDisabled();
    if (!model.renderStyle.empty())
    {
        RenderUsers("##StyleUsers", counts.RenderStyle(model.renderStyle), &FindRenderStyleUsers, model.renderStyle);
        LookValues(draft, "StyleValues", Data::Looks::g_LookStore.FindRenderStyle(model.renderStyle),
                   model.renderStyleValues);
    }
    LookCombo(
        draft, I18N::Editor::ItemEffect, model.itemEffect, Render::Items::ItemEffects::GetNames(),
        [&](std::string_view name) { return counts.ItemEffect(name); }, &Render::Items::ItemEffects::ExistsInCode);
    ImGui::SameLine();
    ImGui::PushID("ItemEffect");
    ImGui::BeginDisabled(model.itemEffect.empty() || !Render::Items::ItemEffects::Exists(model.itemEffect));
    if (ImGui::SmallButton(I18N::Editor::OpenInStyleEditor))
    {
        g_MuStyleEditorUI.ShowItemEffect(model.itemEffect);
    }
    ImGui::EndDisabled();
    ImGui::PopID();
    if (!model.itemEffect.empty())
    {
        RenderUsers("##EffectUsers", counts.ItemEffect(model.itemEffect), &FindItemEffectUsers, model.itemEffect);
        LookValues(draft, "EffectValues", Data::Looks::g_LookStore.FindItemEffect(model.itemEffect),
                   model.itemEffectValues);
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::ItemLooksNote);
    ImGui::PopTextWrapPos();
}
} // namespace MuEditor::Items::ModelForms

#endif // _EDITOR
