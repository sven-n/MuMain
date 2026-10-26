#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <string_view>

namespace MuEditor
{
// What the problems of an issue table (IssueTable) are about: items,
// monsters. It names the subject of a problem in the table and selects it.
class IssueSubjects
{
public:
    // The header of the subject column (translated, so read it when drawing)
    // and its width before the editor UI scale.
    virtual const char* GetHeader() const = 0;
    virtual float GetColumnWidth() const = 0;
    // The subject column of the problem's row.
    virtual void RenderCell(const Data::Items::ItemDataIssue& issue) const = 0;
    // The field as the field column shows it.
    virtual std::string_view GetField(const Data::Items::ItemDataIssue& issue) const = 0;
    // Selects what the problem is about. A click (`clicked`) also opens the
    // tool of its field in front; an arrow key keeps the keyboard in the list.
    virtual void Open(const Data::Items::ItemDataIssue& issue, bool clicked) = 0;

protected:
    ~IssueSubjects() = default;
};
} // namespace MuEditor

#endif // _EDITOR
