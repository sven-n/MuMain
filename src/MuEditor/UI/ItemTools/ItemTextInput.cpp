#include "stdafx.h"

#ifdef _EDITOR

#include "ItemTextInput.h"

namespace MuEditor::Items
{
namespace
{
int ResizeString(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* text = static_cast<std::string*>(data->UserData);
        text->resize(data->BufTextLen);
        data->Buf = text->data();
    }
    return 0;
}
} // namespace

bool InputString(const char* label, std::string& text, ImGuiInputTextFlags flags)
{
    return ImGui::InputText(label, text.data(), text.capacity() + 1, flags | ImGuiInputTextFlags_CallbackResize,
                            ResizeString, &text);
}

bool InputStringWithHint(const char* label, const char* hint, std::string& text, ImGuiInputTextFlags flags)
{
    return ImGui::InputTextWithHint(label, hint, text.data(), text.capacity() + 1,
                                    flags | ImGuiInputTextFlags_CallbackResize, ResizeString, &text);
}

bool InputStringOnLeave(const char* label, const char* hint, const std::string& value, std::string& typed)
{
    // The field being typed in and its text; one field is active at a time.
    static ImGuiID s_editing = 0;
    static std::string s_text;
    const ImGuiID id = ImGui::GetID(label);
    std::string shown = value;
    std::string& text = s_editing == id ? s_text : shown;
    InputStringWithHint(label, hint, text);
    const bool active = ImGui::IsItemActive();
    const bool left = ImGui::IsItemDeactivated();
    if (active && s_editing != id)
    {
        s_editing = id;
        s_text = text;
    }
    // Let go without being left (its window was hidden): the value shows again.
    if (!active && s_editing == id)
    {
        s_editing = 0;
    }
    if (!left || !ImGui::IsItemDeactivatedAfterEdit())
    {
        return false;
    }
    typed = text;
    return true;
}
} // namespace MuEditor::Items

#endif // _EDITOR
