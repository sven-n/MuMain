#include "stdafx.h"

#include "UI/Character/WorldLabelLayer.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementText.h>

#include <cstdio>
#include <cwchar>

namespace
{
constexpr const char* kDocumentPath = "Data/Interface/RmlUi/world_labels.rml";

Rml::Colourb ColourFromAbgr(std::uint32_t abgr)
{
    return Rml::Colourb(static_cast<Rml::byte>(abgr & 0xFFu), static_cast<Rml::byte>((abgr >> 8) & 0xFFu),
                        static_cast<Rml::byte>((abgr >> 16) & 0xFFu), static_cast<Rml::byte>((abgr >> 24) & 0xFFu));
}

std::uint32_t AbgrFromArgb(std::uint32_t argb)
{
    return (argb & 0xFF00FF00u) | ((argb >> 16) & 0xFFu) | ((argb & 0xFFu) << 16);
}

void SetPx(Rml::Element* element, Rml::PropertyId id, float& cached, float value)
{
    if (cached == value)
        return;
    cached = value;
    element->SetProperty(id, Rml::Property(value, Rml::Unit::PX));
}

void SetDisplay(Rml::Element* element, bool shown)
{
    element->SetProperty(Rml::PropertyId::Display,
                         Rml::Property(shown ? Rml::Style::Display::Block : Rml::Style::Display::None));
}

// "/Data/Interface/x.tga": absolute to the file interface root, as CGlobalBitmap loads it.
std::string ImageSource(const wchar_t* fileName)
{
    std::string source = "/";
    for (const wchar_t* c = fileName; c != nullptr && *c != L'\0'; ++c)
        source.push_back(*c == L'\\' ? '/' : static_cast<char>(*c));
    return source;
}
} // namespace

UI::Character::WorldLabelLayer::~WorldLabelLayer()
{
    Release();
}

bool UI::Character::WorldLabelLayer::Create()
{
    if (!m_registered && RmlUiRuntime::Instance().GetBackgroundContext() != nullptr)
    {
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadTheme(); });
        m_registered = true;
    }
    Build();
    return IsAvailable();
}

void UI::Character::WorldLabelLayer::Release()
{
    if (m_registered)
    {
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_registered = false;
    }
    // Unload the document while its context still exists; at shutdown the runtime may already be
    // gone, and with it the document. Idempotent.
    if (m_document != nullptr && RmlUiRuntime::Instance().IsCreated())
    {
        if (Rml::Context* context = RmlUiRuntime::Instance().GetBackgroundContext())
            context->UnloadDocument(m_document);
    }
    m_document = nullptr;
    m_slots.clear();
    m_used = 0;
    m_shown = 0;
}

void UI::Character::WorldLabelLayer::Build()
{
    if (m_document != nullptr)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetBackgroundContext();
    if (context == nullptr)
        return;
    m_document = UI::RmlBridge::LoadThemedDocument(context, kDocumentPath);
    m_slots.clear();
    m_used = 0;
    m_shown = 0;
}

void UI::Character::WorldLabelLayer::ReloadTheme()
{
    if (m_document == nullptr)
        return;
    const bool wasVisible = m_document->IsVisible();
    m_document->GetContext()->UnloadDocument(m_document);
    m_document = nullptr;
    Build();
    if (m_document != nullptr && wasVisible)
        UI::RmlBridge::SyncDocumentVisibilityBehind(m_document, true);
}

void UI::Character::WorldLabelLayer::Hide()
{
    if (m_document != nullptr)
        UI::RmlBridge::SyncDocumentVisibility(m_document, false);
}

void UI::Character::WorldLabelLayer::BeginFrame()
{
    m_used = 0;
    if (m_document == nullptr)
        return;

    // Under every window, as the original's depth-1.0 window: behind every other document of the
    // background context (the duel and siege boards, the docked panels' frames), which itself
    // renders before the native windows and the main context.
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_document, true);
    Rml::Context* context = m_document->GetContext();
    if (context->GetNumDocuments() > 1 && context->GetDocument(0) != m_document)
        m_document->PushToBack();
}

