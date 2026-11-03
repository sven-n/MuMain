#include "Render/Lighting/SunShadow.h"

#include <doctest.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <initializer_list>

namespace
{
using Render::Lighting::BuildSunShadowView;
using Render::Lighting::SunDirection;

// A camera like the game's: high above the ground, looking down at the hero at `target`.
glm::mat4 SceneViewProjection(const glm::vec3& target)
{
    const glm::mat4 projection = glm::perspectiveRH_ZO(glm::radians(35.0f), 16.0f / 9.0f, 20.0f, 5000.0f);
    const glm::mat4 view = glm::lookAtRH(target + glm::vec3(0.0f, -900.0f, 1300.0f), target, glm::vec3(0, 0, 1));
    return projection * view;
}

glm::vec3 Ndc(const glm::mat4& viewProjection, const glm::vec3& world)
{
    const glm::vec4 clip = viewProjection * glm::vec4(world, 1.0f);
    return glm::vec3(clip) / clip.w;
}
} // namespace

TEST_CASE("the sun stands opposite where the shadows fall, as high as their length says [render][sun]")
{
    // Shadows towards -x (Classic), half as long as the caster is tall.
    const glm::vec3 toSun = SunDirection(180.0f, 0.5f);
    CHECK(glm::length(toSun) == doctest::Approx(1.0f));
    CHECK(toSun.x > 0.0f);
    CHECK(toSun.y == doctest::Approx(0.0f).epsilon(1e-6));
    CHECK(toSun.x / toSun.z == doctest::Approx(0.5f));

    const glm::vec3 east = SunDirection(90.0f, 0.5f);
    CHECK(east.y < 0.0f);
    CHECK(east.x == doctest::Approx(0.0f).epsilon(1e-6));
}

TEST_CASE("the shadow map covers the ground the camera sees [render][sun]")
{
    const glm::vec3 hero(12800.0f, 12800.0f, 200.0f);
    const glm::mat4 scene = SceneViewProjection(hero);
    const auto view = BuildSunShadowView(SunDirection(180.0f, 0.5f), scene, 0.0f, 600.0f, 2048u);

    // Every ground point at the screen's corners and centre lands inside the map's depth and area.
    const glm::mat4 sceneFromClip = glm::inverse(scene);
    for (const float x : {-1.0f, 0.0f, 1.0f})
    {
        for (const float y : {-1.0f, 0.0f, 1.0f})
        {
            const glm::vec4 nearPoint = sceneFromClip * glm::vec4(x, y, 0.0f, 1.0f);
            const glm::vec4 farPoint = sceneFromClip * glm::vec4(x, y, 1.0f, 1.0f);
            const glm::vec3 a = glm::vec3(nearPoint) / nearPoint.w;
            const glm::vec3 b = glm::vec3(farPoint) / farPoint.w;
            const glm::vec3 ground = a + (b - a) * ((hero.z - a.z) / (b.z - a.z));
            const glm::vec3 shadow = Ndc(view.viewProjection, ground);
            CHECK(std::abs(shadow.x) < 1.0f);
            CHECK(std::abs(shadow.y) < 1.0f);
            CHECK(shadow.z > 0.0f);
            CHECK(shadow.z < 1.0f);
        }
    }
    // A caster high above the ground still lies in front of the map's near plane.
    CHECK(Ndc(view.viewProjection, hero + glm::vec3(0.0f, 0.0f, 2000.0f)).z > 0.0f);
    // Clip w is 2, so the vertex shaders' near-plane clip (w - 1) keeps every vertex.
    CHECK((view.viewProjection * glm::vec4(hero, 1.0f)).w == doctest::Approx(2.0f));
}

TEST_CASE("a recorded scene matrix turns into the sun's [render][sun]")
{
    const glm::mat4 scene = SceneViewProjection({12800.0f, 12800.0f, 200.0f});
    const auto view = BuildSunShadowView(SunDirection(135.0f, 0.5f), scene, 0.0f, 600.0f, 2048u);
    // A model's draw records scene * model; the sun pass draws it with sunFromScene * that.
    const glm::mat4 model = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(12900.0f, 12750.0f, 180.0f)), 0.7f,
                                        glm::vec3(0.0f, 0.0f, 1.0f));
    for (const glm::vec3 vertex : {glm::vec3(0.0f), glm::vec3(40.0f, -30.0f, 150.0f), glm::vec3(-60.0f, 10.0f, 5.0f)})
    {
        const glm::vec3 expected = Ndc(view.viewProjection * model, vertex);
        const glm::vec3 converted = Ndc(view.sunFromScene * (scene * model), vertex);
        CHECK(converted.x == doctest::Approx(expected.x).epsilon(1e-4));
        CHECK(converted.y == doctest::Approx(expected.y).epsilon(1e-4));
        CHECK(converted.z == doctest::Approx(expected.z).epsilon(1e-4));
    }
}

TEST_CASE("a camera moving less than a texel keeps the shadow map where it is [render][sun]")
{
    const glm::vec3 toSun = SunDirection(180.0f, 0.5f);
    const glm::vec3 hero(12800.0f, 12800.0f, 200.0f);
    const auto view = BuildSunShadowView(toSun, SceneViewProjection(hero), 0.0f, 600.0f, 2048u);
    const float texel = view.box.size / 2048.0f;
    const auto nudged = BuildSunShadowView(toSun, SceneViewProjection(hero + glm::vec3(texel * 0.01f, 0.0f, 0.0f)),
                                           0.0f, 600.0f, 2048u);
    CHECK(nudged.box == view.box);

    // Further away, the map follows, by whole texels.
    const auto moved =
        BuildSunShadowView(toSun, SceneViewProjection(hero + glm::vec3(300.0f, 200.0f, 0.0f)), 0.0f, 600.0f, 2048u);
    CHECK_FALSE(moved.box == view.box);
    const glm::vec2 steps = moved.box.origin / (moved.box.size / 2048.0f);
    CHECK(steps.x == doctest::Approx(std::round(steps.x)).epsilon(1e-3));
    CHECK(steps.y == doctest::Approx(std::round(steps.y)).epsilon(1e-3));
}
