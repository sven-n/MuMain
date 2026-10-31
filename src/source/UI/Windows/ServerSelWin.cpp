
#include "stdafx.h"
#include "ServerSelWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

#include "Network/Server/ServerListManager.h"
#include "Network/Reconnect/LoginSceneReconnect.h"
#include "Core/Globals/_enum.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlNativeText.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include "Core/Color/Rgb.h"
#include "Data/GameData/ServerListData/LoadBar.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace
{
// Caps how many groups a server can grow this window's data to.
constexpr std::size_t kMaxGroups = 20;
constexpr float kPercent = 100.0f;
// The font the server buttons are drawn in (server_select.rcss).
constexpr const char* kLabelFontToken = "font-body";
// Languages whose glyphs come from the Chinese and Japanese fonts, which have no italic faces.
constexpr std::array<std::string_view, 3> kLocalesWithoutItalic = {"zh-CN", "zh-TW", "ja"};

// Whether italic text can be drawn as such; where not, a load text is slanted instead.
bool CanDrawItalic()
{
    const char* locale = I18N::GetCurrentLocale();
    const std::string_view current = locale != nullptr ? locale : "";
    if (std::find(kLocalesWithoutItalic.begin(), kLocalesWithoutItalic.end(), current) != kLocalesWithoutItalic.end())
        return false;
    return UI::RmlBridge::ActiveThemeFontHasItalic(kLabelFontToken);
}

// The segments of a server row's bar its load text covers as laid out; none while the row isn't laid out yet.
std::optional<Data::ServerList::CoveredSegments> MeasureCoveredSegments(Rml::Element& row, Rml::Element& text)
{
    Rml::ElementList segments;
    row.QuerySelectorAll(segments, ".server-gauge-segment");
    std::vector<Data::ServerList::SegmentSpan> spans;
    for (Rml::Element* segment : segments)
        spans.push_back(
            {segment->GetAbsoluteOffset(Rml::BoxArea::Border).x, segment->GetBox().GetSize(Rml::BoxArea::Border).x});
    const float left = text.GetAbsoluteOffset(Rml::BoxArea::Border).x;
    const float width = text.GetBox().GetSize(Rml::BoxArea::Border).x;
    if (spans.empty() || spans.front().width <= 0.0f || width <= 0.0f)
        return std::nullopt;
    return Data::ServerList::GetCoveredSegments(spans, left, left + width);
}
} // namespace

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

    // Create() runs on every entry to the login scene: start with no group chosen, so the list
    // does not reopen on the group an earlier visit picked.
    m_iSelectServerBtnIndex = -1;

    // A button in a color of the server list's own has the shade of the theme it was built
    // in (CServerListManager::GetServerButtonColor): a theme switch builds the buttons again.
    m_RmlView.SetAfterReload([this] { UpdateDisplay(); });
    m_RmlView.Ensure();

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SERVER_SELECT, this);

    Show(false);

    UpdateDisplay();
}

