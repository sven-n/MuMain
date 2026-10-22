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
// a copy of its own and leaves it. The shared models no item uses can be
// removed.
class CItemSharedModelForm
{
public:
    // `model` is the item's model entry.
    void Render(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts);
    // The warning that lists the items a change reaches, before it is applied.
    // Drawn outside the item's ID scope: a click in its list selects another
    // item, and the warning stays open.
    void RenderConfirm();

private:
    void RenderPick(const Data::Items::ItemModelDefinition& model, ItemLookUseCounts& counts);
    void RenderShared(const Data::Items::ItemModelDefinition& model);
    void RenderEdit();
    void RenderUnused(ItemLookUseCounts& counts);
    void LoadEdit(const Data::Items::SharedItemModel& shared);
    // Whether the copy is as it was loaded or applied (the user did not
    // change it since).
    bool IsEditUnchanged() const;

    // The shared model as the user changes it, before it is applied.
    Data::Items::SharedItemModel m_edit;
    // The shared model as it was last loaded into m_edit or applied from it.
    Data::Items::SharedItemModel m_loaded;
    std::string m_editOf;
    int m_editVersion = -1;
    std::string m_meshesText;
    std::vector<int> m_confirmUsers;
    // Apply was clicked: RenderConfirm opens the warning in its own ID scope.
    bool m_openConfirm = false;
};
} // namespace MuEditor::Items

#endif // _EDITOR
