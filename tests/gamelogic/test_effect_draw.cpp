#include "stdafx.h"
#include "doctest.h"

#include "Camera/CameraState.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/GameMaps/GMHellas.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
using Render::EffectDrawing::Blend;
using Render::EffectDrawing::SpriteDraw;

// Records every call that reaches the renderer, with its arguments, so two draw paths can be compared exactly.
class RecordingRenderer final : public mu::IMuRenderer
{
public:
    std::vector<std::string> calls;

    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t texture) override { Note("Quad2D", texture); }
    void RenderTriangles(std::span<const mu::Vertex3D> v, std::uint32_t texture) override { Vertices("Triangles", v, texture); }
    void RenderQuad3D(std::span<const mu::Vertex3D> v, std::uint32_t texture) override { Vertices("Quad3D", v, texture); }
    void RenderQuadStrip(std::span<const mu::Vertex3D> v, std::uint32_t texture) override { Vertices("Strip", v, texture); }
    void RenderLines(std::span<const mu::Vertex3D> v, std::uint32_t texture) override { Vertices("Lines", v, texture); }
    void SetBlendMode(mu::BlendMode mode) override { Note("Blend", static_cast<int>(mode)); }
    void DisableBlend() override { Note("DisableBlend", 0); }
    void SetDepthTest(bool on) override { Note("DepthTest", on); }
    void SetDepthMask(bool on) override { Note("DepthMask", on); }
    void SetCullFace(bool on) override { Note("CullFace", on); }
    void SetAlphaTest(bool on) override { Note("AlphaTest", on); }
    void SetTexture2D(bool on) override { Note("Texture2D", on); }
    void SetFogEnabled(bool on) override { Note("Fog", on); }
    void BindTexture(int texture) override { Note("Bind", texture); }
    void BeginBloomExclusion() override
    {
        ++m_bloomExcluded;
        Note("BeginBloomExclusion", 0);
    }
    void EndBloomExclusion() override
    {
        --m_bloomExcluded;
        Note("EndBloomExclusion", 0);
    }
    [[nodiscard]] bool IsBloomExcluded() const override { return m_bloomExcluded > 0; }
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}

private:
    int m_bloomExcluded = 0;

    void Note(const char* name, long long value) { calls.push_back(std::string(name) + " " + std::to_string(value)); }

    void Vertices(const char* name, std::span<const mu::Vertex3D> vertices, std::uint32_t texture)
    {
        std::string call = std::string(name) + " " + std::to_string(texture);
        for (const mu::Vertex3D& vertex : vertices)
        {
            char bytes[sizeof(mu::Vertex3D) * 2 + 1];
            const auto* raw = reinterpret_cast<const unsigned char*>(&vertex);
            for (std::size_t i = 0; i < sizeof(mu::Vertex3D); ++i)
                std::snprintf(bytes + i * 2, 3, "%02x", raw[i]);
            call += " ";
            call += bytes;
        }
        calls.push_back(call);
    }
};

// Installs the recorder, with the camera at the origin looking down -z and the render state reset.
struct RecordingScope
{
    RecordingRenderer renderer;
    float savedMatrix[3][4];

    RecordingScope()
    {
        std::memcpy(savedMatrix, g_Camera.Matrix, sizeof(savedMatrix));
        std::memset(g_Camera.Matrix, 0, sizeof(g_Camera.Matrix));
        for (int i = 0; i < 3; ++i)
            g_Camera.Matrix[i][i] = 1.f;
        mu::SetRendererOverride(&renderer);
    }

    ~RecordingScope()
    {
        mu::SetRendererOverride(nullptr);
        std::memcpy(g_Camera.Matrix, savedMatrix, sizeof(savedMatrix));
    }

    // Runs a draw from the same starting render state and returns what reached the renderer.
    template <typename Draw> std::string Record(Draw draw)
    {
        DisableAlphaBlend();
        renderer.calls.clear();
        draw();
        std::string joined;
        for (const std::string& call : renderer.calls)
            joined += call + '\n';
        return joined;
    }
};

