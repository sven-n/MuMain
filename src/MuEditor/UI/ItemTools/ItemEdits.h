#pragma once

#ifdef _EDITOR

#include "ItemEditHistory.h"

#include <span>
#include <string>
#include <string_view>

namespace MuEditor::Items
{
// The one way the item tools change items: each change goes through
// CItemDataHandler (the item database, ItemAttribute and the opened models
// stay in step) and is recorded in the undo history under `label`, the text
// the Undo button shows.
class ItemEdits
{
public:
    static ItemEdits& GetInstance();

    void SetItem(std::string label, const Data::Items::ItemDefinition& item);
    void SetItems(std::string label, std::span<const Data::Items::ItemDefinition> items);
    // CItemDataHandler::SetItemName; false when it refused the name.
    bool SetItemName(std::string label, int itemType, std::string_view locale, std::string text);
    bool AddItem(std::string label, const Data::Items::ItemDefinition& item,
                 const Data::Items::ItemModelDefinition* model);
    bool RemoveItem(std::string label, int itemType);
    bool MoveItem(std::string label, int fromItemType, int toItemType);
    void SetItemModel(std::string label, const Data::Items::ItemModelDefinition& model);
    void SetSharedModel(std::string label, const Data::Items::SharedItemModel& sharedModel,
                        std::string_view previousName = {});
    bool RemoveSharedModel(std::string label, std::string_view name);

    // Undo and Redo select the item they changed. False when there is
    // nothing to do or the items changed past the history.
    bool Undo();
    bool Redo();

    ItemEditHistory& GetHistory()
    {
        return m_history;
    }

    // The item and its model entry as the history records them.
    static ItemSlotState ReadSlot(int itemType);

private:
    ItemEdits();

    // Records the slots of `itemTypes` around `change`, as one entry.
    template <typename TChange> void RecordSlots(std::string label, std::span<const int> itemTypes, TChange&& change);

    ItemEditHistory m_history;
};
} // namespace MuEditor::Items

#define g_ItemEdits MuEditor::Items::ItemEdits::GetInstance()

#endif // _EDITOR
