#pragma once

#ifdef _EDITOR

#include "ServerListEdits.h"

#include "../MuEditor/UI/Common/UndoTarget.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <map>
#include <string>
#include <vector>

// The Server List window: the server groups of the login screen
// (Data/Local/ServerList.json), their names, order and channel types. The login
// screen shows each change right away; Save writes the file. The editor bar's
// Undo and Redo act on its changes while it is the tool worked in last.
class CMuServerListUI final : public MuEditor::UndoTarget
{
public:
    // The load of each server the connect server last sent, by group ID and
    // channel number - 1.
    using SentChannels = std::map<int, std::map<int, int>>;

    static CMuServerListUI& GetInstance();

    void Render(bool* open);

    // MuEditor::UndoTarget
    bool IsOpen() const override;
    bool CanUndo() const override;
    bool CanRedo() const override;
    std::string GetUndoLabel() const override;
    std::string GetRedoLabel() const override;
    void Undo() override;
    void Redo() override;

private:
    // Begins the window with the editor's bar; false (and ended) while hidden.
    bool BeginWindow(bool* open);
    // The group list and the selected group; true when the groups changed.
    bool RenderPanes();
    // After the widgets: the login screen shows a change, and a finished edit
    // becomes one undo step.
    void AfterEdits(bool changed);
    // What changed since the last undo step becomes one: before a button, box
    // or list changes the groups, so that what was typed before stays a step
    // of its own.
    void CommitPending();
    // Right after a text field: leaving it ends its edit, which becomes one
    // undo step.
    void CommitWhenFieldLeft();
    void LoadFile();
    void Save();
    // Ctrl+Z and Ctrl+Y (or Ctrl+Shift+Z) while this window has the focus.
    void HandleUndoKeys();
    // After an undo or redo: the selection stays in the list, and the login
    // screen shows the groups.
    void AfterHistoryStep();
    void RenderToolbar();
    void RenderStatus();
    // Each of these that changes the groups returns true.
    bool RenderGroupList();
    bool RenderListButtons();
    bool RenderGroupsNotInList();
    bool RenderSelectedGroup();
    bool RenderGroupId(Data::ServerList::ServerGroup& group);
    bool RenderGroupFields(Data::ServerList::ServerGroup& group);
    bool RenderChannels(Data::ServerList::ServerGroup& group);
    // The game names the servers of the connect server with the groups now.
    void ShowInGame();
    void Select(int index);

    MuEditor::ServerList::ServerListEdits m_edits;
    bool m_loaded = false;
    // The fields changed the groups since the last undo step.
    bool m_uncommitted = false;
    int m_selected = -1;
    // An ID typed for the selected group that another group has, or -1.
    int m_takenId = -1;
    // What the last read or save found, and what it said.
    std::vector<Data::Items::ItemDataIssue> m_issues;
    std::string m_status;
    bool m_statusIsError = false;
    // Found again when the connect server sent a new list
    // (CServerListManager::GetReceivedServersVersion).
    SentChannels m_sent;
    int m_sentVersion = -1;
};

#define g_MuServerListUI CMuServerListUI::GetInstance()

#endif // _EDITOR