// What RenderSprites did for one sprite before effects were drawn from descriptions: the loop's blend, its bloom
// exclusion, then the quad.
void LegacySpriteDraw(const SpriteDraw& d, int type, int subType)
{
    if (type == BITMAP_FORMATION_MARK)
        EnableAlphaTest();
    else if (subType == 0)
        EnableAlphaBlend();
    else if (subType == 1)
        EnableAlphaBlendMinus();
    else if (subType == 2)
        EnableAlphaTest();
    else if (subType == 3)
        EnableAlphaBlend2();
    std::optional<mu::ScopedBloomExclusion> exclusion;
    if (d.bloomExcluded)
        exclusion.emplace();
    vec3_t position = {d.position[0], d.position[1], d.position[2]};
    vec3_t light = {d.light[0], d.light[1], d.light[2]};
    RenderSprite(d.texture, position, d.width, d.height, light, d.rotation, d.u, d.v, d.uWidth, d.vHeight);
}

OBJECT MakeSprite(int type, int subType)
{
    OBJECT o;
    o.Live = true;
    o.Type = type;
    o.SubType = subType;
    o.Scale = 1.f;
    o.AnimationFrame = 1.f;
    Vector(10.f, -20.f, -100.f, o.Position);
    Vector(0.9f, 0.5f, 0.25f, o.Light);
    return o;
}
} // namespace

TEST_CASE("A sprite drawn from its description reaches the renderer exactly as the old sprite draw did [effects]")
{
    RecordingScope scope;
    for (const int subType : {0, 1, 2, 3, 4})
    {
        for (const bool excluded : {false, true})
        {
            for (const float rotation : {0.f, 37.f})
            {
                CAPTURE(subType);
                CAPTURE(excluded);
                CAPTURE(rotation);
                OBJECT o = MakeSprite(BITMAP_LIGHT, subType);
                o.Angle[2] = rotation;
                const SpriteDraw draw = DescribeSprite(o, excluded);
                const auto expected = scope.Record([&] { LegacySpriteDraw(draw, o.Type, o.SubType); });
                const auto actual =
                    scope.Record([&] { Render::EffectDrawing::ActiveEffectDrawer().DrawSprite(draw); });
                CHECK(actual == expected);
                CHECK_FALSE(actual.empty());
            }
        }
    }
}

TEST_CASE("A sprite's description carries its blend, its quad and its bloom exclusion [effects]")
{
    OBJECT o = MakeSprite(BITMAP_LIGHT, 1);
    o.Angle[2] = 45.f;
    SpriteDraw draw = DescribeSprite(o, true);
    CHECK(draw.state.blend == Blend::Subtract);
    CHECK(draw.texture == BITMAP_LIGHT);
    CHECK(draw.position[2] == -100.f);
    CHECK(draw.light[1] == 0.5f);
    CHECK(draw.rotation == 45.f);
    CHECK(draw.bloomExcluded);
    CHECK(draw.u == 0.f);
    CHECK(draw.uWidth == 1.f);

    CHECK(DescribeSprite(MakeSprite(BITMAP_LIGHT, 0), false).state.blend == Blend::Glow);
    CHECK(DescribeSprite(MakeSprite(BITMAP_LIGHT, 2), false).state.blend == Blend::AlphaTest);
    CHECK(DescribeSprite(MakeSprite(BITMAP_LIGHT, 3), false).state.blend == Blend::Blend2);
    // Any other subtype keeps the state the previous sprite left, as the old loop did.
    CHECK(DescribeSprite(MakeSprite(BITMAP_LIGHT, 4), false).state.blend == Blend::Unchanged);

    // The formation mark draws one cell of its 3x3 sheet, 64 units wide, alpha-tested whatever its subtype.
    const SpriteDraw mark = DescribeSprite(MakeSprite(BITMAP_FORMATION_MARK, 5), false);
    CHECK(mark.state.blend == Blend::AlphaTest);
    CHECK(mark.width == 64.f);
    CHECK(mark.height == 64.f);
    CHECK(mark.u == 0.66f);
    CHECK(mark.v == 0.33f);
    CHECK(mark.uWidth == 0.33f);
    CHECK(mark.vHeight == 0.33f);
}

TEST_CASE("Another effect drawer receives the sprites instead of the default one [effects]")
{
    struct Counting final : Render::EffectDrawing::EffectDrawer
    {
        int sprites = 0;
        void SetState(const Render::EffectDrawing::EffectState&) override {}
        void DrawSprite(const SpriteDraw&) override { ++sprites; }
        void DrawQuads(const Render::EffectDrawing::QuadsDraw&) override {}
        void DrawDecal(const Render::EffectDrawing::DecalDraw&) override {}
        void DrawCircle(const Render::EffectDrawing::CircleDraw&) override {}
        void DrawPlane(const Render::EffectDrawing::PlaneDraw&) override {}
    } counting;
    RecordingScope scope;
    Render::EffectDrawing::SetEffectDrawer(&counting);
    const auto calls =
        scope.Record([] { Render::EffectDrawing::ActiveEffectDrawer().DrawSprite(SpriteDraw{}); });
    Render::EffectDrawing::SetEffectDrawer(nullptr);
    CHECK(counting.sprites == 1);
    CHECK(calls.empty());
}

