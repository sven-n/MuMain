#pragma once

#ifdef _EDITOR

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace Data
{
class LocalizedString;
}

namespace MuEditor
{
// What the names grid (NamesGrid) shows and edits: the rows of one kind of
// data (items, monsters), each with an id and its names by language, and the
// selection its tools share.
class NamesGridRows
{
public:
    // The ids of the rows for a search and the language whose missing names
    // are listed (empty for all rows). The list may be kept until the search,
    // the language or the data changes.
    virtual const std::vector<int>& List(std::string_view search, std::string_view missingLocale) = 0;
    // Lists the rows again, also the ones whose name got filled.
    virtual void Refresh() = 0;

    // Changes when the data changes, so the grid counts again.
    virtual int GetVersion() const = 0;
    // For each language, how many rows have no name of their own in it.
    virtual std::vector<int> CountMissing(std::span<const char* const> locales) const = 0;

    // The names of the row; nullptr when it is gone.
    virtual const Data::LocalizedString* FindNames(int id) const = 0;
    // The id as the first column shows it.
    virtual void FormatId(int id, char* buffer, std::size_t size) const = 0;
    // Stores a changed name (an empty text removes a translation), with the
    // data's own checks and undo; the grid calls it only for a text that
    // differs from the row's own name.
    virtual void CommitName(int id, const char* locale, const char* text) = 0;

    // The selection the data's tools share.
    virtual int GetSelected() const = 0;
    virtual int GetSelectionSerial() const = 0;
    virtual void Select(int id) = 0;
    // Brings the data's browser to the front, after a click on an id.
    virtual void ShowBrowser() = 0;

    // The tooltip of an id and the "all rows" entry of the missing filter.
    virtual const char* GetSelectTooltip() const = 0;
    virtual const char* GetAllRowsLabel() const = 0;

protected:
    ~NamesGridRows() = default;
};
} // namespace MuEditor

#endif // _EDITOR
