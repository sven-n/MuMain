#include "Render/RmlUi/RmlPremultiply.h"

void Render::RmlUi::PremultiplyAlpha(std::uint8_t* rgba, std::size_t pixelCount)
{
    if (rgba == nullptr)
        return;
    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        std::uint8_t* pixel = rgba + i * 4;
        const int alpha = pixel[3];
        for (int c = 0; c < 3; ++c)
            pixel[c] = static_cast<std::uint8_t>(int(pixel[c]) * alpha / 255);
    }
}