namespace
{
using Render::EffectDrawing::EffectState;
using Render::EffectDrawing::Toggle;

// Keeps what an effect describes, in order, as text.
struct ListingDrawer final : Render::EffectDrawing::EffectDrawer
{
    std::vector<std::string> items;
    std::vector<SpriteDraw> sprites;

    void SetState(const EffectState& state) override
    {
        items.push_back("state " + std::to_string(static_cast<int>(state.blend)) + " " +
                        std::to_string(static_cast<int>(state.depthTest)) + " " +
                        std::to_string(static_cast<int>(state.additiveTexture)));
    }

    void DrawSprite(const SpriteDraw& draw) override
    {
        items.push_back("sprite " + std::to_string(draw.texture));
        sprites.push_back(draw);
    }

    void DrawQuads(const Render::EffectDrawing::QuadsDraw& draw) override
    {
        items.push_back("quads " + std::to_string(draw.vertices.size()) + " " + std::to_string(draw.texture));
    }

    void DrawDecal(const Render::EffectDrawing::DecalDraw& draw) override
    {
        items.push_back("decal " + std::to_string(draw.texture));
    }

    void DrawCircle(const Render::EffectDrawing::CircleDraw& draw) override
    {
        items.push_back("circle " + std::to_string(draw.texture));
    }

    void DrawPlane(const Render::EffectDrawing::PlaneDraw& draw) override
    {
        items.push_back("plane " + std::to_string(draw.width) + " " + std::to_string(draw.height));
    }
};

PARTICLE MakeParticle(int type, int subType)
{
    PARTICLE p{};
    p.Live = true;
    p.Type = type;
    p.TexType = type;
    p.SubType = subType;
    p.Scale = 1.f;
    p.Rotation = 30.f;
    Vector(10.f, -20.f, -100.f, p.Position);
    Vector(0.9f, 0.5f, 0.25f, p.Light);
    return p;
}
} // namespace

TEST_CASE("Each effect render state reaches the renderer as the legacy state call did [effects]")
{
    RecordingScope scope;
    struct Case
    {
        EffectState state;
        void (*legacy)();
    };
    const Case cases[] = {
        {{.blend = Blend::AlphaTest}, [] { EnableAlphaTest(); }},
        {{.blend = Blend::AlphaTestKeepDepthMask}, [] { EnableAlphaTest(false); }},
        {{.blend = Blend::Glow}, [] { EnableAlphaBlend(); }},
        {{.blend = Blend::Subtract}, [] { EnableAlphaBlendMinus(); }},
        {{.blend = Blend::Blend2}, [] { EnableAlphaBlend2(); }},
        {{.blend = Blend::Blend3}, [] { EnableAlphaBlend3(); }},
        {{.depthTest = Toggle::On}, [] { EnableDepthTest(); }},
        {{.depthTest = Toggle::Off}, [] { DisableDepthTest(); }},
        {{.additiveTexture = Toggle::On},
         [] { mu::GetRenderer().SetTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); }},
        {{.additiveTexture = Toggle::Off},
         [] { mu::GetRenderer().SetTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); }},
    };
    int index = 0;
    for (const Case& c : cases)
    {
        CAPTURE(index++);
        // Each from the state the other leaves, so the wrappers' caches don't hide a call.
        for (const bool depthOn : {false, true})
        {
            const auto start = [depthOn]
            {
                if (depthOn)
                    EnableDepthTest();
                else
                    DisableDepthTest();
            };
            start();
            const auto expected = scope.Record(c.legacy);
            start();
            const auto actual =
                scope.Record([&] { Render::EffectDrawing::ActiveEffectDrawer().SetState(c.state); });
            CHECK(actual == expected);
        }
    }
}

