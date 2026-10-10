#include "stdafx.h"
#include "UI/Social/LetterReadView.h"

#include "UI/Social/SocialWindowManager.h"
#include "UI/Social/SocialWorkspace.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include <RmlUi/Core.h>
#include <algorithm>

namespace UI::Social
{
namespace
{

Rml::String Text(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}
} // namespace

LetterReadView::LetterReadView(CUILetterReadWindow& owner) : m_Owner(owner)
{
}

LetterReadView::~LetterReadView()
{
    Unload();
}

void LetterReadView::RegisterModel(Rml::DataModelConstructor& c, Model& m)
{
    m.Bind(c);
    c.BindEventCallback("letter_action",
                        [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                        {
                            if (args.empty())
                                return;
                            event.StopPropagation();
                            m_Actions.push_back({args[0].Get<Rml::String>()});
                        });
}

void LetterReadView::Build()
{
    if (m_View.ModelName().empty())
        m_View.SetModelName("letter_read_" + Rml::ToString(static_cast<int>(m_Owner.GetUIID())));
    m_View.Ensure();
}

void LetterReadView::OnBuilt()
{
    m_View.Document()->AddEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->AddEventListener(Rml::EventId::Handledrag, this);
    // A strip of its own rather than window_shell's rail: the rail's 67dp is the banner art's
    // height and reaches down over the rows, so a press on a label would drag the window.
    auto* grip = m_View.Document()->GetElementById("drag_strip");
    if (!grip)
        grip = m_View.Document()->GetElementById("window_shell_header");
    if (grip)
        UI::RmlBridge::MakeDraggable(grip, m_View.Document(), nullptr, [this] { SyncDraggedPosition(); });

    auto& model = m_View.GetModel();
    model.replyLabel = Text(I18N::Game::Reply);
    model.deleteLabel = Text(I18N::Game::Delete);
    model.closeLabel = Text(I18N::Game::Close388);
    model.prevLabel = Text(I18N::Game::Previous);
    model.nextLabel = Text(I18N::Game::Next);
    for (const char* key : {"reply_label", "delete_label", "close_label", "prev_label", "next_label"})
        m_View.MarkDirty(key);

    SyncWorkspace();
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}

void LetterReadView::OnUnload()
{
    m_PhotoControl.Detach();
    m_View.Document()->RemoveEventListener(Rml::EventId::Mousedown, this);
    m_View.Document()->RemoveEventListener(Rml::EventId::Handledrag, this);
    m_Placed = false;
    m_Settled = false;
}

void LetterReadView::Unload()
{
    m_PhotoControl.Detach();
    m_View.Release();
    m_WorkspaceHeight = 0;
}

void LetterReadView::SetLetter(const wchar_t* sender, const wchar_t* date, const wchar_t* time,
                               const wchar_t* body)
{
    auto& m = m_View.GetModel();
    wchar_t header[256] = {0};
    mu_swprintf(header, I18N::Game::SenderSSS, sender, date, time);
    m.header = Text(header);
    m.lines.clear();
    // Split on newlines exactly as SetLetter() fed the native box line by line; the wrapping the
    // box did with CutText3 is RmlUi's job now.
    std::wstring text(body ? body : L"");
    size_t start = 0;
    while (start <= text.size())
    {
        const size_t end = text.find(L'\n', start);
        const std::wstring line = text.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start);
        m.lines.push_back({Text(line.c_str())});
        if (end == std::wstring::npos)
            break;
        start = end + 1;
    }
    m_View.MarkDirty("header");
    m_View.MarkDirty("lines");
}

bool LetterReadView::Sync(bool shown)
{
    if (!shown)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_View.Document(), false);
        m_PhotoControl.Suspend();
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
    SyncPhoto();
    return m_View.Document()->IsVisible() && !wasVisible;
}




// RCSS owns where the portrait sits; the native viewer is told to follow that box.
void LetterReadView::SyncPhoto()
{
    auto* slot = m_View.Document()->GetElementById("photo_slot");
    if (!slot)
        return;
    m_PhotoControl.Attach(m_View.Document(), m_Owner.m_Photo);
    // Until the panel has settled its slot is still where window_shell centred it.
    if (!m_Settled)
        return;
    m_PhotoControl.Sync();
}

void LetterReadView::PullToFront()
{
    if (m_View.Document())
        m_View.Document()->PullToFront();
}

