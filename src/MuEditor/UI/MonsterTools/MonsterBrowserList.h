#pragma once

#ifdef _EDITOR

#include "MonsterTreeModel.h"

#include "Data/GameData/MapData/MapRef.h"
#include "Data/GameData/MonsterData/MonsterKind.h"

#include <map>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The rows of the Monster Browser (D61): number, name in the editor language,
// kind and maps of each listed monster and number, the selected one
// highlighted. A monster with several numbers opens to them; its row's kind
// is the one its numbers share. A number's shared kind and name are grey, its
// own yellow (Q-MN50); a right click opens the row's actions.
class MonsterBrowserList
{
public:
    // What a row's context menu asked for.
    enum class Action
    {
        None,
        AddNumber,
        CopyMonster,
        CopyNumber,
        MoveNumber,
        SplitNumber,
        RemoveNumber,
        RemoveMonster,
    };

    struct Result
    {
        // The number of the row that was clicked (or moved onto with the arrow
        // keys), or -1; whether it is a monster's row.
        int clicked = -1;
        bool clickedMonster = false;
        // The row whose kind combo picked another kind, or -1, and that kind.
        int kindChanged = -1;
        bool kindOfMonster = false;
        Data::Monsters::MonsterKind kind = Data::Monsters::MonsterKind::Monster;
        // The monster row opened or closed, or -1.
        int toggled = -1;
        bool open = false;
        // The action a context menu picked, for the number of its row.
        Action action = Action::None;
        int actionNumber = -1;
        // The number whose maps the map picker changed, or -1, and its maps.
        int mapsChanged = -1;
        std::vector<Data::Maps::MapRef> maps;
    };

    // Draws the rows that are in view. `reveal` scrolls to the selected row
    // once (it was selected elsewhere).
    Result Render(const std::vector<MonsterTreeModel::Row>& rows, int selectedNumber, bool selectedWhole, bool reveal);

private:
    // The texts of a monster row that are made, not read: its label ("3
    // numbers") and the maps of all its numbers. Kept, not made each frame,
    // until the data or the language changes.
    struct MonsterRowTexts
    {
        std::string label;
        std::string maps;
    };

    // Drops the kept texts when the data or the language changed.
    void ForgetChangedTexts();
    const MonsterRowTexts& GetMonsterRowTexts(const MonsterTreeModel::Row& row);

    // By the monster row's first number.
    std::map<int, MonsterRowTexts> m_monsterRowTexts;
    int m_textsVersion = -1;
    std::string m_textsLocale;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
