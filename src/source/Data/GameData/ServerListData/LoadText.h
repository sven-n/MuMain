#pragma once

#include "Data/GameData/Common/LocalizedString.h"

#include <string>

namespace Data::ServerList
{
// A load text in UTF-16 units.
inline constexpr int MaxLoadTextLength = 32;

// The {load} text of a full or an overloaded server, and how it looks.
struct LoadText
{
    LocalizedString text;
    // "default" (the button's own color), a theme color's name or a CustomColor's id.
    std::string color = "default";
    bool bold = false;
    bool italic = false;

    bool operator==(const LoadText&) const = default;
};
} // namespace Data::ServerList