void UI::Character::WorldLabelLayer::EndFrame()
{
    // Only the slots used last frame but not this one: the ones beyond were hidden already.
    for (std::size_t i = m_used; i < m_shown && i < m_slots.size(); ++i)
        SetKind(m_slots[i], SlotKind::Hidden);
    m_shown = m_used;
}

UI::Character::WorldLabelLayer::Slot& UI::Character::WorldLabelLayer::NextSlot(SlotKind kind)
{
    if (m_used == m_slots.size())
    {
        Slot slot;
        Rml::ElementPtr box = m_document->CreateElement("div");
        box->SetClass("label", true);
        slot.box = m_document->AppendChild(std::move(box));
        SetDisplay(slot.box, false);
        m_slots.push_back(slot);
    }
    Slot& slot = m_slots[m_used++];
    SetKind(slot, kind);
    return slot;
}

void UI::Character::WorldLabelLayer::SetKind(Slot& slot, SlotKind kind)
{
    if (slot.kind == kind)
        return;

    if (kind == SlotKind::Text && slot.text == nullptr)
    {
        Rml::ElementPtr text = m_document->CreateElement("span");
        text->SetClass("label-text", true);
        slot.text = slot.box->AppendChild(std::move(text));
        slot.textNode = static_cast<Rml::ElementText*>(slot.text->AppendChild(m_document->CreateTextNode("")));
    }
    if (kind == SlotKind::Bitmap && slot.image == nullptr)
    {
        Rml::ElementPtr image = m_document->CreateElement("img");
        image->SetClass("label-image", true);
        slot.image = slot.box->AppendChild(std::move(image));
    }

    SetDisplay(slot.box, kind != SlotKind::Hidden);
    if (slot.text != nullptr)
        SetDisplay(slot.text, kind == SlotKind::Text);
    if (slot.image != nullptr)
        SetDisplay(slot.image, kind == SlotKind::Bitmap);
    if (kind != SlotKind::Hidden)
    {
        // A box turns transparent for a bitmap and is set again by the next text or quad.
        slot.boxColor = 1;
    }
    slot.kind = kind;
}

void UI::Character::WorldLabelLayer::SetBox(Slot& slot, const Rect& rect, std::uint32_t abgr)
{
    SetPx(slot.box, Rml::PropertyId::Left, slot.x, rect.x);
    SetPx(slot.box, Rml::PropertyId::Top, slot.y, rect.y);
    SetPx(slot.box, Rml::PropertyId::Width, slot.width, rect.width);
    SetPx(slot.box, Rml::PropertyId::Height, slot.height, rect.height);
    if (slot.boxColor != abgr)
    {
        slot.boxColor = abgr;
        slot.box->SetProperty(Rml::PropertyId::BackgroundColor, Rml::Property(ColourFromAbgr(abgr), Rml::Unit::COLOUR));
    }
}

void UI::Character::WorldLabelLayer::RecordText(const Render::Renderer::RecordedText& text)
{
    Slot& slot = NextSlot(SlotKind::Text);
    // The native renderer draws no box for a fully transparent background colour; a box keeps the
    // blend state it was drawn under (opaque after the F8 health bars' DisableAlphaBlend()).
    const Rect box{text.boxX, text.boxY, text.boxWidth, text.boxHeight};
    if ((text.backColor >> 24) == 0)
    {
        SetBox(slot, box, 0u);
        SetAdditive(slot, false, 0u);
    }
    else
    {
        SetFill(slot, box, text.backColor, text.backBlend);
    }
    SetPx(slot.text, Rml::PropertyId::Left, slot.textOffset, text.textX - text.boxX);
    SetPx(slot.text, Rml::PropertyId::FontSize, slot.textPixelSize, text.textPixelSize);
    SetPx(slot.text, Rml::PropertyId::LineHeight, slot.lineHeight, text.lineHeight);
    if (slot.bold != text.bold)
    {
        slot.bold = text.bold;
        slot.text->SetClass("bold", text.bold);
    }
    if (slot.textColor != text.textColor)
    {
        slot.textColor = text.textColor;
        slot.text->SetProperty(Rml::PropertyId::Color,
                               Rml::Property(ColourFromAbgr(text.textColor), Rml::Unit::COLOUR));
    }
    if (slot.utf8 != text.utf8)
    {
        slot.utf8.assign(text.utf8);
        slot.textNode->SetText(slot.utf8);
    }
}

