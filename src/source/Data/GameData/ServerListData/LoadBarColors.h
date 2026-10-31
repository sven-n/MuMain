#pragma once

#include <string>

namespace Data::ServerList
{
// The colors of the load bar: a theme color's name or a CustomColor's id each. "default" is the
// theme's own, its server-load-base, server-load-full and server-load-empty tokens.
struct LoadBarColors
{
    // What the segments are mixed from and toward.
    std::string base = "default";
    std::string full = "default";
    // The unlit part of a segment.
    std::string empty = "default";

    bool operator==(const LoadBarColors&) const = default;
};
} // namespace Data::ServerList
