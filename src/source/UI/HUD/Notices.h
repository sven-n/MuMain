#pragma once

// On-screen notice banner: the scrolling system messages drawn over the HUD
// (events, server announcements, etc.). Extracted from ZzzInterface.cpp.
namespace UI::Notices
{
    // Push a notice line. Long text is split across two lines automatically.
    void Create(const wchar_t* text, int color);

    // Drop all current notices.
    void Clear();

    // Per-frame timing: expires the oldest notice over time.
    void Move();

    // Draw the active notices.
    void Render();
}
