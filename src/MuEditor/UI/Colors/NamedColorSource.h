#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/NamedTreeRoot.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace MuEditor
{
class UndoTarget;
}

namespace MuEditor::Colors
{
// The texts a root shows for its entries. They are read each time the window is drawn, so they follow the editor
// language.
struct NamedColorTexts
{
    // The root's name in the tree.
    const char* root = "";
    // The grey line of the root: what the entries are for.
    const char* note = "";
    // The undo steps of Remove and of Remove them.
    const char* removeOne = "";
    const char* removeUnused = "";
    // The button that applies a changed value.
    const char* apply = "";
    // The title of the list of the users a change reaches.
    const char* change = "";
    // "Used by {0} items", and "The change reaches these {0} items:", {0} being the count.
    const char* usedBy = "";
    const char* changeReaches = "";
    // "No item uses these ...: {0}", {0} being the names.
    const char* unused = "";
    // Why an entry that stays can't be removed.
    const char* kept = "";
};

// Who uses an entry: an item, a rule, a part of the load. The panel lists them by `label`; a click runs `select`.
struct NamedColorUser
{
    std::string label;
    std::function<void()> select;
    bool selected = false;
};

// What a root of the Colors window lists (D69): its named entries and the folders they are sorted in (NamedTreeRoot),
// who uses each entry, and what a change of one does. The window draws the tree (NamedTree), NamedColorList the picked
// entry. Each root keeps its own entries and values, because they differ: a glow tints a model, a label style is a
// text look, a server list color has a shade per theme.
class NamedColorSource : public NamedTreeRoot
{
public:
    virtual NamedColorTexts GetTexts() const = 0;

    const char* GetRootName() const final
    {
        return GetTexts().root;
    }
    // The look of the entry, in front of its name in the tree.
    bool HasSwatches() const final
    {
        return true;
    }
    void RenderSwatch(const std::string& name) const override = 0;
    // "gold (3)": the name and how many use it.
    std::string GetRowLabel(const std::string& name) final;

    virtual int CountUsers(const std::string& name) = 0;
    virtual std::vector<NamedColorUser> FindUsers(const std::string& name) const = 0;
    // An entry that stays although nothing uses it, such as a default glow color.
    virtual bool IsKept(const std::string& name) const = 0;
    // Removes the entries as one undo step named `step`.
    virtual void Remove(const char* step, std::span<const std::string> names) = 0;

    // The dialog for a new entry, opened with the value of `from` ("" while none is picked). RenderNew gives the new
    // entry's name in the frame it is added; it goes into the folder at `folder` (the empty path: under the root) in
    // the same undo step.
    virtual void OpenNew(const std::string& from) = 0;
    virtual std::optional<std::string> RenderNew(const Data::Colors::ColorFolderPath& folder) = 0;

    // The fields of the picked entry's value. The edit follows the entry (an undo, a redo) until it is changed.
    virtual void RenderEdit(const std::string& name) = 0;
    // Whether the fields hold a value other than the entry's.
    virtual bool IsEdited(const std::string& name) const = 0;
    virtual void ApplyEdit(const std::string& name) = 0;
    virtual void RevertEdit(const std::string& name) = 0;
    // The name the entry has after ApplyEdit; another one when the edit renamed it.
    virtual std::string GetNameAfterEdit(const std::string& name) const
    {
        return name;
    }

    // The undo history the root's changes go to, for the window's Undo and Redo.
    virtual MuEditor::UndoTarget& GetUndoTarget() = 0;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
