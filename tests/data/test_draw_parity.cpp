#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Drawing/DeferredDraws.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/ZzzEffect.h"
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

// Every call that reaches the renderer, with a hash of what it draws, skinned draws' bones included.
class CallLog final : public mu::IMuRenderer
{
public:
    std::vector<std::string> calls;

    void RenderQuad2D(std::span<const mu::Vertex2D> v, std::uint32_t texture) override
    {
        Note("Quad2D", texture, HashSpan(v));
    }
    void RenderTriangles(std::span<const mu::Vertex3D> v, std::uint32_t texture) override
    {
        Note("Triangles", texture, HashSpan(v));
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
    void SetBlendMode(mu::BlendMode mode) override
    {
        Note("Blend", static_cast<int>(mode));
    }
    void DisableBlend() override
    {
        Note("DisableBlend", 0);
    }
    void SetDepthTest(bool on) override
    {
        Note("DepthTest", on);
    }
    void SetDepthMask(bool on) override
    {
        Note("DepthMask", on);
    }
    void SetCullFace(bool on) override
    {
        Note("CullFace", on);
    }
    void SetAlphaTest(bool on) override
    {
        Note("AlphaTest", on);
    }
    void SetTexture2D(bool on) override
    {
        Note("Texture2D", on);
    }
    void SetFogEnabled(bool on) override
    {
        Note("Fog", on);
    }
    void BindTexture(int texture) override
    {
        Note("Bind", texture);
    }
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}

private:
    std::map<const void*, std::uint64_t> m_cached;

    void Note(const char* name, long long value, std::uint64_t contents = 0)
    {
        calls.push_back(std::string(name) + " " + std::to_string(value) + " " + std::to_string(contents));
    }

    bool Cache(const void* mesh, std::uint64_t contents)
    {
        m_cached[mesh] = contents;
        return true;
    }

    // The palette version is a cache key, not what is drawn: the bone matrices themselves are compared.
    bool Skinned(const char* name, std::uint64_t vertices, std::uint32_t color, std::uint32_t texture,
                 const mu::SkinningParameters& p)
    {
        std::uint64_t hash = HashSpan(p.boneMatrices);
        hash = Hash(p.bodyOrigin, sizeof(p.bodyOrigin), hash);
        hash = Hash(&p.bodyScale, sizeof(float) * 3, hash);
        hash = Hash(p.lightDirection, sizeof(p.lightDirection), hash);
        hash = Hash(&p.highlightKnee, sizeof(float), hash);
        hash = Hash(p.textureCoordinateOffset, sizeof(p.textureCoordinateOffset), hash);
        hash = Hash(&p.chromeWave, sizeof(float) * 5, hash);
        hash = Hash(&p.textureCoordinates, sizeof(p.textureCoordinates), hash);
        calls.push_back(std::string(name) + " " + std::to_string(texture) + " " + std::to_string(color) + " " +
                        std::to_string(vertices) + " " + std::to_string(hash));
        return true;
    }
};

// The calls a scene makes, drawn at once or later.
template <typename Scene> std::vector<std::string> CallsOf(Scene scene, bool later)
{
    CallLog log;
    mu::SetRendererOverride(&log);
    // Both runs start from the same state: the legacy state calls skip what they think is set already.
    DisableAlphaBlend();
    EnableDepthTest();
    EnableDepthMask();
    log.calls.clear();
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
    mu::SetRendererOverride(nullptr);
    return log.calls;
}

// `path` names the call the path under test makes ("CachedSkinned" on the GPU, "Triangles" for the vertices the model
// skins on the CPU), so the check shows that path ran.
template <typename Scene> void CheckDrawnLaterAsAtOnce(Scene scene, const char* path = nullptr)
{
    const std::vector<std::string> atOnce = CallsOf(scene, false);
    const std::vector<std::string> later = CallsOf(scene, true);
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
