#pragma once

#ifdef _EDITOR

#include "ItemNumberHolderView.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <string>

namespace MuEditor::Items
{
// The dialog that adds an item (section 7): a group, a free number (one with
// neither an entry in the item files nor a model entry), the English name,
// and either a copy of the selected item's values or a size and slot of its
// own. Duplicate copies an item with its model; a model file of its own
// becomes a shared model that both items use, so it is still opened once.
class CItemAddDialog
{
public:
    // `templateItemType`: the item whose values a new item may copy, -1 for none.
    void OpenAdd(int templateItemType);
    void OpenDuplicate(int sourceItemType);
    // The dialog while it is open; the added item is selected.
    void Render();

private:
    enum class Mode
    {
        Add,
        Duplicate,
    };

    void RenderNumber();
    void RenderValues();
    void RenderModelChoice();
    bool CanConfirm() const;
    void Confirm();
    void AddNew();
    void Duplicate();

    Mode m_mode = Mode::Add;
    bool m_open = false;
    int m_sourceItemType = -1;
    int m_group = 0;
    int m_number = 0;
    std::string m_name;
    bool m_copySource = false;
    int m_width = 1;
    int m_height = 1;
    Data::Items::ItemSlot m_slot = Data::Items::ItemSlot::None;
    bool m_shareModel = true;
    CItemNumberHolderView m_numberHolder;
};
} // namespace MuEditor::Items

#endif // _EDITOR
