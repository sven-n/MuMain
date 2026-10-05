#include "stdafx.h"
#include "RmlDraggable.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Property.h>

namespace UI::RmlBridge
{
namespace
{
// The left/top that keep the panel where it is drawn: from its containing block, net of its
// margins. A centred panel is drawn through its transform, so its visible box is read and the
// `dragged` class drops the transform (an inline transform override does not hold here; see
// window_shell.rml).
Rml::Vector2f PinnedPosition(Rml::Element* panel)
{
    const Rml::Box& box = panel->GetBox();
    const Rml::Vector2f margin{box.GetEdge(Rml::BoxArea::Margin, Rml::BoxEdge::Left),
                               box.GetEdge(Rml::BoxArea::Margin, Rml::BoxEdge::Top)};
    const Rml::Vector2f offset{panel->GetOffsetLeft(), panel->GetOffsetTop()};
    Rml::Rectanglef drawn;
    if (!panel->IsClassSet("center-both") || !Rml::ElementUtilities::GetBoundingBox(drawn, panel, Rml::BoxArea::Border))
        return offset - margin;
    const Rml::Vector2f origin = panel->GetAbsoluteOffset(Rml::BoxArea::Border) - offset;
    panel->SetClass("dragged", true);
    return drawn.Position() - origin - margin;
}

// Self-owning: deletes itself when its last attachment is detached, so the caller never
// needs to track or clean this up. One instance listens for four events and RmlUi calls
// OnAttach()/OnDetach() once per event, so it counts them: deleting on the first OnDetach()
// left the other entries pointing at freed memory, and unloading the document (a theme
// switch) crashed in EventDispatcher::DetachAllEvents().
class DragMoveListener : public Rml::EventListener
{
public:
    DragMoveListener(Rml::Element* panel, OnPanelMoved onMove, OnDragEnd onDragEnd)
        : m_Panel(panel), m_OnMove(std::move(onMove)), m_OnDragEnd(std::move(onDragEnd))
    {
    }

    void ProcessEvent(Rml::Event& event) override
    {
        // A control inside the handle with its own `drag` (a gauge, a scrollbar thumb) is the drag
        // element for its own drags, which bubble up here; only the handle's own drags move the panel.
        if (event.GetId() != Rml::EventId::Mousedown && event.GetTargetElement() != event.GetCurrentElement())
            return;

        switch (event.GetId())
        {
        case Rml::EventId::Mousedown:
            m_PressMouseX = event.GetParameter<int>("mouse_x", 0);
            m_PressMouseY = event.GetParameter<int>("mouse_y", 0);
            break;

        case Rml::EventId::Dragstart:
        {
            // RmlUi gives a transformed panel's events its local coordinates: a centred panel's
            // press arrives shifted by its centring, until the pin below drops it.
            Rml::Rectanglef drawn;
            m_LocalShift = {0.f, 0.f};
            if (m_Panel->GetComputedValues().has_local_transform()
                && Rml::ElementUtilities::GetBoundingBox(drawn, m_Panel, Rml::BoxArea::Border))
                m_LocalShift = m_Panel->GetAbsoluteOffset(Rml::BoxArea::Border) - drawn.Position();
            // From the press, not dragstart's own mouse_x/y: that is the pointer's previous position,
            // which need not be where the button went down.
            m_DragStartMouseX = m_PressMouseX - static_cast<int>(std::lround(m_LocalShift.x));
            m_DragStartMouseY = m_PressMouseY - static_cast<int>(std::lround(m_LocalShift.y));
            // Read from the laid-out box rather than the left/top Property -- unit-agnostic,
            // unlike Property::Get<float>() which returns the raw authored number.
            const Rml::Vector2f start = PinnedPosition(m_Panel);
            m_DragStartPanelLeft = start.x;
            m_DragStartPanelTop = start.y;
            break;
        }

        case Rml::EventId::Drag:
        {
            // A step that arrives before the pin's layout still has the local shift.
            const Rml::Vector2f shift =
                m_Panel->GetComputedValues().has_local_transform() ? m_LocalShift : Rml::Vector2f(0.f, 0.f);
            const int mouseX = event.GetParameter<int>("mouse_x", 0) - static_cast<int>(std::lround(shift.x));
            const int mouseY = event.GetParameter<int>("mouse_y", 0) - static_cast<int>(std::lround(shift.y));
            const float newLeftPx = m_DragStartPanelLeft + static_cast<float>(mouseX - m_DragStartMouseX);
            const float newTopPx = m_DragStartPanelTop + static_cast<float>(mouseY - m_DragStartMouseY);

            // Write in dp, not raw px -- px doesn't scale with UIScalePercent and would
            // drift from every dp-authored sibling. Divide by the same ratio
            // RmlUiRuntime::ApplyUIScale() sets on the context.
            const float dpRatio = m_Panel->GetOwnerDocument()->GetContext()->GetDensityIndependentPixelRatio();
            const float newLeftDp = dpRatio > 0.0f ? newLeftPx / dpRatio : newLeftPx;
            const float newTopDp = dpRatio > 0.0f ? newTopPx / dpRatio : newTopPx;
            m_Panel->SetProperty("left", std::to_string(newLeftDp) + "dp");
            m_Panel->SetProperty("top", std::to_string(newTopDp) + "dp");

            // onMove's contract is real window pixels, for syncing a hybrid window's
            // legacy CWin position -- pass px, not the dp values just written above.
            if (m_OnMove)
                m_OnMove(newLeftPx, newTopPx);
            break;
        }

        case Rml::EventId::Dragend:
            if (m_OnDragEnd)
                m_OnDragEnd();
            break;

        default:
            break;
        }
    }

