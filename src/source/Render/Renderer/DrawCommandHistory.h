#pragma once

#include <array>
#include <cstddef>

namespace Render
{
enum class DrawCommandFamily : std::size_t
{
    Triangles3D,
    Quads3D,
    TextTriangles2D,
    ScreenQuads2D,
    Count,
};

using DrawCommandHistory = std::array<std::size_t, static_cast<std::size_t>(DrawCommandFamily::Count)>;

[[nodiscard]] constexpr std::size_t& PreviousDrawCommand(DrawCommandHistory& history, DrawCommandFamily family)
{
    return history[static_cast<std::size_t>(family)];
}

[[nodiscard]] constexpr const std::size_t& PreviousDrawCommand(const DrawCommandHistory& history,
                                                               DrawCommandFamily family)
{
    return history[static_cast<std::size_t>(family)];
}
}
