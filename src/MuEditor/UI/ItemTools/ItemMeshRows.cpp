#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshRows.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/TextureScript.h"

#include <algorithm>
#include <cstring>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemGlowMeshes;
using Data::Items::ItemModelDefinition;

bool Contains(const std::vector<int>& meshes, int mesh)
{
    return std::find(meshes.begin(), meshes.end(), mesh) != meshes.end();
}

MeshUses FindGlowMeshUses(const ItemGlowMeshes& meshes, int mesh, MeshUses only, MeshUses allBut)
{
    MeshUses uses = 0;
    if (Contains(meshes.only, mesh))
    {
        uses |= only;
    }
    if (meshes.hidden == mesh)
    {
        uses |= allBut;
    }
    return uses;
}

void DescribeTexture(const BMD& model, const Mesh_t& mesh, ItemMeshRow& row)
{
    if (model.Textures == nullptr || mesh.Texture < 0 || mesh.Texture >= model.NumMeshs)
    {
        return;
    }
    const Texture_t& texture = model.Textures[mesh.Texture];
    row.texture.assign(texture.FileName, strnlen(texture.FileName, sizeof(texture.FileName)));
    row.hidden = Data::Items::IsHiddenTexture(row.texture);
    if (model.IndexTexture == nullptr || model.IndexTexture[mesh.Texture] == BITMAP_HIDE)
    {
        return;
    }
    const BITMAP_t* bitmap = Bitmaps.FindTexture(model.IndexTexture[mesh.Texture]);
    if (bitmap == nullptr)
    {
        return;
    }
    row.textureFile = Core::Text::ToUtf8(bitmap->FileName);
    row.textureWidth = static_cast<int>(bitmap->Width);
    row.textureHeight = static_cast<int>(bitmap->Height);
    row.holes = bitmap->hasHoles;
}

void DescribeScript(const Mesh_t& mesh, ItemMeshRow& row)
{
    row.noChrome = mesh.NoneBlendMesh;
    TextureScript* script = mesh.m_csTScript;
    if (script == nullptr)
    {
        return;
    }
    row.hidden = row.hidden || script->getHiddenMesh();
    row.glows = script->getBright();
    row.scrolls = script->getStreamMesh();
    row.noChrome = row.noChrome || script->getNoneBlendMesh();
}
} // namespace

std::vector<ItemMeshRow> DescribeMeshes(const BMD& model)
{
    std::vector<ItemMeshRow> rows;
    if (model.Meshs == nullptr)
    {
        return rows;
    }
    rows.reserve(static_cast<std::size_t>(std::max<short>(model.NumMeshs, 0)));
    for (int index = 0; index < model.NumMeshs; ++index)
    {
        const Mesh_t& mesh = model.Meshs[index];
        ItemMeshRow row;
        row.mesh = index;
        row.vertices = mesh.NumVertices;
        row.triangles = mesh.NumTriangles;
        DescribeTexture(model, mesh, row);
        DescribeScript(mesh, row);
        rows.push_back(std::move(row));
    }
    return rows;
}

MeshUses FindMeshUses(const ItemModelDefinition& entry, int mesh)
{
    MeshUses uses = 0;
    if (Contains(entry.noneBlendMeshes, mesh))
    {
        uses |= MeshUse::NoneBlend;
    }
    uses |= FindGlowMeshUses(entry.glow.meshes, mesh, MeshUse::Glow, MeshUse::GlowAllBut);
    uses |= FindGlowMeshUses(entry.glow.shineMeshes, mesh, MeshUse::Shine, MeshUse::ShineAllBut);
    if (entry.glow.excellentMesh == mesh)
    {
        uses |= MeshUse::Excellent;
    }
    if (entry.glow.excellentMeshWithoutSkin == mesh)
    {
        uses |= MeshUse::ExcellentWithoutSkin;
    }
    return uses;
}
} // namespace MuEditor::Items

#endif // _EDITOR
