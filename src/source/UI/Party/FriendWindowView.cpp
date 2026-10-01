#include "stdafx.h"

#include "UI/Party/FriendWindowView.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Party/UIWindows.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/Text/CUIRenderText.h"

#include <algorithm>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Elements/ElementFormControlTextArea.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Input.h>

namespace
{
constexpr const char* DocumentPath = "Data/Interface/RmlUi/friend_window.rml";
constexpr const char* ModelPlaceholder = "data-model=\"friend_window\"";
} // namespace

FriendWindowRmlBuilder::FriendWindowRmlBuilder(int originX, int originY, std::vector<FriendWindowPart>& parts,
                                               std::vector<FriendWindowPart>& underlayParts)
    : m_OriginX(originX), m_OriginY(originY), m_Transform(UI::Scaling::GetActiveTransform()), m_Main{&parts},
      m_Underlay{&underlayParts}
{
}

namespace
{
template <typename T, typename U> void Assign(T& field, const U& value, bool& changed)
{
    if (field != value)
    {
        field = value;
        changed = true;
    }
}
} // namespace

FriendWindowPart& FriendWindowRmlBuilder::NextPart(int kind, const char* role, double x, double y, double width,
                                                   double height)
{
    PartList& list = *m_Active;
    if (list.count == list.parts->size())
    {
        list.parts->emplace_back();
        list.changed = true;
    }
    FriendWindowPart& part = (*list.parts)[list.count++];
    Assign(part.kind, kind, list.changed);
    if (part.role != role)
    {
        part.role = role;
        list.changed = true;
    }
    Assign(part.left, static_cast<float>(x - m_OriginX), list.changed);
    Assign(part.top, static_cast<float>(y - m_OriginY), list.changed);
    Assign(part.width, static_cast<float>(width), list.changed);
    Assign(part.height, static_cast<float>(height), list.changed);
    return part;
}

bool FriendWindowRmlBuilder::PartList::Finish()
{
    if (count < parts->size())
    {
        parts->resize(count);
        changed = true;
    }
    return changed;
}

void FriendWindowRmlBuilder::UnderlayFill(const char* role, double x, double y, double width, double height)
{
    m_Active = &m_Underlay;
    Fill(role, x, y, width, height);
    m_Active = &m_Main;
}

void FriendWindowRmlBuilder::Fill(const char* role, double x, double y, double width, double height)
{
    if (width <= 0.0 || height <= 0.0)
        return;
    NextPart(FriendWindowPart::Fill, role, x, y, width, height);
}

void FriendWindowRmlBuilder::Sprite(const char* role, double x, double y, double width, double height)
{
    if (width <= 0.0 || height <= 0.0)
        return;
    NextPart(FriendWindowPart::Sprite, role, x, y, width, height);
}

void FriendWindowRmlBuilder::Text(const wchar_t* text, double x, double y, DWORD color, bool bold, double boxWidth,
                                  int align)
{
    if (text == nullptr || text[0] == L'\0')
        return;
    // RenderText() takes whole units.
    FriendWindowPart& part =
        NextPart(FriendWindowPart::Text, "text", static_cast<int>(x), static_cast<int>(y), boxWidth, 0.0);
    if (part.sourceText != text || part.text.empty())
    {
        part.sourceText = text;
        part.text = StringUtils::WideToNarrow(text);
        m_Active->changed = true;
    }
    Assign(part.textPx,
           UI::Scaling::NativeTextPixelSize(bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal,
                                            m_Transform),
           m_Active->changed);
    Assign(part.align, align, m_Active->changed);
    Assign(part.bold, bold, m_Active->changed);
    if (part.sourceColor != color || part.color.empty())
    {
        part.sourceColor = color;
        part.color = UI::RmlBridge::RgbaToCss(color);
        m_Active->changed = true;
    }
}

