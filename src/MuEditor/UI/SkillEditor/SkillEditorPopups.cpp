#include "stdafx.h"

#ifdef _EDITOR

#include "SkillEditorPopups.h"
#include "imgui.h"
#include "Translation/i18n.h"

// Static member initialization
bool CSkillEditorPopups::s_showSaveSuccess = false;
bool CSkillEditorPopups::s_showSaveFailure = false;
bool CSkillEditorPopups::s_showExportCSVSuccess = false;
bool CSkillEditorPopups::s_showExportCSVFailure = false;
std::string CSkillEditorPopups::s_changeLog = "";

// ===== SHOW POPUP FUNCTIONS =====

void CSkillEditorPopups::ShowSaveSuccessPopup(const std::string& changeLog)
{
    s_changeLog = changeLog;
    s_showSaveSuccess = true;
    ImGui::OpenPopup("Skill Save Success");
}

void CSkillEditorPopups::ShowSaveFailedPopup()
{
    s_showSaveFailure = true;
    ImGui::OpenPopup("Skill Save Failed");
}

void CSkillEditorPopups::ShowExportCSVSuccessPopup()
{
    s_showExportCSVSuccess = true;
    ImGui::OpenPopup("Skill Export CSV Success");
}

void CSkillEditorPopups::ShowExportCSVFailedPopup()
{
    s_showExportCSVFailure = true;
    ImGui::OpenPopup("Skill Export CSV Failed");
}

// ===== RENDER FUNCTIONS =====

void CSkillEditorPopups::RenderPopups()
{
    // Save Success popup with changelog
    if (s_showSaveSuccess && ImGui::BeginPopupModal("Skill Save Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", EDITOR_TEXT("popup_save_skills_success"));

        if (!s_changeLog.empty())
        {
            ImGui::Separator();
            ImGui::TextWrapped("%s", s_changeLog.c_str());
        }

        if (ImGui::Button(EDITOR_TEXT("btn_ok"), ImVec2(120, 0)))
        {
            s_showSaveSuccess = false;
            s_changeLog.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Save Failed popup
    if (s_showSaveFailure && ImGui::BeginPopupModal("Skill Save Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", EDITOR_TEXT("popup_save_skills_failed"));

        if (ImGui::Button(EDITOR_TEXT("btn_ok"), ImVec2(120, 0)))
        {
            s_showSaveFailure = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Export CSV Success popup
    if (s_showExportCSVSuccess && ImGui::BeginPopupModal("Skill Export CSV Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", EDITOR_TEXT("popup_export_csv_skills_success"));

        if (ImGui::Button(EDITOR_TEXT("btn_ok"), ImVec2(120, 0)))
        {
            s_showExportCSVSuccess = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Export CSV Failed popup
    if (s_showExportCSVFailure && ImGui::BeginPopupModal("Skill Export CSV Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", EDITOR_TEXT("popup_export_csv_skills_failed"));

        if (ImGui::Button(EDITOR_TEXT("btn_ok"), ImVec2(120, 0)))
        {
            s_showExportCSVFailure = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

#endif // _EDITOR
