#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorPopups.h"
#include "imgui.h"

bool CItemEditorPopups::RenderSimplePopup(const char* popupId, const char* message)
{
    if (ImGui::BeginPopupModal(popupId, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", message);
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

void CItemEditorPopups::RenderAll()
{
    // Save popups
    RenderSimplePopup("Save Success", "Items saved successfully!");
    RenderSimplePopup("Save Failed", "Failed to save items!");

    // CSV Export popups
    RenderSimplePopup("Export CSV Success", "Items exported as CSV successfully!");
    RenderSimplePopup("Export CSV Failed", "Failed to export item attributes as CSV!");

    // S6E3 Export popups
    RenderSimplePopup("Export S6E3 Success", "Items exported as S6E3 legacy format successfully!");
    RenderSimplePopup("Export S6E3 Failed", "Failed to export item attributes as S6E3 format!");
}

#endif // _EDITOR
