#pragma once

namespace Scenes::Lighting
{

// Publishes this frame's scene lighting to the renderer. Call once per frame, after the scene has
// set up its camera and fog (BeginOpengl and any map fog override) and before its lit draws.
void SubmitSceneLighting();

// Publishes the dynamic lights recorded this frame to the renderer (none unless Enhanced dynamic
// lights are active). Call once per frame after the scene has drawn, so lights its draws added
// count too; the renderer applies them when it replays the frame.
void SubmitDynamicLights();

// The point lights the ground applies per pixel (Enhanced dynamic lights), from the lights recorded
// so far. Call right before the scene draws its terrain.
void PinWorldDynamicLights();

} // namespace Scenes::Lighting
