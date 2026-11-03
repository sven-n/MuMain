#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Render/Drawing/DeferredDraws.h"
#include "Render/Drawing/LegacyCalls.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

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

std::uint64_t Hash(const void* data, std::size_t size, std::uint64_t hash = 1469598103934665603ull)
{
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i)
        hash = (hash ^ bytes[i]) * 1099511628211ull;
    return hash;
}

template <typename T> std::uint64_t HashSpan(std::span<const T> values)
{
    return Hash(values.data(), values.size_bytes());
}

template <typename T> void HashInto(std::uint64_t& hash, const T& value)
{
    hash = Hash(&value, sizeof(value), hash);
}

// What a GPU-skinned draw is drawn with, field by field: the palette version and the bones' address are cache keys,
// the matrices themselves are compared.
std::uint64_t HashOf(const mu::SkinningParameters& p)
{
    std::uint64_t hash = HashSpan(p.boneMatrices);
    HashInto(hash, p.bodyOrigin);
    HashInto(hash, p.bodyScale);
    HashInto(hash, p.boneScale);
    HashInto(hash, p.restPoseScale);
    HashInto(hash, p.lightDirection);
    HashInto(hash, p.highlightKnee);
    HashInto(hash, p.textureCoordinateOffset);
    HashInto(hash, p.chromeWave);
    HashInto(hash, p.chromeWave2);
    HashInto(hash, p.chromeLight);
    HashInto(hash, p.chromeTimeTerm);
    HashInto(hash, p.textureCoordinates);
    HashInto(hash, p.translate);
    HashInto(hash, p.lightEnabled);
    HashInto(hash, p.specularStrength);
    HashInto(hash, p.normalTextureId);
    HashInto(hash, p.normalMapScale);
    return hash;
}

std::uint64_t HashOf(const mu::ShadowProjection& projection)
{
    std::uint64_t hash = Hash(projection.origin, sizeof(projection.origin));
    HashInto(hash, projection.shearX);
    HashInto(hash, projection.shearHeight);
    HashInto(hash, projection.specialHeight);
    HashInto(hash, projection.terrain);
    return hash;
}

// Every draw that reaches the renderer, with a hash of what it draws (skinned draws' bones included) and the state
// it is drawn with.
class CallLog final : public mu::IMuRenderer
{
public:
    std::vector<std::string> calls;
    // The vertices of each RenderTriangles call.
    std::vector<std::vector<mu::Vertex3D>> triangles;
    // Casts body shadows on the GPU (RenderCachedSkinnedShadow); otherwise the models project them on the CPU.
    bool gpuShadows = false;

    // What the draws and state calls leave after the last draw, which the next draws in the frame inherit.
    void NoteFinalState()
    {
        calls.push_back("final" + State());
    }

