#include "stdafx.h"
#include "UI/Party/LetterWrite.h"

#include "UI/Party/UIWindows.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"
#include <RmlUi/Core.h>
#include <algorithm>

namespace UI::Party
{
namespace
{
constexpr const char* DocumentPath = "Data/Interface/RmlUi/letter_write.rml";
constexpr const char* ModelPlaceholder = "data-model=\"letter_write\"";
// Document order, which is also the order Tab walked the native boxes in.
constexpr const char* FieldIds[] = {"mailto_field", "subject_field", "body_field"};

Rml::String Text(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}
} // namespace

LetterWriteView::LetterWriteView(CUILetterWriteWindow& owner) : m_Owner(owner)
{
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadTheme(); });
}

LetterWriteView::~LetterWriteView()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    Unload();
}

void LetterWriteView::RegisterModel(Rml::DataModelConstructor& c, Model& m)
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

void LetterWriteView::Build()
{
    if (m_Document || !RmlUiRuntime::Instance().IsCreated())
        return;
    auto* context = RmlUiRuntime::Instance().GetContext();
    if (m_ModelName.empty())
        m_ModelName = "letter_write_" + Rml::ToString(static_cast<int>(m_Owner.GetUIID()));
    if (!m_Binder.Create(context, m_ModelName, [this](auto& c, auto& m) { RegisterModel(c, m); }))
        return;
    m_Document = UI::RmlBridge::LoadThemedDocument(context, DocumentPath, ModelPlaceholder,
                                                   "data-model=\"" + m_ModelName + "\"");
    if (!m_Document)
        return;
    m_Document->AddEventListener(Rml::EventId::Mousedown, this);
    m_Document->AddEventListener(Rml::EventId::Handledrag, this);
    // A strip of its own rather than window_shell's rail: the rail's 67dp is the banner art's
    // height and reaches down over the rows, so a press on a label would drag the window.
    auto* grip = m_Document->GetElementById("drag_strip");
    if (!grip)
        grip = m_Document->GetElementById("window_shell_header");
    if (grip)
        UI::RmlBridge::MakeDraggable(grip, m_Document, nullptr, [this] { SyncDraggedPosition(); });

    auto& model = m_Binder.GetModel();
    model.receiverLabel = Text(I18N::Game::Receiver);
    model.subjectLabel = Text(I18N::Game::Title1030);
    model.sendLabel = Text(I18N::Game::Send);
    model.closeLabel = Text(I18N::Game::Close388);
    model.prevPoseLabel = Text(I18N::Game::PrevAction);
    model.nextPoseLabel = Text(I18N::Game::NextAction);
    for (const char* key : {"receiver_label", "subject_label", "send_label", "close_label", "prev_pose_label",
                            "next_pose_label"})
        m_Binder.MarkDirty(key);

    SyncWorkspace();
    m_Document->UpdateDocument();
    SyncGeometry();
    // Native put the caret in the receiver box the moment the window existed.
    m_PendingFocus = 0;
}

void LetterWriteView::Unload()
{
    m_PhotoControl.Detach();
    auto* context = RmlUiRuntime::Instance().GetContext();
    if (!m_Document)
    {
        if (context)
            m_Binder.Destroy(context);
        return;
    }
    // A hidden document whose <input> still holds focus leaves the client believing text input is
    // active, and every hotkey dies with it.
    if (auto* focused = context->GetFocusElement(); focused && focused->GetOwnerDocument() == m_Document)
        focused->Blur();
    m_Document->RemoveEventListener(Rml::EventId::Mousedown, this);
    m_Document->RemoveEventListener(Rml::EventId::Handledrag, this);
    context->UnloadDocument(m_Document);
    m_Document = nullptr;
    m_Placed = false;
    m_Binder.Destroy(context);
}

void LetterWriteView::ReloadTheme()
{
    if (!m_Document)
        return;
    Model model = m_Binder.GetModel();
    Unload();
    m_Binder.GetModel() = std::move(model);
    Build();
}

// ---------------------------------------------------------------------------------------------
// Fields

std::wstring LetterWriteView::Mailto() const
{
    return StringUtils::NarrowToWide(m_Binder.GetModel().mailto);
}

std::wstring LetterWriteView::Subject() const
{
    return StringUtils::NarrowToWide(m_Binder.GetModel().subject);
}

std::wstring LetterWriteView::Body() const
{
    return StringUtils::NarrowToWide(m_Binder.GetModel().body);
}

void LetterWriteView::SetMailto(const wchar_t* text)
{
    m_Binder.GetModel().mailto = Text(text);
    m_Binder.MarkDirty("mailto");
    // SetMailtoText moved the caret on to the title, as native did.
    FocusField(1);
}

void LetterWriteView::SetSubject(const wchar_t* text)
{
    m_Binder.GetModel().subject = Text(text);
    m_Binder.MarkDirty("subject");
    FocusField(2);
}

void LetterWriteView::SetBody(const wchar_t* text)
{
    m_Binder.GetModel().body = Text(text);
    m_Binder.MarkDirty("body");
}

void LetterWriteView::FocusField(int index)
{
    m_PendingFocus = std::clamp(index, 0, 2);
    m_LastFocus = m_PendingFocus;
}

// UI_MESSAGE_SELECTED put the caret back where it was.
void LetterWriteView::RestoreFocus()
{
    m_PendingFocus = m_LastFocus;
}

bool LetterWriteView::AnyFieldHasFocus() const
{
    if (!m_Document)
        return false;
    for (const char* id : FieldIds)
    {
        auto* field = m_Document->GetElementById(id);
        if (field && field->IsPseudoClassSet("focus"))
            return true;
    }
    return false;
}

