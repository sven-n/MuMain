#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterToolWindow.h"

#include "MonsterEdits.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/ToolGroups.h"
#include "../MuEditor/UI/Common/ToolWindowFrame.h"
#include "../MuEditor/UI/Common/UndoTarget.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "I18N/All.h"

#include <string>

namespace MuEditor::Monsters::ToolWindow
{
namespace
{
void LogRefusal()
{
    if (g_MonsterEdits.GetHistory().GetLastRefusal() == MonsterEditHistory::Refusal::MonstersChanged)
    {
        g_MuEditorConsoleUI.LogEditor(I18N::Editor::TheMonstersChangedSinceThisStepItIsNotUndone);
    }
}

// The monster edit history, for the editor bar's Undo and Redo.
class MonsterUndoTarget final : public UndoTarget
{
public:
    // While no monster tool is open, the bar is the item tools' again.
    bool IsOpen() const override
    {
        return GetMonsterTools().IsAnyOpen(g_MuEditorCore.GetToolWindows());
    }
    bool CanUndo() const override
    {
        return g_MonsterEdits.GetHistory().CanUndo();
    }
    bool CanRedo() const override
    {
        return g_MonsterEdits.GetHistory().CanRedo();
    }
    std::string GetUndoLabel() const override
    {
        return g_MonsterEdits.GetHistory().GetUndoLabel();
    }
    std::string GetRedoLabel() const override
    {
        return g_MonsterEdits.GetHistory().GetRedoLabel();
    }
    void Undo() override
    {
        if (CanUndo() && !g_MonsterEdits.Undo())
        {
            LogRefusal();
        }
    }
    void Redo() override
    {
        if (CanRedo() && !g_MonsterEdits.Redo())
        {
            LogRefusal();
        }
    }
};
} // namespace

bool Begin(const char* title, const char* windowId, bool* open)
{
    return ToolWindowFrame::Begin(title, windowId, open, GetMonsterTools(), GetUndoTarget());
}

void End()
{
    ToolWindowFrame::End();
}

UndoTarget& GetUndoTarget()
{
    static MonsterUndoTarget target;
    return target;
}

int GetDataVersion()
{
    return g_MonsterDatabase.GetVersion() + g_MonsterModelDatabase.GetVersion();
}
} // namespace MuEditor::Monsters::ToolWindow

#endif // _EDITOR
