#include "stdafx.h"
#include "UI/Social/PhotoViewerControl.h"

#include "UI/Social/PhotoViewer.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"

#include <RmlUi/Core.h>

#include <cmath>

namespace UI::Social
{
namespace
{
constexpr const char* SlotId = "photo_slot";
constexpr const char* ImageId = "photo_image";
constexpr const char* HelpId = "photo_help";
// A letter from Webzen shows its logo where a sender would stand.
constexpr const char* WebzenLogo = "/Data/Local/Webzenlogo.jpg";

// RmlUi reports the cursor in its own pixels; the viewer thinks in native reference pixels, the
// same conversion LetterReadView::SyncPhoto() uses to place it. Turning by the converted delta
// keeps a drag feeling identical at every UI scale.
float ToNativePixels(float rmlPixels)
{
    const auto native = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight);
    return native.scaleX > 0 ? rmlPixels / native.scaleX : rmlPixels;
}
} // namespace

PhotoViewerControl::~PhotoViewerControl()
{
    Detach();
}

void PhotoViewerControl::Attach(Rml::ElementDocument* document, CUIPhotoViewer& viewer)
{
    if (m_Document == document)
        return;
    Detach();
    if (document == nullptr)
        return;
    m_Document = document;
    m_Viewer = &viewer;
    if (auto* slot = document->GetElementById(SlotId))
        slot->AddEventListener(Rml::EventId::Mousedown, this);
    if (auto* help = document->GetElementById(HelpId))
        help->AddEventListener(Rml::EventId::Mousedown, this);
    // On the document, not the slot: a turn that leaves the slot has to keep turning, and the
    // release that ends it usually lands somewhere else entirely.
    document->AddEventListener(Rml::EventId::Mousemove, this);
    document->AddEventListener(Rml::EventId::Mouseup, this);
}

void PhotoViewerControl::Detach()
{
    Suspend();
    if (m_Document != nullptr)
    {
        if (auto* slot = m_Document->GetElementById(SlotId))
            slot->RemoveEventListener(Rml::EventId::Mousedown, this);
        if (auto* help = m_Document->GetElementById(HelpId))
            help->RemoveEventListener(Rml::EventId::Mousedown, this);
        m_Document->RemoveEventListener(Rml::EventId::Mousemove, this);
        m_Document->RemoveEventListener(Rml::EventId::Mouseup, this);
    }
    m_Document = nullptr;
    m_Viewer = nullptr;
    m_Turning = false;
}

void PhotoViewerControl::Sync()
{
    if (m_Document == nullptr || m_Viewer == nullptr)
        return;
    const bool webzen = m_Viewer->IsWebzenMail();
    auto& target = m_Viewer->Target();
    target.SetEnabled(!webzen);
    if (auto* slot = m_Document->GetElementById(SlotId))
        slot->SetClass("webzen", webzen);
    if (auto* image = m_Document->GetElementById(ImageId))
    {
        // The image's own box in RmlUi pixels, which are physical ones: the character is drawn at
        // exactly the size it is shown, never scaled.
        if (!webzen)
        {
            const auto size = image->GetBox().GetSize(Rml::BoxArea::Content);
            target.Resize(static_cast<std::uint32_t>(std::lround(size.x)),
                          static_cast<std::uint32_t>(std::lround(size.y)));
        }
        const Rml::String source = webzen ? Rml::String(WebzenLogo) : target.Source();
        if (image->GetAttribute<Rml::String>("src", "") != source)
            image->SetAttribute("src", source);
    }
    SyncHelp();
}

void PhotoViewerControl::Suspend()
{
    if (m_Viewer != nullptr)
        m_Viewer->Target().SetEnabled(false);
    UI::RmlBridge::Tooltip::Hide(this);
}

// Three white lines left-aligned in a box centred on the well, ending at its bottom -- where
// RenderTipTextList(..., RT3_SORT_LEFT) put them. Above the character now, since the character is
// an image inside this same document.
void PhotoViewerControl::SyncHelp()
{
    const bool shown = m_Viewer->IsHelpShown() && !m_Viewer->IsWebzenMail();
    if (auto* help = m_Document->GetElementById(HelpId))
        help->SetClass("active", shown);
    auto* slot = m_Document->GetElementById(SlotId);
    if (!shown || slot == nullptr)
    {
        UI::RmlBridge::Tooltip::Hide(this);
        return;
    }

    UI::RmlBridge::Tooltip::Config config;
    for (const wchar_t* text :
         {I18N::Game::WheelButtonZoomInOut, I18N::Game::LeftClickRotation, I18N::Game::RightClickDefault})
    {
        UI::RmlBridge::Tooltip::Line line;
        line.text = StringUtils::WideToNarrow(text);
        config.lines.push_back(std::move(line));
    }
    const auto size = slot->GetBox().GetSize(Rml::BoxArea::Border);
    config.anchorX = slot->GetAbsoluteLeft() + size.x * 0.5f;
    config.anchorY = slot->GetAbsoluteTop() + size.y;
    config.anchor = UI::RmlBridge::Tooltip::AnchorPoint::AboveLeft;
    config.centerHorizontally = true;
    config.transform = UI::Scaling::FloatingWorkspaceTransform(WindowWidth, WindowHeight);
    UI::RmlBridge::Tooltip::Show(config, this);
}

void PhotoViewerControl::ProcessEvent(Rml::Event& event)
{
    if (m_Viewer == nullptr)
        return;

    const Rml::Element* target = event.GetTargetElement();
    const bool onHelp = target != nullptr && target->GetId() == HelpId;

    switch (event.GetId())
    {
    case Rml::EventId::Mousedown:
    {
        const int button = event.GetParameter<int>("button", 0);
        if (onHelp)
        {
            if (button == 0)
            {
                m_Viewer->ToggleHelp();
                event.StopPropagation(); // Never starts a turn as well.
            }
            return;
        }
        if (button == 1)
            m_Viewer->ResetView();
        else if (button == 0)
        {
            m_Turning = true;
            m_LastX = event.GetParameter<float>("mouse_x", 0.0f);
        }
        return;
    }
    case Rml::EventId::Mousemove:
    {
        if (!m_Turning)
            return;
        const float x = event.GetParameter<float>("mouse_x", 0.0f);
        m_Viewer->TurnBy(ToNativePixels(x - m_LastX));
        m_LastX = x;
        return;
    }
    case Rml::EventId::Mouseup:
        m_Turning = false;
        return;
    default:
        return;
    }
}
} // namespace UI::Social
