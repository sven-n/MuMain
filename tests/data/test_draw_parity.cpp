#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Render/Drawing/DeferredDraws.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Drawing/LegacyCalls.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>

// Drawing later gives what drawing at once gives (D5.7): each scene is drawn once with the draws carried out as
// they come and once through Render::Drawing::DeferredDraws, which draws them when flushed, and the calls that reach
// the renderer are compared, contents included.

namespace
{
const std::filesystem::path PlayerFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player";

using Render::Drawing::Hash;
using Render::Drawing::HashSpan;
using CallLog = Render::Drawing::DrawCallLog;

// Draws a scene into `log`, at once or later.
template <typename Scene> void Draw(CallLog& log, Scene scene, bool later)
{
    mu::SetRendererOverride(&log);
    // Both runs start from the same state, told to the log.
    Render::Drawing::ResetLegacyState();
    log.calls.clear();
    log.triangles.clear();
    if (later)
    {
        Render::Drawing::DeferredDraws deferred;
        scene();
        deferred.Flush();
    }
    else
    {
        scene();
    }
    log.NoteFinalState();
    mu::SetRendererOverride(nullptr);
}

// The calls a scene makes, drawn at once or later.
template <typename Scene> std::vector<std::string> CallsOf(Scene scene, bool later, bool gpuShadows)
{
    CallLog log;
    log.gpuShadows = gpuShadows;
    Draw(log, scene, later);
    return log.calls;
}

// `path` names the call the path under test makes ("CachedSkinned" on the GPU, "Triangles" for the vertices the model
// skins on the CPU), so the check shows that path ran.
template <typename Scene> void CheckDrawnLaterAsAtOnce(Scene scene, const char* path = nullptr, bool gpuShadows = false)
{
    const std::vector<std::string> atOnce = CallsOf(scene, false, gpuShadows);
    const std::vector<std::string> later = CallsOf(scene, true, gpuShadows);
    REQUIRE_FALSE(atOnce.empty());
    if (path != nullptr)
    {
        const char* expected = path;
        CHECK(std::any_of(atOnce.begin(), atOnce.end(),
                          [expected](const std::string& call) { return call.rfind(expected, 0) == 0; }));
    }
    CHECK(later.size() == atOnce.size());
    // The first call that differs, and what drawing at once made there.
    const auto [atOnceCall, laterCall] = std::mismatch(atOnce.begin(), atOnce.end(), later.begin(), later.end());
    if (atOnceCall != atOnce.end() && laterCall != later.end())
    {
        INFO("call ", atOnceCall - atOnce.begin(), " drawn at once: ", *atOnceCall);
        CHECK(*laterCall == *atOnceCall);
    }
}

// A player's skeleton with a body part on it, the part a model of meshes.
struct PlayerModels
{
    TestModelSlots slots;

