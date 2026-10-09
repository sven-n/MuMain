#include "stdafx.h"
#include "UI/Social/ChatRoomView.h"

#include "UI/Social/SocialWindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlPointer.h"
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
constexpr int SystemSpeaker = 255;
Rml::String Text(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}
} // namespace

ChatRoomView::ChatRoomView(CUIChatWindow& owner) : m_Owner(owner)
{
}

ChatRoomView::~ChatRoomView()
{
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
    // One model per window: two rooms sharing a name would share their state.
    if (m_View.ModelName().empty())
        m_View.SetModelName("chat_room_" + Rml::ToString(static_cast<int>(m_Owner.GetUIID())));
    m_View.Ensure();
}

void ChatRoomView::OnBuilt()
{
    m_View.Document()->AddEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->AddEventListener(Rml::EventId::Handledrag, this);
    if (auto* header = m_View.Document()->GetElementById("window_shell_header"))
        UI::RmlBridge::MakeDraggable(header, m_View.Document(), nullptr, [this] { SyncDraggedPosition(); });

    auto& model = m_View.GetModel();
    model.inviteButtonLabel = Text(model.showInvite ? I18N::Game::CloseInvitation : I18N::Game::Invite);
    model.inviteLabel = Text(I18N::Game::Invite);
    m_View.MarkDirty("invite_button_label");
    m_View.MarkDirty("invite_label");

    SyncWorkspace();
    m_View.Document()->UpdateDocument();
    SyncGeometry();
    m_ScrollToEnd = true;
}

void ChatRoomView::OnUnload()
{
    m_View.Document()->RemoveEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->RemoveEventListener(Rml::EventId::Handledrag, this);
    m_Placed = false;
    m_Settled = false;
}

void ChatRoomView::Unload()
{
    m_View.Release();
}

bool ChatRoomView::Sync(bool shown)
{
    if (!shown)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), false);
        return false;
    }
    Build();
    if (!m_View.Document())
        return false;
    if (m_Title != m_Owner.GetTitle())
        PublishTitle();
    const bool wasVisible = m_View.Document()->IsVisible();
    SyncWorkspace();
    SyncGeometry();
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
    return m_View.Document()->IsVisible() && !wasVisible;
}

void ChatRoomView::PullToFront()
{
    if (m_View.Document())
        m_View.Document()->PullToFront();
}

void ChatRoomView::PublishTitle()
{
    m_Title = m_Owner.GetTitle();
    m_View.GetModel().title = Text(m_Title.c_str());
    m_View.MarkDirty("title");
}

// ---------------------------------------------------------------------------------------------
// Participants

void ChatRoomView::AddPal(const wchar_t* name, BYTE number)
{
    auto& m = m_View.GetModel();
    const auto narrow = Text(name);
    const auto it = std::find_if(m.pals.begin(), m.pals.end(),
                                 [&](const auto& pal) { return pal.name == narrow; });
    if (it != m.pals.end())
        it->number = number;
    else
        m.pals.push_back({narrow, number});
    m_View.MarkDirty("pals");
    SyncPalVisibility();
}

void ChatRoomView::RemovePal(const wchar_t* name)
{
    auto& m = m_View.GetModel();
    const auto narrow = Text(name);
    std::erase_if(m.pals, [&](const auto& pal) { return pal.name == narrow; });
    m_View.MarkDirty("pals");
    SyncPalVisibility();
}

// Native only showed the column once the room was more than a pair, or while inviting.
void ChatRoomView::SyncPalVisibility()
{
    auto& m = m_View.GetModel();
    const bool show = m.pals.size() > 2 || m.showInvite;
    if (m.showPals == show)
        return;
    m.showPals = show;
    m_View.MarkDirty("show_pals");
}

// ---------------------------------------------------------------------------------------------
// Lines

void ChatRoomView::AddLine(BYTE byIndex, const wchar_t* text, int type)
{
    if (!text || text[0] == L'\0')
        return;
    auto& m = m_View.GetModel();
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
    m_View.MarkDirty("lines");
    m_ScrollToEnd = true;
}

void ChatRoomView::ScrollLogToEnd()
{
    auto* log = m_View.Document() ? m_View.Document()->GetElementById("chat_log") : nullptr;
    if (!log)
        return;
    m_View.Document()->UpdateDocument();
    log->SetScrollTop(log->GetScrollHeight());
}

void ChatRoomView::SetLocked(bool locked)
{
    auto& m = m_View.GetModel();
    if (m.locked == locked)
        return;
    m.locked = locked;
    m_View.MarkDirty("locked");
}

void ChatRoomView::FocusField()
{
    m_FocusField = true;
}

Rml::Element* ChatRoomView::Field() const
{
    return m_View.Document() ? m_View.Document()->GetElementById("chat_field") : nullptr;
}

// ---------------------------------------------------------------------------------------------
// Invitations

bool ChatRoomView::InviteShown() const
{
    return m_View.GetModel().showInvite;
}

void ChatRoomView::RefreshInviteList()
{
    auto& m = m_View.GetModel();
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
    m_View.MarkDirty("invite_pals");
    m_View.MarkDirty("selected_invite");
}

