#include "stdafx.h"

#include "UI/Character/WorldLabelLayer.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementText.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/TextShapingContext.h>

#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <functional>
#include <string_view>
#include <utility>

namespace
{

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

// The width RmlUi draws a label's text at. Its face size is the font-size truncated to whole pixels
// and its advances are hinted, so with some families (DejaVu Sans) it comes out a few pixels wider
// than the native measurement the label box was sized from.
float DrawnTextWidth(const Rml::ComputedValues& values, bool bold, float pixelSize, const std::string& utf8)
{
    Rml::FontEngineInterface* fonts = Rml::GetFontEngineInterface();
    const Rml::Style::FontWeight weight = bold ? Rml::Style::FontWeight::Bold : Rml::Style::FontWeight::Normal;
    const Rml::FontFaceHandle face = fonts->GetFontFaceHandle(values.font_family(), Rml::Style::FontStyle::Normal,
                                                              weight, static_cast<int>(pixelSize));
    if (face == 0)
        return 0.0f;
    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          values.letter_spacing()};
    return static_cast<float>(fonts->GetStringWidth(face, utf8, shaping));
}

std::size_t CombineKey(std::size_t key, std::size_t value)
{
    return key ^ (value + 0x9e3779b97f4a7c15ull + (key << 6) + (key >> 2));
}

std::size_t TextKey(const Render::Renderer::RecordedText& text)
{
    const std::size_t key = CombineKey(std::hash<std::string_view>{}(text.utf8), std::hash<float>{}(text.textPixelSize));
    return CombineKey(key, text.bold ? 1u : 0u);
}

std::size_t BitmapKey(const Render::Renderer::RecordedBitmap& bitmap)
{
    std::size_t key = std::hash<std::wstring_view>{}(bitmap.fileName != nullptr ? bitmap.fileName : L"");
    for (const float value : {bitmap.sourceX, bitmap.sourceY, bitmap.sourceWidth, bitmap.sourceHeight})
        key = CombineKey(key, std::hash<float>{}(value));
    return key;
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
    m_view.Ensure();
    return IsAvailable();
}

void UI::Character::WorldLabelLayer::Release()
{
    m_view.Release();
    OnBuilt();
}

// A rebuilt document has none of the old one's slots.
void UI::Character::WorldLabelLayer::OnBuilt()
{
    m_slots.clear();
    m_shownByContent.clear();
    m_hiddenSlots.clear();
    m_drawn = 0;
}

void UI::Character::WorldLabelLayer::Hide()
{
    if (m_view.Document() != nullptr)
        UI::RmlBridge::SyncDocumentVisibility(m_view.Document(), false);
}

void UI::Character::WorldLabelLayer::BeginFrame()
{
    m_drawn = 0;
    m_shownByContent.clear();
    m_hiddenSlots.clear();
    for (std::size_t i = 0; i < m_slots.size(); ++i)
    {
        m_slots[i].drawn = false;
        if (m_slots[i].kind == SlotKind::Hidden)
            m_hiddenSlots.push_back(i);
        else
            m_shownByContent.emplace(m_slots[i].contentKey, i);
    }
    if (m_view.Document() == nullptr)
        return;

    // Under every window, as the original's depth-1.0 window: behind every other document.
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_view.Document(), true);
    Rml::Context* context = m_view.Document()->GetContext();
    if (context->GetNumDocuments() > 1 && context->GetDocument(0) != m_view.Document())
        m_view.Document()->PushToBack();
}

void UI::Character::WorldLabelLayer::EndFrame()
{
    for (Slot& slot : m_slots)
        if (!slot.drawn)
            SetKind(slot, SlotKind::Hidden);
}

