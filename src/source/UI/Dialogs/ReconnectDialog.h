#pragma once

// Modal overlay shown while ReconnectManager is auto-reconnecting after a disconnect;
// renders a dimmed backdrop, status panel, progress bar, and Cancel button. No-op if inactive.
namespace UI::Reconnect
{
    // Captures the current frame at disconnect so the re-login phase (world torn
    // down, can't render) shows a frozen frame instead of black.
    void CaptureBackground();

    void RenderDialog();
}
