#pragma once

#ifdef _EDITOR

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace MuEditor::Colors
{
// The texts a tab shows for its entries. They are read each time the tab is drawn, so they follow the editor language.
struct NamedColorTexts
{
    // The button that opens the dialog for a new entry.
    const char* newEntry = "";
    // The grey line under it: what the entries are for.
    const char* note = "";
    // Shown while no entry is picked.
    const char* pickAnEntry = "";
    // The button that applies a changed value.
    const char* apply = "";
    // The title of the list of the items a change reaches.
    const char* change = "";
    // The undo steps of Remove and of Remove them.
    const char* removeOne = "";
    const char* removeUnused = "";
    // "No item uses these ...: {0}", {0} being the names.
    const char* unused = "";
    // Why an entry that stays can't be removed.
    const char* kept = "";
};

// What a tab of the Colors window lists (D69): its named entries, the items that use each, and what a change of one
// does. NamedColorList draws it. Each tab keeps its own entries and values, because they differ: a glow tints a model,
// a label style is a text look.
class NamedColorSource
{
public:
    virtual ~NamedColorSource() = default;

    virtual NamedColorTexts GetTexts() const = 0;

    // The entries, in list order.
    virtual std::size_t GetCount() const = 0;
    virtual const std::string& GetName(std::size_t index) const = 0;
    virtual bool Has(const std::string& name) const = 0;
    // The look of the entry at `index`, in front of its name in the list.
    virtual void RenderSwatch(std::size_t index) const = 0;

    virtual int CountUsers(const std::string& name) = 0;
    virtual std::vector<int> FindUsers(const std::string& name) const = 0;
    // An entry that stays although no item uses it, such as a default glow color.
    virtual bool IsKept(const std::string& name) const = 0;
    // Removes the entries as one undo step named `step`.
    virtual void Remove(const char* step, std::span<const std::string> names) = 0;

    // The dialog for a new entry, opened with the value of `from` ("" while none is picked). RenderNew gives the new
    // entry's name in the frame it is added.
    virtual void OpenNew(const std::string& from) = 0;
    virtual std::optional<std::string> RenderNew() = 0;

    // The fields of the picked entry's value. The edit follows the entry (an undo, a redo) until it is changed.
    virtual void RenderEdit(const std::string& name) = 0;
    // Whether the fields hold a value other than the entry's.
    virtual bool IsEdited(const std::string& name) const = 0;
    virtual void ApplyEdit(const std::string& name) = 0;
    virtual void RevertEdit(const std::string& name) = 0;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
