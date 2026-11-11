#pragma once

#include <cstdint>
#include <span>

// Vault and NPC-shop contents the server reports, applied to those windows' item grids. Item data
// is the item's serialized form, borrowed for the call.
namespace UI::Storage
{
struct ContainerItem
{
    int slot;
    std::span<const std::uint8_t> itemData;
};

// The open container's full contents: the NPC shop's when it is open, else the vault's.
void ContainerListed(std::span<const ContainerItem> items);
// An item moved into the vault (main or extended part, by slot).
void VaultItemPlaced(int slot, std::span<const std::uint8_t> itemData);

enum class VaultStatus
{
    Unlocked,
    Locked,
    WrongPassword,
    AlreadyLocked,
    PasswordAccepted,
    PasswordRejected,
};

void VaultStatusChanged(VaultStatus status);
}