    void RenderQuad2D(std::span<const mu::Vertex2D> v, std::uint32_t texture) override
    {
        Note("Quad2D", texture, HashSpan(v));
    }
    void RenderTriangles(std::span<const mu::Vertex3D> v, std::uint32_t texture) override
    {
        Note("Triangles", texture, HashSpan(v));
        triangles.emplace_back(v.begin(), v.end());
    }
    void RenderQuad3D(std::span<const mu::Vertex3D> v, std::uint32_t texture) override
    {
        Note("Quad3D", texture, HashSpan(v));
    }
    void RenderQuadStrip(std::span<const mu::Vertex3D> v, std::uint32_t texture) override
    {
        Note("Strip", texture, HashSpan(v));
    }
    void RenderLines(std::span<const mu::Vertex3D> v, std::uint32_t texture) override
    {
        Note("Lines", texture, HashSpan(v));
    }
    [[nodiscard]] bool RenderSkinnedTriangles(std::span<const mu::SkinnedVertex3D> v, std::uint32_t color,
                                              std::uint32_t texture, const mu::SkinningParameters& p) override
    {
        return Skinned("Skinned", HashSpan(v), color, texture, p);
    }
    [[nodiscard]] bool RenderWeightedSkinnedTriangles(std::span<const mu::SkinnedWeightedVertex3D> v,
                                                      std::uint32_t color, std::uint32_t texture,
                                                      const mu::SkinningParameters& p) override
    {
        return Skinned("WeightedSkinned", HashSpan(v), color, texture, p);
    }
    [[nodiscard]] bool IsSkinnedMeshCached(const void* mesh) const override
    {
        return m_cached.contains(mesh);
    }
    [[nodiscard]] bool CacheSkinnedMesh(const void* mesh, std::span<const mu::SkinnedVertex3D> v) override
    {
        return Cache(mesh, HashSpan(v));
    }
    [[nodiscard]] bool CacheWeightedSkinnedMesh(const void* mesh,
                                                std::span<const mu::SkinnedWeightedVertex3D> v) override
    {
        return Cache(mesh, HashSpan(v));
    }
    [[nodiscard]] bool RenderCachedSkinnedMesh(const void* mesh, std::uint32_t color, std::uint32_t texture,
                                               const mu::SkinningParameters& p) override
    {
        const auto cached = m_cached.find(mesh);
        return cached != m_cached.end() && Skinned("CachedSkinned", cached->second, color, texture, p);
    }
    [[nodiscard]] bool SetTerrainHeights(std::span<const float>, std::span<const std::uint16_t>, std::uint32_t) override
    {
        return gpuShadows;
    }
    [[nodiscard]] bool RenderCachedSkinnedShadow(const void* mesh, std::uint32_t color, const mu::SkinningParameters& p,
                                                 const mu::ShadowProjection& projection) override
    {
        const auto cached = m_cached.find(mesh);
        if (!gpuShadows || cached == m_cached.end())
            return false;
        calls.push_back("CachedShadow " + std::to_string(color) + " " + std::to_string(cached->second) + " " +
                        std::to_string(HashOf(p)) + " " + std::to_string(HashOf(projection)) + State());
        return true;
    }
    // State calls set what the following draws are drawn with; a draw is logged with it, as the SDL_GPU renderer
    // records it with each command.
    void SetBlendMode(mu::BlendMode mode) override
    {
        m_blend = static_cast<int>(mode);
    }
    void DisableBlend() override
    {
        m_blend = -1;
    }
    void SetDepthTest(bool on) override
    {
        m_depthTest = on;
    }
    void SetDepthMask(bool on) override
    {
        m_depthMask = on;
    }
    void SetCullFace(bool on) override
    {
        m_cullFace = on;
    }
    void SetAlphaTest(bool on) override
    {
        m_alphaTest = on;
    }
    void SetTexture2D(bool on) override
    {
        m_texture2D = on;
    }
    void SetFogEnabled(bool on) override
    {
        m_fog = on;
    }
    void BindTexture(int texture) override
    {
        m_bound = texture;
    }
    void SetTexEnv(int, int, int mode) override
    {
        m_textureMode = mode;
    }
    void BeginBloomExclusion() override
    {
        ++m_bloomExclusions;
    }
    void EndBloomExclusion() override
    {
        --m_bloomExclusions;
    }
    [[nodiscard]] bool IsBloomExcluded() const override
    {
        return m_bloomExclusions > 0;
    }
    [[nodiscard]] std::size_t BeginReorderableDraws() override
    {
        return m_reorderable++;
    }
    void EndReorderableDraws(std::size_t mark) override
    {
        m_reorderable = mark;
    }
    void PushMatrix() override
    {
        m_matrices.push_back(m_matrices.back());
    }
    void PopMatrix() override
    {
        m_matrices.pop_back();
    }
    void LoadIdentity() override
    {
        m_matrices.back().clear();
    }
    void Translate(float x, float y, float z) override
    {
        m_matrices.back() += " T" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    }
    void Rotate(float angle, float x, float y, float z) override
    {
        m_matrices.back() +=
            " R" + std::to_string(angle) + "," + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    }
    void Scale(float x, float y, float z) override
    {
        m_matrices.back() += " S" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    }
    void MultMatrix(const float* m) override
    {
        m_matrices.back() += " M" + std::to_string(Hash(m, sizeof(float) * 16));
    }
    void LoadMatrix(const float* m) override
    {
        m_matrices.back() = " L" + std::to_string(Hash(m, sizeof(float) * 16));
    }
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}

private:
    std::map<const void*, std::uint64_t> m_cached;
    int m_blend = 0;
    bool m_depthTest = true;
    bool m_depthMask = true;
    bool m_cullFace = true;
    bool m_alphaTest = false;
    bool m_texture2D = true;
    bool m_fog = false;
    int m_bound = -1;
    int m_textureMode = 0;
    int m_bloomExclusions = 0;
    std::size_t m_reorderable = 0;
    // The scene's camera, which the sprite matrix replaces.
    std::vector<std::string> m_matrices{std::string(" camera")};

    // The state a draw is drawn with.
    [[nodiscard]] std::string State() const
    {
        return " | blend " + std::to_string(m_blend) + " depth " + std::to_string(m_depthTest) +
               std::to_string(m_depthMask) + " cull " + std::to_string(m_cullFace) + " alphaTest " +
               std::to_string(m_alphaTest) + " texture " + std::to_string(m_texture2D) + " " + std::to_string(m_bound) +
               " mode " + std::to_string(m_textureMode) + " fog " + std::to_string(m_fog) + " bloomExcluded " +
               std::to_string(m_bloomExclusions > 0) + " reorderable " + std::to_string(m_reorderable) + " matrix" +
               m_matrices.back();
    }

    void Note(const char* name, long long value, std::uint64_t contents = 0)
    {
        calls.push_back(std::string(name) + " " + std::to_string(value) + " " + std::to_string(contents) + State());
    }

    bool Cache(const void* mesh, std::uint64_t contents)
    {
        m_cached[mesh] = contents;
        return true;
    }

    bool Skinned(const char* name, std::uint64_t vertices, std::uint32_t color, std::uint32_t texture,
                 const mu::SkinningParameters& p)
    {
        calls.push_back(std::string(name) + " " + std::to_string(texture) + " " + std::to_string(color) + " " +
                        std::to_string(vertices) + " " + std::to_string(HashOf(p)) + State());
        return true;
    }
};

// Draws a scene into `log`, at once or later.
template <typename Scene> void Draw(CallLog& log, Scene scene, bool later)
{
    mu::SetRendererOverride(&log);
    // Both runs start from the same state, told to the log: the legacy state calls skip what they think is set
    // already, so each is set the other way first.
    EnableAlphaTest();
    DisableTexture();
    DisableDepthTest();
    DisableDepthMask();
    DisableCullFace();
    EnableAlphaBlend();
    DisableAlphaBlend();
    EnableDepthTest();
    BindTexture(0);
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
