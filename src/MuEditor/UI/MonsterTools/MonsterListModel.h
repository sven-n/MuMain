#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Monsters
{
// The monsters the monster tools list: every number with an entry, in number
// order, narrowed by a search and filters. Kept apart from ImGui so it can be
// tested; the Monster Browser and Monster Translations use it.
class MonsterListModel
{
public:
    struct Filter
    {
        // The monster number, or a part of a name in any language; letters A
        // to Z ignore case.
        std::string search;
        // Only the monsters without a name of their own in this language;
        // empty for all monsters.
        std::string missingLocale;
        // Only the monsters of this kind.
        std::optional<Data::Monsters::MonsterKind> kind;

        bool operator==(const Filter& other) const = default;
    };

    // The monster numbers; built again when the filter or the monster table
    // changed, or after Refresh.
    const std::vector<int>& GetNumbers(const Filter& filter);
    // After an edit made through the list: the rows stay as they are until the
    // filter changes or Refresh, so a filled name or a changed kind does not
    // take its row away while the list is being worked through.
    void KeepRowsAfterOwnEdit();
    void Refresh();

private:
    std::vector<int> m_numbers;
    Filter m_filter;
    int m_version = -1;
    bool m_built = false;
};

// Whether the monster matches the search text (see MonsterListModel::Filter):
// the whole text is its number, or a name of it contains the text.
bool MatchesMonsterSearch(const Data::Monsters::MonsterDefinition& monster, std::string_view search);

// Whether a new monster can take the number: 0-1023 and no entry, also not
// one with an empty name (252).
bool IsFreeMonsterNumber(int number);
// The free numbers, in order.
std::vector<int> GetFreeMonsterNumbers();
// The first free number from `from` on, else the first free number; 0 when
// none is free.
int GetNextFreeMonsterNumber(int from);
} // namespace MuEditor::Monsters

#endif // _EDITOR
