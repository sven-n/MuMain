//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <vector>

namespace Rml { class ElementDocument; }

class CServerGroup;

// Pure RmlUi 2D UI, the CMsgWin reference shape (no live-3D content, unlike CCharMakeWin) --
// RmlUi owns 100% of this window's rendering and click handling. No panel background/frame --
// the real legacy screen is fully transparent, just buttons and text floating over the login art.
//
// Two always-visible columns, laid out entirely by server_select.rcss's own flexbox rules (no
// C++-computed positions): the left column lists every server group (dynamic-count data-for,
// click routed by a bound index field rather than it_index, since the array is compacted and
// doesn't line up with CServerGroup::m_iBtnPos 1:1), the right column lists the individual servers
// within whichever group is currently selected -- empty/hidden until a group is clicked (see
// ServerSelRmlModel's own comment). Previously the group buttons were split into two fixed-count
// columns with the server list as a flyout that C++ docked beside whichever button was clicked
// (kGroupBtnRowHeight and friends); that positioning math is gone now that both lists are normal
// RCSS flow instead of absolutely-positioned/C++-pushed boxes.
//
// UpdateDisplay() is not one-shot: it re-runs every time the server-list packet arrives, every
// time a group button is clicked (re-requests the list), and after a resolution change
// (RepositionSceneUI()) -- the model is rebuilt and re-marked dirty each time, matching
// CBuffStrip's "resize + unconditional MarkDirty" convention for a runtime-variable-length array.
//
// Self-centers via base.rcss's .center-both (same convention as CMsgWin/CSysMenuWin) -- no
// SetPosition()/GetWidth()/GetHeight() contract for callers to push/read. UpdateMouseEvent() still
// claims only its own rect (not a full-screen swallow -- CCreditWin's absolute depth-100 override
// handles full-screen claims separately, and clicks outside this panel must fall through to
// whatever's behind it), computed from this window's own authored footprint centered the same way
// SceneUICoordinator.cpp centers every other window.
class CServerSelWin : public mu::ui::window::CObject
{
public:
    CServerSelWin();
    ~CServerSelWin() override;

    void Create();
    void Release();
    void UpdateDisplay();
    void Show(bool bShow) override;

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Same
    // immediate-call convention as every other migrated window's RmlClickX() (see
    // CLoginMainWin::RmlClickMenu()'s header comment for why this is safe to call straight into
    // the action instead of polling a flag next frame).
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
    // Below CCreditWin's 100.0f (its own absolute mouse-claim override, unaffected either way) and
    // CSysMenuWin's 40.0f (the two can be visible simultaneously; CSysMenuWin just wins click
    // priority) -- unchanged from the legacy value, no coexistence behavior to preserve differently
    // here.
    float GetLayerDepth() override
    {
        return 20.0f;
    }

private:
    void SelectGroup(int nBtnPos);
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
        // Discrete bool-per-color flags matching the legacy adwServerBtnClr[byNonPvP] mapping --
        // RmlUi's data-class-X binds one class per boolean, not a class name, so this can't be a
        // single string field. byNonPvP==1 is the implicit default (neither flag set):
        // server_select.rcss's plain .server-row color applies.
        bool colorGray = false;
        bool colorOrange = false;
        // No "checked"/selected state here -- clicking a server row connects immediately.
    };
    struct ServerSelRmlModel
    {
        // One merged, order-preserving list -- the old leftGroups/rightGroups split mirrored a
        // legacy two-column layout that no longer exists; CServerGroup::SBP_LEFT/SBP_RIGHT/
        // SBP_CENTER now only matter for the one real business rule they carry (dedupe multiple
        // SBP_CENTER "test server" groups down to the first, see UpdateDisplay()), not for which
        // UI column an entry lands in.
        std::vector<GroupEntry> groups;
        std::vector<ServerEntry> servers;

        // True once a group has been clicked and its server list requested/populated -- the right
        // column is empty/hidden before that, same as the old flyout being entirely absent.
        bool serverListVisible = false;

        bool pvpNotice = false;
        Rml::String pvpNoticeLine0, pvpNoticeLine1, pvpNoticeLine2;

        Rml::String descriptionText;
    };
    RmlModelBinder<ServerSelRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    int m_iSelectServerBtnIndex = -1;
    CServerGroup* m_pSelectServerGroup = nullptr;

    // Fixed, author-chosen footprint (this is a fresh RmlUi layout, not a port of the legacy
    // asset-derived geometry -- see ui-target-architecture.md's note on why CWinEx's real pixel
    // size couldn't safely be preserved). Used only for UpdateMouseEvent()'s own-rect check
    // (self-centered the same way SceneUICoordinator.cpp centers every other window). Both the
    // group and server columns now cap their own height with max-height+overflow:auto
    // (server_select.rcss) instead of the old flyout's open-ended worst-case extent, so this
    // click-gate just needs to cover #panel's real max footprint (both columns side by side, plus
    // the pvp notice/description text below) with some slack -- not a separate oversized
    // reservation the way the old flyout-docking design needed.
    static constexpr int kPanelWidth = 420;
    static constexpr int kPanelHeight = 460;
};

// Replaces CUIMng's old `CServerSelWin m_ServerSelWin;` member, same convention as g_CreditWin.
extern CServerSelWin g_ServerSelWin;
