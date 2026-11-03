#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <string_view>
#include <vector>

namespace MuEditor::DataChecks
{
// One area of game data in the Data Checks window (the items, the monsters,
// the server list, ...): its checks, and what its problems are about.
class DataCheckArea
{
public:
    virtual ~DataCheckArea() = default;

    // The area's name in its filter box and in the Area column (translated,
    // so read it when drawing).
    virtual const char* GetName() const = 0;
    // Appends the problems the area has now.
    virtual void Check(std::vector<Data::Items::ItemDataIssue>& issues) = 0;
    // The What column: the item, monster or file a problem is about.
    virtual void RenderSubject(const Data::Items::ItemDataIssue& issue) const = 0;
    // The field as the Field column shows it: the problem's own field, unless
    // an area shows it shorter (the monsters leave out the monster).
    virtual std::string_view GetField(const Data::Items::ItemDataIssue& issue) const
    {
        return issue.field;
    }
    // Selects what the problem is about. A click (`clicked`) also opens the
    // tool of its field in front; an arrow key keeps the keyboard in the list.
    virtual void Open(const Data::Items::ItemDataIssue& issue, bool clicked) = 0;
};
} // namespace MuEditor::DataChecks

#endif // _EDITOR
