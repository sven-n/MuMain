#include "stdafx.h"
#include "UI/Social/FriendShell.h"

#include "UI/Social/FriendWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/TextWrap.h"
#include "Core/Utilities/StringUtils.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include <RmlUi/Core.h>
#include <algorithm>

#include "UI/Social/SocialUpdates.h"

namespace UI::Social
{
namespace
{
constexpr int FriendsTab = 0;
constexpr int LettersTab = 1;
constexpr int WindowsTab = 2;
constexpr int UnavailableServer = 0xFD;
constexpr int LastOnlineServer = 0xFC;

Rml::String Text(const wchar_t* text) { return StringUtils::WideToNarrow(text); }

Rml::String ServerLabel(int server)
{
    if (server == UnavailableServer)
        return Text(I18N::Game::CannotUse);
    if (server >= LastOnlineServer)
        return Text(I18N::Game::Offline1039);
    wchar_t label[MAX_TEXT_LENGTH + 1]{};
    mu_swprintf(label, I18N::Game::_2dServer, server + 1);
    return Text(label);
}
}

FriendShell::FriendShell(CUIFriendWindow& owner) : m_Owner(owner)
{
}

FriendShell::~FriendShell()
{
    Unload();
}

void FriendShell::BindLabels(Rml::DataModelConstructor& c)
{
    const std::pair<const char*, const wchar_t*> labels[] = {
        {"friends_label", I18N::Game::FriendsList}, {"mail_label", I18N::Game::LetterBox},
        {"windows_label", I18N::Game::WindowList}, {"reject_label", I18N::Game::RefuseChat},
        {"name_label", I18N::Game::FriendSName}, {"server_label", I18N::Game::Server},
        {"add_label", I18N::Game::AddFriend}, {"delete_friend_label", I18N::Game::DeleteFriend},
        {"chat_label", I18N::Game::Chat}, {"write_label", I18N::Game::Write},
        {"sender_label", I18N::Game::Sender}, {"date_label", I18N::Game::DateRcvd},
        {"title_label", I18N::Game::Title1030}, {"read_label", I18N::Game::Read},
        {"reply_label", I18N::Game::Reply}, {"delete_label", I18N::Game::Delete},
        {"window_label", I18N::Game::WindowTitle}, {"hide_all_label", I18N::Game::HideAll},
        {"close_label", I18N::Game::Close388}};
    static_assert(std::size(labels) <= std::tuple_size_v<decltype(m_Labels)>);
    for (size_t i = 0; i < std::size(labels); ++i)
    {
        m_Labels[i] = Text(labels[i].second);
        c.Bind(labels[i].first, &m_Labels[i]);
    }
}

void FriendShell::RegisterModel(Rml::DataModelConstructor& c, Model& m)
{
    m.Bind(c);
    BindLabels(c);
    c.BindEventCallback("shell_action", [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
    {
        if (args.empty())
            return;
        event.StopPropagation();
        m_Actions.push_back({args[0].Get<Rml::String>(), args.size() > 1 ? args[1] : Rml::Variant{}});
    });
}

void FriendShell::Build()
{
    m_View.Ensure();
}

void FriendShell::OnBuilt()
{
    m_View.Document()->AddEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->AddEventListener(Rml::EventId::Keydown, this);
    m_View.Document()->AddEventListener(Rml::EventId::Handledrag, this);
    if (auto* header = m_View.Document()->GetElementById("window_shell_header"))
        UI::RmlBridge::MakeDraggable(header, m_View.Document(), nullptr, [this] { SyncDraggedPosition(); });
    SyncWorkspace();
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}

void FriendShell::OnUnload()
{
    if (auto* pane = ActivePane())
        m_Scroll[m_View.GetModel().tab] = pane->GetScrollTop() / m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    m_View.Document()->RemoveEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->RemoveEventListener(Rml::EventId::Keydown, this);
    m_View.Document()->RemoveEventListener(Rml::EventId::Handledrag, this);
    m_Placed = false;
    m_Settled = false;
}

void FriendShell::Unload()
{
    m_View.Release();
}

bool FriendShell::Sync(bool shown)
{
    if (!shown)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), false);
        return false;
    }
    Build();
    if (!m_View.Document())
        return false;
    auto& model = m_View.GetModel();
    if (m_Title != m_Owner.GetTitle())
    {
        m_Title = m_Owner.GetTitle();
        model.title = Text(m_Title.c_str());
        m_View.MarkDirty("title");
    }
    const bool reject = g_pWindowMgr->GetChatReject() != FALSE;
    if (model.rejectChat != reject) { model.rejectChat = reject; m_View.MarkDirty("reject_chat"); }
    const bool wasVisible = m_View.Document()->IsVisible();
    SyncWorkspace();
    SyncGeometry();
    // A hidden document has no box, so neither the resting place nor a layout the manager restored
    // before this document existed can be applied until the first frame it has a size.
    if (!m_Placed && m_Width > 0 && m_Height > 0)
    {
        m_Placed = true;
        if (!m_CustomPosition)
            PlaceAtRest();
        ApplyLayout();
        m_View.Document()->UpdateDocument();
        SyncGeometry();
    }
    else if (m_Placed)
        m_Settled = true;
    // window_shell places #panel through the data model, which the context applies only after this
    // runs, so a document shown on the frame it is placed still renders once where .center-both
    // left it -- centred and unsized.
    UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), m_Settled);
    if (m_Settled && m_RestoreScroll)
    {
        if (auto* pane = ActivePane())
            pane->SetScrollTop(m_Scroll[model.tab] * m_View.Document()->GetContext()->GetDensityIndependentPixelRatio());
        m_RestoreScroll = false;
    }
    if (m_Settled && m_FocusPane)
    {
        m_FocusPane = false;
        FocusActivePane();
    }
    return m_View.Document()->IsVisible() && !wasVisible;
}

