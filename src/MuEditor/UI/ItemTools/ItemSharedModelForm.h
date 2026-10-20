#pragma once

#ifdef _EDITOR

#include "ItemLookUseCounts.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

#include <string>
#include <vector>

namespace MuEditor::Items
{
// The model tool's part for shared models (D26). An item without one can pick
// one. For the shared model an item uses: its users, and its name, file,
// texture folders and none-blend meshes, changed in a copy and applied only
// after a warning that lists every item the change reaches; or the item takes
// a copy of its own and leaves it.
class CItemSharedModelForm
{
public:
    // `model` is the item's model entry.
    void Render(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts);

private:
    void RenderPick(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts);
    void RenderShared(const Data::Items::ItemModelDefinition& model);
    void RenderEdit();
    void RenderConfirm();
    void LoadEdit(const Data::Items::SharedItemModel& shared);

    // The shared model as the user changes it, before it is applied.
    Data::Items::SharedItemModel m_edit;
    std::string m_editOf;
    int m_editVersion = -1;
    std::string m_meshesText;
    std::vector<int> m_confirmUsers;
};
} // namespace MuEditor::Items

#endif // _EDITOR