    PlayerModels()
    {
        const std::wstring folder = PlayerFolder.wstring() + L"/";
        REQUIRE(Models[MODEL_PLAYER].Open2(folder.c_str(), L"player.bmd"));
        REQUIRE(Models[MODEL_BODY_ARMOR].Open2(folder.c_str(), L"ArmorClass01.bmd"));
    }
};

void Place(OBJECT& o, float x, float frame, float bones[MAX_BONES][3][4])
{
    o.Type = MODEL_PLAYER;
    o.BoneTransform = bones;
    o.CurrentAction = 1;
    o.PriorAction = 0;
    o.AnimationFrame = frame;
    o.PriorAnimationFrame = frame - 1.f;
    o.Scale = 1.f;
    o.Alpha = 1.f;
    o.HiddenMesh = o.BlendMesh = -1;
    Vector(x, 300.f, 0.f, o.Position);
    Vector(0.f, 0.f, 45.f, o.Angle);
    Vector(0.f, 0.f, 0.f, o.HeadAngle);
}

// Poses `model` for `o` from the shared bones, as an object's draw does, without drawing it.
void Pose(BMD& model, const OBJECT& o)
{
    Models[MODEL_PLAYER].Animation(BoneTransform, o.AnimationFrame, o.PriorAnimationFrame, o.PriorAction, o.Angle,
                                   o.HeadAngle, false, false);
    const vec3_t box = {0.f, 0.f, 0.f};
    model.BodyScale = 1.f;
    VectorCopy(o.Position, model.BodyOrigin);
    Vector(1.f, 1.f, 1.f, model.BodyLight);
    model.Transform(BoneTransform, box, box, nullptr, true);
}

// What the cloth simulation of a part writes into its mesh (CPhysicsClothMesh::Render): a height no skin gives.
constexpr float ClothHeight = 20000.f;
void WriteCloth(const BMD& model, int mesh)
{
    // Whole positions of this pose, as the cloth writes them, whatever an earlier scene left in the scratch.
    model.EnsureCpuVertices(mesh);
    for (int j = 0; j < model.Meshs[mesh].NumVertices; ++j)
        VertexTransform[mesh][j][1] = ClothHeight;
    model.MarkCpuVerticesExternallyWritten(mesh);
}

bool DrewCloth(const CallLog& log)
{
    return std::any_of(log.triangles.begin(), log.triangles.end(),
                       [](const std::vector<mu::Vertex3D>& vertices)
                       {
                           return std::any_of(vertices.begin(), vertices.end(),
                                              [](const mu::Vertex3D& v) { return v.y == ClothHeight; });
                       });
}

// A cloth of 2 by 3 points, placed by the test instead of a simulation.
class GridCloth final : public CPhysicsCloth
{
public:
    GridCloth()
    {
        m_iNumHor = 2;
        m_iNumVer = 3;
        m_iNumVertices = m_iNumHor * m_iNumVer;
        m_pVertices = new CPhysicsVertex[m_iNumVertices];
    }
    ~GridCloth() override
    {
        Destroy();
    }

    // How the cloth blends (PCT_MASK_BLIT, PCT_MASK_BLEND, ...), which it sets before drawing its faces.
    void SetDrawType(DWORD type)
    {
        m_dwType = type;
    }

    void Place(float x)
    {
        for (int i = 0; i < m_iNumVertices; ++i)
            m_pVertices[i].Init(x + static_cast<float>(i % 2) * 10.f, 300.f + static_cast<float>(i / 2) * 10.f, 50.f);
    }
};

// A model file's skin for a BMD: every vertex on its own bone with weight 1, so it skins as the BMD does.
std::shared_ptr<const void> OneJointSkin(const BMD& model)
{
    auto skin = std::make_shared<Render::Models::Gltf::SkinnedModel>();
    skin->meshVertices.resize(static_cast<std::size_t>(model.NumMeshs));
    for (int i = 0; i < model.NumMeshs; ++i)
    {
        const Mesh_t& mesh = model.Meshs[i];
        auto& vertices = skin->meshVertices[static_cast<std::size_t>(i)];
        vertices.resize(static_cast<std::size_t>(std::max(mesh.NumVertices, mesh.NumNormals)));
        for (int j = 0; j < mesh.NumVertices; ++j)
        {
            const Vertex_t& source = mesh.Vertices[j];
            auto& vertex = vertices[static_cast<std::size_t>(j)];
            vertex.count = 1;
            vertex.joints[0] = source.Node;
            vertex.weights[0] = 1.f;
            vertex.positions[0] = {source.Position[0], source.Position[1], source.Position[2]};
        }
    }
    return skin;
}
} // namespace

