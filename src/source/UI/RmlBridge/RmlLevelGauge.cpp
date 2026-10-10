#include "stdafx.h"
#include "UI/RmlBridge/RmlLevelGauge.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Rectangle.h>

#include <algorithm>

namespace UI::RmlBridge
{
    std::optional<int> ApplyLevelGaugeEvent(Rml::Event& event, int level, int maxLevel,
                                            const LevelFromPointer& fromPointer)
    {
        int next = level;
        switch (event.GetId())
        {
        case Rml::EventId::Mousescroll:
        {
            // RmlUi reports wheel-down as positive.
            const float delta = event.GetParameter<float>("wheel_delta_y", 0.f);
            event.StopPropagation();
            if (delta == 0.f)
                return std::nullopt;
            next = level + (delta < 0.f ? 1 : -1);
            break;
        }
        case Rml::EventId::Mousedown:
        case Rml::EventId::Drag:
        {
            if (event.GetId() == Rml::EventId::Mousedown && event.GetParameter<int>("button", -1) != 0)
                return std::nullopt;

            Rml::Element* hit = event.GetCurrentElement();
            Rml::Element* bar = hit ? hit->GetParentNode() : nullptr;
            if (!bar)
                return std::nullopt;

            Rml::Rectanglef drawn;
            if (!Rml::ElementUtilities::GetBoundingBox(drawn, bar, Rml::BoxArea::Border))
                return std::nullopt;
            const float width = bar->GetBox().GetSize(Rml::BoxArea::Border).x;
            if (width <= 0.f || drawn.Width() <= 0.f)
                return std::nullopt;

            // The event's mouse_x can be in a transformed panel's local coordinates; compare the
            // unprojected pointer with where the bar is drawn, in the bar's own units.
            const float scale = drawn.Width() / width;
            next = fromPointer((event.GetUnprojectedMouseScreenPos().x - drawn.Left()) / scale, width);
            break;
        }
        default:
            return std::nullopt;
        }

        next = std::clamp(next, 0, maxLevel);
        if (next == level)
            return std::nullopt;
        return next;
    }
}
