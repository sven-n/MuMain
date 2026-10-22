#include <doctest.h>

#include "Render/Renderer/SceneLighting.h"

#include <cstddef>

TEST_CASE("scene lighting uniform matches the HLSL cbuffer registers [render][scene_lighting]")
{
    CHECK(sizeof(mu::SceneLightingUniform) == 128);
    CHECK(offsetof(mu::SceneLightingUniform, lightDirection) == 0);
    CHECK(offsetof(mu::SceneLightingUniform, lightColor) == 16);
    CHECK(offsetof(mu::SceneLightingUniform, diffuseCurve) == 32);
    CHECK(offsetof(mu::SceneLightingUniform, rimParameters) == 48);
    CHECK(offsetof(mu::SceneLightingUniform, specularParameters) == 64);
    CHECK(offsetof(mu::SceneLightingUniform, fogColor) == 80);
    CHECK(offsetof(mu::SceneLightingUniform, fogParameters) == 96);
    CHECK(offsetof(mu::SceneLightingUniform, cameraPosition) == 112);
}

TEST_CASE("the light mood's highlight knee rides in the diffuse curve [render][scene_lighting]")
{
    mu::SceneLighting lighting;
    lighting.highlightKnee = 0.8f;

    CHECK(mu::PackSceneLightingUniform(lighting, {}).diffuseCurve[3] == 0.8f);
}

TEST_CASE("default scene lighting keeps the Classic diffuse curve [render][scene_lighting]")
{
    const mu::SceneLightingUniform uniform = mu::PackSceneLightingUniform({}, {});

    CHECK(uniform.diffuseCurve[0] == doctest::Approx(0.8f));
    CHECK(uniform.diffuseCurve[1] == doctest::Approx(0.4f));
    CHECK(uniform.diffuseCurve[2] == doctest::Approx(0.2f));
    // The Classic cut-off: no highlight roll-off.
    CHECK(uniform.diffuseCurve[3] == 1.0f);
    CHECK(uniform.rimParameters[0] == doctest::Approx(0.35f));
    CHECK(uniform.rimParameters[1] == doctest::Approx(3.0f));
    CHECK(uniform.specularParameters[0] == doctest::Approx(1.0f));
    CHECK(uniform.specularParameters[1] == doctest::Approx(32.0f));
    CHECK(uniform.fogColor[3] == 0.0f);
    CHECK(uniform.fogParameters[2] == 0.0f);
}

TEST_CASE("scene lighting packs light, fog and camera into their registers [render][scene_lighting]")
{
    mu::SceneLighting lighting{};
    lighting.lightDirection[0] = 3.0f;
    lighting.lightDirection[1] = 0.0f;
    lighting.lightDirection[2] = 4.0f;
    lighting.lightColor[0] = 0.5f;
    lighting.cameraPosition[0] = 12800.0f;
    lighting.cameraPosition[1] = 13000.0f;
    lighting.cameraPosition[2] = 300.0f;
    lighting.rimStrength = 0.0f;

    mu::SceneFog fog{};
    fog.enabled = true;
    fog.start = 2000.0f;
    fog.end = 2500.0f;
    fog.color[2] = 0.25f;

    const mu::SceneLightingUniform uniform = mu::PackSceneLightingUniform(lighting, fog);

    CHECK(uniform.lightDirection[0] == doctest::Approx(0.6f));
    CHECK(uniform.lightDirection[1] == doctest::Approx(0.0f));
    CHECK(uniform.lightDirection[2] == doctest::Approx(0.8f));
    CHECK(uniform.lightColor[0] == doctest::Approx(0.5f));
    CHECK(uniform.rimParameters[0] == 0.0f);
    CHECK(uniform.fogColor[2] == doctest::Approx(0.25f));
    CHECK(uniform.fogColor[3] == 1.0f);
    CHECK(uniform.fogParameters[0] == doctest::Approx(2000.0f));
    CHECK(uniform.fogParameters[1] == doctest::Approx(2500.0f));
    CHECK(uniform.fogParameters[2] == doctest::Approx(mu::EnhancedFogDensity(2000.0f, 2500.0f)));
    CHECK(uniform.cameraPosition[0] == doctest::Approx(12800.0f));
    CHECK(uniform.cameraPosition[1] == doctest::Approx(13000.0f));
    CHECK(uniform.cameraPosition[2] == doctest::Approx(300.0f));
}

TEST_CASE("Enhanced fog reaches 0.99 of the fog colour at the Classic fog end [render][scene_lighting]")
{
    constexpr float fogStart = 2000.0f;
    constexpr float fogEnd = 2500.0f;
    const float density = mu::EnhancedFogDensity(fogStart, fogEnd);

    CHECK(mu::EnhancedFogAmount(fogEnd, fogStart, density) == doctest::Approx(0.99f).epsilon(1e-4));
    CHECK(mu::EnhancedFogAmount(fogEnd * 2.0f, fogStart, density) > 0.99f);
}

TEST_CASE("Enhanced fog is clear up to the Classic fog start and rises monotonically [render][scene_lighting]")
{
    constexpr float fogStart = 1500.0f;
    constexpr float fogEnd = 1875.0f;
    const float density = mu::EnhancedFogDensity(fogStart, fogEnd);

    CHECK(mu::EnhancedFogAmount(0.0f, fogStart, density) == 0.0f);
    CHECK(mu::EnhancedFogAmount(fogStart, fogStart, density) == 0.0f);
    float previous = 0.0f;
    for (float depth = fogStart; depth <= fogEnd; depth += 25.0f)
    {
        const float amount = mu::EnhancedFogAmount(depth, fogStart, density);
        CHECK(amount >= previous);
        previous = amount;
    }
}

TEST_CASE("an empty fog range disables Enhanced fog [render][scene_lighting]")
{
    CHECK(mu::EnhancedFogDensity(2000.0f, 2000.0f) == 0.0f);
    CHECK(mu::EnhancedFogDensity(2000.0f, 1000.0f) == 0.0f);
    CHECK(mu::EnhancedFogAmount(5000.0f, 2000.0f, 0.0f) == 0.0f);
}

TEST_CASE("a zero light direction packs as zero instead of NaN [render][scene_lighting]")
{
    mu::SceneLighting lighting{};
    lighting.lightDirection[0] = lighting.lightDirection[1] = lighting.lightDirection[2] = 0.0f;

    const mu::SceneLightingUniform uniform = mu::PackSceneLightingUniform(lighting, {});

    CHECK(uniform.lightDirection[0] == 0.0f);
    CHECK(uniform.lightDirection[1] == 0.0f);
    CHECK(uniform.lightDirection[2] == 0.0f);
}