void FriendWindowRmlBuilder::Field(int slot, CUITextInputBox& box)
{
    if (slot < 0 || slot >= FriendWindowFieldLayout::SlotCount || box.GetState() == UISTATE_HIDE)
        return;
    const auto x = static_cast<double>(box.GetPosition_x());
    const auto y = static_cast<double>(box.GetPosition_y());
    const auto w = static_cast<double>(box.GetWidth());
    const auto h = static_cast<double>(box.GetHeight());
    // RenderPortable()'s background: UIOPTION_PAINTBACK paints it black (the family's fields set
    // no other back colour).
    if (box.CheckOption(UIOPTION_PAINTBACK))
        Fill("field-back", x, y, w, h);

    FriendWindowFieldLayout& field = m_Fields[slot];
    field.shown = true;
    field.left = static_cast<float>(x - m_OriginX);
    field.top = static_cast<float>(y - m_OriginY);
    field.width = static_cast<float>(w);
    field.height = static_cast<float>(h);
    field.color = box.GetTextColor();
}

void FriendWindowRmlBuilder::Button(CUIButton& button)
{
    const auto x = static_cast<float>(button.GetPosition_x());
    const auto y = static_cast<float>(button.GetPosition_y());
    const auto w = static_cast<float>(button.GetWidth());
    const auto h = static_cast<float>(button.GetHeight());
    const bool disabled = button.GetState() == UISTATE_DISABLE;
    const bool pressed = !disabled && button.IsPressedLook();
    if (pressed)
        Sprite("button pressed", x + 1, y + 1, w - 1, h - 1);
    else
        Sprite(disabled ? "button disabled" : "button", x, y, w, h);

    const wchar_t* caption = button.GetCaption();
    if (caption == nullptr)
        return;
    g_pRenderText->SetFont(g_hFont);
    const SIZE size = g_pRenderText->MeasureText(caption, lstrlen(caption));
    const float left = (w - static_cast<float>(size.cx) + 0.5f) / 2;
    const float top = (h - static_cast<float>(size.cy) + 0.5f) / 2;
    if (pressed)
        Text(caption, x + 1 + left, y + 2 + top, RGBA(230, 220, 200, 255));
    else
        Text(caption, x + left, y + 1 + top, RGBA(230, 220, 200, 255));
}

void FriendWindowRmlBuilder::CheckBox(double x, double y, bool checked)
{
    Fill("check-edge", x, y, 9, 1);
    Fill("check-edge", x, y + 8, 9, 1);
    Fill("check-edge", x, y, 1, 9);
    Fill("check-edge", x + 8, y, 1, 9);
    if (checked)
        Sprite("check-mark", x + 2, y + 2, 5, 5);
}

template <typename List> void FriendWindowRmlBuilder::ListScrollBar(List& list)
{
    const TextListScrollBarGeometry bar = list.ComputeLegacyScrollBar();
    const auto x = static_cast<float>(list.GetPosition_x());
    const auto bottom = static_cast<float>(list.GetPosition_y());
    const auto top = bottom - static_cast<float>(list.GetHeight());
    const auto width = static_cast<float>(list.GetWidth());

    const bool upPressed =
        MouseLButtonPush && ::CheckMouseIn(static_cast<int>(x + width - 12), static_cast<int>(top - 1), 13, 13) == TRUE;
    Sprite(upPressed ? "scroll-up pressed" : "scroll-up", x + width - 12, top - 1, 13, 13);
    const bool downPressed = MouseLButtonPush && ::CheckMouseIn(static_cast<int>(x + width - 12),
                                                                static_cast<int>(bottom - 12), 13, 13) == TRUE;
    Sprite(downPressed ? "scroll-down pressed" : "scroll-down", x + width - 12, bottom - 12, 13, 13);

    Fill("scroll-track", x + width - bar.barWidth + 1, bar.rangeTop, 1, bar.rangeBottom - bar.rangeTop);
    Fill("scroll-track", x + width, bar.rangeTop, 1, bar.rangeBottom - bar.rangeTop);

    const float thumbX = x + width - bar.barWidth + 2;
    const float thumbWidth = bar.barWidth - 2;
    if (list.GetLineNum() >= list.GetBoxSize())
    {
        Sprite("scroll-thumb", thumbX, bar.thumbTop, thumbWidth, bar.thumbHeight);
        Sprite("scroll-thumb-top", thumbX, bar.thumbTop, thumbWidth, 1);
        Sprite("scroll-thumb-bottom", thumbX, bar.thumbTop + bar.thumbHeight - 1, thumbWidth, 1);
    }
    else
    {
        // The original filled the whole track and closed it at the thumb's height (not the track's).
        Sprite("scroll-thumb", thumbX, bar.rangeTop, thumbWidth, bar.rangeBottom - bar.rangeTop);
        Sprite("scroll-thumb-top", thumbX, bar.rangeTop, thumbWidth, 1);
        Sprite("scroll-thumb-bottom", thumbX, bar.rangeTop + bar.thumbHeight - 1, thumbWidth, 1);
    }
}

