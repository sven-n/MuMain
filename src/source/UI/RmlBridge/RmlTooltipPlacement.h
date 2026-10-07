#pragma once

// The shared tooltip's placement rules, free of RmlUi so they can be tested on their own.
namespace UI::RmlBridge::TooltipPlacement
{
// CNewUIButton::Render()'s hint around a button drawn at (left, top, width, height) in window
// pixels, `unit` pixels per original unit: its centre 3 units right of the button's, its top 2 units
// below the button, or its bottom 2 units above it.
struct ButtonHintAnchor
{
    float x = 0.f;
    float belowY = 0.f;
    float aboveY = 0.f;
};
ButtonHintAnchor ForButton(float left, float top, float width, float height, float unit);

// The top of a box `height` tall: below `anchorY` (its top there) or above it (its bottom there),
// as asked. When that leaves the viewport and a `flipAnchorY` is given, the other way from that
// anchor, if it fits there; then clamped into the viewport either way.
float Top(float anchorY, bool above, const float* flipAnchorY, float height, float viewportHeight);
} // namespace UI::RmlBridge::TooltipPlacement
