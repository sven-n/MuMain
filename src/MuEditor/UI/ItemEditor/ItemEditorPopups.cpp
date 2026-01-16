#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorPopups.h"
#include "imgui.h"
#include "Translation/i18n.h"

bool CItemEditorPopups::RenderSimplePopup(const char* popupId, const char* message)
{
    if (ImGui::BeginPopupModal(popupId, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", message);
        if (ImGui::Button(EDITOR_TEXT("btn_ok"), ImVec2(120, 0)))
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
    RenderSimplePopup("Save Success", EDITOR_TEXT("popup_save_success"));
    RenderSimplePopup("Save Failed", EDITOR_TEXT("popup_save_failed"));

    // CSV Export popups
    RenderSimplePopup("Export CSV Success", EDITOR_TEXT("popup_export_csv_success"));
    RenderSimplePopup("Export CSV Failed", EDITOR_TEXT("popup_export_csv_failed"));

    // S6E3 Export popups
    RenderSimplePopup("Export S6E3 Success", EDITOR_TEXT("popup_export_s6e3_success"));
    RenderSimplePopup("Export S6E3 Failed", EDITOR_TEXT("popup_export_s6e3_failed"));
}

#endif // _EDITOR
