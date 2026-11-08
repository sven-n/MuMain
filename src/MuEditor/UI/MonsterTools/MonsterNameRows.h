#pragma once

#ifdef _EDITOR

#include "MonsterListModel.h"

#include "../MuEditor/UI/Common/NamesGridRows.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <map>
#include <vector>

namespace MuEditor::Monsters
{
// The monsters as rows of the names grid: the monster numbers with an entry,
// their names and the monster tools' selection; a name edit goes through the
// monster tools' history and reaches the monsters in the game at once. A
// monster with several numbers is one row for the names they share (its
// first number plus ShownValues::WholeMonsterKey), with a row for each of its
// numbers with names of their own or made their own (D61, Q-MN50, Q-MN96).
class MonsterNameRows final : public NamesGridRows
{
public:
    const std::vector<int>& List(std::string_view search, std::string_view missingLocale) override;
    void Refresh() override;
    int GetVersion() const override;
    std::vector<int> CountMissing(std::span<const char* const> locales) const override;
    const Data::LocalizedString* FindNames(int id) const override;
    void FormatId(int id, char* buffer, std::size_t size) const override;
    void CommitName(int id, const char* locale, const char* text) override;
    int GetSelected() const override;
    int GetSelectionSerial() const override;
    void Select(int id) override;
    void ShowBrowser() override;
    const char* GetSelectTooltip() const override;
    const char* GetAllRowsLabel() const override;

private:
    struct Row
    {
        int id = 0;
        // The names and number the filter reads.
        Data::Monsters::MonsterDefinition values;
        // The numbers a search finds the row by: all of a monster's for its
        // row.
        std::vector<int> numbers;
    };

    // Every row, in the order of the monsters' first numbers.
    std::vector<Row> CollectRows() const;
    // Whether the row passes m_filter.
    bool Lists(const Row& row) const;

    // Assigned in place, so the strings keep their memory from frame to frame.
    MonsterListModel::Filter m_filter;
    MonsterListModel::Filter m_builtFilter;
    std::vector<int> m_rows;
    int m_builtVersion = -1;
    bool m_built = false;
    // The shared names of the monster rows, by id, which FindNames points to.
    mutable std::map<int, Data::LocalizedString> m_sharedNames;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
