// The JSON payloads exchanged with the local Discord app, kept apart from the
// transport so they can be checked without Discord running.
#pragma once

#include "Integration/Discord/Activity.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Integration::Discord::Rpc
{
// Discord rejects text fields longer than this many characters.
inline constexpr std::size_t MaxTextLength = 128;

// Payload of the opening handshake frame.
[[nodiscard]] std::string Handshake(std::string_view applicationId);

// SET_ACTIVITY for the given process. No activity clears the presence.
[[nodiscard]] std::string SetActivity(const std::optional<Activity>& activity, std::uint32_t processId,
                                      std::uint64_t nonce);

// Whether a frame is the READY event Discord answers a good handshake with.
[[nodiscard]] bool IsReadyEvent(std::string_view payload);

// The message of an ERROR event, empty for any other payload.
[[nodiscard]] std::string ErrorMessage(std::string_view payload);

// Cuts the text to MaxTextLength bytes without splitting a UTF-8 sequence.
[[nodiscard]] std::string ClampText(std::string_view text);
} // namespace Integration::Discord::Rpc
