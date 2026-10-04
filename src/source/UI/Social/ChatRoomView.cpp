#include "stdafx.h"
#include "UI/Social/ChatRoomView.h"

#include "UI/Social/SocialWindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include <RmlUi/Core.h>
#include <algorithm>

namespace UI::Social
{
namespace
{
constexpr const char* DocumentPath = "Data/Interface/RmlUi/chat_room.rml";
// Rewritten per instance by LoadThemedDocument()'s own per-window overload, so each room binds a
// data model of its own rather than every room sharing one.
constexpr const char* ModelPlaceholder = "data-model=\"chat_room\"";
constexpr int SystemSpeaker = 255;

Rml::String Text(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}
} // namespace

ChatRoomView::ChatRoomView(CUIChatWindow& owner) : m_Owner(owner)
{
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadTheme(); });
}

ChatRoomView::~ChatRoomView()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    Unload();
}

void ChatRoomView::RegisterModel(Rml::DataModelConstructor& c, Model& m)
{
    m.Bind(c);
    c.BindEventCallback("room_action",
                        [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                        {
                            if (args.empty())
                                return;
                            event.StopPropagation();
                            m_Actions.push_back({args[0].Get<Rml::String>(),
                                                 args.size() > 1 ? args[1] : Rml::Variant{}});
                        });
}

void ChatRoomView::Build()
{
    if (m_Document || !RmlUiRuntime::Instance().IsCreated())
        return;
    auto* context = RmlUiRuntime::Instance().GetContext();
    // One model per window: two rooms sharing a name would share their state.
    if (m_ModelName.empty())
        m_ModelName = "chat_room_" + Rml::ToString(static_cast<int>(m_Owner.GetUIID()));
    if (!m_Binder.Create(context, m_ModelName, [this](auto& c, auto& m) { RegisterModel(c, m); }))
        return;
    m_Document = UI::RmlBridge::LoadThemedDocument(context, DocumentPath, ModelPlaceholder,
                                                  "data-model=\"" + m_ModelName + "\"");
    if (!m_Document)
        return;
    m_Document->AddEventListener(Rml::EventId::Mousedown, this);
    m_Document->AddEventListener(Rml::EventId::Handledrag, this);
    if (auto* header = m_Document->GetElementById("window_shell_header"))
        UI::RmlBridge::MakeDraggable(header, m_Document, nullptr, [this] { SyncDraggedPosition(); });

    auto& model = m_Binder.GetModel();
    model.inviteButtonLabel = Text(model.showInvite ? I18N::Game::CloseInvitation : I18N::Game::Invite);
    model.inviteLabel = Text(I18N::Game::Invite);
    m_Binder.MarkDirty("invite_button_label");
    m_Binder.MarkDirty("invite_label");

    SyncWorkspace();
    m_Document->UpdateDocument();
    SyncGeometry();
    m_ScrollToEnd = true;
}

void ChatRoomView::Unload()
{
    auto* context = RmlUiRuntime::Instance().GetContext();
    if (!m_Document)
    {
        if (context)
            m_Binder.Destroy(context);
        return;
    }
    if (auto* focused = context->GetFocusElement(); focused && focused->GetOwnerDocument() == m_Document)
        focused->Blur();
    m_Document->RemoveEventListener(Rml::EventId::Mousedown, this);
    m_Document->RemoveEventListener(Rml::EventId::Handledrag, this);
    context->UnloadDocument(m_Document);
    m_Document = nullptr;
    m_Placed = false;
    m_Settled = false;
    m_Binder.Destroy(context);
}

void ChatRoomView::ReloadTheme()
{
    if (!m_Document)
        return;
    Model model = m_Binder.GetModel();
    Unload();
    m_Binder.GetModel() = std::move(model);
    Build();
}

bool ChatRoomView::Sync(bool shown)
{
    if (!shown)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_Document, false);
        return false;
    }
    Build();
    if (!m_Document)
        return false;
    if (m_Title != m_Owner.GetTitle())
        PublishTitle();
    const bool wasVisible = m_Document->IsVisible();
    SyncWorkspace();
    SyncGeometry();
    if (!m_Placed && m_Width > 0 && m_Height > 0)
    {
        m_Placed = true;
        if (!m_CustomPosition)
            PlaceAtRest();
        ApplyLayout();
        m_Document->UpdateDocument();
        SyncGeometry();
    }
    else if (m_Placed)
        m_Settled = true;
    // window_shell places #panel through the data model, which the context applies only after this
    // runs, so a document shown on the frame it is placed still renders once where .center-both
    // left it -- centred and unsized.
    UI::RmlBridge::SyncDocumentVisibility(m_Document, m_Settled);
    if (m_Settled && m_ScrollToEnd)
    {
        m_ScrollToEnd = false;
        ScrollLogToEnd();
    }
    if (m_Settled && m_FocusField)
    {
        m_FocusField = false;
        if (auto* field = Field())
            field->Focus();
    }
    return m_Document->IsVisible() && !wasVisible;
}

