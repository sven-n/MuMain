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
#include "I18N/All.h"
#include "imgui.h"

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

bool RenderSelectedMonster(int number)
{
    if (number < 0 || number >= Data::Monsters::MonsterNumberCount)
    {
        ImGui::TextDisabled("%s", I18N::Editor::SelectAMonster);
        return false;
    }
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        ImGui::TextDisabled("(%d) %s", number, I18N::Editor::NoMonsterAtThisNumber);
        return false;
    }
    // In the UI language, as the browser shows it. The name is an argument,
    // never the format: names may hold a "%".
    ImGui::Text("%s (%d)", monster->names.Get(I18N::GetCurrentLocale()).c_str(), number);
    return true;
}

int GetDataVersion()
{
    return g_MonsterDatabase.GetVersion();
}
} // namespace MuEditor::Monsters::ToolWindow

#endif // _EDITOR
