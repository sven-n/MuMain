#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <doctest.h>

#include "Render/Text/SdlTtfFontAscent.h"

#include <string>

namespace
{
const std::string kFontDirectory = MU_TEST_FONT_DIR;
const std::string kDejaVuPath = kFontDirectory + "/DejaVuSans.ttf";
const std::string kCousinePath = kFontDirectory + "/Cousine-Regular.ttf";
const std::string kNanumPath = kFontDirectory + "/NanumGothic-Regular.ttf";
const std::string kNotoTcPath = kFontDirectory + "/NotoSansTC-Regular.otf";

constexpr float kPointSizes[] = {8.0f, 11.0f, 12.0f, 13.0f, 14.0f, 16.0f, 18.0f, 24.0f, 32.0f};
constexpr Uint32 kFullwidthLatinH = 0xFF28; // Ｈ: on the baseline, only in Noto Sans TC
// Faint enough to count the thin anti-aliased stems of small glyphs as ink.
constexpr Uint8 kInkAlpha = 32;

struct TtfSession
{
    TtfSession()
    {
        initialized = TTF_Init();
    }
    ~TtfSession()
    {
        if (initialized)
            TTF_Quit();
    }

    bool initialized = false;
};

struct FontPair
{
    FontPair(const std::string& primaryPath, const std::string& fallbackPath, float pointSize)
    {
        primary = TTF_OpenFont(primaryPath.c_str(), pointSize);
        if (primary)
            fallback = Render::Text::OpenFontWithAscent(fallbackPath.c_str(), pointSize, TTF_GetFontAscent(primary));
    }
    ~FontPair()
    {
        if (primary)
            TTF_CloseFont(primary);
        if (fallback)
            TTF_CloseFont(fallback);
    }

    TTF_Font* primary = nullptr;
    TTF_Font* fallback = nullptr;
};

// The lowest row with ink in the columns [firstColumn, endColumn).
int LowestInkRow(SDL_Surface* surface, int firstColumn, int endColumn)
{
    for (int y = surface->h - 1; y >= 0; --y)
    {
        for (int x = firstColumn; x < endColumn; ++x)
        {
            Uint8 red = 0, green = 0, blue = 0, alpha = 0;
            SDL_ReadSurfacePixel(surface, x, y, &red, &green, &blue, &alpha);
            if (alpha >= kInkAlpha)
                return y;
        }
    }
    return -1;
}
} // namespace

TEST_CASE("the sfnt ascender is read from hhea [text][sdl_ttf_font_ascent]")
{
    SDL_IOStream* noto = SDL_IOFromFile(kNotoTcPath.c_str(), "rb");
    REQUIRE(noto != nullptr);
    const auto notoAscender = Render::Text::ReadSfntAscender(noto);
    SDL_CloseIO(noto);
    REQUIRE(notoAscender.has_value());
    CHECK(notoAscender->unitsPerEm == 1000);
    CHECK(notoAscender->value == 1160);
    CHECK(notoAscender->offsetCount == 1);

    SDL_IOStream* dejaVu = SDL_IOFromFile(kDejaVuPath.c_str(), "rb");
    REQUIRE(dejaVu != nullptr);
    const auto dejaVuAscender = Render::Text::ReadSfntAscender(dejaVu);
    SDL_CloseIO(dejaVu);
    REQUIRE(dejaVuAscender.has_value());
    CHECK(dejaVuAscender->unitsPerEm == 2048);
    CHECK(dejaVuAscender->value == 1901);
}

TEST_CASE("data that is not an sfnt font has no ascender [text][sdl_ttf_font_ascent]")
{
    const char notAFont[] = "not a font file at all";
    SDL_IOStream* io = SDL_IOFromConstMem(notAFont, sizeof(notAFont));
    REQUIRE(io != nullptr);
    CHECK_FALSE(Render::Text::ReadSfntAscender(io).has_value());
    SDL_CloseIO(io);
}

