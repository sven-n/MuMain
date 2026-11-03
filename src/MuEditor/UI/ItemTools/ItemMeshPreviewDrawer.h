#pragma once

#ifdef _EDITOR

#include "ItemMeshPicking.h"

#include "Render/Models/ModelDraw.h"

#include <bitset>
#include <cstddef>
#include <optional>

class BMD;

namespace MuEditor::Items
{
// The model drawer (Render/Models/ModelDraw.h) of the Item Model preview while it draws the item, for its lifetime.
// Of the item's model, the meshes other than the shown one are drawn see-through: dim and glowing, without writing
// depth, so a part behind them shows. With a ray from a click, it finds the mesh the ray hits first. Other models
// and the shadows go on to the drawer it replaced.
class CItemMeshPreviewDrawer final : public Render::ModelDrawing::ModelDrawer
{
public:
    // The light the other meshes keep.
    static constexpr float SeeThroughLight = 0.35f;

    CItemMeshPreviewDrawer(const BMD* model, std::optional<int> shownMesh, std::optional<MeshPicking::Ray> pickRay);
    ~CItemMeshPreviewDrawer() override;
    CItemMeshPreviewDrawer(const CItemMeshPreviewDrawer&) = delete;
    CItemMeshPreviewDrawer& operator=(const CItemMeshPreviewDrawer&) = delete;

    // As the drawer it replaced: a draw for one that draws later takes what it reads (BMD::RenderMesh).
    [[nodiscard]] bool DrawsLater() const override
    {
        return m_next.DrawsLater();
    }

    void DrawMesh(const Render::ModelDrawing::MeshDraw& draw) override;
    void DrawShadow(const Render::ModelDrawing::ShadowDraw& draw) override;

    // The mesh the pick ray hit first, if any.
    [[nodiscard]] std::optional<int> HitMesh() const
    {
        return m_hitMesh;
    }

    // A draw as the see-through meshes are drawn.
    [[nodiscard]] static Render::ModelDrawing::MeshDraw SeeThrough(const Render::ModelDrawing::MeshDraw& draw);

private:
    // At least MAX_MESH (ZzzBMD.h).
    static constexpr std::size_t MeshSlots = 64;

    void DrawSeeThrough(const Render::ModelDrawing::MeshDraw& draw);
    void Pick(const Render::ModelDrawing::MeshDraw& draw);

    Render::ModelDrawing::ModelDrawer& m_next;
    const BMD* m_model;
    std::optional<int> m_shownMesh;
    std::optional<MeshPicking::Ray> m_pickRay;
    // Each mesh is drawn in several passes; its triangles are tested once.
    std::bitset<MeshSlots> m_tested;
    std::optional<int> m_hitMesh;
    float m_hitDistance = 0.0f;
};
} // namespace MuEditor::Items

#endif // _EDITOR