void UI::Character::WorldLabelLayer::RecordQuad(const Render::Renderer::RecordedQuad& quad)
{
    Slot& slot = NextSlot(SlotKind::Quad);
    SetFill(slot, {quad.x, quad.y, quad.width, quad.height}, AbgrFromArgb(quad.argb), quad.blend);
}

void UI::Character::WorldLabelLayer::SetFill(Slot& slot, const Rect& rect, std::uint32_t abgr,
                                             Render::Renderer::RecordedBlend blend)
{
    // Added to what is behind it (Render::RmlUi::RegisterAdditiveFillDecorator), no plain fill; an
    // opaque draw ignores the alpha.
    const bool additive = blend == Render::Renderer::RecordedBlend::Additive;
    if (additive)
        SetBox(slot, rect, 0u);
    else
        SetBox(slot, rect, blend == Render::Renderer::RecordedBlend::Opaque ? abgr | 0xFF000000u : abgr);
    SetAdditive(slot, additive, abgr);
}

void UI::Character::WorldLabelLayer::SetAdditive(Slot& slot, bool additive, std::uint32_t abgr)
{
    // Formats the decorator only when the fill changes.
    if (slot.additive == additive && (!additive || slot.additiveColor == abgr))
        return;
    slot.additive = additive;
    slot.additiveColor = abgr;
    if (!additive)
    {
        slot.box->RemoveProperty(Rml::PropertyId::Decorator);
        return;
    }
    char fill[40];
    std::snprintf(fill, sizeof(fill), "additive-fill(#%02x%02x%02x)", abgr & 0xFFu, (abgr >> 8) & 0xFFu,
                  (abgr >> 16) & 0xFFu);
    slot.box->SetProperty("decorator", fill);
}

void UI::Character::WorldLabelLayer::RecordBitmap(const Render::Renderer::RecordedBitmap& bitmap)
{
    Slot& slot = NextSlot(SlotKind::Bitmap);
    SetBox(slot, {bitmap.x, bitmap.y, bitmap.width, bitmap.height}, 0u);
    SetAdditive(slot, false, 0u);

    // The recorded file names are CGlobalBitmap's own, stable strings: the src is rebuilt only when
    // the name changes (compared by content, so a different pointer to the same name is equal).
    if (slot.imageFile == nullptr || bitmap.fileName == nullptr || std::wcscmp(slot.imageFile, bitmap.fileName) != 0)
    {
        slot.imageFile = bitmap.fileName;
        slot.image->SetAttribute("src", ImageSource(bitmap.fileName));
    }
    if (slot.sourceX != bitmap.sourceX || slot.sourceY != bitmap.sourceY || slot.sourceWidth != bitmap.sourceWidth ||
        slot.sourceHeight != bitmap.sourceHeight)
    {
        slot.sourceX = bitmap.sourceX;
        slot.sourceY = bitmap.sourceY;
        slot.sourceWidth = bitmap.sourceWidth;
        slot.sourceHeight = bitmap.sourceHeight;
        char rect[96];
        std::snprintf(rect, sizeof(rect), "%g %g %g %g", bitmap.sourceX, bitmap.sourceY, bitmap.sourceWidth,
                      bitmap.sourceHeight);
        slot.image->SetAttribute("rect", Rml::String(rect));
    }
    if (slot.imageAlpha != bitmap.alpha)
    {
        slot.imageAlpha = bitmap.alpha;
        slot.image->SetProperty(Rml::PropertyId::Opacity, Rml::Property(bitmap.alpha, Rml::Unit::NUMBER));
    }
}
