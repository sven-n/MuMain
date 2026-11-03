#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshPreviewDrawer.h"

#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"

namespace MuEditor::Items
{
using Render::ModelDrawing::MeshDraw;

namespace
{
// Whether the draw's texture has pixels the alpha test drops. A color pass draws no texture.
bool HasHoles(const MeshDraw& draw)
{
    if ((draw.flags & RENDER_COLOR) == RENDER_COLOR)
    {
        return false;
    }
    int textureIndex = 0;
    BITMAP_t* texture = nullptr;
    return draw.model->DrawnMesh(draw, textureIndex, texture) != nullptr && texture->hasHoles;
}
} // namespace

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
    DrawSeeThrough(draw);
}

// Added light fills the holes the alpha test cuts into a texture, and EnableAlphaBlend turns the test off, so a
// texture with holes keeps the test on for its draw: the draw's EnableAlphaBlend turns it off only when it knows it
// on, and it is put in that state first. A drawer that draws later makes the state calls later.
void CItemMeshPreviewDrawer::DrawSeeThrough(const MeshDraw& draw)
{
    const MeshDraw seeThrough = SeeThrough(draw);
    if (m_next.DrawsLater() || !HasHoles(draw))
    {
        m_next.DrawMesh(seeThrough);
        return;
    }
    EnableAlphaBlend();
    EnableAlphaTestRaw();
    m_next.DrawMesh(seeThrough);
    DisableAlphaTestRaw();
}

void CItemMeshPreviewDrawer::DrawShadow(const Render::ModelDrawing::ShadowDraw& draw)
{
    m_next.DrawShadow(draw);
}

// Added light without depth writes (RENDER_BRIGHT), dimmed; a darkening pass would hide what is behind. A texture's
// holes stay holes (DrawSeeThrough).
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

// The mesh's vertices as this draw poses them (the model's current skin), tested once per mesh. VertexTransform has
// MAX_MESH meshes; EnsureCpuVertices skins none past them.
void CItemMeshPreviewDrawer::Pick(const MeshDraw& draw)
{
    if (draw.mesh < 0 || draw.mesh >= MAX_MESH || m_tested.test(draw.mesh))
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
