#pragma once

#include <cstdint>
#include <optional>
#include <span>

typedef struct tagITEM ITEM;
enum struct STORAGE_TYPE;

namespace UI::Inventory
{
// Applies server item changes to the equipment and native inventory containers.
// Indices use the protocol's combined equipment/inventory/shop slot space.
void ClearAllItems();
void RemoveItem(int index);
void RemoveDroppedItem(int index);
bool InsertItem(int index, std::span<const std::uint8_t> itemData);
void DiscardPickedItem();
// After the server updated the item at `index`: discards the held item if that update ends it, i.e. it was used on
// another item (the update of the target answers the use) or it's the updated item itself. Any other held item stays
// in the hand, e.g. when a rented item expires while the player moves something else.
void DiscardPickedItemUpdatedBy(int index);
// The rule of DiscardPickedItemUpdatedBy: whether the server's update of the item at `updatedIndex` ends a held item
// which came from `sourcePos` of `source` and was used on the item at `usedOnIndex`, if any.
bool IsHeldItemEndedByUpdate(STORAGE_TYPE source, int sourcePos, std::optional<int> usedOnIndex, int updatedIndex);
// The server refused the use of the held item (ItemConsumptionFailed): it's a normal held item again, which no later
// item update ends.
void RefusePickedItemUse();
void RestorePickedItem();
int TransferSourceIndex();
bool ReceivePlayerTransfer(int index, std::span<const std::uint8_t> itemData);
void RejectTransfer();
bool HasPickedItem();
ITEM* FindPlayerItem(int index);
// The main grid only, without the extension or equipment.
ITEM* FindMainInventoryItem(int index);
void DeleteMainInventoryItem(int index);
// The item the player chose for an event entry, and its slot; nullptr if none.
ITEM* StandbyItem();
int StandbyItemIndex();
}