void CServerSelWin::BindRmlModel(Rml::DataModelConstructor& c, ServerSelRmlModel& model)
{
    auto group = c.RegisterStruct<GroupEntry>();
    group.RegisterMember("label", &GroupEntry::label);
    group.RegisterMember("btn_pos", &GroupEntry::btnPos);
    group.RegisterMember("checked", &GroupEntry::checked);
    c.RegisterArray<std::vector<GroupEntry>>();

    auto segment = c.RegisterStruct<SegmentEntry>();
    segment.RegisterMember("lit", &SegmentEntry::lit);
    segment.RegisterMember("color", &SegmentEntry::color);
    c.RegisterArray<std::vector<SegmentEntry>>();

    auto color = c.RegisterStruct<ColorEntry>();
    color.RegisterMember("gray", &ColorEntry::gray);
    color.RegisterMember("orange", &ColorEntry::orange);
    color.RegisterMember("silver", &ColorEntry::silver);
    color.RegisterMember("style", &ColorEntry::style);

    auto server = c.RegisterStruct<ServerEntry>();
    server.RegisterMember("label", &ServerEntry::label);
    server.RegisterMember("load_text", &ServerEntry::loadText);
    server.RegisterMember("index", &ServerEntry::index);
    server.RegisterMember("color", &ServerEntry::color);
    server.RegisterMember("load_color", &ServerEntry::loadColor);
    server.RegisterMember("load_bold", &ServerEntry::loadBold);
    server.RegisterMember("load_italic", &ServerEntry::loadItalic);
    server.RegisterMember("load_slant", &ServerEntry::loadSlant);
    server.RegisterMember("segments", &ServerEntry::segments);
    server.RegisterMember("empty_style", &ServerEntry::emptyStyle);
    server.RegisterMember("text_from", &ServerEntry::textFrom);
    server.RegisterMember("text_to", &ServerEntry::textTo);
    c.RegisterArray<std::vector<ServerEntry>>();

    auto loadSizer = c.RegisterStruct<LoadSizerEntry>();
    loadSizer.RegisterMember("text", &LoadSizerEntry::text);
    loadSizer.RegisterMember("bold", &LoadSizerEntry::bold);
    loadSizer.RegisterMember("italic", &LoadSizerEntry::italic);
    c.RegisterArray<std::vector<LoadSizerEntry>>();
    c.RegisterArray<std::vector<Rml::String>>();

    c.Bind("groups", &model.groups);
    c.Bind("servers", &model.servers);
    c.Bind("sizer_names", &model.sizerNames);
    c.Bind("sizer_loads", &model.sizerLoads);
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
    model.pvpNoticeLine0 = StringUtils::WideToNarrow(I18N::Game::SinceHelheimServer);
    model.pvpNoticeLine1 = StringUtils::WideToNarrow(I18N::Game::TendsToBeCrowded);
    model.pvpNoticeLine2 = StringUtils::WideToNarrow(I18N::Game::WeRecommendThatYouUseOtherServers);
}

void CServerSelWin::Release()
{
    m_RmlView.Hide();
}

void CServerSelWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    if (m_RmlView.Document())
    {
        if (bShow) { SyncRmlModel(); m_RmlView.Document()->Show(); }
        else       m_RmlView.Document()->Hide();
    }
    if (bShow)
        m_fitPending = true;
}

void CServerSelWin::UpdateDisplay()
{
    auto& model = m_RmlView.GetModel();

    model.groups.clear();
    std::vector<const CServerGroup*> shownGroups;

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
                shownGroups.push_back(pServerGroup);
                continue;
            }

            GroupEntry entry;
            entry.label = label;
            entry.btnPos = nNextBtnPos++;
            pServerGroup->m_iBtnPos = entry.btnPos;
            model.groups.push_back(entry);
            shownGroups.push_back(pServerGroup);
        }
    }
    FillSizers(model, shownGroups);

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
            model.servers.push_back(MakeServerEntry(*pServerInfo, nServerIndex));
            ++nServerIndex;
        }
    }

    // Unconditional: this is a genuine rebuild (clear + repopulate), not a per-frame poll.
    m_RmlView.MarkDirty("groups");
    m_RmlView.MarkDirty("servers");
    m_RmlView.MarkDirty("sizer_names");
    m_RmlView.MarkDirty("sizer_loads");
    m_RmlView.MarkDirty("pvp_notice");
    m_RmlView.MarkDirty("description_text");
    m_fitPending = true;

    SyncRmlModel();
}

void CServerSelWin::FillSizers(ServerSelRmlModel& model, const std::vector<const CServerGroup*>& groups)
{
    model.sizerNames.clear();
    // Each name once, in the order first met: up to 20 channels of 20 groups.
    std::set<std::string, std::less<>> seen;
    for (const CServerGroup* group : groups)
    {
        // Every channel the group can have, not only those the connect server sent.
        for (int index = 1; index <= MAX_SERVER_PER_GROUP; ++index)
        {
            std::string name = g_ServerListManager->GetServerButtonName(*group, index);
            if (seen.insert(name).second)
                model.sizerNames.push_back(std::move(name));
        }
    }
    model.sizerLoads.clear();
    for (CServerListManager::ButtonLoadText& text : g_ServerListManager->GetButtonLoadTexts())
    {
        // A slanted text (SetLoadTextLook) is as wide as the upright one.
        model.sizerLoads.push_back({std::move(text.text), text.bold, text.italic && CanDrawItalic()});
    }
}