void ChatRoomView::PullToFront()
{
    if (m_Document)
        m_Document->PullToFront();
}

void ChatRoomView::PublishTitle()
{
    m_Title = m_Owner.GetTitle();
    m_Binder.GetModel().title = Text(m_Title.c_str());
    m_Binder.MarkDirty("title");
}

// ---------------------------------------------------------------------------------------------
// Participants

int ChatRoomView::AddPal(const wchar_t* name, BYTE number)
{
    auto& m = m_Binder.GetModel();
    const auto narrow = Text(name);
    const auto it = std::find_if(m.pals.begin(), m.pals.end(),
                                 [&](const auto& pal) { return pal.name == narrow; });
    if (it != m.pals.end())
        it->number = number;
    else
        m.pals.push_back({narrow, number});
    m_Binder.MarkDirty("pals");
    SyncPalVisibility();
    return static_cast<int>(m.pals.size());
}

void ChatRoomView::RemovePal(const wchar_t* name)
{
    auto& m = m_Binder.GetModel();
    const auto narrow = Text(name);
    std::erase_if(m.pals, [&](const auto& pal) { return pal.name == narrow; });
    m_Binder.MarkDirty("pals");
    SyncPalVisibility();
}

int ChatRoomView::PalCount() const
{
    return static_cast<int>(m_Binder.GetModel().pals.size());
}

// Native only showed the column once the room was more than a pair, or while inviting.
void ChatRoomView::SyncPalVisibility()
{
    auto& m = m_Binder.GetModel();
    const bool show = m.pals.size() > 2 || m.showInvite;
    if (m.showPals == show)
        return;
    m.showPals = show;
    m_Binder.MarkDirty("show_pals");
}

const wchar_t* ChatRoomView::ChatFriend(int* result)
{
    const auto& m = m_Binder.GetModel();
    if (m.pals.size() > 2)
    {
        if (result)
            *result = 2;
        return nullptr;
    }
    const auto self = Text(Hero->ID);
    for (const auto& pal : m.pals)
    {
        if (pal.name == self)
            continue;
        m_NameLookup = StringUtils::NarrowToWide(pal.name);
        return m_NameLookup.c_str();
    }
    return nullptr;
}

// CUIChatPalListBox::MakeTitleText: up to three names other than the player's, then an ellipsis.
// Its own separator was ", L" -- a stray literal prefix left in the original, corrected here.
void ChatRoomView::MakeTitleText(wchar_t* out, size_t capacity) const
{
    if (!out || capacity == 0)
        return;
    const auto self = Text(Hero->ID);
    std::wstring built;
    int named = 0;
    for (const auto& pal : m_Binder.GetModel().pals)
    {
        if (pal.name == self)
            continue;
        if (named > 0)
            built += L", ";
        built += StringUtils::NarrowToWide(pal.name);
        if (++named >= 3)
        {
            built += L"...";
            break;
        }
    }
    const size_t used = wcslen(out);
    if (used + 1 >= capacity)
        return;
    wcsncat(out, built.c_str(), capacity - used - 1);
}

