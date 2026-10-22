#pragma once

#ifdef _EDITOR

#include "ServerListEdits.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <map>
#include <string>
#include <vector>

// The Server List window: the server groups of the login screen
// (Data/Local/ServerList.json), their names, order and channel types. The login
// screen shows each change right away; Save writes the file.
class CMuServerListUI
{
public:
    // The load of each server the connect server last sent, by group ID and
    // channel number - 1.
    using SentChannels = std::map<int, std::map<int, int>>;

    static CMuServerListUI& GetInstance();

    void Render(bool* open);

private:
    void LoadFile();
    void Save();
    // Each Render* that changes the groups returns true.
    bool RenderToolbar();
    void RenderStatus();
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
    // Found again each frame the window is drawn.
    SentChannels m_sent;
};

#define g_MuServerListUI CMuServerListUI::GetInstance()

#endif // _EDITOR
