#pragma once

#ifdef _EDITOR

#include "Data/GameData/MapData/Warp.h"

#include <cstddef>
#include <string>
#include <vector>

namespace MuEditor::Warps
{
using Data::Maps::Warp;
using Data::Maps::WarpList;

// The warp list the Warp List window changes (Data/Maps/Warps.json): what the
// editor shows, what the file has, and an undo list for this session.
class WarpListEdits
{
public:
    static constexpr size_t MaxUndoSteps = 256;

    // The list of the file, just read or saved: nothing to undo, nothing
    // changed. `complete` is false when the list is not the file's (a read
    // with fatal problems): a save would then replace the file with it.
    void Reset(WarpList list, bool complete = true);

    const WarpList& GetList() const
    {
        return m_list;
    }
    // For the window's fields, which change the list in place; Commit makes
    // what they changed one undo step.
    WarpList& EditList()
    {
        return m_list;
    }
    // What changed since the last step becomes one undo step.
    void Commit();

    bool CanUndo() const
    {
        return !m_undo.empty();
    }
    bool CanRedo() const
    {
        return !m_redo.empty();
    }
    void Undo();
    void Redo();
    // What Undo or Redo would take back or do again ("Remove Icarus",
    // "Icarus: Level"); empty when there is nothing.
    std::string GetUndoLabel() const;
    std::string GetRedoLabel() const;

    bool HasUnsavedChanges() const
    {
        return m_list != m_saved;
    }
    void MarkSaved()
    {
        m_saved = m_list;
    }
    bool IsComplete() const
    {
        return m_complete;
    }

    // A new warp with the smallest free index (named "Warp<index>", also as
    // /move name), at `position` (the end when past it). Returns its position.
    size_t AddWarp(size_t position);
    // A copy of the warp with the smallest free index, its /move name and names
    // with that index added (Arena7), right after it. Returns its position.
    size_t CopyWarp(size_t position);
    void RemoveWarp(size_t position);
    // Moves the warp one place up (-1) or down (+1); false at the end of the list.
    bool MoveWarp(size_t position, int direction);

    bool IsIndexUsed(int index) const;
    // A warp has the text as its /move name or as a name in any language, in any case.
    bool IsNameUsed(const std::string& text) const;
    // The smallest index from 1 on that no warp has.
    int FindFreeIndex() const;

private:
    struct Step
    {
        WarpList list;
        std::string label;
    };

    std::string GetPendingLabel() const;

    WarpList m_list;
    // The list as the last undo step left it.
    WarpList m_committed;
    WarpList m_saved;
    std::vector<Step> m_undo;
    std::vector<Step> m_redo;
    // Set by Add, Copy, Remove and Move until the next Commit; a change of
    // fields is described by comparing the lists instead.
    std::string m_pendingLabel;
    bool m_complete = true;
};

// The name a step gives a warp: its English name, or "#<index>" without one.
std::string GetStepName(const Warp& warp);

// A change of fields: the first warp whose fields differ and its first field
// ("Icarus: Level"), or the class level discount.
std::string DescribeFieldChange(const WarpList& before, const WarpList& after);
} // namespace MuEditor::Warps

#endif // _EDITOR
