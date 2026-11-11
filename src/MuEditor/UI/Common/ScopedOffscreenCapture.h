#pragma once

#ifdef _EDITOR

namespace MuEditor
{
// Ends the renderer's offscreen capture on every way out of the scope it
// lives in. Make it right after BeginOffscreenCapture succeeds: a capture left
// open makes the renderer refuse every later one, so every preview after it
// would stay empty. A refused capture is the caller's to handle.
class ScopedOffscreenCapture
{
public:
    ScopedOffscreenCapture() = default;
    ~ScopedOffscreenCapture();

    ScopedOffscreenCapture(const ScopedOffscreenCapture&) = delete;
    ScopedOffscreenCapture& operator=(const ScopedOffscreenCapture&) = delete;
};
} // namespace MuEditor

#endif // _EDITOR