// ---------------------------------------------------------------------------------------------
// Lines

void ChatRoomView::AddLine(BYTE byIndex, const wchar_t* text, int type)
{
    if (!text || text[0] == L'\0')
        return;
    auto& m = m_Binder.GetModel();
    Rml::String speaker;
    if (byIndex != SystemSpeaker)
    {
        const auto it = std::find_if(m.pals.begin(), m.pals.end(),
                                     [byIndex](const auto& pal) { return pal.number == byIndex; });
        if (it != m.pals.end())
            speaker = it->name;
    }
    // Native pushed each line to the front of a list drawn bottom-up, so arrival order ran down
    // the box: appended here and read top-down, which is the same order.
    m.lines.push_back({speaker, Text(text), type});
    m_Binder.MarkDirty("lines");
    m_ScrollToEnd = true;
}

void ChatRoomView::ScrollLogToEnd()
{
    auto* log = m_Document ? m_Document->GetElementById("chat_log") : nullptr;
    if (!log)
        return;
    m_Document->UpdateDocument();
    log->SetScrollTop(log->GetScrollHeight());
}

void ChatRoomView::SetLocked(bool locked)
{
    auto& m = m_Binder.GetModel();
    if (m.locked == locked)
        return;
    m.locked = locked;
    m_Binder.MarkDirty("locked");
}

void ChatRoomView::FocusField()
{
    m_FocusField = true;
}

Rml::Element* ChatRoomView::Field() const
{
    return m_Document ? m_Document->GetElementById("chat_field") : nullptr;
}

// ---------------------------------------------------------------------------------------------
// Invitations

bool ChatRoomView::InviteShown() const
{
    return m_Binder.GetModel().showInvite;
}

void ChatRoomView::RefreshInviteList()
{
    auto& m = m_Binder.GetModel();
    std::deque<GUILDLIST_TEXT> entries;
    g_pFriendList->UpdateFriendList(entries, nullptr);
    m.invitePals.clear();
    const auto self = Text(Hero->ID);
    for (const auto& entry : entries)
    {
        // Offline friends and anyone already in the room cannot be invited.
        if (entry.m_Server >= 0xFC)
            continue;
        const auto narrow = Text(entry.m_szID);
        if (narrow == self)
            continue;
        if (std::any_of(m.pals.begin(), m.pals.end(), [&](const auto& pal) { return pal.name == narrow; }))
            continue;
        m.invitePals.push_back({narrow, entry.m_Number});
    }
    if (std::none_of(m.invitePals.begin(), m.invitePals.end(),
                     [&](const auto& pal) { return pal.name == m.selectedInvite; }))
        m.selectedInvite.clear();
    m_Binder.MarkDirty("invite_pals");
    m_Binder.MarkDirty("selected_invite");
}

const wchar_t* ChatRoomView::SelectedInvite()
{
    const auto& selected = m_Binder.GetModel().selectedInvite;
    if (selected.empty())
        return nullptr;
    m_SelectedInvite = StringUtils::NarrowToWide(selected);
    return m_SelectedInvite.c_str();
}

void ChatRoomView::ToggleInvite()
{
    auto& m = m_Binder.GetModel();
    // Measured before the column is hidden, so closing gives back exactly what opening took.
    const float column = m.showInvite ? InviteColumnWidth() : 0.f;
    m.showInvite = !m.showInvite;
    m.inviteButtonLabel = Text(m.showInvite ? I18N::Game::CloseInvitation : I18N::Game::Invite);
    m_Binder.MarkDirty("show_invite");
    m_Binder.MarkDirty("invite_button_label");
    if (m.showInvite)
        RefreshInviteList();
    SyncPalVisibility();
    m_Maximized = false;
    m.maximized = false;
    m_Binder.MarkDirty("maximized");

    // Native widened the window to make room rather than squeezing the log, then pulled it back
    // on-screen if that pushed its right edge off.
    if (!m_Document)
        return;
    m_Document->UpdateDocument();
    const float opened = m.showInvite ? InviteColumnWidth() : column;
    if (opened <= 0)
        return;
    m_CustomSize = true;
    RestoreLayout(m_Left, m_Top, m_Width + (m.showInvite ? opened : -opened), m_Height, true);
    m_Document->UpdateDocument();
    SyncGeometry();
}

