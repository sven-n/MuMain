#include "stdafx.h"

#ifdef _EDITOR

#include "LookParts.h"

#include "../MuEditor/UI/ItemTools/ItemMeshRows.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>

namespace MuEditor::Looks
{
std::vector<LookPart> PartsOf(int itemType)
{
    std::vector<LookPart> parts;
    if (!Data::Items::IsValidItemType(itemType))
    {
        return parts;
    }
    const BMD& model = Models[Data::Items::ToModelSlot(itemType)];
    const Data::Items::ItemModelDefinition* entry = g_ItemModelDatabase.Find(itemType);
    for (const Items::ItemMeshRow& row : Items::DescribeMeshes(model))
    {
        LookPart part{.mesh = row.mesh, .texture = row.texture};
        if (entry != nullptr)
        {
            if (const auto named = entry->meshNames.find(row.mesh); named != entry->meshNames.end())
            {
                part.name = named->second;
            }
        }
        parts.push_back(std::move(part));
    }
    return parts;
}

std::vector<std::size_t> LayersOn(const Data::Looks::LookDefinition& look, int mesh)
{
    std::vector<std::size_t> layers;
    for (std::size_t index = 0; index < look.layers.size(); ++index)
    {
        const Data::Looks::MeshSet& meshes = look.layers[index].meshes;
        const bool everyPart = meshes.IsAll() || meshes.allBut;
        const bool listed = !everyPart && std::ranges::find(meshes.meshes, mesh) != meshes.meshes.end();
        if (mesh == EveryPart ? everyPart : listed)
        {
            layers.push_back(index);
        }
    }
    return layers;
}

std::string PartLabel(const LookPart& part)
{
    if (!part.name.empty())
    {
        return part.name;
    }
    return std::to_string(part.mesh) + (part.texture.empty() ? "" : " \xc2\xb7 " + part.texture);
}
} // namespace MuEditor::Looks

#endif // _EDITOR
