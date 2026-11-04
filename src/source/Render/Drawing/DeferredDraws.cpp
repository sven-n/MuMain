#include "stdafx.h"

#include "Render/Drawing/DeferredDraws.h"

#include <algorithm>
#include <optional>
#include <type_traits>

namespace Render::Drawing
{
namespace
{
bool s_drawsWorldLater = false;
} // namespace

bool DrawsWorldLater()
{
    return s_drawsWorldLater;
}

void SetDrawsWorldLater(bool later)
{
    s_drawsWorldLater = later;
}

DeferredDraws::DeferredDraws()
    : m_models(&ModelDrawing::ActiveModelDrawer()), m_effects(&EffectDrawing::ActiveEffectDrawer()),
      m_states(ActiveLegacyCallRecorder())
{
    ModelDrawing::SetModelDrawer(this);
    EffectDrawing::SetEffectDrawer(this);
    SetLegacyCallRecorder(this);
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
    SetLegacyCallRecorder(m_states);
    for (const Recorded& record : m_draws)
    {
        std::optional<mu::ScopedBloomExclusion> exclusion;
        if (record.bloomExcluded && !mu::GetRenderer().IsBloomExcluded())
            exclusion.emplace();
        std::visit(
            [this](const auto& recorded)
            {
                using Kind = std::decay_t<decltype(recorded)>;
                if constexpr (std::is_same_v<Kind, ModelDrawing::MeshDraw>)
                    m_models->DrawMesh(recorded);
                else if constexpr (std::is_same_v<Kind, ModelDrawing::ShadowDraw>)
                    m_models->DrawShadow(recorded);
                else if constexpr (std::is_same_v<Kind, EffectDrawing::EffectState>)
                    m_effects->SetState(recorded);
                else if constexpr (std::is_same_v<Kind, EffectDrawing::SpriteDraw>)
                    m_effects->DrawSprite(recorded);
                else if constexpr (std::is_same_v<Kind, Quads>)
                    m_effects->DrawQuads({.vertices = std::span(m_vertices).subspan(recorded.first, recorded.count),
                                          .texture = recorded.texture,
                                          .triangles = recorded.triangles});
                else if constexpr (std::is_same_v<Kind, EffectDrawing::DecalDraw>)
                    m_effects->DrawDecal(recorded);
                else if constexpr (std::is_same_v<Kind, EffectDrawing::CircleDraw>)
                    m_effects->DrawCircle(recorded);
                else if constexpr (std::is_same_v<Kind, EffectDrawing::PlaneDraw>)
                    m_effects->DrawPlane(recorded);
                else
                    MakeLegacyCall(recorded.call, recorded.argument);
            },
            record.draw);
    }
    m_draws.clear();
    m_vertices.clear();
    m_kept.clear();
    m_keptBones.clear();
    m_keptShadowMeshes.clear();
    m_keptClothes.clear();
}

template <typename T> void DeferredDraws::Record(const T& draw)
{
    m_draws.push_back({.draw = draw, .bloomExcluded = mu::GetRenderer().IsBloomExcluded()});
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
    Record(kept);
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
    if (!kept.clothes.empty())
    {
        auto& clothes = m_keptClothes.emplace_back(std::make_unique<ModelDrawing::ClothShadow[]>(kept.clothes.size()));
        for (std::size_t i = 0; i < kept.clothes.size(); ++i)
        {
            clothes[i] = kept.clothes[i];
            const auto points =
                static_cast<std::size_t>(clothes[i].columns) * static_cast<std::size_t>(clothes[i].rows);
            clothes[i].points = reinterpret_cast<const float (*)[3]>(Keep(&clothes[i].points[0][0], points * 3u));
        }
        kept.clothes = {clothes.get(), kept.clothes.size()};
    }
    Record(kept);
}

void DeferredDraws::SetState(const EffectDrawing::EffectState& state)
{
    Record(state);
}

void DeferredDraws::DrawSprite(const EffectDrawing::SpriteDraw& draw)
{
    Record(draw);
}

void DeferredDraws::DrawQuads(const EffectDrawing::QuadsDraw& draw)
{
    Record(Quads{.first = m_vertices.size(),
                 .count = draw.vertices.size(),
                 .texture = draw.texture,
                 .triangles = draw.triangles});
    m_vertices.insert(m_vertices.end(), draw.vertices.begin(), draw.vertices.end());
}

void DeferredDraws::DrawDecal(const EffectDrawing::DecalDraw& draw)
{
    Record(draw);
}

void DeferredDraws::DrawCircle(const EffectDrawing::CircleDraw& draw)
{
    Record(draw);
}

void DeferredDraws::DrawPlane(const EffectDrawing::PlaneDraw& draw)
{
    Record(draw);
}

void DeferredDraws::RecordLegacyCall(LegacyCall call, int argument)
{
    Record(State{.call = call, .argument = argument});
}
} // namespace Render::Drawing