const wchar_t* ChatRoomView::SelectedInvite()
{
    const auto& selected = m_View.GetModel().selectedInvite;
    if (selected.empty())
        return nullptr;
    m_SelectedInvite = StringUtils::NarrowToWide(selected);
    return m_SelectedInvite.c_str();
}

void ChatRoomView::ToggleInvite()
{
    auto& m = m_View.GetModel();
    // Measured before the column is hidden, so closing gives back exactly what opening took.
    const float column = m.showInvite ? InviteColumnWidth() : 0.f;
    m.showInvite = !m.showInvite;
    m.inviteButtonLabel = Text(m.showInvite ? I18N::Game::CloseInvitation : I18N::Game::Invite);
    m_View.MarkDirty("show_invite");
    m_View.MarkDirty("invite_button_label");
    if (m.showInvite)
        RefreshInviteList();
    SyncPalVisibility();
    m_Maximized = false;
    m.maximized = false;
    m_View.MarkDirty("maximized");

    // Native widened the window to make room rather than squeezing the log, then pulled it back
    // on-screen if that pushed its right edge off.
    if (!m_View.Document())
        return;
    m_View.Document()->UpdateDocument();
    const float opened = m.showInvite ? InviteColumnWidth() : column;
    if (opened <= 0)
        return;
    m_CustomSize = true;
    RestoreLayout(m_Left, m_Top, m_Width + (m.showInvite ? opened : -opened), m_Height, true);
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}

// ---------------------------------------------------------------------------------------------
// Input

bool ChatRoomView::FieldHasFocus() const
{
    auto* field = Field();
    return field != nullptr && field->IsPseudoClassSet("focus");
}

void ChatRoomView::SubmitDraft()
{
    auto& m = m_View.GetModel();
    const auto line = StringUtils::NarrowToWide(m.draft);
    m_Owner.SubmitLine(line);
    m.draft.clear();
    m_View.MarkDirty("draft");
}

float ChatRoomView::InviteColumnWidth() const
{
    auto* pane = m_View.Document() ? m_View.Document()->GetElementById("invite_pane") : nullptr;
    if (!pane)
        return 0.f;
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
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
        m_View.GetModel().maximized = false;
        m_View.MarkDirty("maximized");
    }
}

bool ChatRoomView::PointerOver() const
{
    return UI::RmlBridge::IsPointerOver(m_View.Document());
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
    auto& m = m_View.GetModel();
    if (a.name == "invite_send_clicked")
    {
        PlayBuffer(SOUND_CLICK01);
        m_Owner.InviteSelected(StringUtils::NarrowToWide(m.selectedInvite));
    }
    else if (a.name == "invite_toggle")
        ToggleInvite();
    else if (a.name == "invite_send")
        m_Owner.InviteSelected(StringUtils::NarrowToWide(m.selectedInvite));
    else if (a.name == "invite_select")
    {
        const auto name = a.value.Get<Rml::String>();
        if (std::any_of(m.invitePals.begin(), m.invitePals.end(),
                        [&](const auto& pal) { return pal.name == name; }))
        {
            m.selectedInvite = name;
            m_View.MarkDirty("selected_invite");
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
    return m_View.Document();
}

void ChatRoomView::SyncGeometry()
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

void ChatRoomView::SyncWorkspace()
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
        ApplyLayout();
    m_View.Document()->UpdateDocument();
}

void ChatRoomView::PublishPosition()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    auto& model = m_View.GetModel();
    model.rootX = m_Left * scale;
    model.rootY = m_Top * scale;
    m_View.MarkDirty("root_x");
    m_View.MarkDirty("root_y");
}

void ChatRoomView::ClampToWorkspace()
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
    if (!m_View.Document())
        return;
    ClampToWorkspace();
    ApplyLayout();
}

void ChatRoomView::RestoreMaximized(bool maximized, float top, float height)
{
    m_Maximized = maximized;
    m_RestoreRect = {m_Left, top, m_Width, height};
    m_View.GetModel().maximized = maximized;
    m_View.MarkDirty("maximized");
}

// Rooms cascade down from the top-left, as CUIWindowMgr::AddWindow() steps each new window by 20.
void ChatRoomView::PlaceAtRest()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float ratio = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight).scaleX / scale;
    m_Left = m_Owner.GetPosition_x() * ratio;
    m_Top = m_Owner.GetPosition_y() * ratio;
}

void ChatRoomView::Maximize()
{
    if (!m_View.Document())
        return;
    if (!m_Maximized)
    {
        m_RestoreRect = {m_Left, m_Top, m_Width, m_Height};
        m_CustomSize = true;
        RestoreLayout(m_Left, 0, m_Width,
                      m_View.GetModel().workspaceHeight /
                          m_View.Document()->GetContext()->GetDensityIndependentPixelRatio());
    }
    else
        RestoreLayout(m_RestoreRect[0], m_RestoreRect[1], m_RestoreRect[2], m_RestoreRect[3]);
    m_Maximized = !m_Maximized;
    m_View.GetModel().maximized = m_Maximized;
    m_View.MarkDirty("maximized");
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}

void ChatRoomView::SyncDraggedPosition()
{
    if (!m_View.Document())
        return;
    m_View.Document()->UpdateDocument();
    SyncGeometry();
    ClampToWorkspace();
    m_CustomPosition = true;
    PublishPosition();
}

} // namespace UI::Social
