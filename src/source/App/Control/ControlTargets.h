// The `target` field several control commands take: an object id or a
// character name, resolved against what the client sees.
#pragma once

#include "App/Control/ControlProtocol.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace App::Control::Targets
{
// What a `target` field turned out to be. A malformed one is a different
// answer from one the client simply does not see, and the commands say so.
enum class TargetLookup : std::uint8_t
{
    Missing,
    Malformed,
    NotInView,
    Found,
};

// Server-assigned id of the object a command names, by id or by character
// name.
[[nodiscard]] TargetLookup ResolveTarget(const Request& request, int& key);

// The error a lookup that did not find an object deserves, or an empty
// string when it did. `required` says whether the command needs one.
[[nodiscard]] std::string TargetError(const Request& request, TargetLookup lookup, std::string_view command,
                                      bool required);
} // namespace App::Control::Targets
