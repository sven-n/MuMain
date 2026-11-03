#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshPreviewDrawer.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"

namespace MuEditor::Items
{
using Render::ModelDrawing::MeshDraw;

CItemMeshPreviewDrawer::CItemMeshPreviewDrawer(const BMD* model, std::optional<int> shownMesh,
                                               std::optional<MeshPicking::Ray> pickRay)
    : m_next(Render::ModelDrawing::ActiveModelDrawer()), m_model(model), m_shownMesh(shownMesh), m_pickRay(pickRay)
{
    static_assert(static_cast<std::size_t>(MAX_MESH) <= MeshSlots);
    Render::ModelDrawing::SetModelDrawer(this);
}

CItemMeshPreviewDrawer::~CItemMeshPreviewDrawer()
{
    Render::ModelDrawing::SetModelDrawer(&m_next);
}

void CItemMeshPreviewDrawer::DrawMesh(const MeshDraw& draw)
{
    if (m_model == nullptr || draw.model != m_model)
    {
        m_next.DrawMesh(draw);
        return;
    }
    if (m_pickRay)
    {
        Pick(draw);
    }
    if (!m_shownMesh || draw.mesh == *m_shownMesh)
    {
        m_next.DrawMesh(draw);
        return;
    }
    m_next.DrawMesh(SeeThrough(draw));
}

void CItemMeshPreviewDrawer::DrawShadow(const Render::ModelDrawing::ShadowDraw& draw)
{
    m_next.DrawShadow(draw);
}

// Added light without depth writes (RENDER_BRIGHT), dimmed; a darkening pass would hide what is behind.
MeshDraw CItemMeshPreviewDrawer::SeeThrough(const MeshDraw& draw)
{
    MeshDraw seeThrough = draw;
    seeThrough.flags = (draw.flags & ~RENDER_DARK) | RENDER_BRIGHT;
    for (float& light : seeThrough.bodyLight)
    {
        light *= SeeThroughLight;
    }
    seeThrough.blendMeshAlpha *= SeeThroughLight;
    return seeThrough;
}

// The mesh's vertices as this draw poses them (the model's current skin), tested once per mesh.
void CItemMeshPreviewDrawer::Pick(const MeshDraw& draw)
{
    if (draw.mesh < 0 || static_cast<std::size_t>(draw.mesh) >= MeshSlots || m_tested.test(draw.mesh))
    {
        return;
    }
    m_tested.set(draw.mesh);
    int textureIndex = 0;
    BITMAP_t* texture = nullptr;
    if (draw.model->DrawnMesh(draw, textureIndex, texture) == nullptr)
    {
        return;
    }
    draw.model->EnsureCpuVertices(draw.mesh);
    const std::optional<float> hit =
        MeshPicking::HitMesh(*m_pickRay, *draw.model, draw.mesh, VertexTransform[draw.mesh]);
    if (hit && (!m_hitMesh || *hit < m_hitDistance))
    {
        m_hitMesh = draw.mesh;
        m_hitDistance = *hit;
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