TEST_CASE("Effect draws drawn later reach the renderer as when drawn at once [draw-parity]")
{
    using namespace Render::EffectDrawing;
    const auto scene = []
    {
        EffectDrawer& out = ActiveEffectDrawer();
        const float light[3] = {1.f, 0.5f, 0.25f};
        const float position[3] = {10.f, 20.f, 30.f};
        out.SetState({.blend = Blend::Glow, .depthMask = Toggle::Off});
        out.DrawSprite(Sprite(BITMAP_LIGHT, position, 2.f, 3.f, light, 45.f));
        // The caller's buffer changes after each call, as a trail's does.
        std::vector<mu::Vertex3D> quad(4);
        for (int i = 0; i < 3; ++i)
        {
            for (int corner = 0; corner < 4; ++corner)
                quad[corner] = {
                    static_cast<float>(i), static_cast<float>(corner), 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0xffffffffu};
            out.DrawQuads({.vertices = quad, .texture = 7u});
        }
        out.SetState({.blend = Blend::AlphaTest});
        out.DrawCircle(Circle(BITMAP_LIGHT, position, 1.f, 2.f, 3.f, 10.f, 1.f, 0.5f));
    };
    CheckDrawnLaterAsAtOnce(scene);
}

TEST_CASE("Characters with poses of their own drawn later are drawn in those poses [draw-parity]")
{
    PlayerModels models;
    static float firstBones[MAX_BONES][3][4];
    static float secondBones[MAX_BONES][3][4];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, firstBones);
    Place(second, 400.f, 5.5f, secondBones);
    AnimateCharacterBones(&first);
    AnimateCharacterBones(&second);
    BeginObjectDraws();
    Engine::Object::Attachments::BeginFrame();

    const vec3_t light = {1.f, 1.f, 1.f};
    int flags = RENDER_TEXTURE;
    const auto scene = [&]
    {
        RenderPartObject(&first, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true, 0, flags);
        RenderPartObject(&second, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true, 0, flags);
    };
    SUBCASE("skinned on the GPU")
    {
        CheckDrawnLaterAsAtOnce(scene, "CachedSkinned");
    }
    SUBCASE("skinned on the CPU (a wave the GPU path does not draw)")
    {
        flags = RENDER_TEXTURE | RENDER_WAVE;
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
    }
}

TEST_CASE("Objects posed into the shared bones and drawn later are drawn in their own poses [draw-parity]")
{
    PlayerModels models;
    BMD& skeleton = Models[MODEL_PLAYER];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, nullptr);
    Place(second, 400.f, 5.5f, nullptr);

    const vec3_t light = {1.f, 1.f, 1.f};
    int flags = RENDER_TEXTURE;
    const auto scene = [&]
    {
        for (OBJECT* o : {&first, &second})
        {
            skeleton.Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle,
                               o->HeadAngle, false, false);
            RenderPartObject(o, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, true, false, true, 0, flags);
        }
    };
    SUBCASE("skinned on the GPU")
    {
        CheckDrawnLaterAsAtOnce(scene, "CachedSkinned");
    }
    SUBCASE("skinned on the CPU (a wave the GPU path does not draw)")
    {
        flags = RENDER_TEXTURE | RENDER_WAVE;
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
    }
}

TEST_CASE("Shadows of objects posed into the shared bones and drawn later fall in their own poses [draw-parity]")
{
    PlayerModels models;
    BMD& skeleton = Models[MODEL_PLAYER];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, nullptr);
    Place(second, 400.f, 5.5f, nullptr);

    const vec3_t light = {1.f, 1.f, 1.f};
    const auto scene = [&]
    {
        for (OBJECT* o : {&first, &second})
        {
            skeleton.Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle,
                               o->HeadAngle, false, false);
            RenderPartObject(o, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, true, false, true);
            // The renderer here casts no shadows of its own, so the model projects its triangles on the CPU.
            Models[MODEL_BODY_ARMOR].RenderBodyShadow();
        }
    };
    CheckDrawnLaterAsAtOnce(scene, "Triangles");
}

