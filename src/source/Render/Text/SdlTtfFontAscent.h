#pragma once

#include <SDL3/SDL_iostream.h>

#include <cstdint>
#include <optional>

struct TTF_Font;

// SDL_ttf draws every glyph at its own font's ascent below the line top. A
// fallback font with a taller ascent than the primary font therefore draws its
// glyphs below the primary font's baseline (Noto Sans TC, 1.16 em, under DejaVu
// Sans, 0.93 em: Han characters sit about 3 px low at UI sizes). These helpers
// open a font whose ascender is rewritten while SDL_ttf reads it, so both fonts
// share one baseline. The font file itself is not changed.
namespace Render::Text
{
// The ascender FreeType uses for an sfnt (TrueType/OpenType) font, and the
// file offsets that hold it.
struct SfntAscender
{
    static constexpr int kMaxOffsets = 2;

    std::uint16_t unitsPerEm = 0;
    std::int16_t value = 0;
    std::int64_t offsets[kMaxOffsets] = {};
    int offsetCount = 0;
};

[[nodiscard]] std::optional<SfntAscender> ReadSfntAscender(SDL_IOStream* io);

// Opens `path` at `pointSize` with an ascent of exactly `ascentPx` pixels.
// Returns nullptr on failure, with the reason in SDL_GetError().
[[nodiscard]] TTF_Font* OpenFontWithAscent(const char* path, float pointSize, int ascentPx);

// Opens `path` at `pointSize` with its ascent limited to `maxAscentPx` pixels.
// A font whose ascent already fits opens unchanged.
[[nodiscard]] TTF_Font* OpenFontWithAscentAtMost(const char* path, float pointSize, int maxAscentPx);
} // namespace Render::Text
