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
bool InsertItem(int index, std::span<const std::uint8_t> itemData);
void DiscardPickedItem();
const ITEM* FindPlayerItem(int index);
}
