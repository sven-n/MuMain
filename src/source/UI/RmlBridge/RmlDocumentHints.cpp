#include "stdafx.h"
#include "UI/RmlBridge/RmlDocumentHints.h"

#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/RmlBridge/RmlTooltipPlacement.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>

#include <algorithm>

namespace UI::RmlBridge::DocumentHints
{
namespace
{
constexpr const char* kHintAttribute = "data-hint";

const char s_Owner = 0;

// RenderCursor() draws the pointer's 24-unit sprite from 2 units above the point it marks.
constexpr float kCursorBottomUnits = 22.f;

Rml::String HintOf(const Rml::Element& element)
{
    const Rml::Variant* hint = element.GetAttribute(kHintAttribute);
    return hint ? hint->Get<Rml::String>() : Rml::String();
}

Rml::Element* HintedElement(Rml::Element* element)
{
    for (; element; element = element->GetParentNode())
    {
        if (!HintOf(*element).empty())
            return element;
    }
    return nullptr;
}

// Pixels per original unit at `element`. A panel under a root transform is laid out in the
// original's units, so its transform's scale is the unit (the window's native transform); a dp
// document's unit is the dp ratio, times the scale its workspace slot puts on it.
float UnitAt(Rml::Element& element, const Rml::Context& context, float drawnWidth)
{
    const float layoutWidth = element.GetBox().GetSize(Rml::BoxArea::Content).x;
    const float transformScale = layoutWidth > 0.f ? drawnWidth / layoutWidth : 1.f;
    bool transformed = false;
    for (Rml::Element* ancestor = &element; ancestor; ancestor = ancestor->GetParentNode())
    {
        if (ancestor->IsClassSet(WorkspacePlacedClass))
            return transformScale * context.GetDensityIndependentPixelRatio();
        transformed = transformed || ancestor->GetComputedValues().has_local_transform();
    }
    return transformed ? transformScale : context.GetDensityIndependentPixelRatio();
}
} // namespace

void Update(Rml::Context& context)
{
    Rml::Element* element = HintedElement(context.GetHoverElement());
    Rml::Vector2f offset, size;
    if (!element || !element->IsVisible(true) || !DrawnContentBox(*element, offset, size))
    {
        Tooltip::Hide(&s_Owner);
        return;
    }

    const float unit = UnitAt(*element, context, size.x);
    const auto anchor = TooltipPlacement::ForButton(offset.x, offset.y, size.x, size.y, unit);

    Tooltip::Line line;
    line.text = HintOf(*element);
    Tooltip::Config config;
    config.lines.push_back(std::move(line));
    config.box = Tooltip::Config::Box::ButtonHint;
    config.textAlign = Tooltip::Config::TextAlign::Center;
    config.centerHorizontally = true;
    config.anchorX = anchor.x;
    // Above first: the pointer's sprite hangs below the point it marks and would cover a hint
    // below. Pushed below (a button at the top of the screen), it starts under the sprite.
    const float cursorBottom = UI::Scaling::PositionY(UI::Scaling::GetActiveTransform(),
                                                      static_cast<float>(MouseY) + kCursorBottomUnits);
    config.anchor = Tooltip::AnchorPoint::AboveLeft;
    config.anchorY = anchor.aboveY;
    config.flipAnchorY = std::max(anchor.belowY, cursorBottom);
    config.transform = UI::Scaling::Transform{unit, unit, 0.f, 0.f, unit};
    Tooltip::Show(config, &s_Owner);
}
} // namespace UI::RmlBridge::DocumentHints