void FriendShell::PullToFront() { if (m_View.Document()) m_View.Document()->PullToFront(); }

void FriendShell::SyncGeometry()
{
    auto* panel = Panel();
    if (!panel)
        return;
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    const auto size = panel->GetBox().GetSize(Rml::BoxArea::Border);
    if (scale <= 0 || size.x <= 0 || size.y <= 0)
        return;
    m_Width = size.x / scale;
    m_Height = size.y / scale;
    // An unplaced document still sits at its own origin; reading that back would overwrite the
    // position the manager assigned, which PlaceAtRest() is about to ask the owner for.
    if (!m_Placed)
        return;
    m_Left = panel->GetAbsoluteLeft() / scale;
    m_Top = panel->GetAbsoluteTop() / scale;
    const auto native = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight);
    const float ratio = scale / native.scaleX;
    m_Owner.SetPosition(static_cast<int>(m_Left * ratio), static_cast<int>(m_Top * ratio));
    m_Owner.SetSize(static_cast<int>(m_Width * ratio), static_cast<int>(m_Height * ratio));
    m_Owner.SetBackPosition(m_Maximized, static_cast<int>(m_RestoreRect[1] * ratio),
                           static_cast<int>(m_RestoreRect[3] * ratio));
}

void FriendShell::SyncWorkspace()
{
    auto* context = m_View.Document()->GetContext();
    const auto viewport = context->GetDimensions();
    const float scale = context->GetDensityIndependentPixelRatio();
    const float height = UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight) *
                         UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight).scaleY;
    auto& model = m_View.GetModel();
    if (model.workspaceHeight == height && m_Viewport == viewport && m_DpRatio == scale)
        return;
    model.workspaceHeight = height;
    m_View.MarkDirty("workspace_height");
    m_Viewport = viewport;
    m_DpRatio = scale;
    if (m_CustomPosition)
        RestoreLayout(m_Left, m_Top, m_Width, m_Maximized ? height / scale : m_Height);
    m_View.Document()->UpdateDocument();
}

void FriendShell::RestoreMaximized(bool maximized, float top, float height)
{
    m_Maximized = maximized;
    m_RestoreRect = {m_Left, top, m_Width, height};
    m_View.GetModel().maximized = maximized;
    m_View.MarkDirty("maximized");
}

