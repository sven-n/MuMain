#include "stdafx.h"
#include "RmlDraggable.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Property.h>

namespace UI::RmlBridge
{
    namespace
    {
        // Self-owning: deletes itself in OnDetach() per RmlUi's AddEventListener contract, so the
        // caller never needs to track or clean this up.
        class DragMoveListener : public Rml::EventListener
        {
        public:
            DragMoveListener(Rml::Element* panel, OnPanelMoved onMove, OnDragEnd onDragEnd)
                : m_Panel(panel), m_OnMove(std::move(onMove)), m_OnDragEnd(std::move(onDragEnd)) {}

            void ProcessEvent(Rml::Event& event) override
            {
                switch (event.GetId())
                {
                case Rml::EventId::Dragstart:
                    m_DragStartMouseX = event.GetParameter<int>("mouse_x", 0);
                    m_DragStartMouseY = event.GetParameter<int>("mouse_y", 0);
                    // GetOffsetLeft()/Top() rather than parsing the left/top Property directly --
                    // unit-agnostic, unlike Property::Get<float>() which returns the raw
                    // authored number with no unit conversion.
                    m_DragStartPanelLeft = m_Panel->GetOffsetLeft();
                    m_DragStartPanelTop = m_Panel->GetOffsetTop();
                    break;

                case Rml::EventId::Drag:
                {
                    const int mouseX = event.GetParameter<int>("mouse_x", 0);
                    const int mouseY = event.GetParameter<int>("mouse_y", 0);
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

            void OnDetach(Rml::Element*) override { delete this; }

        private:
            Rml::Element* m_Panel;
            OnPanelMoved m_OnMove;
            OnDragEnd m_OnDragEnd;
            int m_DragStartMouseX = 0;
            int m_DragStartMouseY = 0;
            float m_DragStartPanelLeft = 0.0f;
            float m_DragStartPanelTop = 0.0f;
        };
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
        handle->AddEventListener(Rml::EventId::Dragstart, listener);
        handle->AddEventListener(Rml::EventId::Drag, listener);
        handle->AddEventListener(Rml::EventId::Dragend, listener);
    }
}
