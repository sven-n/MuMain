#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
// Data/Items/Tooltips/NameCases.json: the grammatical cases each language declines item names in, so a text of that
// language can name an item in the form its sentence needs ({item:14,25:acc}; docs/item-data.md, "Name forms"). The
// items' forms are in their "nameForms"; a language without cases here has none.
//
//   {"formatVersion": 1, "languages": {"de": ["gen", "dat", "acc"], "pl": ["gen", "dat", "acc", "ins", "loc"]}}
class ItemNameCases
{
public:
    // The case keys of the language in the order of Data::Translation::ItemCaseKeys; empty when it declines none.
    const std::vector<std::string>& GetCases(std::string_view locale) const;
    bool Uses(std::string_view locale, std::string_view caseKey) const;

    // Sets the language's cases (put in the case key order); none removes the language.
    void SetCases(std::string_view locale, std::vector<std::string> cases);

    const std::map<std::string, std::vector<std::string>, std::less<>>& GetLanguages() const
    {
        return m_languages;
    }

    bool operator==(const ItemNameCases& other) const = default;

private:
    std::map<std::string, std::vector<std::string>, std::less<>> m_languages;
};

inline constexpr const char* ItemNameCasesFileName = "NameCases.json";

void ReadItemNameCasesJson(std::string_view text, const std::string& source, ItemNameCases& cases,
                           std::vector<ItemDataIssue>& issues);
std::string WriteItemNameCasesJson(const ItemNameCases& cases);
} // namespace Data::Items