void FriendShell::SyncDraggedPosition()
{
    if (!m_View.Document())
        return;
    m_View.Document()->UpdateDocument();
    SyncGeometry();
    // Pull it back inside only where there is room to: a panel taller than the workspace would
    // otherwise be yanked to the top edge at the end of every drag.
    ClampToWorkspace();
    m_CustomPosition = true;
    PublishPosition();
}

// window_shell takes its top-left through the model, in real device pixels, and cancels its own
// centering transform while `positioned`.
void FriendShell::PublishPosition()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    auto& model = m_View.GetModel();
    model.rootX = m_Left * scale;
    model.rootY = m_Top * scale;
    m_View.MarkDirty("root_x");
    m_View.MarkDirty("root_y");
}

// Records the layout whether or not a document exists yet: the manager hands back the geometry it
// kept from the last time the window was open as soon as it recreates this window, which is before
// anything has been built. Sync() applies whatever is recorded once the panel has a box.
void FriendShell::RestoreLayout(float x, float y, float width, float height, bool resize)
{
    m_CustomSize |= resize;
    m_Width = width;
    m_Height = height;
    m_Left = x;
    m_Top = y;
    m_CustomPosition = true;
    if (!m_View.Document())
        return;
    ClampToWorkspace();
    ApplyLayout();
}

void FriendShell::ClampToWorkspace()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float maxLeft = WindowWidth / scale - m_Width;
    const float maxTop = m_View.GetModel().workspaceHeight / scale - m_Height;
    if (maxLeft > 0)
        m_Left = std::clamp(m_Left, 0.f, maxLeft);
    if (maxTop > 0)
        m_Top = std::clamp(m_Top, 0.f, maxTop);
}

void FriendShell::ApplyLayout()
{
    auto* panel = Panel();
    if (!panel)
        return;
    ClampToWorkspace();
    PublishPosition();
    // Size stays a direct property: it has no binding of its own.
    if (m_CustomSize)
    {
        panel->SetProperty("width", Rml::ToString(m_Width) + "dp");
        panel->SetProperty("height", Rml::ToString(m_Height) + "dp");
    }
}

// Where the theme's "friends" slot puts it; without one, the bottom-right corner of the usable
// viewport (native's own Init() opened it at (50, 50), against the top-left). The bottom edge is
// the workspace's, not the window's, so it sits on top of the bottom HUD rather than under it.
void FriendShell::PlaceAtRest()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    float x = 0.f;
    float y = 0.f;
    if (UI::Placement::InitialPosition("friends", m_Width * scale, m_Height * scale, x, y))
    {
        m_Left = std::max(0.f, x / scale);
        m_Top = std::max(0.f, y / scale);
        return;
    }
    m_Left = std::max(0.f, WindowWidth / scale - m_Width);
    m_Top = std::max(0.f, m_View.GetModel().workspaceHeight / scale - m_Height);
}

Rml::Element* FriendShell::Panel() const
{
    return m_View.Document();
}

void FriendShell::Maximize()
{
    if (!m_View.Document())
        return;
    if (!m_Maximized)
    {
        m_RestoreRect = {m_Left, m_Top, m_Width, m_Height};
        m_CustomSize = true;
        RestoreLayout(m_Left, 0, m_Width,
                      m_View.GetModel().workspaceHeight / m_View.Document()->GetContext()->GetDensityIndependentPixelRatio());
    }
    else
        RestoreLayout(m_RestoreRect[0], m_RestoreRect[1], m_RestoreRect[2], m_RestoreRect[3]);
    m_Maximized = !m_Maximized;
    m_View.GetModel().maximized = m_Maximized;
    m_View.MarkDirty("maximized");
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}


void FriendShell::RefreshFriends()
{
    auto& m = m_View.GetModel();
    std::deque<GUILDLIST_TEXT> entries;
    g_pFriendList->UpdateFriendList(entries, nullptr);
    m.friends.clear();
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        m.friends.push_back({Text(it->m_szID), ServerLabel(it->m_Server), it->m_Server});
    // A selection that survives the refresh is kept by name; one that does not clears it, as
    // SLSetSelectLine(0) did, so the action buttons stay inert until a row is picked.
    if (std::none_of(m.friends.begin(), m.friends.end(), [&](const auto& row) { return row.name == m.selectedFriend; }))
        m.selectedFriend.clear();
    m.friendSort = g_pFriendList->GetCurrentSortType();
    m_View.MarkDirty("friends");
    m_View.MarkDirty("selected_friend");
    m_View.MarkDirty("friend_sort");
}

