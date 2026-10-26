#pragma once

#ifdef _EDITOR

#include <string>

namespace MuEditor
{
// A tool with an undo history of its own (the item tools, the monster tools,
// the Server List window). The editor bar's Undo and Redo act on the tool the
// user worked in last.
class UndoTarget
{
public:
    // Whether the tool's window is open: a closed tool leaves the bar to the
    // fallback (the item tools).
    virtual bool IsOpen() const = 0;
    virtual bool CanUndo() const = 0;
    virtual bool CanRedo() const = 0;
    // What the next Undo or Redo would do, for the buttons' tooltips.
    virtual std::string GetUndoLabel() const = 0;
    virtual std::string GetRedoLabel() const = 0;
    virtual void Undo() = 0;
    virtual void Redo() = 0;

protected:
    ~UndoTarget() = default;
};

namespace UndoTargets
{
// A tool window calls this while it has the focus: the bar's Undo and Redo then
// act on its tool, also after the focus went elsewhere.
void Activate(UndoTarget& target);

// The editor bar's Undo and Redo, for the tool worked in last, or for
// `fallback` before any was or while that tool is closed. A click does not take
// the focus from the tool.
void RenderBarButtons(UndoTarget& fallback);
} // namespace UndoTargets
} // namespace MuEditor

#endif // _EDITOR
