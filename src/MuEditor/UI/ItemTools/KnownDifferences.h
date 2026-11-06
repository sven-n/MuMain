#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <filesystem>
#include <functional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace MuEditor::Items
{
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
} // namespace MuEditor::Items

#endif // _EDITOR
