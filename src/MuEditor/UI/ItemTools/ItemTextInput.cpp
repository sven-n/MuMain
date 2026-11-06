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
} // namespace MuEditor::Items

#endif // _EDITOR