TEST_CASE("a fallback takes the ascent of the font it backs [text][sdl_ttf_font_ascent]")
{
    TtfSession session;
    REQUIRE(session.initialized);
    for (const std::string& primaryPath : {kDejaVuPath, kCousinePath})
    {
        for (const std::string& fallbackPath : {kNotoTcPath, kNanumPath})
        {
            for (float pointSize : kPointSizes)
            {
                CAPTURE(primaryPath);
                CAPTURE(fallbackPath);
                CAPTURE(pointSize);
                FontPair fonts(primaryPath, fallbackPath, pointSize);
                REQUIRE(fonts.primary != nullptr);
                REQUIRE(fonts.fallback != nullptr);
                CHECK(TTF_GetFontAscent(fonts.fallback) == TTF_GetFontAscent(fonts.primary));
            }
        }
    }
}

TEST_CASE("an aligned fallback keeps its glyphs and descent [text][sdl_ttf_font_ascent]")
{
    TtfSession session;
    REQUIRE(session.initialized);
    TTF_Font* natural = TTF_OpenFont(kNotoTcPath.c_str(), 14.0f);
    FontPair fonts(kDejaVuPath, kNotoTcPath, 14.0f);
    REQUIRE(natural != nullptr);
    REQUIRE(fonts.fallback != nullptr);

    CHECK(TTF_GetFontAscent(natural) > TTF_GetFontAscent(fonts.primary));
    CHECK(TTF_GetFontDescent(fonts.fallback) == TTF_GetFontDescent(natural));
    CHECK(TTF_FontHasGlyph(fonts.fallback, 0x4E2D)); // 中
    CHECK(TTF_FontHasGlyph(fonts.fallback, 0x9AD4)); // 體
    TTF_CloseFont(natural);
}

TEST_CASE("fallback glyphs sit on the primary font's baseline [text][sdl_ttf_font_ascent]")
{
    TtfSession session;
    REQUIRE(session.initialized);
    for (float pointSize : kPointSizes)
    {
        CAPTURE(pointSize);
        FontPair fonts(kDejaVuPath, kNotoTcPath, pointSize);
        REQUIRE(fonts.fallback != nullptr);
        REQUIRE(TTF_AddFallbackFont(fonts.primary, fonts.fallback));

        // "H" comes from DejaVu Sans, the fullwidth "Ｈ" from Noto Sans TC.
        int latinWidth = 0;
        REQUIRE(TTF_GetStringSize(fonts.primary, "H", 0, &latinWidth, nullptr));
        SDL_Surface* surface = TTF_RenderText_Blended(fonts.primary, "H\xEF\xBC\xA8", 0, SDL_Color{255, 255, 255, 255});
        REQUIRE(surface != nullptr);
        REQUIRE(TTF_FontHasGlyph(fonts.fallback, kFullwidthLatinH));

        CHECK(LowestInkRow(surface, latinWidth, surface->w) == LowestInkRow(surface, 0, latinWidth));

        SDL_DestroySurface(surface);
    }
}

TEST_CASE("a font taller than the limit opens with the limit as its ascent [text][sdl_ttf_font_ascent]")
{
    TtfSession session;
    REQUIRE(session.initialized);
    constexpr float kPointSize = 14.0f;
    TTF_Font* dejaVu = TTF_OpenFont(kDejaVuPath.c_str(), kPointSize);
    REQUIRE(dejaVu != nullptr);
    const int limit = TTF_GetFontAscent(dejaVu);

    TTF_Font* noto = Render::Text::OpenFontWithAscentAtMost(kNotoTcPath.c_str(), kPointSize, limit);
    TTF_Font* cousine = Render::Text::OpenFontWithAscentAtMost(kCousinePath.c_str(), kPointSize, limit);
    TTF_Font* naturalCousine = TTF_OpenFont(kCousinePath.c_str(), kPointSize);
    REQUIRE(noto != nullptr);
    REQUIRE(cousine != nullptr);
    REQUIRE(naturalCousine != nullptr);

    CHECK(TTF_GetFontAscent(noto) == limit);
    CHECK(TTF_GetFontAscent(cousine) == TTF_GetFontAscent(naturalCousine));
    CHECK(TTF_GetFontAscent(cousine) < limit);

    TTF_CloseFont(naturalCousine);
    TTF_CloseFont(cousine);
    TTF_CloseFont(noto);
    TTF_CloseFont(dejaVu);
}
