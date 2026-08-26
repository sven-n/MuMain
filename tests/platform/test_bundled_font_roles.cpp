#include <string_view>

#include <doctest.h>

#include "Core/Platform/BundledFonts.h"

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
