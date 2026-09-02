#include <filesystem>
#include <string_view>

#include <doctest.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Core/Platform/BundledFonts.h"
#include "UI/Scaling/UITransform.h"

int MouseX;
int MouseY;

TEST_CASE("bundled font roles resolve deterministically [platform][bundled_font]")
{
    CHECK(ResolveBundledFont("").family == std::string_view("DejaVu Sans"));
    CHECK(ResolveBundledFont("unknown").family == std::string_view("DejaVu Sans"));
    CHECK(ResolveBundledFont("Liberation Sans").regular ==
          std::string_view("fonts/LiberationSans-Regular.ttf"));
    CHECK(ResolveBundledFont("Liberation Sans").bold ==
          std::string_view("fonts/LiberationSans-Bold.ttf"));
    CHECK(std::string_view(kBundledFixedFont.family) == "Cousine");
    CHECK(std::string_view(kBundledFixedFont.regular) == "fonts/Cousine-Regular.ttf");
    CHECK(std::string_view(kBundledFallbackFont.family) == "Nanum Gothic");
    CHECK(std::string_view(kBundledFallbackFont.regular) == "fonts/NanumGothic-Regular.ttf");
}

TEST_CASE("bundled font paths follow the normalized runtime directory [platform][bundled_font]")
{
    const std::filesystem::path relativePath = "fonts/DejaVuSans.ttf";
    CHECK(ResolveBundledFontPath(relativePath) == std::filesystem::current_path() / relativePath);
}

TEST_CASE("bundled font roles load their maximum responsive size [platform][bundled_font]")
{
    using UI::Scaling::FontRole;
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Normal) == 16);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Bold) == 16);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Big) == 32);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Fixed) == 18);
}

TEST_CASE("bundled fallback font covers Hangul and Latin accents [platform][bundled_font]")
{
    REQUIRE(TTF_Init());
    const std::filesystem::path fallbackPath =
        std::filesystem::path(MU_TEST_ASSET_SOURCE) / kBundledFallbackFont.regular;
    const std::filesystem::path basePath =
        std::filesystem::path(MU_TEST_ASSET_SOURCE) / ResolveBundledFont("DejaVu Sans").regular;
    TTF_Font* base = TTF_OpenFont(basePath.string().c_str(), 16.0f);
    TTF_Font* fallback = TTF_OpenFont(fallbackPath.string().c_str(), 16.0f);
    REQUIRE(base != nullptr);
    REQUIRE(fallback != nullptr);

    CHECK_FALSE(TTF_FontHasGlyph(base, 0xD55C)); // 한
    REQUIRE(TTF_AddFallbackFont(base, fallback));
    CHECK(TTF_FontHasGlyph(base, 0xD55C)); // 한
    CHECK(TTF_FontHasGlyph(base, 0xAE00)); // 글
    CHECK(TTF_FontHasGlyph(base, 0x00E1)); // á
    CHECK(TTF_FontHasGlyph(base, 0x00E3)); // ã

    TTF_CloseFont(base);
    TTF_CloseFont(fallback);
    TTF_Quit();
}
