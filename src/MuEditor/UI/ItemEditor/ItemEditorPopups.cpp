#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorPopups.h"
#include "imgui.h"
#include "I18N/All.h"

bool CItemEditorPopups::RenderSimplePopup(const char* popupId, const char* message)
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

void CItemEditorPopups::RenderAll()
{
    // Save popups
    RenderSimplePopup("Save Success", I18N::Editor::ItemsSavedSuccessfully);
    RenderSimplePopup("Save Failed", I18N::Editor::FailedToSaveItems);

    // CSV Export popups
    RenderSimplePopup("Export CSV Success", I18N::Editor::ItemsExportedAsCSVSuccessfully);
    RenderSimplePopup("Export CSV Failed", I18N::Editor::FailedToExportItemAttributesAsCSV);

    // S6E3 Export popups
    RenderSimplePopup("Export S6E3 Success", I18N::Editor::ExportedAsLegacyFormatSuccessfully);
    RenderSimplePopup("Export S6E3 Failed", I18N::Editor::FailedToExportAsLegacyFormat);
}

#endif // _EDITOR
