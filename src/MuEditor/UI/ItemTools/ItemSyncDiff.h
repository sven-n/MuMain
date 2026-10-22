#pragma once

#ifdef _EDITOR

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

// The diff of the Item Sync window (phase 7): the client's items and item formulas against an
// item exchange file, field by field, and the side that changes with the checked rows taken
// from the other. The fields are named by their keys in the item files ("width",
// "requirements.strength", "name.de", "tags.ammunition") and in the item stat file
// ("dropLevelBonus.excellent", "tables.damageIncreaseWeapons", "default.damage.excellent",
// "profiles.weapon.requirements.strength"; "profiles.weapon" for a profile as a whole).
namespace MuEditor::Items
{
// Which side takes the other's values.
enum class SyncDirection : uint8_t
{
    // The file's values into the client's data (Import, Update from server).
    Import,
    // The client's values into the file (Export).
    Export,
};

// The items' fields and formula entries meant to differ between the client and a server
// (Data/Items/Sync/KnownDifferences.json): the diff shows them unchecked.
struct KnownDifferences
{
    // (group, number, field); an empty field names the item as a whole (new or removed).
    std::set<std::tuple<int, int, std::string>> fields;
    std::set<std::string, std::less<>> formulas;

    bool Contains(int group, int number, const std::string& field) const;
    bool Contains(const std::string& formula) const;
    bool operator==(const KnownDifferences& other) const = default;
};

std::filesystem::path GetKnownDifferencesFile();
// Nothing known when the file is not there.
KnownDifferences LoadKnownDifferences(const std::filesystem::path& file,
                                      std::vector<Data::Items::ItemDataIssue>& issues);
std::string WriteKnownDifferencesJson(const KnownDifferences& known);

struct SyncField
{
    std::string field;
    // As JSON; nothing when that side has the field's default (it leaves the field out).
    std::optional<std::string> current;
    std::optional<std::string> incoming;
    bool known = false;
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
    // The English name of the side that has the item (the incoming one when both have it).
    std::string name;
    // Changed: the fields that differ; New: the fields of the incoming item.
    std::vector<SyncField> fields;
    // New and Removed rows. Changed rows apply their checked fields.
    bool checked = false;
    bool known = false;
    std::string locked;
};

struct SyncFormula
{
    std::string entry;
    std::optional<std::string> current;
    std::optional<std::string> incoming;
    bool known = false;
    bool checked = false;
};

struct ItemSyncDiff
{
    SyncDirection direction = SyncDirection::Import;
    std::vector<SyncItem> items;
    std::vector<SyncFormula> formulas;
    // Import: the languages names are taken in (the client's UI languages).
    std::vector<std::string> takenLocales;

    bool IsEmpty() const
    {
        return items.empty() && formulas.empty();
    }
};

struct SyncOptions
{
    SyncDirection direction = SyncDirection::Import;
    const KnownDifferences* known = nullptr;
    // Import: names in other languages are not taken from a file.
    std::span<const std::string> uiLocales;
    // Import: an item the game holds keeps its size and slot and is not removed.
    std::function<bool(int itemType)> isHeld;
};

// The differences between the client's data (`client`, BuildItemExchange) and `file`. Checked
// by default: what the incoming side changes; unchecked: removals, known differences, inexact
// and locked fields. A language one side lacks stays as it is on the other.
ItemSyncDiff BuildItemSyncDiff(const Data::Items::ItemExchangeFile& client, const Data::Items::ItemExchangeFile& file,
                               const SyncOptions& options);

// The side that changes (the client for an import, the file for an export) with the checked
// rows taken from the other. Formulas the item stat file's checks refuse are left as they were,
// with their problems in `issues`.
Data::Items::ItemExchangeFile ApplyItemSyncDiff(const Data::Items::ItemExchangeFile& client,
                                                const Data::Items::ItemExchangeFile& file, const ItemSyncDiff& diff,
                                                std::vector<Data::Items::ItemDataIssue>& issues);
} // namespace MuEditor::Items

#endif // _EDITOR
