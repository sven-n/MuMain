
#include "stdafx.h"
#include "ServerSelWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

#include "Network/Server/ServerListManager.h"
#include "Core/Globals/_enum.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>

namespace
{
    // Mirrors server_select.rcss's .group-btn height (32px) + margin-bottom (4px) -- used to align
    // the server-list flyout's top with whichever group button row was clicked.
    constexpr float kGroupBtnRowHeight = 36.0f;
    // Mirrors #group_row's own padding-top (reserves room for the centered test-server button
    // above the two columns) -- row 0's actual on-screen top, in #panel's coordinate space.
    constexpr float kGroupRowTopPadding = 34.0f;
    // Mirrors #group_row's own fixed height -- a worst-case reservation for 10 group buttons per
    // column, not the real button count. server_select.rcss vertically centers each column's real
    // buttons within this space (justify-content:center) rather than packing them to the top, so
    // the flyout's top must add back that same centering offset below, or it docks beside where a
    // button would sit in a full column instead of where the actual (usually shorter) column put it.
    constexpr float kGroupColumnReservedHeight = 400.0f;
    // Fixed top for the rare center/test-server-button flyout, below the whole group row (mirrors
    // #group_row's authored height + a small gap). CSS can't override this per case since
    // data-style-top sets an inline style (always wins over a class rule), so every case --
    // including this one -- is resolved to a concrete value here, not left partly to RCSS.
    constexpr float kCenterFlyoutTop = 410.0f;
}

CServerSelWin g_ServerSelWin;

CServerSelWin::CServerSelWin()
{
}

CServerSelWin::~CServerSelWin()
{
    Release();
}

void CServerSelWin::Create()
{
    Release();

    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "server_select",
            [this](Rml::DataModelConstructor& c, ServerSelRmlModel& model)
            {
                auto group = c.RegisterStruct<GroupEntry>();
                group.RegisterMember("label", &GroupEntry::label);
                group.RegisterMember("btn_pos", &GroupEntry::btnPos);
                group.RegisterMember("checked", &GroupEntry::checked);
                c.RegisterArray<std::vector<GroupEntry>>();

                auto server = c.RegisterStruct<ServerEntry>();
                server.RegisterMember("label", &ServerEntry::label);
                server.RegisterMember("index", &ServerEntry::index);
                server.RegisterMember("load_fraction", &ServerEntry::loadFraction);
                server.RegisterMember("color_gray", &ServerEntry::colorGray);
                server.RegisterMember("color_orange", &ServerEntry::colorOrange);
                c.RegisterArray<std::vector<ServerEntry>>();

                c.Bind("test_server_visible", &model.testServerVisible);
                c.Bind("test_server_label", &model.testServerLabel);
                c.Bind("test_server_checked", &model.testServerChecked);

                c.Bind("left_groups", &model.leftGroups);
                c.Bind("right_groups", &model.rightGroups);
                c.Bind("servers", &model.servers);

                c.Bind("server_list_visible", &model.serverListVisible);
                c.Bind("server_list_top", &model.serverListTop);
                c.Bind("server_list_dock_left", &model.serverListDockLeft);
                c.Bind("server_list_dock_right", &model.serverListDockRight);

                c.Bind("pvp_notice", &model.pvpNotice);
                c.Bind("pvp_notice_line0", &model.pvpNoticeLine0);
                c.Bind("pvp_notice_line1", &model.pvpNoticeLine1);
                c.Bind("pvp_notice_line2", &model.pvpNoticeLine2);

                c.Bind("description_text", &model.descriptionText);

                c.BindEventCallback("serversel_select_group",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectGroup(arguments[0].Get<int>(-1));
                    });
                c.BindEventCallback("serversel_select_server",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectServer(arguments[0].Get<int>(-1));
                    });
            });

        if (modelCreated)
        {
            m_RmlBinder.GetModel().pvpNoticeLine0 = StringUtils::WideToNarrow(I18N::Game::SinceHelheimServer);
            m_RmlBinder.GetModel().pvpNoticeLine1 = StringUtils::WideToNarrow(I18N::Game::TendsToBeCrowded);
            m_RmlBinder.GetModel().pvpNoticeLine2 = StringUtils::WideToNarrow(I18N::Game::WeRecommendThatYouUseOtherServers);

            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/server_select.rml");
        }
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SERVER_SELECT, this);

    Show(false);

    UpdateDisplay();
}

