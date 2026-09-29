#pragma once

namespace Render::RmlUi
{
// Registers `decorator: additive-fill(<colour>)`: fills the element's padding box so that its colour
// is ADDED to what is behind it, like the native BlendMode::Glow (ONE, ONE) that EnableAlphaBlend()
// selects (the world labels' health bars). RmlUi blends premultiplied (ONE, ONE_MINUS_SRC_ALPHA), so
// the fill is the colour with zero alpha. Call once after Rml::Initialise().
void RegisterAdditiveFillDecorator();
} // namespace Render::RmlUi
