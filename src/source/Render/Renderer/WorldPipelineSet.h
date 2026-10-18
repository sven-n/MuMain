// WorldPipelineSet.h: one pipeline per blend mode for each depth/cull state a world draw uses.
#pragma once

#include <array>
#include <cstddef>

namespace Render
{

template <typename Pipeline, std::size_t BlendModeCount> struct WorldPipelineSet
{
    std::array<Pipeline*, BlendModeCount> culled{};
    std::array<Pipeline*, BlendModeCount> noCull{};
    std::array<Pipeline*, BlendModeCount> depthOff{};
    std::array<Pipeline*, BlendModeCount> depthReadOnly{};

    // Same precedence as the Classic sets: no depth test, then read-only depth, then culling.
    [[nodiscard]] Pipeline* Select(std::size_t blendIndex, bool depthTest, bool depthWrite, bool cull) const
    {
        if (!depthTest)
        {
            return depthOff[blendIndex];
        }
        if (!depthWrite)
        {
            return depthReadOnly[blendIndex];
        }
        return cull ? culled[blendIndex] : noCull[blendIndex];
    }

    [[nodiscard]] bool IsComplete() const
    {
        for (std::size_t index = 0; index < BlendModeCount; ++index)
        {
            if (!culled[index] || !noCull[index] || !depthOff[index] || !depthReadOnly[index])
            {
                return false;
            }
        }
        return true;
    }

    template <typename ReleaseFunction> void Release(ReleaseFunction release)
    {
        for (auto* states : {&culled, &noCull, &depthOff, &depthReadOnly})
        {
            for (Pipeline*& pipeline : *states)
            {
                if (pipeline)
                {
                    release(pipeline);
                    pipeline = nullptr;
                }
            }
        }
    }
};

} // namespace Render