void LetterReadView::ProcessEvent(Rml::Event& event)
{
    if (event.GetId() == Rml::EventId::Mousedown)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_Owner.GetUIID(), 0);
    else if (event.GetId() == Rml::EventId::Handledrag)
    {
        m_CustomSize = true;
        m_Maximized = false;
    }
}

bool LetterReadView::PointerOverPhoto() const
{
    Rml::ElementDocument* document = m_View.Document();
    return m_Settled && document && UI::RmlBridge::IsPointerWithin(document->GetElementById("photo_slot"));
}

bool LetterReadView::PointerOver() const
{
    return UI::RmlBridge::IsPointerOver(m_View.Document());
}

void LetterReadView::ProcessActions()
{
    if (m_Actions.empty())
        return;
    std::vector<Action> actions;
    actions.swap(m_Actions);
    for (const auto& action : actions)
        ActionRequested(action);
}

void LetterReadView::ActionRequested(const Action& a)
{
    if (a.name == "reply")
        m_Owner.Reply();
    else if (a.name == "delete")
        m_Owner.AskDelete();
    else if (a.name == "close")
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, m_Owner.GetUIID(), 0);
    else if (a.name == "prev")
        m_Owner.StepLetter(-1);
    else if (a.name == "next")
        m_Owner.StepLetter(1);
    else if (a.name == "minimize")
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, m_Owner.GetUIID(), 0);
    else if (a.name == "maximize")
        Maximize();
}

// ---------------------------------------------------------------------------------------------
// Geometry in RmlUi dp, mirrored into the manager's native reference coordinates.

Rml::Element* LetterReadView::Panel() const
{
    return m_View.Document();
}

void LetterReadView::SyncGeometry()
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
    m_Owner.SetPosition(static_cast<int>(m_Left), static_cast<int>(m_Top));
    m_Owner.SetSize(static_cast<int>(m_Width), static_cast<int>(m_Height));
}

void LetterReadView::SyncWorkspace()
{
    auto* context = m_View.Document()->GetContext();
    const auto viewport = context->GetDimensions();
    const float scale = context->GetDensityIndependentPixelRatio();
    const float height = FreeAreaBottomPx();
    if (m_WorkspaceHeight == height && m_Viewport == viewport && m_DpRatio == scale)
        return;
    m_WorkspaceHeight = height;
    m_Viewport = viewport;
    m_DpRatio = scale;
    if (m_CustomPosition)
        ApplyLayout();
    m_View.Document()->UpdateDocument();
}

void LetterReadView::PublishPosition()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    auto& model = m_View.GetModel();
    model.rootX = m_Left * scale;
    model.rootY = m_Top * scale;
    m_View.MarkDirty("root_x");
    m_View.MarkDirty("root_y");
}

void LetterReadView::ClampToWorkspace()
{
    const float scale = m_View.Document()->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float maxLeft = WindowWidth / scale - m_Width;
    const float maxTop = m_WorkspaceHeight / scale - m_Height;
    if (maxLeft > 0)
        m_Left = std::clamp(m_Left, 0.f, maxLeft);
    if (maxTop > 0)
        m_Top = std::clamp(m_Top, 0.f, maxTop);
}

void LetterReadView::ApplyLayout()
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

void LetterReadView::RestoreLayout(float x, float y, float width, float height, bool resize)
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

// Where the manager cascaded it, in dp.
void LetterReadView::PlaceAtRest()
{
    m_Left = m_Owner.GetPosition_x();
    m_Top = m_Owner.GetPosition_y();
}

void LetterReadView::Maximize()
{
    if (!m_View.Document())
        return;
    if (!m_Maximized)
    {
        m_RestoreRect = {m_Left, m_Top, m_Width, m_Height};
        m_CustomSize = true;
        RestoreLayout(m_Left, 0, m_Width,
                      m_WorkspaceHeight /
                          m_View.Document()->GetContext()->GetDensityIndependentPixelRatio());
    }
    else
        RestoreLayout(m_RestoreRect[0], m_RestoreRect[1], m_RestoreRect[2], m_RestoreRect[3]);
    m_Maximized = !m_Maximized;
    m_View.Document()->UpdateDocument();
    SyncGeometry();
}

void LetterReadView::SyncDraggedPosition()
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