void ChatRoomView::InviteSelected()
{
    const auto& m = m_Binder.GetModel();
    if (m.locked || m.selectedInvite.empty())
        return;
    if (m.pals.size() <= 1)
        return;
    if (m.pals.size() >= 30)
    {
        AddLine(SystemSpeaker, I18N::Game::YouHaveReachedTheMaximumNumberOfFriendsYouCanList, 1);
        return;
    }
    const auto name = StringUtils::NarrowToWide(m.selectedInvite);
    SocketClient->ToGameServer()->SendChatRoomInvitationRequest(MU_C16(name.c_str()), m_Owner.GetRoomNumber(),
                                                               m_Owner.GetUIID());
}

// ---------------------------------------------------------------------------------------------
// Input

bool ChatRoomView::FieldHasFocus() const
{
    auto* field = Field();
    return field != nullptr && field->IsPseudoClassSet("focus");
}

// The native path, kept whole: the same line twice running is dropped rather than sent again
// (m_szLastText), the field clears either way, and a room left alone locks itself.
void ChatRoomView::SubmitDraft()
{
    auto& m = m_Binder.GetModel();
    const auto line = StringUtils::NarrowToWide(m.draft);
    if (line != m_LastSent)
    {
        m_LastSent = line;
        if (!m.locked && !line.empty())
        {
            if (auto* connection = m_Owner.GetCurrentSocket())
                connection->ToChatServer()->SendChatMessageExt(0, line.c_str());
        }
    }
    m.draft.clear();
    m_Binder.MarkDirty("draft");
    if (m.pals.size() < 2)
        m_Owner.Lock(TRUE);
}

float ChatRoomView::InviteColumnWidth() const
{
    auto* pane = m_Document ? m_Document->GetElementById("invite_pane") : nullptr;
    if (!pane)
        return 0.f;
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    const float width = pane->GetBox().GetSize(Rml::BoxArea::Border).x;
    return scale > 0 ? width / scale : 0.f;
}

void ChatRoomView::ProcessEvent(Rml::Event& event)
{
    if (event.GetId() == Rml::EventId::Mousedown)
    {
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_Owner.GetUIID(), 0);
    }
    else if (event.GetId() == Rml::EventId::Handledrag)
    {
        m_CustomSize = true;
        m_Maximized = false;
        m_Binder.GetModel().maximized = false;
        m_Binder.MarkDirty("maximized");
    }
}

void ChatRoomView::ProcessActions()
{
    if (m_Actions.empty())
        return;
    std::vector<Action> actions;
    actions.swap(m_Actions);
    for (const auto& action : actions)
        ActionRequested(action);
}

void ChatRoomView::ActionRequested(const Action& a)
{
    auto& m = m_Binder.GetModel();
    if (a.name == "invite_send_clicked")
    {
        PlayBuffer(SOUND_CLICK01);
        InviteSelected();
    }
    else if (a.name == "invite_toggle")
        ToggleInvite();
    else if (a.name == "invite_send")
        InviteSelected();
    else if (a.name == "invite_select")
    {
        const auto name = a.value.Get<Rml::String>();
        if (std::any_of(m.invitePals.begin(), m.invitePals.end(),
                        [&](const auto& pal) { return pal.name == name; }))
        {
            m.selectedInvite = name;
            m_Binder.MarkDirty("selected_invite");
        }
    }
    else if (a.name == "close")
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, m_Owner.GetUIID(), 0);
    else if (a.name == "minimize")
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, m_Owner.GetUIID(), 0);
    else if (a.name == "maximize")
        Maximize();
}

// ---------------------------------------------------------------------------------------------
// Geometry -- the same bookkeeping FriendShell does; see this class's own header comment.

