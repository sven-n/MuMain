#pragma once

#ifdef _EDITOR

// Handles Save/Export action buttons for the Item Editor
class CItemEditorActions
{
public:
    static void RenderSaveButton();
    static void RenderExportS6E3Button();
    static void RenderExportCSVButton();

    // Render all action buttons in a row
    static void RenderAllButtons();
};

#endif // _EDITOR
