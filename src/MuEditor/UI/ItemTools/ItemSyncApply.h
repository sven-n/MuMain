#pragma once

#ifdef _EDITOR

#include "ItemSyncDiff.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemExchangeFile.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <filesystem>
#include <string>
#include <vector>

// What the Item Sync window does (phase 7): an import changes the client's data with the checked
// rows of a diff as one step of the undo history; an export writes the client's items and
// formulas into a file, without a diff (OpenMU's import page shows one).
namespace MuEditor::Items
{
// The client's items and item formulas as an exchange file.
Data::Items::ItemExchangeFile BuildClientExchange();
// BuildClientExchange with the client's entries without a name at the numbers `file` has items
// at, which an import fills in: what Import and Update from server compare `file` with. Where
// the file has no item either, neither side has one. The export leaves them out (the server
// cannot use an item without a name).
Data::Items::ItemExchangeFile BuildClientComparison(const Data::Items::ItemExchangeFile& file);

// The client's item `current` (nullptr for a new one) with the shared fields of `exchange`.
// Its client-only fields stay; a new item and an entry without a name get the two-handed flag
// and the weapon kind its group gives. The item follows its profile rules (`profiles`) when the file's profile is the
// one they give; another one is a pick.
Data::Items::ItemDefinition ToClientItem(const Data::Items::ItemDefinition* current,
                                         const Data::Items::ItemDefinition& exchange,
                                         const Data::Items::ItemStatProfiles& profiles);

// What a new item of the file lacks for the client: a model entry, or what its profile rules
// need and the file cannot give (a wing tier, a tag).
std::vector<std::string> DescribeNewItem(const Data::Items::ItemDefinition& exchange);

// Applies the checked rows of an import of `file` to the client's data as one step of the undo
// history: the item formulas first, so the items follow the new profile rules, then the
// items. The data as it would be is checked first, with the checks of a save; with fatal problems
// nothing changes, false is returned and the fatal problems are in `issues`.
bool ApplyItemSyncImport(const ItemSyncDiff& diff, const Data::Items::ItemExchangeFile& file,
                         std::vector<Data::Items::ItemDataIssue>& issues);

// What an export would change in the file at `path`, for the confirmation before it replaces
// the file. The items are compared as a whole (a name in a language only the file has counts
// too, as the file loses it), the formulas by entry.
struct ItemSyncExportPreview
{
    bool fileExists = false;
    // An exchange file the reader takes (an item it has twice is no obstacle: it is replaced).
    bool readable = false;
    int added = 0;
    int changed = 0;
    int removed = 0;
    bool fileHasFormulas = false;
    // The formula entries that change, while the file has formulas.
    int formulaEntries = 0;

    // A readable file that already has the client's items and formulas.
    bool ChangesNothing() const
    {
        return readable && added == 0 && changed == 0 && removed == 0 && fileHasFormulas && formulaEntries == 0;
    }
};
ItemSyncExportPreview PreviewItemSyncExport(const std::filesystem::path& path);

// Writes the client's items and formulas into `path` (source MuMain), replacing what it holds;
// `writtenItems` is how many items it has.
bool WriteItemSyncExport(const std::filesystem::path& path, size_t& writtenItems,
                         std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Items

#endif // _EDITOR
