#pragma once

// Modal overlay shown while ReconnectManager is auto-reconnecting after a disconnect;
// renders a dimmed backdrop, status panel, progress bar, and Cancel button (an RmlUi document,
// reconnect_dialog.rml, over every other document; natively without RmlUi). Hides it if inactive.
namespace UI::Reconnect
{
    // Captures the current frame at disconnect so the re-login phase (world torn
    // down, can't render) shows a frozen frame instead of black.
    void CaptureBackground();

    void RenderDialog();
}
