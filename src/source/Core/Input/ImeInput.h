#pragma once

// Windows IME (Input Method Editor) state handling: positions the composition
// window and toggles/queries the conversion mode so Latin input stays Latin in
// chat fields. Extracted from ZzzInterface.cpp.
namespace Input::IME
{
    // Place the IME composition window at a reference-space coordinate.
    void SetCompositionWindow(float x, float y);

    // Remember the current conversion/sentence mode so it can be restored later.
    void SaveStatus();

    // Restore the saved mode, or force half-width Latin when halfShape is true.
    void SetStatus(bool halfShape);

    // Query whether IME conversion is active; optionally stash and/or reset it.
    bool CheckStatus(bool change, int mode);

    // Debug overlay: dump the current/old IME conversion state on screen.
    void RenderStatus();
}
