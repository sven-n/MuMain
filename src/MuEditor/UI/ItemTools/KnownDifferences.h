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
    // The file's value is right: the diff checks the difference, so Apply takes it.
    Client,
    // The client's value is right: the diff leaves the difference unchecked.
    Server,
};

// The items' fields and formula entries whose wrong side was marked
// (Data/Items/Sync/KnownDifferences.json), so every comparison starts with that choice.
struct KnownDifferences
{
    // (group, number, field); an empty field names the item as a whole: a new or removed one, or
    // every field of a changed one that has no mark of its own.
    std::map<std::tuple<int, int, std::string>, WrongSide> fields;
    std::map<std::string, WrongSide, std::less<>> formulas;

    WrongSide Find(int group, int number, const std::string& field) const;
    WrongSide Find(const std::string& formula) const;
    // WrongSide::None takes the entry out.
    void Set(int group, int number, const std::string& field, WrongSide side);
    void Set(const std::string& formula, WrongSide side);
    // Takes out the item's marks, the whole item's and its fields'.
    void ClearItem(int group, int number);
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