void LetterWriteView::SetSending(bool sending)
{
    auto& m = m_Binder.GetModel();
    if (m.sending == sending)
        return;
    m.sending = sending;
    m_Binder.MarkDirty("sending");
}

// ---------------------------------------------------------------------------------------------

bool LetterWriteView::Sync(bool shown)
{
    if (!shown)
    {
        UI::RmlBridge::SyncDocumentVisibility(m_Document, false);
        return false;
    }
    Build();
    if (!m_Document)
        return false;
    auto& model = m_Binder.GetModel();
    if (m_Title != m_Owner.GetTitle())
    {
        m_Title = m_Owner.GetTitle();
        model.title = Text(m_Title.c_str());
        m_Binder.MarkDirty("title");
    }
    const bool wasVisible = m_Document->IsVisible();
    UI::RmlBridge::SyncDocumentVisibility(m_Document, true);
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
    SyncPhoto();
    if (m_PendingFocus >= 0)
    {
        // Deliberately after the document is shown: a hidden document cannot take focus.
        if (auto* field = m_Document->GetElementById(FieldIds[m_PendingFocus]))
            field->Focus();
        m_PendingFocus = -1;
    }
    return !wasVisible;
}




// RCSS owns where the portrait sits; the native viewer is told to follow that box.
void LetterWriteView::SyncPhoto()
{
    auto* slot = m_Document->GetElementById("photo_slot");
    if (!slot)
        return;
    m_PhotoControl.Attach(m_Document, m_Owner.m_Photo);
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    const auto native = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight);
    if (scale <= 0 || native.scaleX <= 0)
        return;
    const float ratio = scale / native.scaleX;
    const auto size = slot->GetBox().GetSize(Rml::BoxArea::Border);
    m_Owner.m_Photo.SetPosition(static_cast<int>(slot->GetAbsoluteLeft() / scale * ratio),
                                static_cast<int>(slot->GetAbsoluteTop() / scale * ratio));
    m_Owner.m_Photo.SetSize(static_cast<int>(size.x / scale * ratio), static_cast<int>(size.y / scale * ratio));
}

void LetterWriteView::PullToFront()
{
    if (m_Document)
        m_Document->PullToFront();
}

void LetterWriteView::ProcessEvent(Rml::Event& event)
{
    if (event.GetId() == Rml::EventId::Mousedown)
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, m_Owner.GetUIID(), 0);
    else if (event.GetId() == Rml::EventId::Handledrag)
    {
        m_CustomSize = true;
        m_Maximized = false;
        m_Binder.GetModel().maximized = false;
        m_Binder.MarkDirty("maximized");
    }
}

void LetterWriteView::ProcessActions()
{
    if (m_Actions.empty())
        return;
    std::vector<Action> actions;
    actions.swap(m_Actions);
    for (const auto& action : actions)
        ActionRequested(action);
}

void LetterWriteView::ActionRequested(const Action& a)
{
    if (a.name == "send")
        m_Owner.Send();
    else if (a.name == "close")
        m_Owner.RequestClose();
    else if (a.name == "prev_pose")
        m_Owner.m_Photo.ChangeAnimation(-1);
    else if (a.name == "next_pose")
        m_Owner.m_Photo.ChangeAnimation(1);
    else if (a.name == "minimize")
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_HIDE, m_Owner.GetUIID(), 0);
    else if (a.name == "maximize")
        Maximize();
}

// ---------------------------------------------------------------------------------------------
// Geometry -- see LetterRead.cpp's own copy; all four views get lifted into one host once the
// transcription layer goes.

Rml::Element* LetterWriteView::Panel() const
{
    return m_Document;
}

void LetterWriteView::SyncGeometry()
{
    auto* panel = Panel();
    if (!panel)
        return;
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    const auto size = panel->GetBox().GetSize(Rml::BoxArea::Border);
    if (scale <= 0 || size.x <= 0 || size.y <= 0)
        return;
    m_Left = panel->GetAbsoluteLeft() / scale;
    m_Top = panel->GetAbsoluteTop() / scale;
    m_Width = size.x / scale;
    m_Height = size.y / scale;
    const auto native = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight);
    const float ratio = scale / native.scaleX;
    m_Owner.SetPosition(static_cast<int>(m_Left * ratio), static_cast<int>(m_Top * ratio));
    m_Owner.SetSize(static_cast<int>(m_Width * ratio), static_cast<int>(m_Height * ratio));
}

void LetterWriteView::SyncWorkspace()
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

void LetterWriteView::PublishPosition()
{
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    auto& model = m_Binder.GetModel();
    model.rootX = m_Left * scale;
    model.rootY = m_Top * scale;
    m_Binder.MarkDirty("root_x");
    m_Binder.MarkDirty("root_y");
}

void LetterWriteView::ClampToWorkspace()
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

void LetterWriteView::ApplyLayout()
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

void LetterWriteView::RestoreLayout(float x, float y, float width, float height, bool resize)
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

void LetterWriteView::PlaceAtRest()
{
    const float scale = m_Document->GetContext()->GetDensityIndependentPixelRatio();
    if (scale <= 0)
        return;
    const float ratio = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight).scaleX / scale;
    m_Left = m_Owner.GetPosition_x() * ratio;
    m_Top = m_Owner.GetPosition_y() * ratio;
}

void LetterWriteView::Maximize()
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

void LetterWriteView::SyncDraggedPosition()
{
    if (!m_Document)
        return;
    m_Document->UpdateDocument();
    SyncGeometry();
    ClampToWorkspace();
    m_CustomPosition = true;
    PublishPosition();
}

} // namespace UI::Party
