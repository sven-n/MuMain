#pragma once

namespace Render::RmlUi
{
// Registers `decorator: additive-image(<colour> <image>)`: stretches the image over the element's
// padding box so that its colour, multiplied by <colour> and the element's opacity, is ADDED to what
// is behind it, like a bitmap the native code draws under BlendMode::Glow (ONE, ONE), which
// EnableAlphaBlend() selects (the login scene's logo glow). RmlUi blends premultiplied (ONE,
// ONE_MINUS_SRC_ALPHA), so the vertices carry the colour with zero alpha. Call once after
// Rml::Initialise().
void RegisterAdditiveImageDecorator();
} // namespace Render::RmlUi
