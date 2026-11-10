#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace MuEditor::Items
{
// Which side of a difference between the client and a server is wrong, as Item Sync's "Wrong in
// client" and "Wrong in server" mark it.
enum class WrongSide : uint8_t
{
    None,
    // The file's value is right. The difference starts as without a mark: whether Apply takes it
    // is the user's choice.
    Client,
    // The client's value is right: the diff leaves the difference unchecked.
    Server,
};

// The field of a mark on a new or removed item as a whole.
inline const std::string AddedOrRemovedItemField;
// The field of a mark on a changed item as a whole: every field without a mark of its own, also
// one that differs only later. Kept apart from the new or removed item's mark, which says nothing
// about the values once both sides have the item.
inline const std::string EveryFieldOfItem = "*";

// The items' fields and formula entries whose wrong side was marked
// (Data/Items/Sync/KnownDifferences.json), so every comparison starts with that choice.
struct KnownDifferences
{
    // (group, number, field); AddedOrRemovedItemField and EveryFieldOfItem name the item as a
    // whole.
    std::map<std::tuple<int, int, std::string>, WrongSide> fields;
    std::map<std::string, WrongSide, std::less<>> formulas;

    WrongSide Find(int group, int number, const std::string& field) const;
    WrongSide Find(const std::string& formula) const;
    // WrongSide::None takes the entry out.
    void Set(int group, int number, const std::string& field, WrongSide side);
    void Set(const std::string& formula, WrongSide side);
    // Takes out the marks of the item while it is changed: EveryFieldOfItem and its fields'. The
    // mark of the item new or removed stays. Whether any went.
    bool ClearChangedItem(int group, int number);
    bool operator==(const KnownDifferences& other) const = default;
};

std::filesystem::path GetKnownDifferencesFile();
// Nothing marked when the file is not there. A file of format version 1 (a list of "known"
// differences) reads as wrong in the server: the client kept its value.
KnownDifferences LoadKnownDifferences(const std::filesystem::path& file,
                                      std::vector<Data::Items::ItemDataIssue>& issues);
std::string WriteKnownDifferencesJson(const KnownDifferences& known);
} // namespace MuEditor::Items

#endif // _EDITOR