TEST_CASE("Shadows of meshes the cloth wrote cast what the cloth wrote, drawn at once or later [draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, nullptr);
    Place(second, 400.f, 5.5f, nullptr);
    // The second pose takes the scratch the first one's cloth wrote before the shadows are drawn later.
    const auto scene = [&]
    {
        for (OBJECT* o : {&first, &second})
        {
            Pose(armor, *o);
            if (o == &first)
                WriteCloth(armor, 0);
            armor.RenderBodyShadow();
        }
    };
    SUBCASE("the shadow casts the cloth")
    {
        for (const bool later : {false, true})
        {
            CAPTURE(later);
            CallLog log;
            Draw(log, scene, later);
            CHECK(DrewCloth(log));
        }
    }
    SUBCASE("projected on the CPU")
    {
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
    }
    SUBCASE("cast on the GPU but for the cloth")
    {
        CheckDrawnLaterAsAtOnce(scene, "CachedShadow", true);
    }
}

TEST_CASE("Shadows of a model file posed into the shared bones and drawn later fall in their own poses [draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    armor.GltfModel = OneJointSkin(armor);
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, nullptr);
    Place(second, 400.f, 5.5f, nullptr);
    bool cloth = false;
    const auto scene = [&]
    {
        for (OBJECT* o : {&first, &second})
        {
            Pose(armor, *o);
            if (cloth && o == &first)
                WriteCloth(armor, 0);
            armor.RenderBodyShadow();
        }
    };
    SUBCASE("skinned on the CPU")
    {
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
    }
    SUBCASE("a mesh the cloth wrote")
    {
        cloth = true;
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
        CallLog log;
        Draw(log, scene, true);
        CHECK(DrewCloth(log));
    }
    armor.GltfModel = nullptr;
}

TEST_CASE("A mesh drawn later leaves what a later pass of the model reads as drawing it at once does [draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    OBJECT o;
    Place(o, 100.f, 2.5f, nullptr);
    const auto scene = [&]
    {
        armor.LightEnable = true;
        Pose(armor, o);
        std::fill_n(IntensityTransform[0], MAX_VERTICES, 0.f);
        // An unlit pass that reads no normals, then a lit pass of the same pose that does.
        armor.LightEnable = false;
        armor.RenderMesh(0, RENDER_COLOR | RENDER_SHADOWMAP);
        armor.LightEnable = true;
        armor.RenderMesh(0, RENDER_TEXTURE | RENDER_WAVE);
    };
    CheckDrawnLaterAsAtOnce(scene, "Triangles");
}

TEST_CASE("The parity log tells GPU draws apart by every skinning parameter [draw-parity]")
{
    static float bones[MAX_BONES][3][4];
    static float sameBones[MAX_BONES][3][4];
    const std::vector<mu::SkinnedVertex3D> vertices(3);
    const auto logOf = [&](const mu::SkinningParameters& p)
    {
        CallLog log;
        CHECK(log.RenderSkinnedTriangles(vertices, 0xffffffffu, 0u, p));
        return log.calls;
    };
    const mu::SkinningParameters drawn{.boneMatrices = {&bones[0][0][0], MAX_BONES * 12u},
                                       .bodyOrigin = {100.f, 200.f, 300.f}};

    mu::SkinningParameters elsewhere = drawn;
    elsewhere.boneMatrices = {&sameBones[0][0][0], MAX_BONES * 12u};
    elsewhere.paletteVersion = 7;
    CHECK(logOf(elsewhere) == logOf(drawn));

    const auto differs = [&](auto change)
    {
        mu::SkinningParameters changed = drawn;
        change(changed);
        return logOf(changed) != logOf(drawn);
    };
    CHECK(differs([](mu::SkinningParameters& p) { p.translate = true; }));
    CHECK(differs([](mu::SkinningParameters& p) { p.lightEnabled = true; }));
    CHECK(differs([](mu::SkinningParameters& p) { p.specularStrength = 0.5f; }));
    CHECK(differs([](mu::SkinningParameters& p) { p.normalTextureId = 3u; }));
    CHECK(differs([](mu::SkinningParameters& p) { p.normalMapScale = 1.f; }));
    CHECK(differs([](mu::SkinningParameters& p) { p.chromeTimeTerm = 1.f; }));
}

