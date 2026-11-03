#include "stdafx.h"

#include "Render/Drawing/DeferredDraws.h"

#include <algorithm>
#include <type_traits>

namespace Render::Drawing
{
DeferredDraws::DeferredDraws()
    : m_models(&ModelDrawing::ActiveModelDrawer()), m_effects(&EffectDrawing::ActiveEffectDrawer())
{
    ModelDrawing::SetModelDrawer(this);
    EffectDrawing::SetEffectDrawer(this);
}

DeferredDraws::~DeferredDraws()
{
    Flush();
}

void DeferredDraws::Flush()
{
    if (m_flushed)
        return;
    m_flushed = true;
    ModelDrawing::SetModelDrawer(m_models);
    EffectDrawing::SetEffectDrawer(m_effects);
    for (const Draw& draw : m_draws)
    {
        std::visit(
            [this](const auto& recorded)
            {
                using Recorded = std::decay_t<decltype(recorded)>;
                if constexpr (std::is_same_v<Recorded, ModelDrawing::MeshDraw>)
                    m_models->DrawMesh(recorded);
                else if constexpr (std::is_same_v<Recorded, ModelDrawing::ShadowDraw>)
                    m_models->DrawShadow(recorded);
                else if constexpr (std::is_same_v<Recorded, EffectDrawing::EffectState>)
                    m_effects->SetState(recorded);
                else if constexpr (std::is_same_v<Recorded, EffectDrawing::SpriteDraw>)
                    m_effects->DrawSprite(recorded);
                else if constexpr (std::is_same_v<Recorded, Quads>)
                    m_effects->DrawQuads({.vertices = std::span(m_vertices).subspan(recorded.first, recorded.count),
                                          .texture = recorded.texture});
                else if constexpr (std::is_same_v<Recorded, EffectDrawing::DecalDraw>)
                    m_effects->DrawDecal(recorded);
                else if constexpr (std::is_same_v<Recorded, EffectDrawing::CircleDraw>)
                    m_effects->DrawCircle(recorded);
                else
                    m_effects->DrawPlane(recorded);
            },
            draw);
    }
    m_draws.clear();
    m_vertices.clear();
    m_kept.clear();
    m_keptBones.clear();
    m_keptShadowMeshes.clear();
}

const float* DeferredDraws::Keep(const float* values, std::size_t count)
{
    auto& block = m_kept.emplace_back(std::make_unique<float[]>(count));
    std::copy_n(values, count, block.get());
    return block.get();
}

void DeferredDraws::KeepBones(ModelDrawing::ModelSkin& skin)
{
    if (skin.bones == nullptr)
        return;
    const auto key = std::make_pair(static_cast<const void*>(skin.bones), skin.boneVersion);
    auto kept = m_keptBones.find(key);
    if (kept == m_keptBones.end())
    {
        const float* copy = Keep(&skin.bones[0][0][0], static_cast<std::size_t>(MAX_BONES) * 12u);
        kept = m_keptBones.emplace(key, reinterpret_cast<const float (*)[3][4]>(copy)).first;
    }
    skin.bones = kept->second;
}

void DeferredDraws::DrawMesh(const ModelDrawing::MeshDraw& draw)
{
    ModelDrawing::MeshDraw kept = draw;
    KeepBones(kept.skin);
    if (kept.cpu.vertices != nullptr)
    {
        const auto vertices = static_cast<std::size_t>(kept.cpu.vertexCount) * 3u;
        kept.cpu.vertices = reinterpret_cast<const float (*)[3]>(Keep(&kept.cpu.vertices[0][0], vertices));
    }
    if (kept.cpu.normals != nullptr)
    {
        const auto normals = static_cast<std::size_t>(kept.cpu.normalCount);
        kept.cpu.normals = reinterpret_cast<const float (*)[3]>(Keep(&kept.cpu.normals[0][0], normals * 3u));
        kept.cpu.intensities = Keep(kept.cpu.intensities, normals);
    }
    m_draws.emplace_back(kept);
}

void DeferredDraws::DrawShadow(const ModelDrawing::ShadowDraw& draw)
{
    ModelDrawing::ShadowDraw kept = draw;
    KeepBones(kept.skin);
    if (!kept.cpu.empty())
    {
        auto& meshes =
            m_keptShadowMeshes.emplace_back(std::make_unique<ModelDrawing::CpuShadowMesh[]>(kept.cpu.size()));
        for (std::size_t i = 0; i < kept.cpu.size(); ++i)
        {
            meshes[i] = kept.cpu[i];
            if (meshes[i].vertices != nullptr)
                meshes[i].vertices = reinterpret_cast<const float (*)[3]>(
                    Keep(&meshes[i].vertices[0][0], static_cast<std::size_t>(meshes[i].vertexCount) * 3u));
        }
        kept.cpu = {meshes.get(), kept.cpu.size()};
    }
    m_draws.emplace_back(kept);
}

void DeferredDraws::SetState(const EffectDrawing::EffectState& state)
{
    m_draws.emplace_back(state);
}

void DeferredDraws::DrawSprite(const EffectDrawing::SpriteDraw& draw)
{
    m_draws.emplace_back(draw);
}

void DeferredDraws::DrawQuads(const EffectDrawing::QuadsDraw& draw)
{
    m_draws.emplace_back(Quads{.first = m_vertices.size(), .count = draw.vertices.size(), .texture = draw.texture});
    m_vertices.insert(m_vertices.end(), draw.vertices.begin(), draw.vertices.end());
}

void DeferredDraws::DrawDecal(const EffectDrawing::DecalDraw& draw)
{
    m_draws.emplace_back(draw);
}

void DeferredDraws::DrawCircle(const EffectDrawing::CircleDraw& draw)
{
    m_draws.emplace_back(draw);
}

void DeferredDraws::DrawPlane(const EffectDrawing::PlaneDraw& draw)
{
    m_draws.emplace_back(draw);
}
} // namespace Render::Drawing