template void FriendWindowRmlBuilder::ListScrollBar<CUIChatPalListBox>(CUIChatPalListBox&);
template void FriendWindowRmlBuilder::ListScrollBar<CUIWindowListBox>(CUIWindowListBox&);
template void FriendWindowRmlBuilder::ListScrollBar<CUILetterListBox>(CUILetterListBox&);
template void FriendWindowRmlBuilder::ListScrollBar<CUISimpleChatListBox>(CUISimpleChatListBox&);
template void FriendWindowRmlBuilder::ListScrollBar<CUILetterTextListBox>(CUILetterTextListBox&);

namespace
{
// Forwards a field's edits and keys to its view.
class FriendWindowFieldListener : public Rml::EventListener
{
public:
    FriendWindowFieldListener(FriendWindowView& view, int slot) : m_View(view), m_Slot(slot) {}

    void ProcessEvent(Rml::Event& event) override
    {
        if (event.GetId() == Rml::EventId::Change)
        {
            m_View.OnFieldEdited(m_Slot);
        }
        else if (event.GetId() == Rml::EventId::Keydown)
        {
            if (m_View.OnFieldKey(m_Slot, event.GetParameter<int>("key_identifier", 0)))
                event.StopPropagation();
        }
    }

private:
    FriendWindowView& m_View;
    int m_Slot;
};

const char* const FieldIds[FriendWindowFieldLayout::SlotCount] = {"field-0", "field-1", "field-2"};

// Character (code point) index -> RmlUi selection index; wchar_t holds code points here.
int ToSelectionIndex(int index)
{
    return index < 0 ? 0 : index;
}
} // namespace

FriendWindowView::FriendWindowView(DWORD windowUIID)
    : m_WindowUIID(windowUIID), m_ModelName("friend_window_" + std::to_string(windowUIID))
{
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadTheme(); });
}

FriendWindowView::~FriendWindowView()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    Unload();
}

namespace
{
bool CreateModel(RmlModelBinder<FriendWindowRmlModel>& binder, Rml::Context* context, const std::string& name)
{
    return binder.Create(context, name,
                         [](Rml::DataModelConstructor& c, FriendWindowRmlModel& model)
                         {
                             c.Bind("root_x", &model.rootX);
                             c.Bind("root_y", &model.rootY);
                             c.Bind("root_scale", &model.rootScale);
                             auto part = c.RegisterStruct<FriendWindowPart>();
                             part.RegisterMember("kind", &FriendWindowPart::kind);
                             part.RegisterMember("role", &FriendWindowPart::role);
                             part.RegisterMember("left", &FriendWindowPart::left);
                             part.RegisterMember("top", &FriendWindowPart::top);
                             part.RegisterMember("width", &FriendWindowPart::width);
                             part.RegisterMember("height", &FriendWindowPart::height);
                             part.RegisterMember("text", &FriendWindowPart::text);
                             part.RegisterMember("text_px", &FriendWindowPart::textPx);
                             part.RegisterMember("align", &FriendWindowPart::align);
                             part.RegisterMember("bold", &FriendWindowPart::bold);
                             part.RegisterMember("color", &FriendWindowPart::color);
                             c.RegisterArray<std::vector<FriendWindowPart>>();
                             c.Bind("parts", &model.parts);
                         });
}

// Root position and scale of a window's document (FloatingWorkspace); marks what changed.
void SyncRoot(RmlModelBinder<FriendWindowRmlModel>& binder, float rootX, float rootY, float scale)
{
    FriendWindowRmlModel& model = binder.GetModel();
    if (model.rootX == rootX && model.rootY == rootY && model.rootScale == scale)
        return;
    model.rootX = rootX;
    model.rootY = rootY;
    model.rootScale = scale;
    binder.MarkDirty("root_x");
    binder.MarkDirty("root_y");
    binder.MarkDirty("root_scale");
    // Text sizes follow the scale; the parts carry them.
}
} // namespace

