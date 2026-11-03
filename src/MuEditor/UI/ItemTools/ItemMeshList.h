#pragma once

#ifdef _EDITOR

#include "ItemMeshRows.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <string>
#include <vector>

class BMD;

namespace MuEditor::Items
{
// The mesh list of the Item Model window, after the model file's fields: a row per mesh of the item's loaded model
// with its number, texture, marks and the fields of the model entry that name it. Pointing at a row shows its mesh
// alone in the preview, a click keeps it (g_ItemMeshFocus).
class CItemMeshList
{
public:
    // `inPreview`: the preview draws this model at its level (CItemModelPreview::ShowsListedModel).
    void Render(int itemType, const Data::Items::ItemModelDefinition& entry, bool inPreview);

private:
    // The rows are read again when the model was opened again or the item changed.
    void Refresh(int itemType, const BMD& model);
    void RenderTable(const Data::Items::ItemModelDefinition& entry);
    void RenderRow(std::size_t index, MeshUses uses, bool scrollToKept);
    const std::string& UsesText(MeshUses uses);

    std::vector<ItemMeshRow> m_rows;
    // Built again for each row, into one reused string each.
    std::string m_marksText;
    std::string m_usesText;
    int m_itemType = -1;
    const void* m_meshes = nullptr;
    const void* m_textures = nullptr;
    int m_meshCount = -1;
    long long m_dataVersion = -1;
};
} // namespace MuEditor::Items

#endif // _EDITOR