void CServerSelWin::Release()
{
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CServerSelWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CServerSelWin::UpdateDisplay()
{
    auto& model = m_RmlBinder.GetModel();

    model.testServerVisible = false;
    model.leftGroups.clear();
    model.rightGroups.clear();

    const int nGroupCount = g_ServerListManager->GetServerGroupSize();
    if (nGroupCount >= 1)
    {
        CServerGroup* pServerGroup = nullptr;
        g_ServerListManager->SetFirst();

        bool bTestServerFound = false;
        while (g_ServerListManager->GetNext(pServerGroup))
        {
            const std::string label = StringUtils::WideToNarrow(pServerGroup->m_szName);

            if (pServerGroup->m_iWidthPos == CServerGroup::SBP_CENTER)
            {
                if (bTestServerFound)
                    continue;

                model.testServerLabel = label;
                pServerGroup->m_iBtnPos = 0;
                model.testServerVisible = true;
                bTestServerFound = true;
            }
            else if (pServerGroup->m_iWidthPos == CServerGroup::SBP_LEFT)
            {
                if (model.leftGroups.size() >= 10)
                    continue;

                GroupEntry entry;
                entry.label = label;
                entry.btnPos = static_cast<int>(model.leftGroups.size()) + 1;
                pServerGroup->m_iBtnPos = entry.btnPos;
                model.leftGroups.push_back(entry);
            }
            else if (pServerGroup->m_iWidthPos == CServerGroup::SBP_RIGHT)
            {
                if (model.rightGroups.size() >= 10)
                    continue;

                GroupEntry entry;
                entry.label = label;
                entry.btnPos = 10 + static_cast<int>(model.rightGroups.size()) + 1;
                pServerGroup->m_iBtnPos = entry.btnPos;
                model.rightGroups.push_back(entry);
            }
        }
    }

    model.servers.clear();
    model.descriptionText.clear();
    model.pvpNotice = false;

    if (m_iSelectServerBtnIndex != -1)
        m_pSelectServerGroup = g_ServerListManager->GetServerGroupByBtnPos(m_iSelectServerBtnIndex);
    else
        m_pSelectServerGroup = nullptr;

    if (m_pSelectServerGroup != nullptr)
    {
        model.descriptionText = StringUtils::WideToNarrow(m_pSelectServerGroup->m_szDescription);
        model.pvpNotice = m_pSelectServerGroup->m_bPvPServer;

        CServerInfo* pServerInfo = nullptr;
        m_pSelectServerGroup->SetFirst();

        int nServerIndex = 0;
        while (m_pSelectServerGroup->GetNext(pServerInfo))
        {
            ServerEntry entry;
            entry.label = StringUtils::WideToNarrow(pServerInfo->m_bName);
            entry.index = nServerIndex;
            entry.loadFraction = static_cast<float>(pServerInfo->m_iPercent) / 100.0f;
            // Matches the legacy adwServerBtnClr[byNonPvP] bucketing (see ServerEntry's own
            // comment): 0 = gray, 1 = the unflagged default, 2/3 = orange.
            entry.colorGray = (pServerInfo->m_byNonPvP == 0);
            entry.colorOrange = (pServerInfo->m_byNonPvP >= 2);
            model.servers.push_back(entry);

            ++nServerIndex;
        }
    }

    // Unconditional -- this is a genuine rebuild (clear + repopulate), not a per-frame poll, same
    // convention as CBuffStrip's "resize + unconditional MarkDirty" (contrast SyncRmlModel() below,
    // which only marks a field dirty when an existing value actually changes).
    m_RmlBinder.MarkDirty("test_server_visible");
    m_RmlBinder.MarkDirty("test_server_label");
    m_RmlBinder.MarkDirty("left_groups");
    m_RmlBinder.MarkDirty("right_groups");
    m_RmlBinder.MarkDirty("servers");
    m_RmlBinder.MarkDirty("pvp_notice");
    m_RmlBinder.MarkDirty("description_text");

    SyncRmlModel();
}

void CServerSelWin::SelectGroup(int nBtnPos)
{
    if (m_iSelectServerBtnIndex != -1)
    {
        for (auto& entry : m_RmlBinder.GetModel().leftGroups)
            if (entry.btnPos == m_iSelectServerBtnIndex) entry.checked = false;
        for (auto& entry : m_RmlBinder.GetModel().rightGroups)
            if (entry.btnPos == m_iSelectServerBtnIndex) entry.checked = false;
        if (m_iSelectServerBtnIndex == 0)
            m_RmlBinder.GetModel().testServerChecked = false;
    }

    m_iSelectServerBtnIndex = nBtnPos;

    SyncRmlModel();

    SocketClient->ToConnectServer()->SendServerListRequest();
}

void CServerSelWin::RmlClickSelectGroup(int nBtnPos)
{
    SelectGroup(nBtnPos);
}

void CServerSelWin::RmlClickSelectServer(int nServerIndex)
{
    if (m_pSelectServerGroup == nullptr)
        return;

    CServerInfo* pServerInfo = m_pSelectServerGroup->GetServerInfo(nServerIndex);
    if (pServerInfo == nullptr)
        return;

    if (pServerInfo->m_iPercent < 100)
    {
        Show(false);

        SocketClient->ToConnectServer()->SendConnectionInfoRequest(static_cast<uint16_t>(pServerInfo->m_iConnectIndex));
        g_pSystemLogBox->AddText(I18N::Game::ConnectingToTheServer, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);

        g_ServerListManager->SetSelectServerInfo(m_pSelectServerGroup->m_szName, pServerInfo->m_iIndex, pServerInfo->m_byNonPvP);
    }
    else if (pServerInfo->m_iPercent < 128)
    {
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_SERVER_BUSY);
    }
}

