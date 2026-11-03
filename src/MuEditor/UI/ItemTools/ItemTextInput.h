#pragma once

#ifdef _EDITOR

#include "imgui.h"

#include <string>

namespace MuEditor::Items
{
// ImGui::InputText on a std::string, which ImGui sizes to the text (as
// misc/cpp/imgui_stdlib does), so a name of any length is edited whole.
bool InputString(const char* label, std::string& text, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
bool InputStringWithHint(const char* label, const char* hint, std::string& text,
                         ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
} // namespace MuEditor::Items

#endif // _EDITOR
