#pragma once
#include "Data/GameData/ItemData/ItemFieldMetadata.h"

#ifdef _EDITOR

#include "Core/Globals/_struct.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include <string>
#include <vector>

// Handles Save/Import/Export action buttons for the Item Editor
// Uses metadata-driven approach for automatic export generation
class CItemEditorActions
{
public:
    // Saves the items to Data/Items (JSON).
    static void RenderSaveButton();
    // Replaces all items with the legacy Item_<language>.bmd files.
    // Returns true when the items were replaced.
    static bool RenderImportBmdButton();
    static void RenderExportBmdButton();
    static void RenderExportS6E3Button();
    static void RenderExportCSVButton();

    // Render all action buttons in a row. Returns true when the items were
    // replaced, so the table must rebuild its list.
    static bool RenderAllButtons();

    // Export item data to CSV format (metadata-driven)
    static std::string ExportItemToCSV(int itemIndex, ITEM_ATTRIBUTE& item);

    // Export item data to readable format (key=value pairs, metadata-driven)
    static std::string ExportItemToReadable(int itemIndex, ITEM_ATTRIBUTE& item);

    // Export both formats combined (readable + CSV)
    static std::string ExportItemCombined(int itemIndex, ITEM_ATTRIBUTE& item);

    // Get CSV header row (metadata-driven)
    static std::string GetCSVHeader();

private:
    static void LogIssues(const std::vector<Data::Items::ItemDataIssue>& issues);
    static void LogImportResult(const Data::Items::ItemBmdImportResult& result);

    // Helper to convert item name to UTF-8
    static void ConvertItemName(char* outBuffer, size_t bufferSize, const wchar_t* name);

    // Get field value as string (descriptor-driven)
    static std::string GetFieldValueAsString(const ITEM_ATTRIBUTE &item, const ItemFieldDescriptor &desc);
};

#endif // _EDITOR