TEST_CASE("A shadow drawn later leaves the vertices a later pass of the model reads as drawing it at once does "
          "[draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    OBJECT o;
    Place(o, 100.f, 2.5f, nullptr);
    bool secondShadow = true;
    // The model moves without a new pose between the passes, as an object's shadow placed on the terrain does.
    const auto scene = [&]
    {
        Pose(armor, o);
        armor.RenderBodyShadow(-1, -1, 0, 1);
        armor.BodyOrigin[0] += 300.f;
        if (secondShadow)
            armor.RenderBodyShadow(-1, -1, 0, 1);
        else
            armor.RenderMesh(0, RENDER_TEXTURE | RENDER_WAVE);
    };
    SUBCASE("another shadow")
    {
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
        CheckDrawnLaterAsAtOnce(scene, "CachedShadow", true);
    }
    SUBCASE("a mesh skinned on the CPU")
    {
        secondShadow = false;
        CheckDrawnLaterAsAtOnce(scene, "Triangles");
        CheckDrawnLaterAsAtOnce(scene, "Triangles", true);
        // Drawn at once, the later pass is the same whether the GPU or the CPU cast the shadow before it.
        CallLog onCpu;
        CallLog onGpu;
        onGpu.gpuShadows = true;
        Draw(onCpu, scene, false);
        Draw(onGpu, scene, false);
        REQUIRE_FALSE(onCpu.triangles.empty());
        REQUIRE_FALSE(onGpu.triangles.empty());
        CHECK(HashSpan(std::span<const mu::Vertex3D>(onCpu.triangles.back())) ==
              HashSpan(std::span<const mu::Vertex3D>(onGpu.triangles.back())));
    }
}

TEST_CASE("State a caller sets between model draws drawn later reaches each draw as when drawn at once "
          "[draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    OBJECT o;
    Place(o, 100.f, 2.5f, nullptr);
    // Flags 0 give a mesh no material of its own: it is drawn with what the caller and the draws before it left.
    const auto scene = [&]
    {
        Pose(armor, o);
        // A glow pass turns the depth mask off; the caller turns blending off again, which turns it back on.
        armor.RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT);
        DisableAlphaBlend();
        armor.RenderMesh(0, 0);
        BindTexture(BITMAP_LIGHT);
        armor.RenderMesh(0, 0);
        DisableDepthTest();
        armor.RenderMesh(0, RENDER_TEXTURE);
        EnableDepthTest();
        EnableAlphaTest(false);
        armor.RenderMesh(0, 0);
        DisableAlphaBlend();
    };
    CheckDrawnLaterAsAtOnce(scene, "CachedSkinned");
}

TEST_CASE("State a caller sets between effect draws drawn later reaches each draw as when drawn at once "
          "[draw-parity]")
{
    using namespace Render::EffectDrawing;
    const auto scene = []
    {
        EffectDrawer& out = ActiveEffectDrawer();
        const float light[3] = {1.f, 0.5f, 0.25f};
        const float position[3] = {10.f, 20.f, 30.f};
        std::vector<mu::Vertex3D> quad(4, {1.f, 2.f, 3.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0xffffffffu});
        EnableAlphaBlend();
        out.DrawSprite(Sprite(BITMAP_LIGHT, position, 2.f, 3.f, light));
        DisableAlphaBlend();
        DisableDepthMask();
        out.DrawQuads({.vertices = quad, .texture = 7u});
        EnableDepthMask();
        // Quads that name no texture draw with the bound one.
        BindTexture(BITMAP_LIGHT);
        out.DrawQuads({.vertices = quad});
    };
    CheckDrawnLaterAsAtOnce(scene, "Quad3D");
}

