#include <filesystem>
#include <string_view>

#include <doctest.h>

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