TEST_CASE("A particle describes its sprites and state changes in the order its draw made them [effects]")
{
    SUBCASE("a bubble draws the cell of its frame")
    {
        ListingDrawer out;
        PARTICLE p = MakeParticle(BITMAP_BUBBLE, 0);
        p.Frame = 5;
        DescribeParticle(p, 0, 32.f, 32.f, out);
        REQUIRE(out.sprites.size() == 1);
        CHECK(out.sprites[0].u == 5 % 3 * 0.25f + 0.005f);
        CHECK(out.sprites[0].v == 5 / 3 * 0.25f + 0.005f);
        CHECK(out.sprites[0].uWidth == 0.25f - 0.01f);
        CHECK(out.sprites[0].rotation == 0.f);
    }
    SUBCASE("a torch fire stacks three sprites, 10 units apart")
    {
        ListingDrawer out;
        DescribeParticle(MakeParticle(BITMAP_TORCH_FIRE, 0), 0, 32.f, 32.f, out);
        REQUIRE(out.sprites.size() == 3);
        CHECK(out.sprites[0].position[2] == -100.f);
        CHECK(out.sprites[1].position[2] == -110.f);
        CHECK(out.sprites[2].position[2] == -120.f);
    }
    SUBCASE("additive smoke switches the texture to additive around its sprite only")
    {
        ListingDrawer out;
        DescribeParticle(MakeParticle(BITMAP_ADV_SMOKE + 1, 2), 0, 32.f, 32.f, out);
        const std::vector<std::string> expected = {
            "state 0 0 1", "state 7 0 0", "sprite " + std::to_string(BITMAP_ADV_SMOKE + 1), "state 0 0 2"};
        CHECK(out.items == expected);
    }
    SUBCASE("a cloud turns one way in even slots and the other way in odd ones")
    {
        ListingDrawer even;
        ListingDrawer odd;
        PARTICLE p = MakeParticle(BITMAP_CLOUD, 0);
        p.TurningForce[0] = 1.f;
        const auto previousTime = WorldTime;
        WorldTime = 1000.0;
        DescribeParticle(p, 2, 32.f, 32.f, even);
        DescribeParticle(p, 3, 32.f, 32.f, odd);
        WorldTime = previousTime;
        REQUIRE(even.sprites.size() == 1);
        REQUIRE(odd.sprites.size() == 1);
        const float start = p.StartPosition[1];
        CHECK(even.sprites[0].rotation - start == doctest::Approx(20.f));
        CHECK(odd.sprites[0].rotation - start == doctest::Approx(-20.f));
    }
}

namespace
{
// A joint with `tails` valid tail entries one unit apart, drawn with face one.
std::unique_ptr<JOINT> MakeJoint(int type, int subType, int tails)
{
    auto joint = std::make_unique<JOINT>();
    *joint = JOINT{};
    joint->Live = true;
    joint->Type = type;
    joint->TexType = BITMAP_JOINT_ENERGY;
    joint->SubType = subType;
    joint->RenderType = RENDER_TYPE_ALPHA_BLEND;
    joint->RenderFace = RENDER_FACE_ONE;
    joint->NumTails = tails;
    joint->MaxTails = tails + 1;
    Vector(1.f, 1.f, 1.f, joint->Light);
    for (int t = 0; t <= tails; ++t)
    {
        for (int corner = 0; corner < 4; ++corner)
            Vector(static_cast<float>(t), static_cast<float>(corner), -100.f, joint->Tails[t][corner]);
    }
    return joint;
}
} // namespace

TEST_CASE("Effect quads and ground marks reach the renderer as the old trail and decal draws did [effects]")
{
    RecordingScope scope;
    const mu::Vertex3D quad[4] = {
        {0.f, 0.f, -10.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0xffffffffu},
        {1.f, 0.f, -10.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0xffffffffu},
        {1.f, 1.f, -10.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0xff808080u},
        {0.f, 1.f, -10.f, 0.f, 0.f, 1.f, 1.f, 1.f, 0xff808080u},
    };
    CHECK(scope.Record([&] { mu::GetRenderer().RenderQuad3D(quad, 7u); }) ==
          scope.Record([&] { Render::EffectDrawing::ActiveEffectDrawer().DrawQuads({quad, 7u}); }));

    vec3_t light = {0.5f, 0.25f, 1.f};
    const auto legacy = scope.Record([&] { RenderTerrainAlphaBitmap(BITMAP_MAGIC + 1, 300.f, 400.f, 2.f, 3.f, light, 45.f, 0.5f, 7.f); });
    const auto described = scope.Record(
        [&]
        {
            Render::EffectDrawing::ActiveEffectDrawer().DrawDecal({.texture = BITMAP_MAGIC + 1,
                                                                   .x = 300.f,
                                                                   .y = 400.f,
                                                                   .sizeX = 2.f,
                                                                   .sizeY = 3.f,
                                                                   .light = {0.5f, 0.25f, 1.f},
                                                                   .rotation = 45.f,
                                                                   .alpha = 0.5f,
                                                                   .height = 7.f});
        });
    CHECK(described == legacy);
}