// A slot still showing what is drawn needs no new text or size, either of which lays the document out
// again. Otherwise a hidden one, so a label never takes the slot of one drawn later this frame.
template <typename Matches>
UI::Character::WorldLabelLayer::Slot& UI::Character::WorldLabelLayer::NextSlot(SlotKind kind, std::size_t contentKey,
                                                                               Matches&& matches)
{
    std::size_t index = m_slots.size();
    const auto [first, last] = m_shownByContent.equal_range(contentKey);
    for (auto it = first; it != last; ++it)
    {
        if (m_slots[it->second].kind == kind && matches(m_slots[it->second]))
        {
            index = it->second;
            m_shownByContent.erase(it);
            break;
        }
    }
    if (index == m_slots.size() && !m_hiddenSlots.empty())
    {
        index = m_hiddenSlots.back();
        m_hiddenSlots.pop_back();
    }
    if (index == m_slots.size())
    {
        Slot slot;
        Rml::ElementPtr box = m_view.Document()->CreateElement("div");
        box->SetClass("label", true);
        slot.box = m_view.Document()->AppendChild(std::move(box));
        SetDisplay(slot.box, false);
        m_slots.push_back(slot);
    }
    const auto drawOrder = static_cast<float>(m_drawn++);
    Slot& slot = m_slots[index];
    slot.drawn = true;
    slot.contentKey = contentKey;
    SetKind(slot, kind);
    if (slot.zIndex != drawOrder)
    {
        slot.zIndex = drawOrder;
        slot.box->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(drawOrder, Rml::Unit::NUMBER));
    }
    return slot;
}

void UI::Character::WorldLabelLayer::SetKind(Slot& slot, SlotKind kind)
{
    if (slot.kind == kind)
        return;

    if (kind == SlotKind::Text && slot.text == nullptr)
    {
        Rml::ElementPtr text = m_view.Document()->CreateElement("span");
        text->SetClass("label-text", true);
        slot.text = slot.box->AppendChild(std::move(text));
        slot.textNode = static_cast<Rml::ElementText*>(slot.text->AppendChild(m_view.Document()->CreateTextNode("")));
    }
    if (kind == SlotKind::Bitmap && slot.image == nullptr)
    {
        Rml::ElementPtr image = m_view.Document()->CreateElement("img");
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
    Slot& slot = NextSlot(SlotKind::Text, TextKey(text), [&](const Slot& candidate)
    {
        return candidate.utf8 == text.utf8 && candidate.textPixelSize == text.textPixelSize && candidate.bold == text.bold;
    });
    const bool textChanged =
        slot.utf8 != text.utf8 || slot.textPixelSize != text.textPixelSize || slot.bold != text.bold;
    const float textOffset = text.textX - text.boxX;
    SetPx(slot.text, Rml::PropertyId::Left, slot.textOffset, textOffset);
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
    // Measured again until the document's font resolves (zero before its first layout).
    if (textChanged || slot.drawnTextWidth == 0.0f)
        slot.drawnTextWidth =
            DrawnTextWidth(m_view.Document()->GetComputedValues(), text.bold, text.textPixelSize, slot.utf8);

    // The native renderer draws no box for a fully transparent background colour; a box keeps the
    // blend state it was drawn under (opaque after the F8 health bars' DisableAlphaBlend()). The box
    // never ends before the text RmlUi draws in it.
    const float boxWidth = std::max(text.boxWidth, textOffset + slot.drawnTextWidth);
    const Rect box{text.boxX, text.boxY, boxWidth, text.boxHeight};
    if ((text.backColor >> 24) == 0)
    {
        SetBox(slot, box, 0u);
        SetAdditive(slot, false, 0u);
    }
    else
    {
        SetFill(slot, box, text.backColor, text.backBlend);
    }
}

void UI::Character::WorldLabelLayer::RecordQuad(const Render::Renderer::RecordedQuad& quad)
{
    // A quad has nothing to keep but its kind.
    Slot& slot = NextSlot(SlotKind::Quad, 0, [](const Slot&) { return true; });
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
    Slot& slot = NextSlot(SlotKind::Bitmap, BitmapKey(bitmap), [&](const Slot& candidate)
    {
        return candidate.imageFile != nullptr && bitmap.fileName != nullptr &&
               std::wcscmp(candidate.imageFile, bitmap.fileName) == 0 && candidate.sourceX == bitmap.sourceX &&
               candidate.sourceY == bitmap.sourceY && candidate.sourceWidth == bitmap.sourceWidth &&
               candidate.sourceHeight == bitmap.sourceHeight;
    });
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
