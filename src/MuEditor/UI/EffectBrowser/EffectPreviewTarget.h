#pragma once

#ifdef _EDITOR

#include <cstdint>

namespace MuEditor::Effects
{
// The texture the effect preview is drawn into. It is made once and drawn
// into every frame. A texture of another size is made only between frames:
// releasing a texture inside a frame makes the renderer skip the game's
// draws of that frame.
class EffectPreviewTarget
{
public:
    // Opens the capture into the texture for this frame; makes the texture
    // the first time. A size other than the texture's waits for the next
    // BeforeFrame when `resizeNow` allows it. Returns the texture, 0 when no
    // capture could be opened.
    std::uint32_t Begin(int width, int height, bool resizeNow);

    // Called between frames: releases the texture when the preview was not
    // drawn in the last frame or a new size is waiting.
    void BeforeFrame();

private:
    std::uint32_t m_texture = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_used = false;
    bool m_resize = false;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
