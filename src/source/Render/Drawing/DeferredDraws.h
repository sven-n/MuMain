#pragma once

#include "Render/Effects/EffectDraw.h"
#include "Render/Models/ModelDraw.h"

#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace Render::Drawing
{
// A drawer that draws later: it records the model and effect draws handed to it, in their order, and draws them when
// flushed, through the drawers it stood in for. What a draw refers to must not change before the flush; the parity
// tests compare its calls with drawing at once. The base of a render thread, which would flush on another thread.
class DeferredDraws final : public ModelDrawing::ModelDrawer, public EffectDrawing::EffectDrawer
{
public:
    // Stands in for the active model and effect drawers until Flush.
    DeferredDraws();
    ~DeferredDraws() override;
    DeferredDraws(const DeferredDraws&) = delete;
    DeferredDraws& operator=(const DeferredDraws&) = delete;

    // Gives the drawers back, then draws what was recorded through them.
    void Flush();

    [[nodiscard]] bool DrawsLater() const override
    {
        return true;
    }

    void DrawMesh(const ModelDrawing::MeshDraw& draw) override;
    void DrawShadow(const ModelDrawing::ShadowDraw& draw) override;

    void SetState(const EffectDrawing::EffectState& state) override;
    void DrawSprite(const EffectDrawing::SpriteDraw& draw) override;
    void DrawQuads(const EffectDrawing::QuadsDraw& draw) override;
    void DrawDecal(const EffectDrawing::DecalDraw& draw) override;
    void DrawCircle(const EffectDrawing::CircleDraw& draw) override;
    void DrawPlane(const EffectDrawing::PlaneDraw& draw) override;

private:
    // Quads keep where their vertices start in m_vertices: the caller's buffer is gone by the flush.
    struct Quads
    {
        std::size_t first = 0;
        std::size_t count = 0;
        std::uint32_t texture = 0;
    };
    using Draw = std::variant<ModelDrawing::MeshDraw, ModelDrawing::ShadowDraw, EffectDrawing::EffectState,
                              EffectDrawing::SpriteDraw, Quads, EffectDrawing::DecalDraw, EffectDrawing::CircleDraw,
                              EffectDrawing::PlaneDraw>;

    // A copy of `count` floats kept until the flush; blocks never move once made.
    const float* Keep(const float* values, std::size_t count);
    // The draw's bones, copied once per pose (pointer and palette version): the shared bones change before the flush.
    void KeepBones(ModelDrawing::ModelSkin& skin);

    std::vector<Draw> m_draws;
    std::vector<mu::Vertex3D> m_vertices;
    std::vector<std::unique_ptr<float[]>> m_kept;
    std::map<std::pair<const void*, std::uint32_t>, const float (*)[3][4]> m_keptBones;
    std::vector<std::unique_ptr<ModelDrawing::CpuShadowMesh[]>> m_keptShadowMeshes;
    ModelDrawing::ModelDrawer* m_models = nullptr;
    EffectDrawing::EffectDrawer* m_effects = nullptr;
    bool m_flushed = false;
};
} // namespace Render::Drawing
