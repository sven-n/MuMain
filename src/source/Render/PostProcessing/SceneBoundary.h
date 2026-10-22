#pragma once

#include <cstddef>
#include <optional>

// Post-processing reads only the 3D scene. A scene marks where its 3D world ends
// (IMuRenderer::EndSceneGeometry); the recorded frame splits there into the scene,
// which is processed, and the UI, which is drawn on the processed image.
namespace Render::PostProcessing
{

struct FrameSplit
{
    // False: the frame replays as one Classic pass, unprocessed.
    bool postProcessed = false;
    // Commands [0, sceneEnd) are the 3D scene; [sceneEnd, commandCount) are the UI.
    std::size_t sceneEnd = 0;
};

// The boundary of one recorded frame.
class SceneBoundary
{
public:
    // Forgets the previous frame's boundary.
    void Reset();

    // Marks the end of the 3D scene at `commandCount` recorded commands. The first
    // mark of a frame wins: everything recorded after it is UI.
    void Mark(std::size_t commandCount);

    [[nodiscard]] bool IsMarked() const;

    // Splits a frame of `commandCount` commands. A frame without a mark, or with
    // post-processing inactive, is not processed. A mark past the last command is
    // the end of the frame.
    [[nodiscard]] FrameSplit Split(std::size_t commandCount, bool postProcessingActive) const;

private:
    std::optional<std::size_t> m_sceneEnd;
};

} // namespace Render::PostProcessing
