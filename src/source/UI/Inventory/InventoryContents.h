#pragma once

#include <cstdint>
#include <span>

typedef struct tagITEM ITEM;

namespace UI::Inventory
{
// Applies server item changes to the equipment and native inventory containers.
// Indices use the protocol's combined equipment/inventory/shop slot space.
void ClearAllItems();
void RemoveItem(int index);
void RemoveDroppedItem(int index);
bool InsertItem(int index, std::span<const std::uint8_t> itemData);
void DiscardPickedItem();
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
