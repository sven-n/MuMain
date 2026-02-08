#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorPopups.h"
#include "imgui.h"
#include "Translation/i18n.h"

bool CSkillEditorPopups::RenderSimplePopup(const char* popupId, const char* message)
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

void CSkillEditorPopups::RenderPopups()
{
    // Save popups
    RenderSimplePopup("Save Success", EDITOR_TEXT("popup_save_skills_success"));
    RenderSimplePopup("Save Failed", EDITOR_TEXT("popup_save_skills_failed"));

    // Legacy Export popups
    RenderSimplePopup("Export S6E3 Success", EDITOR_TEXT("popup_export_s6e3_success"));
    RenderSimplePopup("Export S6E3 Failed", EDITOR_TEXT("popup_export_s6e3_failed"));

    // CSV Export popups
    RenderSimplePopup("Export CSV Success", EDITOR_TEXT("popup_export_csv_skills_success"));
    RenderSimplePopup("Export CSV Failed", EDITOR_TEXT("popup_export_csv_skills_failed"));
}

#endif // _EDITOR