TEST_CASE("Draws asked for inside a bloom exclusion and drawn later are drawn inside one [draw-parity]")
{
    PlayerModels models;
    static float firstBones[MAX_BONES][3][4];
    static float secondBones[MAX_BONES][3][4];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, firstBones);
    Place(second, 400.f, 5.5f, secondBones);
    AnimateCharacterBones(&first);
    AnimateCharacterBones(&second);
    BeginObjectDraws();
    Engine::Object::Attachments::BeginFrame();

    const vec3_t light = {1.f, 1.f, 1.f};
    const auto scene = [&]
    {
        {
            // As a character is drawn in Classic, so post-processing leaves it alone.
            const mu::ScopedBloomExclusion classic;
            RenderPartObject(&first, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true, 0,
                             RENDER_TEXTURE | RENDER_BRIGHT);
        }
        RenderPartObject(&second, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true, 0,
                         RENDER_TEXTURE | RENDER_BRIGHT);
    };
    CheckDrawnLaterAsAtOnce(scene, "CachedSkinned");
}

TEST_CASE("A cloth's shadow drawn later falls where the cloth was when it was asked for [draw-parity]")
{
    PlayerModels models;
    BMD& armor = Models[MODEL_BODY_ARMOR];
    OBJECT o;
    Place(o, 100.f, 2.5f, nullptr);
    GridCloth cape;
    GridCloth otherCape;
    const auto scene = [&]
    {
        cape.Place(100.f);
        otherCape.Place(700.f);
        Pose(armor, o);
        armor.RenderBodyShadow(-1, -1, -1, -1, &cape, 1);
        armor.RenderBodyShadow(-1, -1, -1, -1, &otherCape, 1);
        // The next update moves the cloth before this frame is drawn, as on a render thread.
        cape.Place(400.f);
    };
    CheckDrawnLaterAsAtOnce(scene, "Triangles");
}

TEST_CASE("A cloth drawn later is drawn with the state it was asked for in [draw-parity]")
{
    GridCloth cape;
    GridCloth otherCape;
    cape.Place(100.f);
    otherCape.Place(400.f);
    // Blended, it glows; drawn when asked for instead of in its place, it would take the state before it.
    otherCape.SetDrawType(PCT_MASK_BLEND);
    const auto scene = [&]
    {
        EnableAlphaBlend();
        cape.Render();
        DisableAlphaBlend();
        BindTexture(BITMAP_LIGHT);
        otherCape.Render();
    };
    CheckDrawnLaterAsAtOnce(scene, "Quad3D");
}

TEST_CASE("Guild marks drawn later are drawn where their characters stand [draw-parity]")
{
    PlayerModels models;
    static float firstBones[MAX_BONES][3][4];
    static float secondBones[MAX_BONES][3][4];
    OBJECT first;
    OBJECT second;
    Place(first, 100.f, 2.5f, firstBones);
    Place(second, 400.f, 5.5f, secondBones);
    AnimateCharacterBones(&first);
    AnimateCharacterBones(&second);
    BeginObjectDraws();
    Engine::Object::Attachments::BeginFrame();
    const auto scene = [&]
    {
        RenderGuild(&first, MODEL_BODY_ARMOR);
        RenderGuild(&second, MODEL_BODY_ARMOR);
    };
    CheckDrawnLaterAsAtOnce(scene);
}

