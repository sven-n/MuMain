#pragma once

#include <cstddef>
#include <cstdint>

namespace Render::RmlUi
{
// RmlUi blends premultiplied (ONE, ONE_MINUS_SRC_ALPHA; RenderInterface_SDL_GPU and RmlUi's own
// LoadTexture premultiply every file they read), while the game's textures are straight alpha:
// scale each RGBA pixel's colour by its alpha, rounding like RmlUi's loader (x * a / 255).
void PremultiplyAlpha(std::uint8_t* rgba, std::size_t pixelCount);
} // namespace Render::RmlUi