CServerSelWin::ColorEntry CServerSelWin::MakeColorEntry(const Data::ServerList::ButtonColor& color)
{
    ColorEntry entry;
    entry.gray = color.theme == Data::ServerList::ThemeColor::Gray;
    entry.orange = color.theme == Data::ServerList::ThemeColor::Orange;
    entry.silver = color.theme == Data::ServerList::ThemeColor::Silver;
    entry.style = color.shade;
    return entry;
}

void CServerSelWin::SetLoadTextLook(ServerEntry& entry, int percent)
{
    const Data::ServerList::ServerListData& list = g_ServerListManager->GetServerList();
    const Data::ServerList::LoadText* look = list.load.Text(Data::ServerList::GetLoadState(percent));
    if (look == nullptr)
        return;
    entry.loadColor =
        MakeColorEntry(Data::ServerList::ResolveButtonColor(list, look->color, UI::RmlBridge::GetActiveThemeName()));
    entry.loadBold = look->bold;
    entry.loadSlant = look->italic && !CanDrawItalic();
    entry.loadItalic = look->italic && !entry.loadSlant;
}

void CServerSelWin::SetLoadBar(ServerEntry& entry, int percent)
{
    using Data::ServerList::LoadBarPart;
    const Data::ServerList::ServerListData& list = g_ServerListManager->GetServerList();
    const std::string& theme = UI::RmlBridge::GetActiveThemeName();
    const Data::ServerList::ThemeTokenReader readToken = [](const char* token)
    { return UI::RmlBridge::GetActiveThemeToken(token); };

    const auto base = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Base, theme, readToken);
    const auto full = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, theme, readToken);
    const Data::ServerList::LoadBarSegments segments = Data::ServerList::GetLoadBarSegments(
        base.value_or(Core::Color::Rgb{}), full.value_or(Core::Color::Rgb{}), percent);
    for (const Data::ServerList::LoadBarSegment& segment : segments)
    {
        const long litPercent = std::lround(segment.lit * kPercent);
        entry.segments.push_back({std::to_string(litPercent) + "%", Core::Color::FormatRgb(segment.color)});
    }
    if (Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Empty, theme))
        return;
    if (const auto empty = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Empty, theme, readToken))
        entry.emptyStyle = Core::Color::FormatRgb(*empty);
}

CServerSelWin::ServerEntry CServerSelWin::MakeServerEntry(const CServerInfo& serverInfo, int index) const
{
    ServerEntry entry;
    entry.label = g_ServerListManager->GetServerButtonName(*m_pSelectServerGroup, serverInfo.m_iIndex);
    entry.loadText = g_ServerListManager->GetServerLoadText(serverInfo);
    if (const auto covered = m_coveredByLoadText.find(entry.loadText); covered != m_coveredByLoadText.end())
    {
        entry.textFrom = covered->second.first;
        entry.textTo = covered->second.end;
    }
    entry.index = index;
    entry.color =
        MakeColorEntry(g_ServerListManager->GetServerButtonColor(serverInfo, UI::RmlBridge::GetActiveThemeName()));
    SetLoadTextLook(entry, serverInfo.m_iPercent);
    SetLoadBar(entry, serverInfo.m_iPercent);
    return entry;
}

void CServerSelWin::SelectGroup(int nBtnPos)
{
    if (m_iSelectServerBtnIndex != -1)
    {
        for (auto& entry : m_RmlView.GetModel().groups)
            if (entry.btnPos == m_iSelectServerBtnIndex) entry.checked = false;
    }

    m_iSelectServerBtnIndex = nBtnPos;

    SyncRmlModel();

    LoginSceneReconnect::Instance().RequestServerList();
}

