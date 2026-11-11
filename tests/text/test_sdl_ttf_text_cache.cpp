#include <SDL3_ttf/SDL_ttf.h>

#include <doctest.h>

#include "Render/Text/SdlTtfTextCache.h"

namespace
{
struct TtfFixture
{
    TtfFixture()
    {
        initialized = TTF_Init();
        if (initialized)
            font = TTF_OpenFont(MU_TEST_FONT_PATH, 14.0f);
        if (font != nullptr)
            engine = TTF_CreateSurfaceTextEngine();
    }

    ~TtfFixture()
    {
        if (engine != nullptr)
            TTF_DestroySurfaceTextEngine(engine);
        if (font != nullptr)
            TTF_CloseFont(font);
        if (initialized)
            TTF_Quit();
    }

    bool initialized = false;
    TTF_Font* font = nullptr;
    TTF_TextEngine* engine = nullptr;
};
}

TEST_CASE("recurring font and string reuse one configured text [text][sdl_ttf_text_cache]")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 4);
    auto first = cache.Prepare(fixture.font, "Character");
    auto second = cache.Prepare(fixture.font, "Character");

    REQUIRE(first.text != nullptr);
    CHECK(first.width > 0);
    CHECK(first.height > 0);
    CHECK(second.text == first.text);
    CHECK(second.width == first.width);
    CHECK(second.height == first.height);
    CHECK(cache.Size() == 1);
}

TEST_CASE("whole-cache eviction is bounded and deterministic [text][sdl_ttf_text_cache]")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 2);
    cache.Prepare(fixture.font, "one");
    cache.Prepare(fixture.font, "two");
    CHECK(cache.Size() == 2);
    cache.Prepare(fixture.font, "three");
    CHECK(cache.Size() == 1);
}

TEST_CASE("zero capacity uses a transient text without retaining it [text][sdl_ttf_text_cache]")
{
    TtfFixture fixture;
    REQUIRE(fixture.engine != nullptr);
    Render::Text::SdlTtfTextCache cache(fixture.engine, 0);
    auto prepared = cache.Prepare(fixture.font, "transient");
    REQUIRE(prepared.text != nullptr);
    CHECK(prepared.transient != nullptr);
    CHECK(cache.Size() == 0);
}
