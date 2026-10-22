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

// What the Item Sync window does with the checked rows of a diff (phase 7): an import changes
// the client's data as one step of the undo history, an export writes the file.
namespace MuEditor::Items
{
// The client's items and item formulas as an exchange file.
Data::Items::ItemExchangeFile BuildClientExchange();

// The client's item `current` (nullptr for a new one) with the shared fields of `exchange`.
// Its client-only fields stay; a new item gets the two-handed flag and the weapon kind its
// group gives. The item follows its profile rules (`profiles`) when the file's profile is the
// one they give; another one is a pick.
Data::Items::ItemDefinition ToClientItem(const Data::Items::ItemDefinition* current,
                                         const Data::Items::ItemDefinition& exchange,
                                         const Data::Items::ItemStatProfiles& profiles);

// What a new item of the file lacks for the client: a model entry, or what its profile rules
// need and the file cannot give (a wing tier, a tag).
std::vector<std::string> DescribeNewItem(const Data::Items::ItemDefinition& exchange);

// Applies the checked rows of an import of `file` to the client's data as one step of the undo
// history: the item formulas first, so the items follow the new profile rules, then the
// items. The data as it would be is checked first, with the checks of a save; with errors
// nothing changes, false is returned and the errors are in `issues`.
bool ApplyItemSyncImport(const ItemSyncDiff& diff, const Data::Items::ItemExchangeFile& file,
                         std::vector<Data::Items::ItemDataIssue>& issues);

// Writes the checked rows of an export into `path` (`file` is what it held). The written file
// comes from the client.
bool ApplyItemSyncExport(const ItemSyncDiff& diff, const Data::Items::ItemExchangeFile& file,
                         const std::filesystem::path& path, std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Items

#endif // _EDITOR
