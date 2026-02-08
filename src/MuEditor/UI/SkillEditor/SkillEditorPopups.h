#pragma once

#ifdef _EDITOR

// Handles popup dialogs for the Skill Editor
class CSkillEditorPopups
{
public:
    // Render all popups (call each frame)
    static void RenderPopups();

private:
    // Helper to render a simple popup with OK button
    static bool RenderSimplePopup(const char* popupId, const char* message);
};

#endif // _EDITOR
