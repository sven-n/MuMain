#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"

extern CHARACTER* Hero;

TEST_CASE("A camera shake applies only from a source near the hero [gamelogic]")
{
    static CHARACTER hero;
    CHARACTER* const savedHero = Hero;
    const float savedQuake = EarthQuake;
    Vector(1000.f, 1000.f, 0.f, hero.Object.Position);
    const float radius = TERRAIN_SCALE * 12.f;

    SUBCASE("no hero, no shake")
    {
        Hero = nullptr;
        EarthQuake = 0.3f;
        vec3_t source = {1000.f, 1000.f, 0.f};
        CHECK_FALSE(SetEarthQuakeAt(source, -0.6f));
        CHECK(EarthQuake == 0.3f);
    }

    SUBCASE("full strength at the hero, fading with distance")
    {
        Hero = &hero;
        vec3_t atHero = {1000.f, 1000.f, 500.f};
        CHECK(SetEarthQuakeAt(atHero, -0.6f));
        CHECK(EarthQuake == doctest::Approx(-0.6f));

        vec3_t halfway = {1000.f + radius * 0.5f, 1000.f, 0.f};
        CHECK(SetEarthQuakeAt(halfway, -0.6f));
        CHECK(EarthQuake == doctest::Approx(-0.3f));
    }

    SUBCASE("a far source neither starts nor clears a shake")
    {
        Hero = &hero;
        EarthQuake = -0.4f;
        vec3_t distant = {1000.f, 1000.f + radius, 0.f};
        CHECK_FALSE(SetEarthQuakeAt(distant, -0.6f));
        CHECK_FALSE(SetEarthQuakeAt(distant, 0.f));
        CHECK(EarthQuake == -0.4f);
    }

    SUBCASE("a near source clears its own shake")
    {
        Hero = &hero;
        EarthQuake = -0.4f;
        vec3_t close = {1100.f, 1000.f, 0.f};
        CHECK(SetEarthQuakeAt(close, 0.f));
        CHECK(EarthQuake == 0.f);
    }

    Hero = savedHero;
    EarthQuake = savedQuake;
}
