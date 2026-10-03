#include "stdafx.h"
#include "UI/Social/PhotoViewerControl.h"

#include "UI/Social/UIWindows.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core.h>

namespace UI::Social
{
namespace
{
constexpr const char* SlotId = "photo_slot";
constexpr const char* HelpId = "photo_help";

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