void FriendWindowView::BuildUnderlay()
{
    if (m_pUnderDoc || !RmlUiRuntime::Instance().IsCreated())
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetBackgroundContext();
    if (!context || !CreateModel(m_UnderBinder, context, m_ModelName + "_under"))
        return;
    m_pUnderDoc = UI::RmlBridge::LoadThemedDocument(context, DocumentPath, ModelPlaceholder,
                                                    "data-model=\"" + m_ModelName + "_under\"");
}

void FriendWindowView::Build()
{
    if (m_pDoc || !RmlUiRuntime::Instance().IsCreated())
        return;
    const bool created = CreateModel(m_Binder, RmlUiRuntime::Instance().GetContext(), m_ModelName);
    if (!created)
        return;
    m_pDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), DocumentPath, ModelPlaceholder,
                                               "data-model=\"" + m_ModelName + "\"");
    if (!m_pDoc)
        return;
    for (int slot = 0; slot < FriendWindowFieldLayout::SlotCount; ++slot)
    {
        Field& field = m_Fields[slot];
        field = Field{};
        field.element = m_pDoc->GetElementById(FieldIds[slot]);
        if (!field.element)
            continue;
        field.listener = std::make_unique<FriendWindowFieldListener>(*this, slot);
        field.element->AddEventListener(Rml::EventId::Change, field.listener.get());
        field.element->AddEventListener(Rml::EventId::Keydown, field.listener.get());
    }
}

void FriendWindowView::Unload()
{
    m_KeyboardSlot = -1;
    if (!RmlUiRuntime::Instance().IsCreated())
    {
        // The context and its documents are gone already.
        m_pDoc = nullptr;
        m_pUnderDoc = nullptr;
        for (Field& field : m_Fields)
            field = Field{};
        return;
    }
    for (Field& field : m_Fields)
    {
        if (field.element && field.listener)
        {
            if (field.element->IsPseudoClassSet("focus"))
                field.element->Blur();
            field.element->RemoveEventListener(Rml::EventId::Change, field.listener.get());
            field.element->RemoveEventListener(Rml::EventId::Keydown, field.listener.get());
        }
        field = Field{};
    }
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    if (m_pDoc)
    {
        context->UnloadDocument(m_pDoc);
        m_pDoc = nullptr;
    }
    m_Binder.Destroy(context);
    if (Rml::Context* underContext = RmlUiRuntime::Instance().GetBackgroundContext())
    {
        if (m_pUnderDoc)
            underContext->UnloadDocument(m_pUnderDoc);
        m_UnderBinder.Destroy(underContext);
    }
    m_pUnderDoc = nullptr;
}

void FriendWindowView::ReloadTheme()
{
    if (!m_pDoc)
        return;
    Unload();
    m_Binder.GetModel().parts.clear();
    m_UnderBinder.GetModel().parts.clear();
    Build();
}

