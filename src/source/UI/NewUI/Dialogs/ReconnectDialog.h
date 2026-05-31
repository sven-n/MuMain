#pragma once

// Modal overlay shown while ReconnectManager is auto-reconnecting after an
// in-game disconnect (issue #338). Reads its state from ReconnectManager and
// renders a dimmed backdrop, a status panel with the current step and a
// progress bar, and a Cancel button. A no-op when no reconnect is in progress.
namespace UI::Reconnect
{
    // Captures the current (clean) game frame into a texture. Called at the
    // moment of disconnect so the brief re-login phase - when the world is torn
    // down and can't render - shows the frozen game instead of a black screen.
    void CaptureBackground();

    void RenderDialog();
}
