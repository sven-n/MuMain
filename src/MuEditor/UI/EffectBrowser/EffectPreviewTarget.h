#pragma once

#ifdef _EDITOR

#include <cstdint>

namespace MuEditor::Effects
{
// The depth buffer a preview draws with: the one all one-shot captures share
// (the renderer makes it again when the size changes), or one of its own (a
// render target, which clears to transparent), for a second picture of
// another size drawn in the same frames.
enum class PreviewDepth
{
    Shared,
    Own,
};

// The texture the effect preview is drawn into. It is made once and drawn
// into every frame the game draws; a frame it skips keeps the last picture
// (KeepLastPicture). A texture of another size replaces it only at a frame
// boundary: the old one is released between frames (BeforeFrame) and the
// next Begin makes the new one, because releasing a texture inside a frame
// makes the renderer skip the game's draws of that frame.
class EffectPreviewTarget
{
public:
    explicit EffectPreviewTarget(PreviewDepth depth = PreviewDepth::Shared) : m_depth(depth) {}

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
    PreviewDepth m_depth;
    std::uint32_t m_texture = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_used = false;
    bool m_resize = false;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
