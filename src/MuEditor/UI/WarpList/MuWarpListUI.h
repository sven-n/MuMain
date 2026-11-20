#pragma once

#ifdef _EDITOR

#include "WarpListEdits.h"
#include "WarpNameRows.h"

#include "../MuEditor/UI/Common/NamesGrid.h"
#include "../MuEditor/UI/Common/UndoTarget.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <string>
#include <string_view>
#include <vector>

// The Warp List window: the warps of the M window (Data/Maps/Warps.json) in
// its order, their English name, /move name, level, zen and move rules, and the
// class level discount of all warps; the Translations tab has the names of all
// languages. The game takes each change at once (the M
// window, /move); Save writes the file. The editor bar's Undo and Redo act on
// its changes while it is the tool worked in last. See docs/warp-list.md.
class CMuWarpListUI final : public MuEditor::UndoTarget
{
public:
    static CMuWarpListUI& GetInstance();

    void Render(bool* open);

    // MuEditor::UndoTarget
    bool IsOpen() const override;
    bool CanUndo() const override;
    bool CanRedo() const override;
    std::string GetUndoLabel() const override;
    std::string GetRedoLabel() const override;
    void Undo() override;
    void Redo() override;

    const MuEditor::Warps::WarpListEdits& GetEdits() const
    {
        return m_edits;
    }
    // The position of the warp shown on the right, or -1.
    int GetSelected() const
    {
        return m_selected;
    }
    // The index of that warp, or -1.
    int GetSelectedIndex() const;
    // Changes when another warp is picked.
    int GetSelectionSerial() const
    {
        return m_selectionSerial;
    }
    // Picks the warp with the index; nothing when no warp has it.
    void SelectWarpIndex(int index);
    // Brings the Warps tab to the front in the next frame.
    void ShowWarpsTab()
    {
        m_showWarpsTab = true;
    }
    // Changes whenever the list changes.
    int GetVersion() const
    {
        return m_version;
    }
    // A name of the warp with the index as one undo step, which the game takes
    // at once; false when no warp has the index or the name is the same.
    bool SetWarpName(int index, std::string_view locale, std::string name);

private:
    // Begins the window with the editor's bar; false (and ended) while hidden.
    bool BeginWindow(bool* open);
    // Each of these that changes the list returns true.
    bool RenderToolbar();
    void RenderStatus();
    // The warps with the picked one, and the class level discount below.
    bool RenderWarpsTab();
    bool RenderWarpList();
    bool RenderSelectedWarp();
    bool RenderIndex(Data::Maps::Warp& warp);
    bool RenderNames(Data::Maps::Warp& warp);
    bool RenderValues(Data::Maps::Warp& warp);
    bool RenderRules(Data::Maps::Warp& warp);
    // One rule list: its entries, each with its kind, item or tag (and the
    // "except" tag with `withExcept`), and Add.
    bool RenderItemMatches(const char* id, std::vector<Data::Maps::WarpItemMatch>& matches, bool withExcept);
    bool RenderBlockedMessage(Data::Maps::Warp& warp);
    bool RenderClassLevelDiscount();
    // A whole number typed into a field, kept within low..high; true when it changed.
    bool InputNumber(const char* label, int& value, int low, int high);
    // A box; what was typed before is a step of its own.
    bool Checkbox(const char* label, bool& value);
    // After the widgets: the game takes a change, and a finished edit becomes
    // one undo step.
    void AfterEdits(bool changed);
    // What changed since the last undo step becomes one: before a button or
    // box changes the list, so that what was typed before stays a step of its
    // own.
    void CommitPending();
    // Right after a text field: leaving it ends its edit, which becomes one
    // undo step.
    void CommitWhenFieldLeft();
    void LoadFile();
    void Save();
    void HandleUndoKeys();
    void AfterHistoryStep();
    // The game's warp list (g_WarpDatabase, the M window's list) is the list.
    void ShowInGame();
    void Select(int position);

    MuEditor::Warps::WarpListEdits m_edits;
    MuEditor::Warps::WarpNameRows m_nameRows;
    MuEditor::NamesGrid m_namesGrid;
    int m_version = 0;
    int m_selectionSerial = 0;
    bool m_showWarpsTab = false;
    bool m_loaded = false;
    bool m_uncommitted = false;
    int m_selected = -1;
    // An index typed for the selected warp that another warp has, or -1.
    int m_takenIndex = -1;
    std::string m_search;
    // The height of the class level discount section last frame, which the
    // panes above leave free.
    float m_bottomHeight = 0.0f;
    // What the last read or save found, and what it said.
    std::vector<Data::Items::ItemDataIssue> m_issues;
    std::string m_status;
    bool m_statusIsError = false;
};

#define g_MuWarpListUI CMuWarpListUI::GetInstance()

#endif // _EDITOR
