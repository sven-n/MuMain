#pragma once

#ifdef _EDITOR

#include "ItemEditHistory.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"

#include <optional>
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

    // False, with nothing changed, when it would change the size or slot of an
    // item the game holds (IsHeldByCharacter), as its undo would be refused.
    bool SetItem(std::string label, const Data::Items::ItemDefinition& item);
    // AsGiven keeps the stat profile each item names as a pick (CItemDataHandler::SetItems).
    bool SetItems(std::string label, std::span<const Data::Items::ItemDefinition> items,
                  Data::Items::StatProfileStore profiles = Data::Items::StatProfileStore::Follow);
    // CItemDataHandler::SetItemName; false when it refused the name.
    bool SetItemName(std::string label, int itemType, std::string_view locale, std::string text);
    bool AddItem(std::string label, const Data::Items::ItemDefinition& item,
                 const Data::Items::ItemModelDefinition* model,
                 const Data::Items::ItemDefinition* copiedFrom = nullptr);
    // False, with nothing changed, also while the game holds the item
    // (IsHeldByCharacter).
    bool RemoveItem(std::string label, int itemType);
    bool MoveItem(std::string label, int fromItemType, int toItemType);
    void SetItemModel(std::string label, const Data::Items::ItemModelDefinition& model);
    void SetSharedModel(std::string label, const Data::Items::SharedItemModel& sharedModel,
                        std::string_view previousName = {});
    bool RemoveSharedModel(std::string label, std::string_view name);
    // CItemDataHandler::SetGlowColor.
    void SetGlowColor(std::string label, const Data::Effects::GlowColor& color);
    // CItemDataHandler::RemoveGlowColor for each name, as one step; false
    // when none could be removed.
    bool RemoveGlowColors(std::string label, std::span<const std::string> names);
    // CItemDataHandler::SetStatFile, with the items in `repointed` stored as
    // they are (their new stat profile), as one step. `file` must be valid.
    void SetStatFile(std::string label, const Data::Items::ItemStatFile& file,
                     std::span<const Data::Items::ItemDefinition> repointed = {});

    // The changes until EndGroup are one undo step under `label` (an item
    // duplicated with its model made shared).
    void BeginGroup(std::string label);
    void EndGroup();

    // Undo and Redo select the item they changed. False when there is
    // nothing to do, the items changed past the history or the step would
    // change a held item (ItemEditHistory::GetLastRefusal).
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
    // Into the history, or into the open group.
    void Record(ItemEditEntry entry);

    ItemEditHistory m_history;
    std::optional<ItemEditEntry> m_group;
};
} // namespace MuEditor::Items

#define g_ItemEdits MuEditor::Items::ItemEdits::GetInstance()

#endif // _EDITOR
