#pragma once

#include <span>
#include <string_view>

// The game texts a warp shows when its equipment rules block it (the
// "blockedMessage" of Data/Maps/Warps.json), by the key of the game text
// (I18N::Game). The game texts have no lookup by key, so the ones a warp may
// name are listed here.
namespace Data::Maps
{
// The key of the general text: "You can't warp there with your equipment."
constexpr std::string_view GeneralWarpMessageKey = "WarpBlockedByEquipment";

// Every key a warp may name, the general one first.
std::span<const std::string_view> GetWarpMessageKeys();

bool IsWarpMessageKey(std::string_view key);

// The text of `key` in the current language; the general text for an empty or
// unknown key.
const wchar_t* GetWarpMessageText(std::string_view key);
} // namespace Data::Maps
