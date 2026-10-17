#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace GameLogic::Items
{
// Most zen the vault dialogs let the vault or the inventory hold after a move. Below the server's
// default vault maximum (2,147,483,647), so it always fits CharacterMachine->StorageGold (an int).
constexpr std::int64_t kMaxVaultZen = 2'100'000'000;

// Digits of kMaxVaultZen: the longest input worth accepting.
constexpr int kMaxVaultZenDigits = 10;

// Most zen a single move can carry, given what the vault and the inventory hold. Never negative.
[[nodiscard]] constexpr std::int64_t MaxVaultDeposit(std::int64_t inventoryZen, std::int64_t vaultZen)
{
    return std::max<std::int64_t>(0, std::min(inventoryZen, kMaxVaultZen - vaultZen));
}

[[nodiscard]] constexpr std::int64_t MaxVaultWithdraw(std::int64_t vaultZen, std::int64_t inventoryZen)
{
    return std::max<std::int64_t>(0, std::min(vaultZen, kMaxVaultZen - inventoryZen));
}

// Parses a digits-only amount, clamped to kMaxVaultZen. Empty input is 0.
[[nodiscard]] inline std::int64_t ParseVaultZenAmount(std::wstring_view digits)
{
    std::int64_t amount = 0;
    for (const wchar_t digit : digits)
    {
        amount = amount * 10 + (digit - L'0');
        if (amount > kMaxVaultZen)
            return kMaxVaultZen;
    }
    return amount;
}
} // namespace GameLogic::Items
