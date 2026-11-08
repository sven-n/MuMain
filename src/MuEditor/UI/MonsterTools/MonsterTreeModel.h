#pragma once

#ifdef _EDITOR

#include "MonsterListModel.h"

#include <set>
#include <vector>

namespace MuEditor::Monsters
{
// The rows of the Monster Browser (D61): the monsters in the order of their
// first number; a monster with several numbers is a row that opens to its
// numbers. A filter lists the monsters with a number that passes it, opened
// to those numbers. Kept apart from ImGui so it can be tested.
class MonsterTreeModel
{
public:
    struct Row
    {
        // The number of a number row or of a monster with one number; the
        // first number for the row of a monster with several.
        int number = 0;
        enum class Kind
        {
            // A monster with one number.
            Single,
            // A monster with several numbers, as a whole.
            Monster,
            // A number of such a monster.
            Number,
        } kind = Kind::Single;
        // How many numbers the monster has.
        int numberCount = 1;
        // A monster row's numbers are listed below it.
        bool open = false;

        bool operator==(const Row&) const = default;
    };

    // The rows; built again when the filter, the monster table, the monsters
    // or an open monster changed.
    const std::vector<Row>& GetRows(const MonsterListModel::Filter& filter);

    // Whether the row shows the selection (MonsterSelection): a monster as a
    // whole on its monster's row, a number on its own row, a monster with one
    // number on its row either way. A monster's row does not show its first
    // number.
    static bool ShowsSelection(const Row& row, int number, bool wholeMonster);

    // Opens or closes the monster of the number.
    void SetOpen(int number, bool open);
    bool IsOpen(int number) const;

    // After an edit made through the list: the rows stay as they are until
    // the filter changes, a monster is opened or closed, or Refresh.
    void KeepRowsAfterOwnEdit();
    void Refresh();

private:
    std::vector<Row> m_rows;
    MonsterListModel::Filter m_filter;
    // The open monsters, by their id (MonsterDataHandler::FindGroupOf), which
    // stays while numbers move in and out.
    std::set<int> m_open;
    int m_version = -1;
    int m_groupsVersion = -1;
    bool m_built = false;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
