#include "Render/PostProcessing/SceneBoundary.h"

#include <algorithm>

namespace Render::PostProcessing
{

void SceneBoundary::Reset()
{
    m_sceneEnd.reset();
}

void SceneBoundary::Mark(std::size_t commandCount)
{
    if (m_sceneEnd)
    {
        return;
    }
    m_sceneEnd = commandCount;
}

bool SceneBoundary::IsMarked() const
{
    return m_sceneEnd.has_value();
}

FrameSplit SceneBoundary::Split(std::size_t commandCount, bool postProcessingActive) const
{
    if (!postProcessingActive || !m_sceneEnd)
    {
        return {false, commandCount};
    }
    return {true, std::min(*m_sceneEnd, commandCount)};
}

} // namespace Render::PostProcessing