bool FriendWindowView::Sync(CUIBaseWindow* window, bool shown, const std::vector<FriendWindowRect>& shades)
{
    if (window == nullptr || !shown)
    {
        m_KeyboardSlot = -1;
        for (Field& field : m_Fields)
        {
            if (field.element && field.element->IsPseudoClassSet("focus"))
                field.element->Blur();
        }
        UI::RmlBridge::SyncDocumentVisibility(m_pDoc, false);
        UI::RmlBridge::SyncDocumentVisibility(m_pUnderDoc, false);
        return false;
    }
    Build();
    if (!m_pDoc)
        return false;

    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    const float rootX = static_cast<float>(window->GetPosition_x()) * transform.scaleX + transform.offsetX;
    const float rootY = static_cast<float>(window->GetPosition_y()) * transform.scaleY + transform.offsetY;
    SyncRoot(m_Binder, rootX, rootY, transform.scaleX);

    FriendWindowRmlBuilder builder(window->GetPosition_x(), window->GetPosition_y(), m_Binder.GetModel().parts,
                                   m_UnderBinder.GetModel().parts);
    window->CollectRmlView(builder);
    // A window in front leaves its photo viewer's box to an underlay under the native pass, which
    // is under this document too: its back is drawn over this window here instead.
    const float windowRight = static_cast<float>(window->GetPosition_x() + window->GetWidth());
    const float windowBottom = static_cast<float>(window->GetPosition_y() + window->GetHeight());
    for (const FriendWindowRect& shade : shades)
    {
        const float left = std::max(shade.left, static_cast<float>(window->GetPosition_x()));
        const float top = std::max(shade.top, static_cast<float>(window->GetPosition_y()));
        const float right = std::min(shade.right, windowRight);
        const float bottom = std::min(shade.bottom, windowBottom);
        builder.Fill("window-back", left, top, right - left, bottom - top);
    }
    if (builder.Finish())
        m_Binder.MarkDirty("parts");

    // The underlay (under a photo viewer): a background-context document, made when first needed.
    const bool underlayChanged = builder.FinishUnderlay();
    const bool hasUnderlay = !m_UnderBinder.GetModel().parts.empty();
    if (hasUnderlay)
        BuildUnderlay();
    if (m_pUnderDoc)
    {
        if (hasUnderlay)
        {
            SyncRoot(m_UnderBinder, rootX, rootY, transform.scaleX);
            if (underlayChanged)
                m_UnderBinder.MarkDirty("parts");
        }
        UI::RmlBridge::SyncDocumentVisibility(m_pUnderDoc, hasUnderlay);
    }

    const bool wasVisible = m_pDoc->IsVisible();
    UI::RmlBridge::SyncDocumentVisibility(m_pDoc, true);
    // The window keeps its field's keyboard under its own question (the letter's quit question),
    // as the native field kept its focus there.
    const DWORD topUIID = g_pWindowMgr->GetTopWindowUIID();
    const auto* question = dynamic_cast<const CUIQuestionWindow*>(g_pWindowMgr->GetWindow(topUIID));
    SyncFields(*window, builder,
               topUIID == m_WindowUIID || (question != nullptr && question->GetReturnWindowUIID() == m_WindowUIID));
    return !wasVisible;
}

void FriendWindowView::PlaceField(Field& field, const FriendWindowFieldLayout& layout,
                                  const UI::Scaling::Transform& transform)
{
    // Physical px in the document, from the window's corner (so a moved window moves it too):
    // sharp text at the native size, the box scaled like the window.
    const FriendWindowRmlModel& model = m_Binder.GetModel();
    if (field.layout == layout && field.scale == model.rootScale && field.rootX == model.rootX &&
        field.rootY == model.rootY)
        return;
    field.layout = layout;
    field.scale = model.rootScale;
    field.rootX = model.rootX;
    field.rootY = model.rootY;
    if (!layout.shown)
    {
        field.element->SetClass("shown", false);
        return;
    }
    const float scale = model.rootScale;
    field.element->SetClass("shown", true);
    field.element->SetProperty("left", Rml::ToString(model.rootX + layout.left * scale) + "px");
    field.element->SetProperty("top", Rml::ToString(model.rootY + layout.top * scale) + "px");
    field.element->SetProperty("width", Rml::ToString(layout.width * scale) + "px");
    field.element->SetProperty("height", Rml::ToString(layout.height * scale) + "px");
    field.element->SetProperty(
        "font-size", Rml::ToString(UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform)) + "px");
    field.element->SetProperty("color", UI::RmlBridge::RgbaToCss(layout.color));
}

void FriendWindowView::PushFieldValue(Field& field, CUITextInputBox& box)
{
    field.syncedValue = box.GetValue();
    field.element->SetAttribute("value", StringUtils::WideToNarrow(field.syncedValue.c_str()));
    field.edited = false;
}

