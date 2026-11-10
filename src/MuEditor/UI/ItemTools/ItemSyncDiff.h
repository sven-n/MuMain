#pragma once

#ifdef _EDITOR

#include "KnownDifferences.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemExchangeFile.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

// The diff of the Item Sync window's import (phase 7): the client's items and item formulas
// against an item exchange file, field by field, and the client's data with the checked rows
// taken from the file. The fields are named by their keys in the item files ("width",
// "requirements.strength", "name.de", "tags.ammunition") and in the item stat file
// ("dropLevelBonus.excellent", "tables.damageIncreaseWeapons", "default.damage.excellent",
// "profiles.weapon.requirements.strength"; "profiles.weapon" for a profile as a whole). An
// export writes the client's data without a diff (ItemSyncApply.h).
namespace MuEditor::Items
{
struct SyncField
{
    std::string field;
    // As JSON; nothing when that side has the field's default (it leaves the field out).
    std::optional<std::string> current;
    std::optional<std::string> incoming;
    // Its own mark, else the mark of its item as a whole.
    WrongSide wrongSide = WrongSide::None;
    // wrongSide is the item's mark: the field has none of its own.
    bool markOfItem = false;
    // Why it cannot be checked (the game holds the item), or empty.
    std::string locked;
    // Why the value is not exact on the other side, or empty.
    std::string inexact;
    bool checked = false;
};

enum class SyncChange : uint8_t
{
    New,
    Changed,
    Removed,
};

struct SyncItem
{
    int group = 0;
    int number = 0;
    SyncChange change = SyncChange::Changed;
    // The English name of the side that has the item (the file's when both have it).
    std::string name;
    // Changed: the fields that differ; New: the fields of the file's item.
    std::vector<SyncField> fields;
    // New and Removed rows. Changed rows apply their checked fields.
    bool checked = false;
    // The item's mark as a whole: a New or Removed item, or the fields of a Changed one that have
    // no mark of their own (also one that differs only later).
    WrongSide wrongSide = WrongSide::None;
    std::string locked;
};

struct SyncFormula
{
    std::string entry;
    std::optional<std::string> current;
    std::optional<std::string> incoming;
    WrongSide wrongSide = WrongSide::None;
    bool checked = false;
};

struct ItemSyncDiff
{
    std::vector<SyncItem> items;
    std::vector<SyncFormula> formulas;
    // What was compared, as the window shows it: the items of both sides (each group and
    // number once) and how many of them are the same; the formula entries likewise, while both
    // sides have formulas. The same ones are not rows.
    int comparedItems = 0;
    int sameItems = 0;
    int comparedFormulaEntries = 0;
    int sameFormulaEntries = 0;

    bool IsEmpty() const
    {
        return items.empty() && formulas.empty();
    }
};

struct SyncOptions
{
    const KnownDifferences* known = nullptr;
    // Names in other languages are not taken from the file.
    std::span<const std::string> uiLocales;
    // An item the game holds keeps its size and slot, is not removed, and a new one of its type
    // is not added.
    std::function<bool(int itemType)> isHeld;
    // A comparison of two files: every language counts, also one that only one side has; a
    // removed item lists its fields; a row of an item a file has twice names that file.
    bool comparingFiles = false;
};

// The differences between the client's data (`client`, BuildItemExchange) and `file`. Checked
// by default: what the file changes; unchecked: removals (items and formula entries the file
// lacks), inexact and locked fields. A difference marked wrong in the server is unchecked; one
// marked wrong in the client starts as without a mark. A language one side lacks stays as it is
// on the other (unless comparingFiles). An item a side has more than once (its repeatedItems) is a
// locked row without fields. The formulas are compared while both sides have them.
ItemSyncDiff BuildItemSyncDiff(const Data::Items::ItemExchangeFile& client, const Data::Items::ItemExchangeFile& file,
                               const SyncOptions& options);

// Whether the row is an item only the client has. Import and Update from server list these
// rows in a section of their own: the file does not change them, and each one is removed only
// when it is checked (its own box or Check all).
bool IsOnlyInClient(const SyncItem& row);
// Checks or unchecks a row's box: a New or Removed item as a whole, the fields of a Changed one.
// Locked items and fields stay unchecked.
void SetRowChecked(SyncItem& row, bool checked);
// Check all and Check none: every row and formula entry, also the items only the client has
// (the confirmation of the import says how many it removes; Undo brings them back).
void SetAllRowsChecked(ItemSyncDiff& diff, bool checked);
// Marks which side of a difference is wrong (WrongSide::None: not decided). Wrong in server
// unchecks it, and leaving that mark gives the default back; otherwise wrong in client leaves
// the box as it is, checking it is the user's. An empty `field` names the item as a whole: a New or
// Removed one, or every field of a Changed one.
void SetWrongSide(SyncItem& row, const std::string& field, WrongSide side);
void SetWrongSide(SyncFormula& row, WrongSide side);
// Gives the diff's rows the marks `known` has now (e.g. read again from the file), each box as
// SetWrongSide changes it; a row whose mark is the same keeps its box.
void ApplyMarks(ItemSyncDiff& diff, const KnownDifferences& known);
// SetWrongSide, and the mark kept in `known` for the next comparisons. A mark of a changed item
// as a whole takes out its fields' own marks (not the mark of the item new or removed, nor the
// other way round); a field's own mark counts for that field instead of the
// item's, which stays for the others (also ones that differ only later).
void MarkWrongSide(KnownDifferences& known, SyncItem& row, const std::string& field, WrongSide side);
void MarkWrongSide(KnownDifferences& known, SyncFormula& row, WrongSide side);
// After the diff's checked rows were applied: takes out of `known` the marks of what was applied,
// either side's, as the client has the file's value now; an applied removal takes every mark of
// the item (an item number used again later starts without one). A changed item's whole-item mark
// goes when all its fields were applied. Whether `known` changed.
bool ForgetAppliedMarks(const ItemSyncDiff& diff, KnownDifferences& known);
// The side marked on a New or Removed item, or the side every field of a Changed one has (its own
// mark or the item's); None when nothing or not every field is marked, or the fields have
// different sides.
WrongSide GetRowWrongSide(const SyncItem& row);
// The rows as a Markdown table, to paste into an issue or a chat, in English whatever the
// editor's language: `title`, an empty line, then a table of each item's group, number and name,
// and `withWrongIn` the side marked wrong (GetRowWrongSide; a comparison of two files has no
// marks). With a `changeColumn` (its head, e.g. "In the client"), the table also has each row's
// change first and its number of differing fields.
std::string WriteSyncRowsMarkdown(std::span<const SyncItem* const> rows, std::string_view title,
                                  std::string_view changeColumn, bool withWrongIn);
// How many formula entries differ between two item stat files: the formula rows a diff lists.
int CountFormulaDifferences(const Data::Items::ItemStatFile& current, const Data::Items::ItemStatFile& incoming);

// The client's data with the checked rows taken from the file. Formulas the item stat file's
// checks refuse are left as they were, with their problems in `issues`.
Data::Items::ItemExchangeFile ApplyItemSyncDiff(const Data::Items::ItemExchangeFile& client,
                                                const Data::Items::ItemExchangeFile& file, const ItemSyncDiff& diff,
                                                std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Items

#endif // _EDITOR
