#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"

#include "Data/GameData/ItemData/ItemEnumNames.h"

#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <vector>

// The fields of the item tool forms: each edits a value of the draft, with
// the field's label as the undo step's name. A number is stored when the
// field is left after a change; a checkbox or list right away.
namespace MuEditor::Items::Fields
{
// The width of a number field, before the editor UI scale.
constexpr float NumberWidth = 120.0f;
// The width of a slider and of the number field beside it, before the editor
// UI scale.
constexpr float SliderWidth = 180.0f;
constexpr float SliderNumberWidth = 90.0f;

// What a slider spans and how it shows its value. The number field beside it
// (or Ctrl+click on the slider) takes a value outside the span too, unless the
// flags clamp it.
struct SliderRange
{
    double min;
    double max;
    const char* format;
    ImGuiSliderFlags flags = ImGuiSliderFlags_None;
};

float ScaledWidth(float width);

template <typename TDraft> void Byte(TDraft& draft, const char* label, std::uint8_t& value)
{
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth));
    ImGui::InputScalar(label, ImGuiDataType_U8, &value);
    draft.AfterField(label);
}

template <typename TDraft> void Word(TDraft& draft, const char* label, std::uint16_t& value)
{
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth));
    ImGui::InputScalar(label, ImGuiDataType_U16, &value);
    draft.AfterField(label);
}

template <typename TDraft> void Int(TDraft& draft, const char* label, int& value)
{
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth));
    ImGui::InputScalar(label, ImGuiDataType_S32, &value);
    draft.AfterField(label);
}

template <typename TDraft> void Double(TDraft& draft, const char* label, double& value, const char* format = "%.4f")
{
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth));
    ImGui::InputScalar(label, ImGuiDataType_Double, &value, nullptr, nullptr, format);
    draft.AfterField(label);
}

// A slider for the value and a number field beside it, to drag or type it: a
// drag is one undo step, stored when it ends; a typed value is stored when the
// field is left.
template <typename TDraft>
void Slider(TDraft& draft, const char* label, double& value, const SliderRange& range, const char* undoLabel = nullptr)
{
    const char* step = undoLabel != nullptr ? undoLabel : label;
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(ScaledWidth(SliderWidth));
    ImGui::SliderScalar("##Slider", ImGuiDataType_Double, &value, &range.min, &range.max, range.format, range.flags);
    draft.AfterField(step);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ScaledWidth(SliderNumberWidth));
    ImGui::InputScalar("##Number", ImGuiDataType_Double, &value, nullptr, nullptr, range.format);
    if (ImGui::IsItemEdited() && (range.flags & ImGuiSliderFlags_ClampOnInput) != 0)
    {
        value = std::clamp(value, range.min, range.max);
    }
    draft.AfterField(step);
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    ImGui::PopID();
}

template <typename TDraft> void Check(TDraft& draft, const char* label, bool& value)
{
    draft.AfterChange(ImGui::Checkbox(label, &value), label);
}

// The entries of an enum list: `first` (the value the item files write by
// leaving the field out, TEnum::None, and the one the item's data gives), then
// the enum's names in the item files (ItemEnumNames); each value once, by its
// first entry.
template <typename TEnum>
std::vector<Data::Items::EnumName<TEnum>>
EnumEntries(std::initializer_list<Data::Items::EnumName<std::type_identity_t<TEnum>>> first)
{
    std::vector<Data::Items::EnumName<TEnum>> entries;
    const auto add = [&entries](const Data::Items::EnumName<TEnum>& entry)
    {
        if (std::ranges::find(entries, entry.value, &Data::Items::EnumName<TEnum>::value) == entries.end())
        {
            entries.push_back(entry);
        }
    };
    std::ranges::for_each(first, add);
    std::ranges::for_each(Data::Items::GetEnumNames(TEnum{}), add);
    return entries;
}

// A list of the entries EnumEntries gives; the value shows by its entry ("?"
// for a value without one).
template <typename TDraft, typename TEnum>
void Enum(TDraft& draft, const char* label, TEnum& value,
          std::initializer_list<Data::Items::EnumName<std::type_identity_t<TEnum>>> first)
{
    const std::vector<Data::Items::EnumName<TEnum>> entries = EnumEntries<TEnum>(first);
    const auto current = std::ranges::find(entries, value, &Data::Items::EnumName<TEnum>::value);
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth * 2.0f));
    if (!ImGui::BeginCombo(label, current != entries.end() ? current->name : "?"))
    {
        return;
    }
    for (const auto& entry : entries)
    {
        if (ImGui::Selectable(entry.name, entry.value == value) && entry.value != value)
        {
            value = entry.value;
            draft.AfterChange(true, label);
        }
    }
    ImGui::EndCombo();
}
} // namespace MuEditor::Items::Fields

#endif // _EDITOR