void FriendWindowView::SyncFields(CUIBaseWindow& window, const FriendWindowRmlBuilder& builder, bool topWindow)
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    for (int slot = 0; slot < FriendWindowFieldLayout::SlotCount; ++slot)
    {
        Field& field = m_Fields[slot];
        CUITextInputBox* box = window.GetRmlTextField(slot);
        if (!field.element)
            continue;
        FriendWindowFieldLayout layout = builder.GetFields()[slot];
        if (box == nullptr)
            layout.shown = false;
        PlaceField(field, layout, transform);
        if (!layout.shown)
        {
            if (field.element->IsPseudoClassSet("focus"))
                field.element->Blur();
            if (m_KeyboardSlot == slot)
                m_KeyboardSlot = -1;
            continue;
        }

        const int maxLength = box->GetTextLimit() > 0 ? box->GetTextLimit() : MAX_TEXT_LENGTH;
        if (field.maxLength != maxLength)
        {
            field.maxLength = maxLength;
            field.element->SetAttribute("maxlength", maxLength);
        }

        // The value: what the player typed goes to the native field; a value the window set (a
        // reply's receiver, a cleared line after Enter) goes to the input.
        if (field.edited)
        {
            field.edited = false;
            const std::wstring typed = StringUtils::NarrowToWide(field.element->GetAttribute<Rml::String>("value", ""));
            if (typed != field.syncedValue)
            {
                box->SetValueFromField(typed);
                field.syncedValue = box->GetValue();
                if (field.syncedValue != typed)
                    PushFieldValue(field, *box);
            }
        }
        else if (box->GetValue() != field.syncedValue)
        {
            PushFieldValue(field, *box);
        }

        // The keyboard: a native GiveFocus() (a click on the field, Tab, the window selected) moves
        // it to the input with the native caret and selection, once the input is laid out (RmlUi
        // drops the focus of a field nobody can see); the input gives it up with the window.
        if (CUITextInputBox::GetFocusedPortable() == box && field.element->IsVisible(true))
        {
            field.element->Focus();
            if (field.element->IsPseudoClassSet("focus"))
            {
                if (auto* input = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(field.element))
                    input->SetSelectionRange(ToSelectionIndex(box->GetSelectionAnchor()),
                                             ToSelectionIndex(box->GetCaret()));
                else if (auto* area = rmlui_dynamic_cast<Rml::ElementFormControlTextArea*>(field.element))
                    area->SetSelectionRange(ToSelectionIndex(box->GetSelectionAnchor()),
                                            ToSelectionIndex(box->GetCaret()));
                CUITextInputBox::ReleaseFocus();
                m_KeyboardSlot = slot;
            }
        }
        else if (!topWindow)
        {
            if (field.element->IsPseudoClassSet("focus"))
                field.element->Blur();
            if (m_KeyboardSlot == slot)
                m_KeyboardSlot = -1;
        }
        else if (m_KeyboardSlot == slot && !field.element->IsPseudoClassSet("focus"))
        {
            // A click elsewhere moved the RmlUi focus away: the native field kept it, so take it
            // back -- unless another field (native, or an RmlUi input such as the chat line) took
            // the keyboard, which released it.
            if (CUITextInputBox::GetFocusedPortable() == nullptr && !RmlUiRuntime::Instance().IsTextInputActive())
                field.element->Focus();
            else
                m_KeyboardSlot = -1;
        }
    }
}

bool FriendWindowView::OnFieldKey(int slot, int keyIdentifier)
{
    const bool enter = keyIdentifier == Rml::Input::KI_RETURN || keyIdentifier == Rml::Input::KI_NUMPADENTER;
    const bool tab = keyIdentifier == Rml::Input::KI_TAB;
    if (!enter && !tab)
        return false;
    if (enter && slot == FriendWindowFieldLayout::MultilineSlot)
        return false; // a new line, typed into the <textarea>
    CUIBaseWindow* window = g_pWindowMgr->GetWindow(m_WindowUIID);
    CUITextInputBox* box = window ? window->GetRmlTextField(slot) : nullptr;
    Field& field = m_Fields[slot];
    if (box == nullptr || field.element == nullptr)
        return false;

    // The native field gets the value typed so far, then the key it handled itself (Enter
    // confirms to its window, Tab gives its tab target the focus -- moved to that input next frame).
    const std::wstring typed = StringUtils::NarrowToWide(field.element->GetAttribute<Rml::String>("value", ""));
    if (typed != field.syncedValue)
    {
        box->SetValueFromField(typed);
        field.syncedValue = box->GetValue();
    }
    field.edited = false;
    box->OnEditKey(enter ? VK_RETURN : VK_TAB, false, false);
    return true;
}

