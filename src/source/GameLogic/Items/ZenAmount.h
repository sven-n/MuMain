#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace GameLogic::Items
{
// Most zen the vault dialogs let the vault hold after a move. Below the server's default vault
// maximum (2,147,483,647), so it always fits CharacterMachine->StorageGold (an int).
constexpr std::int64_t kMaxVaultZen = 2'100'000'000;

// Most zen the client lets the inventory hold, as in the native client.
constexpr std::int64_t kMaxInventoryZen = 2'000'000'000;

[[nodiscard]] constexpr int DigitCount(std::int64_t value)
{
    int digits = 1;
    for (; value >= 10; value /= 10)
        ++digits;
    return digits;
}

// The longest input worth accepting.
constexpr int kMaxVaultZenDigits = DigitCount(kMaxVaultZen);

// Most zen a single move can carry, given what the vault and the inventory hold. Never negative.
[[nodiscard]] constexpr std::int64_t MaxVaultDeposit(std::int64_t inventoryZen, std::int64_t vaultZen)
{
    return std::max<std::int64_t>(0, std::min(inventoryZen, kMaxVaultZen - vaultZen));
}

[[nodiscard]] constexpr std::int64_t MaxVaultWithdraw(std::int64_t vaultZen, std::int64_t inventoryZen)
{
    return std::max<std::int64_t>(0, std::min(vaultZen, kMaxInventoryZen - inventoryZen));
}

// Parses the leading digits, like _wtoi. Empty input is 0. Stops reading once the amount is above
// kMaxVaultZen, so a longer input can't overflow: any result above it is too much for a move.
[[nodiscard]] inline std::int64_t ParseVaultZenAmount(std::wstring_view text)
{
    std::int64_t amount = 0;
    for (const wchar_t digit : text)
    {
        if (digit < L'0' || digit > L'9' || amount > kMaxVaultZen)
            break;
        amount = amount * 10 + (digit - L'0');
    }
    return amount;
}
} // namespace GameLogic::Items
