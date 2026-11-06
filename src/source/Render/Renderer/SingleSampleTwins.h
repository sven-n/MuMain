#pragma once

#include <cstddef>
#include <span>
#include <unordered_map>

// While anti-aliasing is on, the passes that render into single-sample textures (offscreen
// captures, render targets, the post-RmlUi pass, FlushRenderCommands) replay each draw with the
// single-sample twin of its multisampled pipeline.
namespace Render
{

// The twins of `multisampled` (each built pipeline, nulls are unbuilt slots) are complete when
// every built pipeline maps to a built twin. An incomplete set must not be used: a draw without
// a twin would vanish, and a draw bound without substitution would pair a multisampled pipeline
// with a single-sample target.
template <typename Pipeline>
[[nodiscard]] bool SingleSampleTwinsComplete(std::span<Pipeline* const> multisampled,
                                             const std::unordered_map<Pipeline*, Pipeline*>& twins)
{
    std::size_t built = 0;
    for (Pipeline* pipeline : multisampled)
    {
        if (!pipeline)
        {
            continue;
        }
        ++built;
        const auto twin = twins.find(pipeline);
        if (twin == twins.end() || !twin->second)
        {
            return false;
        }
    }
    return built == 0 || !twins.empty();
}

} // namespace Render
