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
        // Self-owning: RmlUi's own contract (Element::AddEventListener's header comment) is that
        // a listener must stay alive until OnDetach() is called, which happens when the handle
        // element is destroyed or the listener is explicitly removed -- deleting itself there
        // means the caller never needs to track or clean this up.
        class DragMoveListener : public Rml::EventListener
        {
        public:
            DragMoveListener(Rml::Element* panel, OnPanelMoved onMove) : m_Panel(panel), m_OnMove(std::move(onMove)) {}

            void ProcessEvent(Rml::Event& event) override
            {
                switch (event.GetId())
                {
                case Rml::EventId::Dragstart:
                    m_DragStartMouseX = event.GetParameter<int>("mouse_x", 0);
                    m_DragStartMouseY = event.GetParameter<int>("mouse_y", 0);
                    // GetOffsetLeft()/Top() -- the element's own resolved pixel offset from its
                    // offset parent -- rather than parsing the "left"/"top" Property directly:
                    // unit-agnostic (works whether the panel's current position came from `px`,
                    // `dp`, or RCSS-authored `%`), unlike Property::Get<float>() which returns the
                    // raw authored number with no unit conversion.
                    m_DragStartPanelLeft = m_Panel->GetOffsetLeft();
                    m_DragStartPanelTop = m_Panel->GetOffsetTop();
                    break;

                case Rml::EventId::Drag:
                {
                    const int mouseX = event.GetParameter<int>("mouse_x", 0);
                    const int mouseY = event.GetParameter<int>("mouse_y", 0);
                    const float newLeftPx = m_DragStartPanelLeft + static_cast<float>(mouseX - m_DragStartMouseX);
                    const float newTopPx = m_DragStartPanelTop + static_cast<float>(mouseY - m_DragStartMouseY);

                    // Write in dp, not raw px (architecture-principles.md §10-11;
                    // layout-and-scaling.md's dp-vs-px rule) -- `px` never scales with
                    // UIScalePercent, so a dragged position would silently drift out of step with
                    // every dp-authored sibling the moment UI scale changes. Divide by the same
                    // density-independent-pixel ratio RmlUiRuntime::ApplyUIScale() sets on the
                    // context -- same source of truth, no new dependency on GameConfig/UI::Scaling
                    // here.
                    const float dpRatio = m_Panel->GetOwnerDocument()->GetContext()->GetDensityIndependentPixelRatio();
                    const float newLeftDp = dpRatio > 0.0f ? newLeftPx / dpRatio : newLeftPx;
                    const float newTopDp = dpRatio > 0.0f ? newTopPx / dpRatio : newTopPx;
                    m_Panel->SetProperty("left", std::to_string(newLeftDp) + "dp");
                    m_Panel->SetProperty("top", std::to_string(newTopDp) + "dp");

                    // onMove's own contract (RmlDraggable.h) is real window pixels, for syncing a
                    // hybrid window's legacy CWin position (CWin::SetPosition() takes real
                    // pixels) -- pass the px values, not the dp ones just written to the property.
                    if (m_OnMove)
                        m_OnMove(newLeftPx, newTopPx);
                    break;
                }

                default:
                    break;
                }
            }

            void OnDetach(Rml::Element*) override { delete this; }

        private:
            Rml::Element* m_Panel;
            OnPanelMoved m_OnMove;
            int m_DragStartMouseX = 0;
            int m_DragStartMouseY = 0;
            float m_DragStartPanelLeft = 0.0f;
            float m_DragStartPanelTop = 0.0f;
        };
    }

    void MakeDraggable(Rml::Element* handle, Rml::Element* panel, OnPanelMoved onMove)
    {
        // RmlUi only generates dragstart/drag events for an element whose computed `drag` style
        // is anything but `none` (Context.cpp's ProcessMouseButtonDown walks up from the hover
        // chain looking for the first such ancestor) -- set it here so the caller doesn't need
        // to remember a matching RCSS rule just to make this actually fire.
        handle->SetProperty("drag", "drag");

        // pointer-events is inherited (see README.md's Gotchas section's pointer-events entry)
        // -- any full-window document following that same fix sets
        // `body { pointer-events: none; }` with only specific interactive elements opting back
        // in via `auto`. A handle that inherited `none` is invisible to RmlUi's own hit-testing
        // (GetElementAtPoint explicitly skips pointer-events:none elements), so it would never
        // become `hover` and dragstart/drag would never fire at all -- confirmed as a real bug
        // hit while first testing this against the login screen. Force it here so a handle just
        // works regardless of what pointer-events state it inherited from its document.
        handle->SetProperty("pointer-events", "auto");

        DragMoveListener* listener = new DragMoveListener(panel, std::move(onMove));
        handle->AddEventListener(Rml::EventId::Dragstart, listener);
        handle->AddEventListener(Rml::EventId::Drag, listener);
    }
}