void FriendWindowView::PullToFront()
{
    if (m_pDoc && m_pDoc->IsVisible())
        m_pDoc->PullToFront();
}

FriendWindowViews::FriendWindowViews() = default;
FriendWindowViews::~FriendWindowViews() = default;

void FriendWindowViews::Sync(const std::list<CUIBaseWindow*>& windows, bool familyShown)
{
    // Drop the documents of removed windows.
    for (auto it = m_Views.begin(); it != m_Views.end();)
    {
        const bool present = std::any_of(windows.begin(), windows.end(),
                                         [&](CUIBaseWindow* w) { return w != nullptr && w->GetUIID() == it->first; });
        if (present)
            ++it;
        else
            it = m_Views.erase(it);
    }

    const auto isShown = [familyShown](CUIBaseWindow* window)
    { return familyShown && window->GetState() != UISTATE_HIDE && window->GetState() != UISTATE_READY; };

    // The underlays of the shown windows, back to front: each is drawn over the windows behind it.
    std::vector<FriendWindowRect> shades;
    std::vector<size_t> shadesInFront; // per window: the first of `shades` that is in front of it
    for (CUIBaseWindow* window : windows)
    {
        FriendWindowRect rect;
        if (window != nullptr && window->HasRmlView() && isShown(window) &&
            window->GetRmlUnderlayRect(rect.left, rect.top, rect.right, rect.bottom))
        {
            shades.push_back(rect);
        }
        shadesInFront.push_back(shades.size());
    }

    bool restack = false;
    std::list<DWORD> order;
    std::vector<FriendWindowRect> windowShades;
    size_t index = 0;
    for (CUIBaseWindow* window : windows)
    {
        const size_t firstShade = shadesInFront[index++];
        if (window == nullptr || !window->HasRmlView())
            continue;
        auto& view = m_Views[window->GetUIID()];
        if (!view)
            view = std::make_unique<FriendWindowView>(window->GetUIID());
        const bool shown = isShown(window);
        windowShades.assign(shades.begin() + static_cast<std::ptrdiff_t>(firstShade), shades.end());
        if (view->Sync(window, shown, windowShades))
            restack = true;
        if (shown)
            order.push_back(window->GetUIID());
    }

    // Stack the shown documents in the manager's draw order, in front of the other windows.
    if (restack || order != m_Order)
    {
        for (DWORD uiid : order)
            m_Views[uiid]->PullToFront();
        m_Order = std::move(order);
    }
}

bool FriendWindowViews::HasFieldFocus(DWORD windowUIID) const
{
    const auto it = m_Views.find(windowUIID);
    return it != m_Views.end() && it->second && it->second->HasFieldFocus();
}

bool CUIWindowMgr::RmlFieldHasFocus(DWORD dwUIID) const
{
    return m_pRmlViews && m_pRmlViews->HasFieldFocus(dwUIID);
}

void CUIWindowMgr::SyncRmlViews(bool familyShown)
{
    if (!m_pRmlViews)
        m_pRmlViews = std::make_unique<FriendWindowViews>();
    // Draw order first, then the windows the arrange list does not hold (hidden ones keep their
    // documents).
    std::list<CUIBaseWindow*> windows;
    for (DWORD uiid : m_WindowArrangeList)
    {
        auto it = m_WindowMap.find(uiid);
        if (it != m_WindowMap.end())
            windows.push_back(it->second);
    }
    for (const auto& [uiid, window] : m_WindowMap)
    {
        if (std::find(m_WindowArrangeList.begin(), m_WindowArrangeList.end(), uiid) == m_WindowArrangeList.end())
            windows.push_back(window);
    }
    m_pRmlViews->Sync(windows, familyShown);
}
