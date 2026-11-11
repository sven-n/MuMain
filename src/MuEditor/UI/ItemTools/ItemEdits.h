#pragma once

#ifdef _EDITOR

#include "ItemEditHistory.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterItemReferences.h"

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
    // With `updateMonsters`, the monsters that wear the item name its new
    // number too, in the same step (D60, Q-MN41); the item tools' Save
    // writes them into the monster list.
    bool MoveItem(std::string label, int fromItemType, int toItemType, bool updateMonsters = false);
    void SetItemModel(std::string label, const Data::Items::ItemModelDefinition& model);
    void SetSharedModel(std::string label, const Data::Items::SharedItemModel& sharedModel,
                        std::string_view previousName = {});
    bool RemoveSharedModel(std::string label, std::string_view name);
    // CItemDataHandler::SetGlowColor.
    void SetGlowColor(std::string label, const Data::Effects::GlowColor& color);
    // A new glow color (CItemDataHandler::SetGlowColor) put into the glow folder at `folder` (the empty path: under
    // the root), as one step.
    void AddGlowColor(std::string label, const Data::Effects::GlowColor& color, std::span<const std::size_t> folder);
    // CItemDataHandler::RemoveGlowColor for each name, as one step; false
    // when none could be removed.
    bool RemoveGlowColors(std::string label, std::span<const std::string> names);
    // CItemDataHandler::SetGroundLabelStyle.
    void SetGroundLabelStyle(std::string label, const Data::Items::GroundLabelStyle& style);
    // CItemDataHandler::SetNameCases, as one step.
    void SetNameCases(std::string label, const Data::Items::ItemNameCases& cases);
    // A new label style (CItemDataHandler::SetGroundLabelStyle) put into the ground label folder at `folder` (the empty
    // path: under the root), as one step.
    void AddGroundLabelStyle(std::string label, const Data::Items::GroundLabelStyle& style,
                             std::span<const std::size_t> folder);
    // CItemDataHandler::RemoveGroundLabelStyle for each name, as one step;
    // false when none could be removed.
    bool RemoveGroundLabelStyles(std::string label, std::span<const std::string> names);
    // The Colors window's roots of the item data, for SetColorFolders and the history.
    static constexpr std::string_view GlowFolders = "glow";
    static constexpr std::string_view GroundLabelFolders = "groundLabels";
    // The folders of the root (CItemDataHandler::SetGlowColorFolders, SetGroundLabelFolders), as one step.
    void SetColorFolders(std::string label, std::string_view root, std::vector<Data::Colors::ColorFolder> folders);
    // The render style file (CItemDataHandler::SetRenderStyles) and the textures its looks name, as one step (design
    // L39); nothing when it is as it was.
    void SetRenderStyles(std::string label, Data::Looks::RenderStylesFile file);
    // The same for the look file of `kind`: the render styles or the item effects.
    void SetLookFile(std::string label, Data::Looks::LookKind kind, Data::Looks::LookFile file);
    // The part models (CItemDataHandler::SetPartModels), as one step; nothing when they are as they were.
    void SetPartModels(std::string label, std::vector<Data::Items::PartModel> parts);
    // Puts the glow colors or the label styles in this order (the same names), as one step.
    void SetGlowColorOrder(std::string label, std::span<const Data::Effects::GlowColor> colors);
    void SetGroundLabelStyleOrder(std::string label, std::span<const Data::Items::GroundLabelStyle> styles);
    // CItemDataHandler::SetStatFile, with the items in `repointed` stored as
    // they are (their new stat profile), as one step. `file` must be valid.
    void SetStatFile(std::string label, const Data::Items::ItemStatFile& file,
                     std::span<const Data::Items::ItemDefinition> repointed = {});

    // The changes until EndGroup are one undo step under `label` (an item
    // duplicated with its model made shared). A group begun inside one is
    // part of it: its label is the outer one's.
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

    // CItemDataHandler::Save, with the monster list as last saved (the file)
    // and the item references the moves changed since the item tools saved
    // (D60), all files or none; then the history's saved state. The monster
    // tools' other unsaved changes stay unsaved. A place of the table that
    // names another item now (the monster tools changed it) is left out.
    Data::Items::ItemDataSaveResult Save(std::vector<Data::Items::ItemDataIssue>& issues);

    // The monster item references the moves changed that the item tools'
    // Save has not written yet.
    std::span<const Data::Monsters::MonsterItemChange> GetUnsavedMonsterItems() const
    {
        return m_monsterItems;
    }

    // The item and its model entry as the history records them.
    static ItemSlotState ReadSlot(int itemType);

private:
    ItemEdits();

    // Records the slots of `itemTypes` around `change`, as one entry.
    template <typename TChange> void RecordSlots(std::string label, std::span<const int> itemTypes, TChange&& change);
    // Into the history, or into the open group.
    void Record(ItemEditEntry entry);
    // The ground label styles from `before` to as they are now, as one entry with the folders' change; false when
    // they did not change.
    bool RecordGroundLabelStyles(std::string label, std::vector<Data::Items::GroundLabelStyle> before,
                                 std::vector<Data::Colors::ColorFolder> foldersBefore);
    // Adds the step of the root's folders from `before` to as they are now, when they changed.
    static void AddFolderStep(ItemEditEntry& entry, std::string_view root, std::vector<Data::Colors::ColorFolder> before);
    // Makes the monsters that name `fromItemType` name `toItemType` in the
    // monster table, and the step that holds the change; nullopt when no
    // monster names it.
    std::optional<ItemEditStep> UpdateMonsterItems(int fromItemType, int toItemType);
    // Keeps the places a monster step changed (done, undone or redone) in
    // m_monsterItems.
    void NoteMonsterItems(const ItemEditStep& step, bool undoing);

    ItemEditHistory m_history;
    std::optional<ItemEditEntry> m_group;
    int m_groupDepth = 0;
    std::vector<Data::Monsters::MonsterItemChange> m_monsterItems;
};
} // namespace MuEditor::Items

#define g_ItemEdits MuEditor::Items::ItemEdits::GetInstance()

#endif // _EDITOR