void CServerSelWin::RmlClickSelectGroup(int nBtnPos)
{
    SelectGroup(nBtnPos);
}

namespace
{
// A server that only MuEditor's preview shows (CServerListManager::SetOfflinePreview): nothing to join, which
// the system messages say.
bool RefuseOfflinePreview(const CServerInfo& server)
{
    if (!server.m_offline)
        return false;
#ifdef _EDITOR
    g_pSystemLogBox->AddText(
        Core::Text::FromUtf8(I18N::Editor::OnlyTheEditorShowsThisServerTheConnectServerDidNotSendIt),
        mu::ui::window::TYPE_SYSTEM_MESSAGE);
#endif
    return true;
}
} // namespace

void CServerSelWin::RmlClickSelectServer(int nServerIndex)
{
    if (m_pSelectServerGroup == nullptr)
        return;

    CServerInfo* pServerInfo = m_pSelectServerGroup->GetServerInfo(nServerIndex);
    if (pServerInfo == nullptr || RefuseOfflinePreview(*pServerInfo))
        return;

    // A full and an overloaded server say so alike; the original client did nothing for an
    // overloaded one (then "Now preparing").
    if (Data::ServerList::GetLoadState(pServerInfo->m_iPercent) == Data::ServerList::LoadState::Open)
    {
        ConnectToServer(pServerInfo, nServerIndex);
    }
    else
    {
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_SERVER_BUSY);
    }
}

bool CServerSelWin::ConnectToServer(CServerInfo* pServerInfo, int serverIndex)
{
    if (pServerInfo == nullptr || m_pSelectServerGroup == nullptr || RefuseOfflinePreview(*pServerInfo))
        return false;

    Show(false);

    // The connect server drops a list left open too long; pick the server again
    // from the fresh list the reconnect brings.
    const std::wstring groupName = m_pSelectServerGroup->m_szName;
    const auto pickAgain = [groupName, serverIndex] { g_ServerSelWin.SelectServer(groupName.c_str(), serverIndex); };
    if (!LoginSceneReconnect::Instance().EnsureConnected(LoginServer::Connect, pickAgain))
        return true;

    SocketClient->ToConnectServer()->SendConnectionInfoRequest(static_cast<uint16_t>(pServerInfo->m_iConnectIndex));
    g_pSystemLogBox->AddText(I18N::Game::ConnectingToTheServer, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);

    g_ServerListManager->SetSelectServerInfo(m_pSelectServerGroup->m_iServerIndex, m_pSelectServerGroup->m_szName,
                                             pServerInfo->m_iIndex, pServerInfo->m_channel);

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
    if (pServerInfo == nullptr ||
        Data::ServerList::GetLoadState(pServerInfo->m_iPercent) != Data::ServerList::LoadState::Open)
    {
        m_iSelectServerBtnIndex = iPreviousBtnIndex;
        UpdateDisplay();
        return false;
    }

    return ConnectToServer(pServerInfo, serverIndex);
}

bool CServerSelWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    // server_select.rcss is in px, or in the legacy theme's scene-window rem (RmlNativeText.h).
    const float scale =
        UI::RmlBridge::SceneWindowPixelRatio(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    const int panelWidth = static_cast<int>(std::lround(kPanelWidth * scale));
    const int panelHeight = static_cast<int>(std::lround(kPanelHeight * scale));
    const int nLeft = (static_cast<int>(WindowWidth) - panelWidth) / 2;
    const int nTop = (static_cast<int>(WindowHeight) - panelHeight) / 2;

    RECT rc;
    ::SetRect(&rc, nLeft, nTop, nLeft + panelWidth, nTop + panelHeight);
    if (::PtInRect(&rc, CInput::Instance().GetCursorPos()))
        return false;

    return true;
}

bool CServerSelWin::Update()
{
    if (m_fitPending || WindowWidth != m_fitWindowWidth || WindowHeight != m_fitWindowHeight)
        FitLoadTexts();
    return true;
}

