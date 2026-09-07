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
// Server-group buttons split into two dynamic-count arrays (leftGroups/rightGroups, one bound
// field per side) plus one fixed "test server" slot, same shape as CMainFrameWindow's
// skillGridCells/petSkillCells (dynamic-count data-for, click routed by a bound index field rather
// than it_index, since the array is compacted and doesn't line up with CServerGroup::m_iBtnPos
// 1:1). The server list within the selected group behaves like a nested/flyout context menu, not a
// fixed panel region: it's absent until a group button is clicked, then docks beside that specific
// button (see ServerSelRmlModel's own comment for the exact fields).
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
        int btnPos = 0; // CServerGroup::m_iBtnPos -- passed back to RmlClickSelectGroup
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
        bool testServerVisible = false;
        Rml::String testServerLabel;
        bool testServerChecked = false;

        std::vector<GroupEntry> leftGroups;
        std::vector<GroupEntry> rightGroups;
        std::vector<ServerEntry> servers;

        // The server list is a flyout, not a fixed panel region -- it only exists once a group is
        // clicked, docked beside that specific button, same as a nested/flyout context menu (the
        // real legacy behavior; a fixed always-present list area was wrong). serverListTop is a px
        // offset matching the clicked button's row within its column. Exactly one of
        // serverListDockLeft/serverListDockRight is true for a left-/right-column click (flyout
        // docks to the inward side, toward the middle); both false means the rare center/test-
        // server button was clicked, and the flyout centers below the whole group row instead.
        bool serverListVisible = false;
        float serverListTop = 0.f;
        bool serverListDockLeft = false;
        bool serverListDockRight = false;

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
    // (self-centered the same way SceneUICoordinator.cpp centers every other window). Width still
    // matches server_select.rcss's #panel width, but height deliberately does NOT mirror #panel's
    // own (auto) height -- kPanelHeight is generously oversized vertically on purpose, since the
    // server-list flyout can dock as low as the bottom group-button row and still extend a full
    // 16-row server list below it, well past the two columns' own footprint, and this click-gate
    // has to cover that worst case or clicks on a low, tall flyout would incorrectly fall through
    // to whatever's behind this window. #panel itself must NOT share this height, or .center-both
    // ends up centering a box far taller than the real visible content (see server_select.rcss's
    // own comment for the centering bug this caused).
    static constexpr int kPanelWidth = 500;
    static constexpr int kPanelHeight = 750;
};

// Replaces CUIMng's old `CServerSelWin m_ServerSelWin;` member, same convention as g_CreditWin.
extern CServerSelWin g_ServerSelWin;