TEST_CASE("A joint describes its blend, its texture and one batch of trail quads [effects]")
{
    ListingDrawer out;
    const auto joint = MakeJoint(BITMAP_JOINT_ENERGY, 0, 3);
    DescribeJoint(*joint, out);
    const std::vector<std::string> expected = {"state 4 0 0", "state 0 0 0",
                                               "quads 12 " + std::to_string(BITMAP_JOINT_ENERGY)};
    CHECK(out.items == expected);
}

TEST_CASE("A falling thunder's ground mark is lit by the fade the update gives it, and its draw leaves it [effects]")
{
    ListingDrawer out;
    const auto joint = MakeJoint(BITMAP_JOINT_THUNDER + 1, 6, 2);
    joint->Velocity = 0.4f;
    DescribeJoint(*joint, out);
    REQUIRE(!out.items.empty());
    CHECK(out.items.back() == "state 1 0 0");
    CHECK(std::find(out.items.begin(), out.items.end(), "decal " + std::to_string(BITMAP_MAGIC + 1)) !=
          out.items.end());
    CHECK(joint->Velocity == 0.4f);
}

TEST_CASE("Effect rings and water marks reach the renderer as the old circle and water draws did [effects]")
{
    RecordingScope scope;
    vec3_t position = {300.f, 400.f, -100.f};
    vec3_t light = {0.5f, 0.25f, 1.f};
    CHECK(scope.Record([&] { RenderCircle(BITMAP_MAGIC + 2, position, 90.f, 130.f, 200.f, 30.f, 0.f, 0.f); }) ==
          scope.Record(
              [&]
              {
                  Render::EffectDrawing::ActiveEffectDrawer().DrawCircle(
                      Render::EffectDrawing::Circle(BITMAP_MAGIC + 2, position, 90.f, 130.f, 200.f, 30.f, 0.f, 0.f));
              }));
    CHECK(scope.Record([&] { RenderWaterTerrain(BITMAP_MAGIC + 1, 300.f, 400.f, 2.f, 3.f, light, -45.f); }) ==
          scope.Record(
              [&]
              {
                  Render::EffectDrawing::ActiveEffectDrawer().DrawDecal(
                      Render::EffectDrawing::WaterDecal(BITMAP_MAGIC + 1, 300.f, 400.f, 2.f, 3.f, light, -45.f));
              }));
}

TEST_CASE("A fading magic mark describes its ground mark with the light the update fades, and leaves it [effects]")
{
    OBJECT o;
    o.Live = true;
    o.Visible = true;
    o.Type = BITMAP_MAGIC + 1;
    o.SubType = 13;
    o.Scale = 2.f;
    o.LifeTime = 10.f;
    Vector(300.f, 400.f, 0.f, o.Position);
    Vector(0.5f, 0.5f, 0.5f, o.Light);
    ListingDrawer out;
    DescribeEffectShadow(o, out);
    CHECK(std::find(out.items.begin(), out.items.end(), "decal " + std::to_string(BITMAP_MAGIC + 1)) !=
          out.items.end());
    CHECK(o.Light[0] == 0.5f);
}

TEST_CASE("A leaf plane reaches the renderer as the old translated plane draw did [effects]")
{
    RecordingScope scope;
    vec3_t angle = {10.f, 20.f, 30.f};
    Render::EffectDrawing::PlaneDraw plane;
    plane.position[0] = 300.f;
    plane.position[1] = 400.f;
    plane.position[2] = 50.f;
    AngleMatrix(angle, plane.rotation);
    plane.width = 3.f;
    plane.height = 4.f;
    const auto legacy = scope.Record(
        [&]
        {
            mu::GetRenderer().PushMatrix();
            mu::GetRenderer().Translate(300.f, 400.f, 50.f);
            float matrix[3][4];
            AngleMatrix(angle, matrix);
            RenderPlane3D(3.f, 4.f, matrix);
            mu::GetRenderer().PopMatrix();
        });
    CHECK(scope.Record([&] { Render::EffectDrawing::ActiveEffectDrawer().DrawPlane(plane); }) == legacy);
}
