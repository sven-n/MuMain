
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

// Caps how many groups a server can grow this window's data to.
namespace
{
    constexpr std::size_t kMaxGroups = 20;
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
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SERVER_SELECT, this);

    Show(false);

    UpdateDisplay();
}

void CServerSelWin::BuildRmlUi()
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

                c.Bind("groups", &model.groups);
                c.Bind("servers", &model.servers);
                c.Bind("server_list_visible", &model.serverListVisible);

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

void CServerSelWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    // No per-frame SyncRmlModel() poll here (see Update()), unlike the other 6 ported windows --
    // a fresh BuildRmlUi() would otherwise come up with an empty/default model and hidden until
    // the player closes/reopens this window, so both are restored explicitly below.
    const bool wasVisible = IsVisible();

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    UpdateDisplay();
    Show(wasVisible);
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

    model.groups.clear();

    const int nGroupCount = g_ServerListManager->GetServerGroupSize();
    if (nGroupCount >= 1)
    {
        CServerGroup* pServerGroup = nullptr;
        g_ServerListManager->SetFirst();

        bool bTestServerFound = false;
        int nNextBtnPos = 1;
        while (g_ServerListManager->GetNext(pServerGroup))
        {
            if (model.groups.size() >= kMaxGroups)
                continue;

            const std::string label = StringUtils::WideToNarrow(pServerGroup->m_szName);

            // SBP_CENTER is the rare "test/staff server" group; only the first sent is ever shown.
            if (pServerGroup->m_iWidthPos == CServerGroup::SBP_CENTER)
            {
                if (bTestServerFound)
                    continue;
                bTestServerFound = true;

                GroupEntry entry;
                entry.label = label;
                entry.btnPos = 0;
                pServerGroup->m_iBtnPos = 0;
                model.groups.push_back(entry);
                continue;
            }

            GroupEntry entry;
            entry.label = label;
            entry.btnPos = nNextBtnPos++;
            pServerGroup->m_iBtnPos = entry.btnPos;
            model.groups.push_back(entry);
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
            // 0 = gray, 1 = the unflagged default, 2/3 = orange.
            entry.colorGray = (pServerInfo->m_byNonPvP == 0);
            entry.colorOrange = (pServerInfo->m_byNonPvP >= 2);
            model.servers.push_back(entry);

            ++nServerIndex;
        }
    }

    // Unconditional: this is a genuine rebuild (clear + repopulate), not a per-frame poll.
    m_RmlBinder.MarkDirty("groups");
    m_RmlBinder.MarkDirty("servers");
    m_RmlBinder.MarkDirty("pvp_notice");
    m_RmlBinder.MarkDirty("description_text");

    SyncRmlModel();
}

void CServerSelWin::SelectGroup(int nBtnPos)
{
    if (m_iSelectServerBtnIndex != -1)
    {
        for (auto& entry : m_RmlBinder.GetModel().groups)
            if (entry.btnPos == m_iSelectServerBtnIndex) entry.checked = false;
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
        ConnectToServer(pServerInfo);
    }
    else if (pServerInfo->m_iPercent < 128)
    {
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_SERVER_BUSY);
    }
}

bool CServerSelWin::ConnectToServer(CServerInfo* pServerInfo)
{
    if (pServerInfo == nullptr || m_pSelectServerGroup == nullptr)
        return false;

    Show(false);

    SocketClient->ToConnectServer()->SendConnectionInfoRequest(static_cast<uint16_t>(pServerInfo->m_iConnectIndex));
    g_pSystemLogBox->AddText(I18N::Game::ConnectingToTheServer, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);

    g_ServerListManager->SetSelectServerInfo(m_pSelectServerGroup->m_szName, pServerInfo->m_iIndex, pServerInfo->m_byNonPvP);

    return true;
}

bool CServerSelWin::SelectServer(const wchar_t* groupName, int serverIndex)
{
    CServerGroup* pChosenGroup = nullptr;
    CServerGroup* pServerGroup = nullptr;

    g_ServerListManager->SetFirst();
    while (g_ServerListManager->GetNext(pServerGroup))
    {
        const bool bWanted = (groupName == nullptr || groupName[0] == L'\0')
                                 ? (pChosenGroup == nullptr)
                                 : (wcscmp(pServerGroup->m_szName, groupName) == 0);
        if (bWanted)
        {
            pChosenGroup = pServerGroup;
            break;
        }
    }

    if (pChosenGroup == nullptr)
        return false;

    // A group the display never placed (never sent this session) has no
    // button position to select.
    const int iBtnPos = pChosenGroup->m_iBtnPos;
    if (iBtnPos < 0)
        return false;

    // Same two steps the click path takes: mark the group, rebuild the
    // server list against it, then connect to the chosen server. The group
    // has to be marked before the server can be looked up, so a failure
    // after this point puts the previous selection back rather than leaving
    // the screen on a group the caller never reached.
    const int iPreviousBtnIndex = m_iSelectServerBtnIndex;
    m_iSelectServerBtnIndex = iBtnPos;
    UpdateDisplay();

    // UpdateDisplay() re-derives m_pSelectServerGroup from the button index,
    // which only agrees with pChosenGroup while the list and the index agree
    // -- the group this call chose is the one it must connect to regardless.
    m_pSelectServerGroup = pChosenGroup;

    CServerInfo* pServerInfo = m_pSelectServerGroup->GetServerInfo(serverIndex);
    if (pServerInfo == nullptr || pServerInfo->m_iPercent >= 100)
    {
        m_iSelectServerBtnIndex = iPreviousBtnIndex;
        UpdateDisplay();
        return false;
    }

    return ConnectToServer(pServerInfo);
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

// Called only at the point of an actual state change, not polled from Render() every frame --
// this window's state only ever changes from its own click handlers.
void CServerSelWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    bool groupsChanged = false;
    for (auto& entry : model.groups)
    {
        const bool checked = (entry.btnPos == m_iSelectServerBtnIndex);
        if (entry.checked != checked) { entry.checked = checked; groupsChanged = true; }
    }
    if (groupsChanged)
        m_RmlBinder.MarkDirty("groups");

    // Derived from m_pSelectServerGroup rather than stored separately, so it can't drift out of sync.
    const bool serverListVisible = (m_pSelectServerGroup != nullptr);
    if (model.serverListVisible != serverListVisible)
    {
        model.serverListVisible = serverListVisible;
        m_RmlBinder.MarkDirty("server_list_visible");
    }
}
