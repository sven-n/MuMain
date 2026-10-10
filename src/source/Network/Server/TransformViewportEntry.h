// Bounds of the entries of head code 45, the transformed players that come into view.
#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "Network/Server/WSclient.h"

namespace Network::Viewport
{
// The length of the transformed player entry at `offset`: its fixed fields and
// the buffs it carries. An entry has room for MAX_BUFF_SLOT_INDEX buffs, but
// only its s_BuffCount buffs are sent. nullopt when the packet ends before the
// entry does, or when the entry carries more buffs than a character can have.
[[nodiscard]] std::optional<std::size_t> TransformViewportEntryLength(std::span<const BYTE> packet, std::size_t offset);
} // namespace Network::Viewport