void FriendShell::RefreshLetters()
{
    auto& m = m_View.GetModel();
    std::deque<LETTERLIST_TEXT> entries;
    g_pLetterList->UpdateLetterList(entries, m.selectedLetter);
    m.letters.clear();
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        m.letters.push_back({static_cast<int>(it->m_dwLetterID), Text(it->m_szID), Text(it->m_szDate),
                            Text(it->m_szText), it->m_bIsRead != FALSE, false});
    if (std::none_of(m.letters.begin(), m.letters.end(), [&](const auto& row) { return row.id == m.selectedLetter; }))
        m.selectedLetter = 0;
    m.letterSort = g_pLetterList->GetCurrentSortType();
    m.checkAll = false;
    g_pLetterList->ResetLetterSelect(FALSE);
    for (const char* key : {"letters", "selected_letter", "letter_sort", "check_all"}) m_View.MarkDirty(key);
}

void FriendShell::AddWindow(DWORD id, const wchar_t* title)
{
    auto& m = m_View.GetModel();
    if (std::any_of(m.windows.begin(), m.windows.end(), [id](const auto& row) { return row.id == id; })) return;
    m.windows.push_back({static_cast<int>(id), Text(title)});
    // Only the first window opened is selected for you; later ones clear the selection.
    m.selectedWindow = m.windows.size() == 1 ? static_cast<int>(id) : 0;
    m_View.MarkDirty("windows");
    m_View.MarkDirty("selected_window");
}

void FriendShell::RemoveWindow(DWORD id)
{
    auto& m = m_View.GetModel();
    std::erase_if(m.windows, [id](const auto& row) { return row.id == id; });
    if (m.selectedWindow == id) m.selectedWindow = m.windows.empty() ? 0 : m.windows.back().id;
    m_View.MarkDirty("windows");
    m_View.MarkDirty("selected_window");
}

void FriendShell::ResetWindows()
{
    m_View.GetModel().windows.clear();
    m_View.GetModel().selectedWindow = 0;
    m_View.MarkDirty("windows");
    m_View.MarkDirty("selected_window");
}

DWORD FriendShell::SelectedWindow() const { return m_View.GetModel().selectedWindow; }
DWORD FriendShell::SelectedLetter() const { return m_View.GetModel().selectedLetter; }
int FriendShell::GetTab() const { return m_View.GetModel().tab; }

void FriendShell::SelectLetterLine(int line)
{
    auto& m = m_View.GetModel();
    if (line <= 0 || line > static_cast<int>(m.letters.size())) return;
    m.selectedLetter = m.letters[m.letters.size() - line].id;
    m_View.MarkDirty("selected_letter");
    ScrollToSelection();
}

Rml::Element* FriendShell::ActivePane() const
{
    static constexpr const char* panes[] = {"friends_pane", "mail_pane", "windows_pane"};
    return m_View.Document() ? m_View.Document()->GetElementById(panes[GetTab()]) : nullptr;
}

void FriendShell::SetTab(int tab)
{
    if (tab < FriendsTab || tab > WindowsTab || tab == GetTab()) return;
    if (auto* pane = ActivePane())
        m_Scroll[GetTab()] = pane->GetScrollTop() / m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    PlayBuffer(SOUND_CLICK01);
    m_View.GetModel().tab = tab;
    m_View.MarkDirty("active_tab");
    m_RestoreScroll = true;
    m_FocusPane = true;
}

// Native pointed g_dwKeyFocusUIID at the active tab's list so the arrow keys moved its selection;
// the pane holds that keyboard here. Deferred to Sync(), because the pane of a tab just switched
// to does not exist until the document has been updated.
void FriendShell::RequestPaneFocus()
{
    m_FocusPane = true;
}

