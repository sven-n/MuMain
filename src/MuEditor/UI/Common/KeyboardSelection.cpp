#include "stdafx.h"

#ifdef _EDITOR

#include "KeyboardSelection.h"

#include "imgui_internal.h"

namespace MuEditor::KeyboardSelection
{
bool MovedOnto()
{
    const ImGuiContext& context = *ImGui::GetCurrentContext();
    return context.NavJustMovedToId != 0 && context.NavJustMovedToId == ImGui::GetItemID() &&
           (context.NavJustMovedToKeyMods & ImGuiMod_Ctrl) == 0;
}
} // namespace MuEditor::KeyboardSelection

#endif // _EDITOR
