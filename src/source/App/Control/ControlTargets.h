// The `target` field several control commands take: an object id, a
// character name, or {"monster": <number>} for the nearest living monster or
// NPC with that monster number, resolved against what the client sees.
#pragma once

#include "App/Control/ControlProtocol.h"

#include <cstdint>
#include <map>
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

// Server-assigned id of the object a command names, by id, by character name
// or by monster number.
[[nodiscard]] TargetLookup ResolveTarget(const Request& request, int& key);

// The monster number of a target written as {"monster": <number>}: false
// unless `monster` is the only field and a whole number from 0 to 1023
// (251 or 251.0; the fields cannot tell the string "251" from the number, so it
// counts too).
[[nodiscard]] bool ReadMonsterNumber(const std::map<std::string, std::string>& fields, int& number);

// The error a lookup that did not find an object deserves, or an empty
// string when it did. `required` says whether the command needs one.
[[nodiscard]] std::string TargetError(const Request& request, TargetLookup lookup, std::string_view command,
                                      bool required);
} // namespace App::Control::Targets
