#pragma once

#include <functional>

// Native drawing that has to composite ABOVE RmlUi's main context, through the seam
// MuRenderer::SetPostRmlUiCallback opens (MuRenderer.h). The main context always composites last
// among the ordinary passes, so live 3D drawn from a window's own Render() lands under every
// panel in the frame; registering it here is what puts it on its window instead of beneath it.
//
// Drawing order is registration order, so a family owning several of these registers one entry and
// walks its own z-order inside it rather than registering each window separately.
namespace UI::RmlBridge::OverlayRender
{
using Owner = const void*;

void Register(Owner owner, std::function<void()> draw);
void Unregister(Owner owner);

// Drained once per frame from Winmain.cpp's SetPostRmlUiCallback.
void RenderAll();
} // namespace UI::RmlBridge::OverlayRender