Rml::Element* ChatRoomView::Panel() const
{
    return m_Document;
}

void ChatRoomView::SyncGeometry()
{
    auto* panel = Panel();
    if (!panel)
        return;
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
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

void ChatRoomView::SyncWorkspace()
{
    auto* context = m_Document->GetContext();
    const auto viewport = context->GetDimensions();
    const float scale = context->GetDensityIndependentPixelRatio();
    const float height = UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight) *
                         UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight).scaleY;
    auto& model = m_Binder.GetModel();
    if (model.workspaceHeight == height && m_Viewport == viewport && m_DpRatio == scale)
        return;
    model.workspaceHeight = height;
    m_Binder.MarkDirty("workspace_height");
    m_Viewport = viewport;
    m_DpRatio = scale;
    if (m_CustomPosition)
        ApplyLayout();
    m_Document->UpdateDocument();
}

void ChatRoomView::PublishPosition()
{
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    auto& model = m_Binder.GetModel();
    model.rootX = m_Left * scale;
    model.rootY = m_Top * scale;
    m_Binder.MarkDirty("root_x");
    m_Binder.MarkDirty("root_y");
}

void ChatRoomView::ClampToWorkspace()
{
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float maxLeft = WindowWidth / scale - m_Width;
    const float maxTop = m_Binder.GetModel().workspaceHeight / scale - m_Height;
    if (maxLeft > 0)
        m_Left = std::clamp(m_Left, 0.f, maxLeft);
    if (maxTop > 0)
        m_Top = std::clamp(m_Top, 0.f, maxTop);
}

void ChatRoomView::ApplyLayout()
{
    auto* panel = Panel();
    if (!panel)
        return;
    ClampToWorkspace();
    PublishPosition();
    if (m_CustomSize)
    {
        panel->SetProperty("width", Rml::ToString(m_Width) + "dp");
        panel->SetProperty("height", Rml::ToString(m_Height) + "dp");
    }
}

void ChatRoomView::RestoreLayout(float x, float y, float width, float height, bool resize)
{
    m_CustomSize |= resize;
    m_Width = width;
    m_Height = height;
    m_Left = x;
    m_Top = y;
    m_CustomPosition = true;
    if (!m_Document)
        return;
    ClampToWorkspace();
    ApplyLayout();
}

void ChatRoomView::RestoreMaximized(bool maximized, float top, float height)
{
    m_Maximized = maximized;
    m_RestoreRect = {m_Left, top, m_Width, height};
    m_Binder.GetModel().maximized = maximized;
    m_Binder.MarkDirty("maximized");
}

// Rooms cascade down from the top-left, as CUIWindowMgr::AddWindow() steps each new window by 20.
void ChatRoomView::PlaceAtRest()
{
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float ratio = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight).scaleX / scale;
    m_Left = m_Owner.GetPosition_x() * ratio;
    m_Top = m_Owner.GetPosition_y() * ratio;
}

void ChatRoomView::Maximize()
{
    if (!m_Document)
        return;
    if (!m_Maximized)
    {
        m_RestoreRect = {m_Left, m_Top, m_Width, m_Height};
        m_CustomSize = true;
        RestoreLayout(m_Left, 0, m_Width,
                      m_Binder.GetModel().workspaceHeight /
                          m_Document->GetContext()->GetDensityIndependentPixelRatio());
    }
    else
        RestoreLayout(m_RestoreRect[0], m_RestoreRect[1], m_RestoreRect[2], m_RestoreRect[3]);
    m_Maximized = !m_Maximized;
    m_Binder.GetModel().maximized = m_Maximized;
    m_Binder.MarkDirty("maximized");
    m_Document->UpdateDocument();
    SyncGeometry();
}

void ChatRoomView::SyncDraggedPosition()
{
    if (!m_Document)
        return;
    m_Document->UpdateDocument();
    SyncGeometry();
    ClampToWorkspace();
    m_CustomPosition = true;
    PublishPosition();
}

} // namespace UI::Social
