#pragma once

#ifdef _EDITOR

// Handles all popup modals for the Item Editor
// Eliminates duplication for success/failed popup patterns
class CItemEditorPopups
{
public:
    // Render all standard item editor popups (call once per frame)
    static void RenderAll();

private:
    // Render a simple success/error popup with OK button
    static bool RenderSimplePopup(const char* popupId, const char* message);
};

#endif // _EDITOR