void FriendShell::FocusActivePane()
{
    if (!m_View.Document() || !m_View.Document()->IsVisible())
        return;
    // Never take the keyboard off a field somebody is typing in -- native's key-focus id and its
    // text focus were separate, so selecting this window never interrupted a chat line.
    if (RmlUiRuntime::Instance().IsTextInputActive())
        return;
    if (auto* pane = ActivePane())
        pane->Focus();
}

void FriendShell::ProcessEvent(Rml::Event& event)
{
    if (event.GetId() == Rml::EventId::Mousedown)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_Owner.GetUIID(), 0);
    else if (event.GetId() == Rml::EventId::Keydown)
    {
        const int key = event.GetParameter<int>("key_identifier", 0);
        if (key == Rml::Input::KI_UP || key == Rml::Input::KI_DOWN || key == Rml::Input::KI_HOME ||
            key == Rml::Input::KI_END || key == Rml::Input::KI_PRIOR || key == Rml::Input::KI_NEXT)
        {
            m_Actions.push_back({"key", Rml::Variant(key)});
            event.StopPropagation();
        }
    }
    else if (event.GetId() == Rml::EventId::Handledrag)
    {
        // Only the resize grip is a <handle>; a scroll thumb is a sliderbar and never lands here.
        m_CustomSize = true;
        m_Maximized = false;
        m_View.GetModel().maximized = false;
        m_View.MarkDirty("maximized");
    }
}

void FriendShell::ProcessActions()
{
    if (m_Actions.empty())
        return;
    std::vector<Action> actions;
    actions.swap(m_Actions);
    for (const auto& action : actions)
        ActionRequested(action);
    actions.clear();
    if (m_Actions.empty())
        actions.swap(m_Actions);
}

bool FriendShell::SelectRow(const Action& a)
{
    auto& m = m_View.GetModel();
    if (a.name == "friend_select" || a.name == "friend_open")
    {
        const auto name = a.value.Get<Rml::String>();
        if (std::none_of(m.friends.begin(), m.friends.end(), [&](const auto& row) { return row.name == name; }))
            return true;
        m.selectedFriend = name;
        m_View.MarkDirty("selected_friend");
        if (a.name == "friend_open") ActivateFriend();
        return true;
    }
    const int id = a.value.Get<int>();
    if (a.name == "letter_select" || a.name == "letter_open")
    {
        if (std::none_of(m.letters.begin(), m.letters.end(), [id](const auto& row) { return row.id == id; }))
            return true;
        m.selectedLetter = id;
        m_View.MarkDirty("selected_letter");
        if (a.name == "letter_open") OpenLetter();
        return true;
    }
    if (a.name != "window_select" && a.name != "window_open")
        return false;
    if (std::none_of(m.windows.begin(), m.windows.end(), [id](const auto& row) { return row.id == id; }))
        return true;
    m.selectedWindow = id;
    m_View.MarkDirty("selected_window");
    if (a.name == "window_open") ActivateWindow();
    return true;
}

void FriendShell::ActionRequested(const Action& a)
{
    if (SelectRow(a))
        return;
    const int value = a.value.Get<int>();
    if (a.name == "tab") SetTab(value);
    else if (a.name == "friend_sort" && value >= 0 && value <= 1) { g_pFriendList->Sort(value); RefreshFriends(); }
    else if (a.name == "letter_sort" && value >= 0 && value <= 3) { g_pLetterList->Sort(value); RefreshLetters(); }
    else if (a.name == "add") g_pWindowMgr->Dialogs().AddFriend(m_Owner.GetUIID());
    else if (a.name == "delete_friend") DeleteFriend();
    else if (a.name == "chat") ActivateFriend();
    else if (a.name == "write_friend") WriteLetter(false, true);
    else if (a.name == "write") WriteLetter(false, false);
    else if (a.name == "reply") WriteLetter(true, false);
    else if (a.name == "read") OpenLetter();
    else if (a.name == "delete_mail") DeleteLetters();
    else if (a.name == "check") ToggleLetter(value);
    else if (a.name == "check_all") ToggleAllLetters();
    else if (a.name == "activate") ActivateWindow();
    else if (a.name == "hide_all") g_pWindowMgr->HideAllWindow(TRUE);
    else if (a.name == "reject") ToggleChat();
    else if (a.name == "close") g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, m_Owner.GetUIID(), 0);
    else if (a.name == "minimize") g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, m_Owner.GetUIID(), 0);
    else if (a.name == "maximize") Maximize();
    else if (a.name == "key") MoveSelection(value);
}

