#pragma once

#ifdef _EDITOR

#include "Core/Globals/_struct.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include <string>
#include <vector>

// Handles the Save/Import/Export action buttons of the item tools
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
    // replaced, so a list of them must be built again.
    static bool RenderAllButtons();

    // After an import: puts back the size and slot (or the whole item, if the
    // import removed it) of the items the game holds that had data, from
    // `before`. A held type without data takes no grid cells, so it takes the
    // imported item as it is.
    static void KeepHeldItemShapes(const std::vector<Data::Items::ItemDefinition>& before);

private:
    static void LogIssues(const std::vector<Data::Items::ItemDataIssue>& issues);
    // After a save: each UI language that misses item names, with how many (the
    // items show the English name there).
    static void LogMissingItemNames();
    static void LogImportResult(const Data::Items::ItemBmdImportResult& result);
};

#endif // _EDITOR