    void OnAttach(Rml::Element*) override
    {
        ++m_Attachments;
    }

    void OnDetach(Rml::Element*) override
    {
        if (--m_Attachments <= 0)
            delete this;
    }

private:
    int m_Attachments = 0;
    Rml::Element* m_Panel;
    OnPanelMoved m_OnMove;
    OnDragEnd m_OnDragEnd;
    Rml::Vector2f m_LocalShift{0.f, 0.f};
    int m_PressMouseX = 0;
    int m_PressMouseY = 0;
    int m_DragStartMouseX = 0;
    int m_DragStartMouseY = 0;
    float m_DragStartPanelLeft = 0.0f;
    float m_DragStartPanelTop = 0.0f;
};
} // namespace

void ResetDraggedPosition(Rml::Element* panel)
{
    if (panel == nullptr)
        return;
    panel->SetClass("dragged", false);
    panel->RemoveProperty(Rml::PropertyId::Left);
    panel->RemoveProperty(Rml::PropertyId::Top);
}

void KeepInsideWindow(Rml::Element* panel)
{
    Rml::Context* context = panel != nullptr ? panel->GetContext() : nullptr;
    Rml::Rectanglef drawn;
    if (context == nullptr || !Rml::ElementUtilities::GetBoundingBox(drawn, panel, Rml::BoxArea::Border))
        return;
    const Rml::Vector2f view(context->GetDimensions());
    const auto shift = [](float start, float size, float limit)
    {
        if (size >= limit || start < 0.f)
            return -start;
        return start + size > limit ? limit - (start + size) : 0.f;
    };
    const float dx = shift(drawn.Left(), drawn.Width(), view.x);
    const float dy = shift(drawn.Top(), drawn.Height(), view.y);
    if (dx == 0.f && dy == 0.f)
        return;
    const Rml::Box& box = panel->GetBox();
    const float left = panel->GetOffsetLeft() - box.GetEdge(Rml::BoxArea::Margin, Rml::BoxEdge::Left) + dx;
    const float top = panel->GetOffsetTop() - box.GetEdge(Rml::BoxArea::Margin, Rml::BoxEdge::Top) + dy;
    const float dpRatio = context->GetDensityIndependentPixelRatio();
    panel->SetProperty("left", std::to_string(dpRatio > 0.0f ? left / dpRatio : left) + "dp");
    panel->SetProperty("top", std::to_string(dpRatio > 0.0f ? top / dpRatio : top) + "dp");
}

void MakeDraggable(Rml::Element* handle, Rml::Element* panel, OnPanelMoved onMove, OnDragEnd onDragEnd)
{
    // RmlUi only fires dragstart/drag for an element whose computed `drag` isn't `none` --
    // set it here so the caller doesn't need a matching RCSS rule.
    handle->SetProperty("drag", "drag");

    // pointer-events is inherited; a handle under a pointer-events:none body is invisible to
    // hit-testing and would never enter the hover chain, so dragstart/drag would never fire.
    // Force it here regardless of what the handle inherited from its document.
    handle->SetProperty("pointer-events", "auto");

    DragMoveListener* listener = new DragMoveListener(panel, std::move(onMove), std::move(onDragEnd));
    handle->AddEventListener(Rml::EventId::Mousedown, listener);
    handle->AddEventListener(Rml::EventId::Dragstart, listener);
    handle->AddEventListener(Rml::EventId::Drag, listener);
    handle->AddEventListener(Rml::EventId::Dragend, listener);
}
} // namespace UI::RmlBridge