void FriendShell::ActivateFriend()
{
    const auto& m = m_View.GetModel();
    const auto it = std::find_if(m.friends.begin(), m.friends.end(), [&](const auto& row) { return row.name == m.selectedFriend; });
    if (it == m.friends.end() || it->server > LastOnlineServer) return;
    const auto name = StringUtils::NarrowToWide(it->name);
    const DWORD existing = g_pFriendMenu->CheckChatRoomDuplication(name.c_str());
    if (existing == 0)
    {
        if (!g_pWindowMgr->GetChatReject() && !g_pFriendMenu->IsRequestWindow(name.c_str()))
        {
            g_pFriendMenu->AddRequestWindow(name.c_str());
            SocketClient->ToGameServer()->SendChatRoomCreateRequest(MU_C16(name.c_str()));
        }
    }
    else if (existing != static_cast<DWORD>(-1))
    {
        auto* room = g_pWindowMgr->GetWindow(existing);
        if (!room)
            return;
        // A room prepared hidden only comes back through the hidden state.
        room->SetState(UISTATE_HIDE);
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, existing, 0);
    }
}

void FriendShell::DeleteFriend()
{
    const auto name = StringUtils::NarrowToWide(m_View.GetModel().selectedFriend);
    if (name.empty()) return;
    wchar_t prompt[MAX_TEXT_LENGTH + 1]{};
    mu_swprintf(prompt, L"%ls %ls", I18N::Game::DoYouReallyWishToDeleteThisFriend, name.c_str());
    g_pWindowMgr->Dialogs().ConfirmAction(prompt, m_Owner.GetUIID(), [name]
    { SocketClient->ToGameServer()->SendFriendDelete(MU_C16(name.c_str())); });
}

void FriendShell::OpenLetter()
{
    const DWORD id = SelectedLetter();
    if (!g_pLetterList->GetLetter(id)) return;
    if (g_pWindowMgr->LetterReadCheck(id))
    {
        const DWORD window = g_pWindowMgr->GetLetterReadWindow(id);
        if (window) g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, window, 0);
        return;
    }
    if (const auto* cached = g_pLetterList->GetLetterText(id))
        UI::Social::ShowLetter(*cached);
    else
        SocketClient->ToGameServer()->SendLetterReadRequest(id);
}

void FriendShell::WriteLetter(bool reply, bool toFriend)
{
    auto* letter = reply ? g_pLetterList->GetLetter(SelectedLetter()) : nullptr;
    if (reply && !letter) return;
    wchar_t title[MAX_TEXT_LENGTH + 1]{};
    mu_swprintf(title, I18N::Game::WriteLetterCostDZen, UI::Social::LetterCost);
    const DWORD id = g_pWindowMgr->AddWindow(UIWNDTYPE_WRITELETTER, UIWND_DEFAULT, UIWND_DEFAULT, title);
    auto* window = dynamic_cast<CUILetterWriteWindow*>(g_pWindowMgr->GetWindow(id));
    if (!window) return;
    if (toFriend && !m_View.GetModel().selectedFriend.empty())
        window->SetMailtoText(StringUtils::NarrowToWide(m_View.GetModel().selectedFriend).c_str());
    if (letter)
    {
        window->SetMailtoText(letter->m_szID);
        mu_swprintf(title, I18N::Game::ReS, letter->m_szText);
        constexpr int SubjectLength = 32;
        wchar_t subject[SubjectLength + 1]{};
        CutText4(title, subject, nullptr, SubjectLength);
        window->SetMainTitleText(subject);
    }
}

void FriendShell::DeleteLetters()
{
    std::vector<DWORD> ids;
    for (const auto& row : m_View.GetModel().letters)
        if (row.checked) ids.push_back(row.id);
    if (ids.empty())
    {
        g_pWindowMgr->Dialogs().Notice(I18N::Game::SelectTheLetterYouDLikeToDelete);
        return;
    }
    g_pWindowMgr->Dialogs().ConfirmAction(I18N::Game::AreYouSureYouWantToDeleteTheLetter, m_Owner.GetUIID(), [ids]
    {
        for (DWORD id : ids)
            if (g_pLetterList->GetLetter(id)) SocketClient->ToGameServer()->SendLetterDeleteRequest(id);
    });
}