void CServerSelWin::FitLoadTexts()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr || !IsVisible())
        return;
    // The rows are built from `servers` by RmlUi's next update, at the end of the frame: until then
    // the document has the rows of the last list, or none, and they say nothing about the new ones.
    if (m_RmlView.IsDirty("servers"))
    {
        m_fitPending = true;
        return;
    }
    m_fitWindowWidth = WindowWidth;
    m_fitWindowHeight = WindowHeight;

    std::vector<ServerEntry>& servers = m_RmlView.GetModel().servers;
    Rml::ElementList rows;
    document->QuerySelectorAll(rows, ".server-option-btn.server-row");
    bool changed = false;
    bool unmeasured = false;
    size_t index = 0;
    for (Rml::Element* row : rows)
    {
        // The sizer row comes after the servers' rows.
        if (row->IsClassSet("server-row-sizer") || index >= servers.size())
            break;
        ServerEntry& entry = servers[index++];
        Rml::Element* text = row->QuerySelector(".server-load");
        if (entry.loadText.empty() || text == nullptr)
            continue;
        const std::optional<Data::ServerList::CoveredSegments> covered = MeasureCoveredSegments(*row, *text);
        // Not laid out yet (a row just built): the next frame has it.
        if (!covered)
        {
            unmeasured = true;
            continue;
        }
        m_coveredByLoadText[entry.loadText] = *covered;
        if (entry.textFrom != covered->first || entry.textTo != covered->end)
        {
            entry.textFrom = covered->first;
            entry.textTo = covered->end;
            changed = true;
        }
    }
    // The hidden segments can lay the row out anew: the next pass checks it.
    m_fitPending = changed || unmeasured;
    if (changed)
        m_RmlView.MarkDirty("servers");
}

bool CServerSelWin::Render()
{
    return true;
}

#ifdef _EDITOR
int CServerSelWin::GetOpenGroupId() const
{
    if (m_iSelectServerBtnIndex == -1)
        return -1;

    // By its button, not m_pSelectServerGroup: that can be left on a deleted group.
    const CServerGroup* pServerGroup = g_ServerListManager->GetServerGroupByBtnPos(m_iSelectServerBtnIndex);
    return pServerGroup != nullptr ? pServerGroup->m_iServerIndex : -1;
}

void CServerSelWin::ShowRebuiltGroups(int openGroupId)
{
    // The first pass gives the rebuilt groups their buttons.
    m_iSelectServerBtnIndex = -1;
    UpdateDisplay();
    if (openGroupId < 0)
        return;

    CServerGroup* pServerGroup = nullptr;
    g_ServerListManager->SetFirst();
    while (g_ServerListManager->GetNext(pServerGroup))
    {
        if (pServerGroup->m_iServerIndex == openGroupId && pServerGroup->m_iBtnPos >= 0)
        {
            m_iSelectServerBtnIndex = pServerGroup->m_iBtnPos;
            UpdateDisplay();
            return;
        }
    }
}
#endif

// Called only at the point of an actual state change, not polled from Render() every frame --
// this window's state only changes from its own click handlers and, in editor builds, from
// ShowRebuiltGroups after the Server List window built the groups again.
void CServerSelWin::SyncRmlModel()
{
    if (!m_RmlView.Document()) return;

    auto& model = m_RmlView.GetModel();

    bool groupsChanged = false;
    for (auto& entry : model.groups)
    {
        const bool checked = (entry.btnPos == m_iSelectServerBtnIndex);
        if (entry.checked != checked) { entry.checked = checked; groupsChanged = true; }
    }
    if (groupsChanged)
        m_RmlView.MarkDirty("groups");

    // Derived from m_pSelectServerGroup rather than stored separately, so it can't drift out of sync.
    const bool serverListVisible = (m_pSelectServerGroup != nullptr);
    if (model.serverListVisible != serverListVisible)
    {
        model.serverListVisible = serverListVisible;
        m_RmlView.MarkDirty("server_list_visible");
    }
}
