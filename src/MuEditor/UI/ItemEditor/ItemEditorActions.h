#pragma once

#ifdef _EDITOR

#include "_struct.h"
#include <string>

// Handles Save/Export action buttons for the Item Editor
class CItemEditorActions
{
public:
    static void RenderSaveButton();
    static void RenderExportS6E3Button();
    static void RenderExportCSVButton();

    // Render all action buttons in a row
    static void RenderAllButtons();

    // Export item data to CSV format
    static std::string ExportItemToCSV(int itemIndex, ITEM_ATTRIBUTE& item);

    // Export item data to readable format (key=value pairs)
    static std::string ExportItemToReadable(int itemIndex, ITEM_ATTRIBUTE& item);

    // Export both formats combined (readable + CSV)
    static std::string ExportItemCombined(int itemIndex, ITEM_ATTRIBUTE& item);

private:
    // Helper to convert item name to UTF-8
    static void ConvertItemName(char* outBuffer, size_t bufferSize, const wchar_t* name);
};

#endif // _EDITOR
