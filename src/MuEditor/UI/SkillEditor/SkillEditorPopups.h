#pragma once

#ifdef _EDITOR

#include <string>

// Handles popup dialogs for the Skill Editor
class CSkillEditorPopups
{
public:
    // Success popups
    static void ShowSaveSuccessPopup(const std::string& changeLog);
    static void ShowExportCSVSuccessPopup();

    // Error popups
    static void ShowSaveFailedPopup();
    static void ShowExportCSVFailedPopup();

    // Render active popups (call each frame)
    static void RenderPopups();

private:
    // Popup state
    static bool s_showSaveSuccess;
    static bool s_showSaveFailure;
    static bool s_showExportCSVSuccess;
    static bool s_showExportCSVFailure;
    static std::string s_changeLog;
};

#endif // _EDITOR
