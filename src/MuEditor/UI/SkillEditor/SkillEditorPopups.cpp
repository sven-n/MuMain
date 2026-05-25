#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorPopups.h"
#include "imgui.h"
#include "I18N/All.h"

bool CSkillEditorPopups::RenderSimplePopup(const char* popupId, const char* message)
{
    if (ImGui::BeginPopupModal(popupId, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", message);
        if (ImGui::Button(I18N::Editor::OK, ImVec2(120, 0)))
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
    RenderSimplePopup("Save Success", I18N::Editor::SkillsSavedSuccessfully);
    RenderSimplePopup("Save Failed", I18N::Editor::FailedToSaveSkills);

    // Legacy Export popups
    RenderSimplePopup("Export S6E3 Success", I18N::Editor::ExportedAsLegacyFormatSuccessfully);
    RenderSimplePopup("Export S6E3 Failed", I18N::Editor::FailedToExportAsLegacyFormat);

    // CSV Export popups
    RenderSimplePopup("Export CSV Success", I18N::Editor::SkillsExportedAsCSVSuccessfully);
    RenderSimplePopup("Export CSV Failed", I18N::Editor::FailedToExportSkillAttributesAsCSV);
}

#endif // _EDITOR
