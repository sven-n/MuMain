#pragma once

#ifdef _EDITOR

#include <cstdint>

namespace MuEditor::Effects
{
// The texture the effect preview is drawn into. It is made once and drawn
// into every frame the game draws; a frame it skips keeps the last picture
// (KeepLastPicture). A texture of another size replaces it only at a frame
// boundary: the old one is released between frames (BeforeFrame) and the
// next Begin makes the new one, because releasing a texture inside a frame
// makes the renderer skip the game's draws of that frame.
class EffectPreviewTarget
{
public:
    // Opens the capture into the texture for this frame; makes the texture
    // the first time. A size other than the texture's waits for the next
    // BeforeFrame when `resizeNow` allows it. Returns the texture, 0 when no
    // capture could be opened.
    std::uint32_t Begin(int width, int height, bool resizeNow);

    // A frame the game skipped (the GPU is behind): nothing can be drawn into
    // the texture, but the editor's own windows may still be drawn from it
    // (CMuEditorCore::RenderPlatformWindows), so they show the last picture.
    // Keeps the texture for the next frame. Returns it, 0 while there is none.
    std::uint32_t KeepLastPicture();

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
