#pragma once

#ifdef _EDITOR

#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace MuEditor::Items
{
// An item and its model entry, as the item tools change them together. The
// item's display name (ItemDefinition::name) is left empty: it follows the UI
// language, which may change between an edit and its undo.
struct ItemSlotState
{
    Data::Items::ItemDefinition item;
    // nullopt: the item type has no model entry.
    std::optional<Data::Items::ItemModelDefinition> model;

    bool operator==(const ItemSlotState&) const = default;
};

// One change an edit made.
struct ItemEditStep
{
    enum class Kind
    {
        // The slot of `itemType` went from `before` to `after`.
        Slot,
        // The item moved from `itemType` to `otherItemType` (with its model):
        // `before` is it at `itemType` before the move, `after` at
        // `otherItemType` after it. The move derives fields from the new
        // number (CItemDataHandler::MoveItem), so these are put back after
        // the move back and again.
        Move,
        // The shared model went from `sharedBefore` to `sharedAfter` (nullopt:
        // it did not exist); the names differ when it was renamed.
        SharedModel,
        // The glow color list went from `colorsBefore` to `colorsAfter`.
        GlowColors,
        // The item stat file went from `statFileBefore` to `statFileAfter`.
        StatFile,
    };

    Kind kind = Kind::Slot;
    int itemType = 0;
    int otherItemType = 0;
    ItemSlotState before;
    ItemSlotState after;
    std::optional<Data::Items::SharedItemModel> sharedBefore;
    std::optional<Data::Items::SharedItemModel> sharedAfter;
    std::vector<Data::Effects::GlowColor> colorsBefore;
    std::vector<Data::Effects::GlowColor> colorsAfter;
    // Shared, so the other steps do not each carry two stat files.
    std::shared_ptr<const Data::Items::ItemStatFile> statFileBefore;
    std::shared_ptr<const Data::Items::ItemStatFile> statFileAfter;
};

// What one action of a tool changed, undone and redone as a whole: an edited
// field, a rule flag set for the selected items, an added item. Each item
// type is in one step of an entry at most, since the steps are checked
// against the items before any of them is applied.
struct ItemEditEntry
{
    std::string label;
    std::vector<ItemEditStep> steps;
};

// Whether an item keeps the cells it takes in an item grid and its slot, which
// must not change while the game holds it (Hooks::isHeld).
bool KeepsShape(const Data::Items::ItemDefinition& from, const Data::Items::ItemDefinition& to);

// The undo history of the item tools for the session. Entries are applied
// back through the hooks (the editor's go through CItemDataHandler), so the
// item database, ItemAttribute and the opened models stay in step. An entry
// is only undone while the items are as it left them: a change that went
// past the history (e.g. Import from bmd clears it, but anything else) makes
// the undo refuse instead of overwriting it.
class ItemEditHistory
{
public:
    struct Hooks
    {
        std::function<ItemSlotState(int itemType)> readSlot;
        // Puts these states back exactly (nothing derived).
        std::function<void(std::span<const ItemSlotState> states)> restoreSlots;
        std::function<bool(int fromItemType, int toItemType)> moveItem;
        std::function<std::optional<Data::Items::SharedItemModel>(const std::string& name)> readSharedModel;
        // Makes the shared model `name` be `model` (nullopt: removed); `model`
        // may have another name (a rename).
        std::function<void(const std::string& name, const std::optional<Data::Items::SharedItemModel>& model)>
            restoreSharedModel;
        // Whether the game holds an item of that type, so its size and slot
        // must not change and it must not move or go (IsHeldByCharacter).
        // Optional.
        std::function<bool(int itemType)> isHeld;
        // The glow color list, and putting one back (the items glow with it).
        std::function<std::vector<Data::Effects::GlowColor>()> readGlowColors;
        std::function<void(std::span<const Data::Effects::GlowColor> colors)> restoreGlowColors;
        // The item stat file, and putting one back (the items take its profiles).
        std::function<Data::Items::ItemStatFile()> readStatFile;
        std::function<void(const Data::Items::ItemStatFile& file)> restoreStatFile;
    };

    // Why the last Undo or Redo did nothing.
    enum class Refusal
    {
        None,
        NothingToDo,
        // The items are no longer as the entry expects.
        ItemsChanged,
        // The entry would change the size or slot of a held item, or move or
        // remove it.
        ItemHeld,
    };

    // How many entries are kept; the oldest go first.
    static constexpr size_t MaxEntries = 256;

    explicit ItemEditHistory(Hooks hooks);

    // Adds an entry after the current one; the entries that were undone are
    // gone. An entry without steps is not added.
    void Record(ItemEditEntry entry);

    bool CanUndo() const;
    bool CanRedo() const;
    const std::string& GetUndoLabel() const;
    const std::string& GetRedoLabel() const;

    // Undoes the last entry, or redoes the next one. False, with nothing
    // changed, when there is none, the items are no longer as it expects or
    // it would change a held item (GetLastRefusal says which).
    bool Undo();
    bool Redo();
    Refusal GetLastRefusal() const
    {
        return m_lastRefusal;
    }

    // The item type the last Undo or Redo changed first, to select it;
    // nullopt for a change to shared models, glow colors or the item stat
    // file only.
    std::optional<int> GetLastChangedItem() const
    {
        return m_lastChangedItem;
    }

    // Drops every entry, for data that replaced all items (Import from bmd):
    // it is not the saved data until MarkSaved.
    void Clear();
    // The data as it is now was saved.
    void MarkSaved();
    // Whether the data changed since it was saved (or loaded).
    bool IsModifiedSinceSave() const;

private:
    bool TryApply(const ItemEditEntry& entry, bool undoing);
    bool IsAsLeft(const ItemEditEntry& entry, bool undoing) const;
    bool ChangesHeldItem(const ItemEditEntry& entry, bool undoing) const;
    void Apply(const ItemEditEntry& entry, bool undoing);
    void ApplyStep(const ItemEditStep& step, bool undoing);

    Hooks m_hooks;
    std::vector<ItemEditEntry> m_entries;
    // Entries before this index are done; the others were undone.
    size_t m_done = 0;
    // m_done when the data was saved; nullopt when that state is gone.
    std::optional<size_t> m_saved = 0;
    std::optional<int> m_lastChangedItem;
    Refusal m_lastRefusal = Refusal::None;
};
} // namespace MuEditor::Items

#endif // _EDITOR
