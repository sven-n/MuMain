//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <vector>

namespace Rml { class ElementDocument; }

class CServerGroup;

// Pure RmlUi 2D UI: RmlUi owns all of this window's rendering and click handling. Two
// always-visible columns laid out by server_select.rcss's flexbox rules -- server groups on the
// left, and the servers within whichever group is selected on the right (empty/hidden until a
// group is clicked). Self-centers via base.rcss's .center-both; UpdateMouseEvent() claims only its
// own rect, computed from this window's authored footprint centered the same way.
class CServerSelWin : public mu::ui::window::CObject
{
public:
    CServerSelWin();
    ~CServerSelWin() override;

    void Create();
    void Release();
    void UpdateDisplay();
    void Show(bool bShow) override;

    void ReloadRmlTheme();

    // Bound to the RmlUi document's data-event-click callbacks; acts immediately.
    void RmlClickSelectGroup(int nBtnPos);
    void RmlClickSelectServer(int nServerIndex);

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CCreditWin/CSysMenuWin: the two can be visible simultaneously, and CSysMenuWin wins click priority.
    float GetLayerDepth() override
    {
        return 20.0f;
    }

private:
    void SelectGroup(int nBtnPos);
    void BuildRmlUi();
    void SyncRmlModel();

    struct GroupEntry
    {
        Rml::String label;
        int btnPos = 0; // CServerGroup::m_iBtnPos -- passed back to RmlClickSelectGroup; 0 is the rare center/"test server" group
        bool checked = false;
    };
    struct ServerEntry
    {
        Rml::String label;
        int index = 0; // position within the selected group's server list (CServerGroup::GetServerInfo(i))
        float loadFraction = 0.f;
        // One bool per color class since RmlUi's data-class-X binds a class per boolean, not a
        // name. Neither flag set is the default (plain .server-row color).
        bool colorGray = false;
        bool colorOrange = false;
        // No "checked"/selected state here -- clicking a server row connects immediately.
    };
    struct ServerSelRmlModel
    {
        // One merged, order-preserving list.
        std::vector<GroupEntry> groups;
        std::vector<ServerEntry> servers;

        // True once a group has been clicked and its server list requested/populated.
        bool serverListVisible = false;

        bool pvpNotice = false;
        Rml::String pvpNoticeLine0, pvpNoticeLine1, pvpNoticeLine2;

        Rml::String descriptionText;
    };
    RmlModelBinder<ServerSelRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    int m_iSelectServerBtnIndex = -1;
    CServerGroup* m_pSelectServerGroup = nullptr;

    // Fixed, author-chosen footprint used only for UpdateMouseEvent()'s own-rect check
    // (self-centered the same way SceneUICoordinator.cpp centers every other window); covers
    // #panel's real max footprint (both columns plus the pvp notice/description text) with slack.
    static constexpr int kPanelWidth = 420;
    static constexpr int kPanelHeight = 460;
};

extern CServerSelWin g_ServerSelWin;