TEST_CASE("Reorderable ranges and the sprite matrix drawn later hold the draws asked for in them [draw-parity]")
{
    PlayerModels models;
    static float firstBones[MAX_BONES][3][4];
    OBJECT first;
    Place(first, 100.f, 2.5f, firstBones);
    AnimateCharacterBones(&first);
    BeginObjectDraws();
    Engine::Object::Attachments::BeginFrame();
    const vec3_t light = {1.f, 1.f, 1.f};
    const auto scene = [&]
    {
        {
            const Render::Drawing::ScopedReorderableDraws reorderable;
            RenderPartObject(&first, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true);
        }
        // Sprites are drawn in view space.
        BeginSprite();
        const float position[3] = {10.f, 20.f, 30.f};
        Render::EffectDrawing::ActiveEffectDrawer().DrawSprite(
            Render::EffectDrawing::Sprite(BITMAP_LIGHT, position, 2.f, 3.f, light));
        EndSprite();
        RenderPartObject(&first, MODEL_BODY_ARMOR, nullptr, light, 1.f, 0, 0, 0, false, false, true);
    };
    CheckDrawnLaterAsAtOnce(scene, "CachedSkinned");
}

TEST_CASE("Damage numbers drawn later are drawn with the state and matrix they were asked for in [draw-parity]")
{
    // In front of a camera that looks down the view axis.
    float savedCamera[3][4];
    std::memcpy(savedCamera, g_Camera.Matrix, sizeof(savedCamera));
    const float identity[3][4] = {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}};
    std::memcpy(g_Camera.Matrix, identity, sizeof(identity));
    // As RenderPoints draws them: in view space, alpha tested, over everything.
    const auto scene = []
    {
        BeginSprite();
        EnableAlphaTest();
        DisableDepthTest();
        const vec3_t position = {10.f, 20.f, -300.f};
        const vec3_t color = {1.f, 0.5f, 0.f};
        RenderNumber(position, 1234, color, 1.f, 10.f);
        RenderNumber(position, -2, color, 1.f, 10.f);
        EnableDepthTest();
        EndSprite();
    };
    CheckDrawnLaterAsAtOnce(scene, "Quad3D");
    std::memcpy(g_Camera.Matrix, savedCamera, sizeof(savedCamera));
}

TEST_CASE("A zen pile drawn later keeps its place among the draws around it [draw-parity]")
{
    PlayerModels models;
    BMD& model = Models[MODEL_BODY_ARMOR];
    const std::array<mu::Vertex3D, 4> quad{};
    const auto scene = [&]
    {
        EnableAlphaBlend();
        Render::EffectDrawing::ActiveEffectDrawer().DrawQuads({.vertices = quad});
        // As RenderZen draws it, with one coin's vertices.
        model.BeginRenderCoinHeap();
        model.EndRenderCoinHeap(1);
        EnableAlphaBlend();
        Render::EffectDrawing::ActiveEffectDrawer().DrawQuads({.vertices = quad});
    };
    CheckDrawnLaterAsAtOnce(scene, "Triangles");
}

TEST_CASE("A mesh's own alpha threshold drawn later applies to that mesh only [draw-parity]")
{
    const std::array<mu::Vertex3D, 4> quad{};
    const auto scene = [&]
    {
        SetAlphaFuncRef(0.f);
        Render::EffectDrawing::ActiveEffectDrawer().DrawQuads({.vertices = quad});
        SetAlphaFuncRef(0.25f);
        Render::EffectDrawing::ActiveEffectDrawer().DrawQuads({.vertices = quad});
    };
    CheckDrawnLaterAsAtOnce(scene, "Quad3D");
    const std::vector<std::string> later = CallsOf(scene, true, false);
    REQUIRE(later.size() >= 2);
    CHECK(later[0].find("alphaRef 0.000000") != std::string::npos);
    CHECK(later[1].find("alphaRef 0.250000") != std::string::npos);
}

TEST_CASE("The log takes a pass that pops what the frame pushed before the log stood in, as the water pass does "
          "[draw-parity]")
{
    CallLog log;
    log.PopMatrix();
    log.PopMatrix();
    const std::array<mu::Vertex3D, 4> quad{};
    log.RenderQuad3D(quad, 0u);
    REQUIRE(log.calls.size() == 1);
    CHECK(log.calls[0].find("matrix camera") != std::string::npos);
}