void FriendShell::ToggleLetter(int id)
{
    auto& m = m_View.GetModel();
    for (auto& row : m.letters) if (row.id == id) row.checked = !row.checked;
    m.checkAll = !m.letters.empty() && std::all_of(m.letters.begin(), m.letters.end(), [](const auto& row) { return row.checked; });
    m_View.MarkDirty("letters");
    m_View.MarkDirty("check_all");
}

void FriendShell::ToggleAllLetters()
{
    auto& m = m_View.GetModel();
    m.checkAll = !m.checkAll;
    for (auto& row : m.letters) row.checked = m.checkAll;
    m_View.MarkDirty("letters");
    m_View.MarkDirty("check_all");
}

void FriendShell::ActivateWindow()
{
    const DWORD id = SelectedWindow();
    auto* window = g_pWindowMgr->GetWindow(id);
    if (!window) return;
    const bool select = window->GetState() == UISTATE_HIDE || g_pWindowMgr->GetTopNotMainWindowUIID() != id;
    g_pWindowMgr->SendUIMessage(select ? UI_MESSAGE_SELECT : UI_MESSAGE_HIDE, id, 0);
    if (select) g_pWindowMgr->HideAllWindowClear();
}

void FriendShell::ToggleChat()
{
    if (g_pWindowMgr->GetChatReject())
    {
        SocketClient->ToGameServer()->SendSetFriendOnlineState(1);
        g_pWindowMgr->SetChatReject(FALSE);
        return;
    }
    g_pWindowMgr->Dialogs().ConfirmAction(I18N::Game::IfYouRefuseChatAllChatWindowsWillClose, m_Owner.GetUIID(), []
    {
        SocketClient->ToGameServer()->SendSetFriendOnlineState(0);
        g_pWindowMgr->SetChatReject(TRUE);
        g_pFriendMenu->CloseAllChatWindow();
    });
}

void FriendShell::MoveSelection(int key)
{
    auto* pane = ActivePane();
    if (!pane) return;
    if (key == Rml::Input::KI_PRIOR || key == Rml::Input::KI_NEXT)
    {
        pane->SetScrollTop(pane->GetScrollTop() + (key == Rml::Input::KI_PRIOR ? -1 : 1) * pane->GetClientHeight());
        return;
    }
    if (key != Rml::Input::KI_UP && key != Rml::Input::KI_DOWN && key != Rml::Input::KI_HOME && key != Rml::Input::KI_END) return;
    auto& m = m_View.GetModel();
    auto move = [&](const auto& rows, auto& selected, auto identity)
    {
        if (rows.empty()) return;
        auto it = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return identity(row) == selected; });
        int index = it == rows.end() ? 0 : static_cast<int>(it - rows.begin());
        if (key == Rml::Input::KI_HOME) index = 0;
        else if (key == Rml::Input::KI_END) index = static_cast<int>(rows.size()) - 1;
        else index = std::clamp(index + (key == Rml::Input::KI_UP ? -1 : 1), 0, static_cast<int>(rows.size()) - 1);
        selected = identity(rows[index]);
    };
    if (m.tab == FriendsTab) move(m.friends, m.selectedFriend, [](const auto& r) { return r.name; });
    else if (m.tab == LettersTab) move(m.letters, m.selectedLetter, [](const auto& r) { return r.id; });
    else move(m.windows, m.selectedWindow, [](const auto& r) { return r.id; });
    for (const char* field : {"selected_friend", "selected_letter", "selected_window"}) m_View.MarkDirty(field);
    ScrollToSelection();
}

void FriendShell::ScrollToSelection()
{
    auto* pane = ActivePane();
    if (!pane) return;
    m_View.Document()->UpdateDocument();
    if (auto* row = pane->QuerySelector(".selected")) row->ScrollIntoView();
}
}
