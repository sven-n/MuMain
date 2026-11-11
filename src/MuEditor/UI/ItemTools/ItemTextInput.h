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
// A text field that shows `value` and changes it only when it is left after typing: true then, with what was typed in
// `typed`, so the caller sees one change instead of one per key. The text being typed is kept here meanwhile.
bool InputStringOnLeave(const char* label, const char* hint, const std::string& value, std::string& typed);
} // namespace MuEditor::Items

#endif // _EDITOR
