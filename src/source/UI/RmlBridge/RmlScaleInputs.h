#pragma once

namespace Rml
{
class Context;
}

// The scales a theme composes its layout from, set once per context on its root element so every
// document inherits them as custom properties:
//   --ui-scale       the UI scale: RmlUi's dp ratio, the panels' and native text's scale
//   --hud-scale      the bottom HUD's scale
//   --dock-scale     the docked windows' scale (a larger cap than the panels')
//   --overlay-scale-x / --overlay-scale-y   the stretched 640x480 screen (world-space overlays)
// base.rcss's .stage and .hud-board place a 640x480 reference screen with them.
namespace UI::RmlBridge
{
void ApplyScaleInputs(Rml::Context* context);
}
