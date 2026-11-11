#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemMeshRows.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <string>
#include <vector>

class BMD;

namespace MuEditor::Items
{
// The mesh list of the Item Model window, after the model file's fields: a row per mesh of the item's loaded model
// with its number, its name (design L38), texture, marks and the fields of the model entry that name it. Pointing at
// a row shows its mesh alone in the preview, a click keeps it (g_ItemMeshFocus). A name is stored when its field is
// left after a change: in the model entry, or in the shared model of an item that names one.
class CItemMeshList
{
public:
    // `inPreview`: the preview draws this model at its level (CItemModelPreview::ShowsListedModel).
    void Render(int itemType, Data::Items::ItemModelDefinition& entry,
                ItemEditDraft<Data::Items::ItemModelDefinition>& draft, bool inPreview);

private:
    // The rows are read again when the model was opened again or the item changed.
    void Refresh(int itemType, const BMD& model);
    void RenderTable(Data::Items::ItemModelDefinition& entry, ItemEditDraft<Data::Items::ItemModelDefinition>& draft);
    void RenderRow(std::size_t index, MeshUses uses, bool scrollToKept, Data::Items::ItemModelDefinition& entry,
                   ItemEditDraft<Data::Items::ItemModelDefinition>& draft);
    void RenderName(int mesh, Data::Items::ItemModelDefinition& entry,
                    ItemEditDraft<Data::Items::ItemModelDefinition>& draft);
    const std::string& UsesText(MeshUses uses);

    std::vector<ItemMeshRow> m_rows;
    // Built again for each row, into one reused string each.
    std::string m_marksText;
    std::string m_usesText;
    // The name field being typed in: its mesh (-1: none) and its text, kept until the field is left.
    int m_nameMesh = -1;
    std::string m_nameText;
    int m_itemType = -1;
    const void* m_meshes = nullptr;
    const void* m_textures = nullptr;
    int m_meshCount = -1;
    long long m_dataVersion = -1;
};
} // namespace MuEditor::Items

#endif // _EDITOR
