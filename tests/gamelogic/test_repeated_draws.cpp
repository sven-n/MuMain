#include "stdafx.h"
#include "doctest.h"

#include "Camera/CameraProjection.h"
#include "Camera/CameraState.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include "RepeatedDraws.h"

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

namespace
{
// A frame's view over the middle of the map, built as the update builds it; the camera is given back afterwards.
struct ViewScope
{
    const CameraState savedCamera = g_Camera;
    const unsigned int savedWidth = WindowWidth;
    const unsigned int savedHeight = WindowHeight;

    ViewScope()
    {
        WindowWidth = 1280;
        WindowHeight = 720;
        g_Camera = CameraState();
        Vector(-48.5f, 0.f, 33.3f, g_Camera.Angle);
        Vector(12800.f, 12800.f, 900.f, g_Camera.Position);
        CameraProjection::UpdateView(g_Camera, 0, 0, 1280, 720);
        CreateFrustrum(1.f, 1.f);
    }

    ~ViewScope()
    {
        g_Camera = savedCamera;
        WindowWidth = savedWidth;
        WindowHeight = savedHeight;
    }

    // A point `distance` along the view, behind the camera when negative.
    void Ahead(float distance, vec3_t out) const
    {
        for (int i = 0; i < 3; i++)
            out[i] = g_Camera.Position[i] - g_Camera.Matrix[2][i] * distance;
    }
};

void ClearEffects()
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Effects[i].Live = false;
}

int CreateEffectAt(const vec3_t position)
{
    vec3_t at;
    VectorCopy(position, at);
    vec3_t angle{0.f, 0.f, 0.f};
    vec3_t light{1.f, 1.f, 1.f};
    CreateEffect(BITMAP_MAGIC, at, angle, light, 8);
    for (int i = MAX_EFFECTS - 1; i >= 0; --i)
    {
        if (Effects[i].Live && VectorCompare(Effects[i].Position, position))
            return i;
    }
    return -1;
}
} // namespace

TEST_CASE(
    "The update culls the effects against the frame's view, and the effect pass only draws what it marked [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    vec3_t behind;
    view.Ahead(1000.f, ahead);
    view.Ahead(-1000.f, behind);
    const int shown = CreateEffectAt(ahead);
    const int hidden = CreateEffectAt(behind);
    REQUIRE(shown >= 0);
    REQUIRE(hidden >= 0);

    CullEffects();
    CHECK(Effects[shown].Visible);
    CHECK_FALSE(Effects[hidden].Visible);
    const unsigned int marked = Effects[shown].DrawnFrame;

    CullEffects();
    CHECK(Effects[shown].DrawnFrame == marked + 1);

    ClearEffects();
}

TEST_CASE("Drawing the effect passes again and again changes nothing the next update reads [effects]")
{
    ViewScope view;
    ClearEffects();
    vec3_t ahead;
    view.Ahead(1000.f, ahead);
    REQUIRE(CreateEffectAt(ahead) >= 0);
    CullEffects();

    RepeatedDraws::CheckChangesNothing(
        []
        {
            RenderEffects();
            RenderEffects(true);
        },
        3);

    ClearEffects();
}
