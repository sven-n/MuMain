#pragma once

#include "Render/Renderer/MuRenderer.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace Render::Drawing
{
inline std::uint64_t Hash(const void* data, std::size_t size, std::uint64_t hash = 1469598103934665603ull)
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
inline std::uint64_t HashOf(const mu::SkinningParameters& p)
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

inline std::uint64_t HashOf(const mu::ShadowProjection& projection)
{
    std::uint64_t hash = Hash(projection.origin, sizeof(projection.origin));
    HashInto(hash, projection.shearX);
    HashInto(hash, projection.shearHeight);
    HashInto(hash, projection.specialHeight);
    HashInto(hash, projection.terrain);
    return hash;
}

// A renderer that draws nothing and logs every draw that reaches it, with a hash of what it draws (skinned draws' bones
// included) and the state it is drawn with, as the SDL_GPU renderer records it with each command. Two logs of the
// same draws compare equal; the parity tests and the "drawparity" control command compare drawing at once and later.
class DrawCallLog final : public mu::IMuRenderer
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
    void SetAlphaFunc(int /*func*/, float ref) override
    {
        m_alphaRef = ref;
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
    void BeginWorldLitGeometry() override
    {
        ++m_worldLit;
    }
    void EndWorldLitGeometry() override
    {
        --m_worldLit;
    }
    void BeginGroundShadow(mu::GroundShadowSource source) override
    {
        ++m_groundShadows[static_cast<std::size_t>(source)];
    }
    void EndGroundShadow(mu::GroundShadowSource source) override
    {
        --m_groundShadows[static_cast<std::size_t>(source)];
    }
    void BeginNoSunCasters() override
    {
        ++m_noSunCasters;
    }
    void EndNoSunCasters() override
    {
        --m_noSunCasters;
    }
    void BeginCharacterDraws() override
    {
        ++m_characterDraws;
    }
    void EndCharacterDraws() override
    {
        --m_characterDraws;
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
        // The water pass pops what the frame pushed before the log stood in.
        if (m_matrices.size() > 1)
            m_matrices.pop_back();
        else
            m_matrices.back() = " camera";
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
    float m_alphaRef = 0.25f;
    bool m_texture2D = true;
    bool m_fog = false;
    int m_bound = -1;
    int m_textureMode = 0;
    int m_bloomExclusions = 0;
    int m_worldLit = 0;
    int m_groundShadows[2] = {};
    int m_characterDraws = 0;
    int m_noSunCasters = 0;
    std::size_t m_reorderable = 0;
    // The scene's camera, which the sprite matrix replaces.
    std::vector<std::string> m_matrices{std::string(" camera")};

    // The state a draw is drawn with.
    [[nodiscard]] std::string State() const
    {
        return " | blend " + std::to_string(m_blend) + " depth " + std::to_string(m_depthTest) +
               std::to_string(m_depthMask) + " cull " + std::to_string(m_cullFace) + " alphaTest " +
               std::to_string(m_alphaTest) + " alphaRef " + std::to_string(m_alphaRef) + " texture " +
               std::to_string(m_texture2D) + " " + std::to_string(m_bound) + " mode " + std::to_string(m_textureMode) +
               " fog " + std::to_string(m_fog) + " bloomExcluded " + std::to_string(m_bloomExclusions > 0) +
               " worldLit " + std::to_string(m_worldLit > 0) + " groundShadow " +
               std::to_string(m_groundShadows[0] > 0) + std::to_string(m_groundShadows[1] > 0) + " characters " +
               std::to_string(m_characterDraws > 0) + " noSunCasters " + std::to_string(m_noSunCasters > 0) +
               " reorderable " + std::to_string(m_reorderable) + " matrix" + m_matrices.back();
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

// Sets every legacy render state the other way, then the known way, so the renderer is told each one: the legacy
// calls skip what they think is set already.
void ResetLegacyState();

// The "drawparity" control command: on request, the next world pass is also drawn at once and later into two logs.
struct DrawParityReport
{
    bool done = false;
    // Whether the second log was drawn later; false draws both at once, the baseline of drawing the pass twice.
    bool later = true;
    std::size_t atOnceCalls = 0;
    std::size_t laterCalls = 0;
    // The first call that differs in place (the number of calls when none does), and what each log made there.
    std::size_t firstDifference = 0;
    std::pair<std::string, std::string> first;
    // Calls one log has more often than the other, whatever their place, and a few of each.
    std::size_t onlyAtOnce = 0;
    std::size_t onlyLater = 0;
    std::vector<std::string> onlyAtOnceExamples;
    std::vector<std::string> onlyLaterExamples;
};
// `later` false draws the second log at once too.
void RequestDrawParity(bool later = true);
// True once per request; `later` gets how the second log is drawn.
[[nodiscard]] bool TakeDrawParityRequest(bool& later);
void ReportDrawParity(const std::vector<std::string>& atOnce, const std::vector<std::string>& later);
[[nodiscard]] const DrawParityReport& LastDrawParityReport();
} // namespace Render::Drawing
