#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"

#include "Data/GameData/ItemData/ItemEnumNames.h"

#include "imgui.h"

#include <cstdint>
#include <string>

// The fields of the item tool forms: each edits a value of the draft, with
// the field's label as the undo step's name. A number is stored when the
// field is left after a change; a checkbox or list right away.
namespace MuEditor::Items::Fields
{
// The width of a number field, before the editor UI scale.
constexpr float NumberWidth = 120.0f;
// The width of a slider, before the editor UI scale.
constexpr float SliderWidth = 220.0f;

// What a slider spans and how it shows its value. Ctrl+click types a value,
// also one outside the span unless the flags clamp it.
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

// A slider for the value: a drag is one undo step, stored when it ends.
template <typename TDraft>
void Slider(TDraft& draft, const char* label, double& value, const SliderRange& range, const char* undoLabel = nullptr)
{
    ImGui::SetNextItemWidth(ScaledWidth(SliderWidth));
    ImGui::SliderScalar(label, ImGuiDataType_Double, &value, &range.min, &range.max, range.format, range.flags);
    draft.AfterField(undoLabel != nullptr ? undoLabel : label);
}

template <typename TDraft> void Check(TDraft& draft, const char* label, bool& value)
{
    draft.AfterChange(ImGui::Checkbox(label, &value), label);
}

// A list of the enum's names in the item files (ItemEnumNames).
template <typename TDraft, typename TEnum> void Enum(TDraft& draft, const char* label, TEnum& value)
{
    const char* current = Data::Items::FindEnumName(value);
    ImGui::SetNextItemWidth(ScaledWidth(NumberWidth * 1.5f));
    if (!ImGui::BeginCombo(label, current != nullptr ? current : "?"))
    {
        return;
    }
    for (const auto& entry : Data::Items::GetEnumNames(TEnum{}))
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