bool CServerSelWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    const int nLeft = (static_cast<int>(WindowWidth) - kPanelWidth) / 2;
    const int nTop = (static_cast<int>(WindowHeight) - kPanelHeight) / 2;

    RECT rc;
    ::SetRect(&rc, nLeft, nTop, nLeft + kPanelWidth, nTop + kPanelHeight);
    if (::PtInRect(&rc, CInput::Instance().GetCursorPos()))
        return false;

    return true;
}

bool CServerSelWin::Update()
{
    return true;
}

bool CServerSelWin::Render()
{
    return true;
}

// Called only at the point of an actual state change (Show(true), UpdateDisplay(), SelectGroup())
// -- not polled from Render() every frame, unlike CSysMenuWin's SyncRmlModel(). This window's
// state (m_iSelectServerBtnIndex) only ever changes from its own click handlers, so there's no
// externally-changing flag to poll for; syncing at the mutation site avoids re-marking every field
// dirty ~60+ times a second, which was reapplying #server_list's data-style-top inline style
// continuously even when the flyout hadn't moved -- suspected cause of the flyout's own rows never
// settling into a stable hit-test box while the fully-static .group-btn elements were unaffected.
// Each field is compared before MarkDirty for the same reason (matches CSysMenuWin::SyncRmlModel's
// real pattern -- diff-then-mark, not mark-unconditionally).
void CServerSelWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    const bool testServerChecked = (m_iSelectServerBtnIndex == 0);
    if (model.testServerChecked != testServerChecked)
    {
        model.testServerChecked = testServerChecked;
        m_RmlBinder.MarkDirty("test_server_checked");
    }

    bool leftChanged = false;
    for (auto& entry : model.leftGroups)
    {
        const bool checked = (entry.btnPos == m_iSelectServerBtnIndex);
        if (entry.checked != checked) { entry.checked = checked; leftChanged = true; }
    }
    if (leftChanged)
        m_RmlBinder.MarkDirty("left_groups");

    bool rightChanged = false;
    for (auto& entry : model.rightGroups)
    {
        const bool checked = (entry.btnPos == m_iSelectServerBtnIndex);
        if (entry.checked != checked) { entry.checked = checked; rightChanged = true; }
    }
    if (rightChanged)
        m_RmlBinder.MarkDirty("right_groups");

    // Flyout docking -- see ServerSelRmlModel's own comment. Derived from m_iSelectServerBtnIndex
    // (the single source of truth, same value UpdateDisplay() already resolves back into
    // m_pSelectServerGroup) rather than stored separately, so it can never drift out of sync with
    // which group is actually selected.
    const bool serverListVisible = (m_pSelectServerGroup != nullptr);
    bool serverListDockLeft = false;
    bool serverListDockRight = false;
    float serverListTop = kCenterFlyoutTop;
    if (m_iSelectServerBtnIndex >= 1 && m_iSelectServerBtnIndex <= 10)
    {
        serverListDockLeft = true;
        const float columnCenteringOffset = (kGroupColumnReservedHeight
            - static_cast<float>(model.leftGroups.size()) * kGroupBtnRowHeight) / 2.0f;
        serverListTop = kGroupRowTopPadding + columnCenteringOffset
            + static_cast<float>(m_iSelectServerBtnIndex - 1) * kGroupBtnRowHeight;
    }
    else if (m_iSelectServerBtnIndex >= 11 && m_iSelectServerBtnIndex <= 20)
    {
        serverListDockRight = true;
        const float columnCenteringOffset = (kGroupColumnReservedHeight
            - static_cast<float>(model.rightGroups.size()) * kGroupBtnRowHeight) / 2.0f;
        serverListTop = kGroupRowTopPadding + columnCenteringOffset
            + static_cast<float>(m_iSelectServerBtnIndex - 11) * kGroupBtnRowHeight;
    }
    // m_iSelectServerBtnIndex == 0 (the center/test button) leaves both dock flags false and
    // serverListTop at kCenterFlyoutTop -- server_select.rcss's .dock-center only handles
    // horizontal centering for that case.

    if (model.serverListVisible != serverListVisible)
    {
        model.serverListVisible = serverListVisible;
        m_RmlBinder.MarkDirty("server_list_visible");
    }
    if (model.serverListDockLeft != serverListDockLeft)
    {
        model.serverListDockLeft = serverListDockLeft;
        m_RmlBinder.MarkDirty("server_list_dock_left");
    }
    if (model.serverListDockRight != serverListDockRight)
    {
        model.serverListDockRight = serverListDockRight;
        m_RmlBinder.MarkDirty("server_list_dock_right");
    }
    if (model.serverListTop != serverListTop)
    {
        model.serverListTop = serverListTop;
        m_RmlBinder.MarkDirty("server_list_top");
    }
}
