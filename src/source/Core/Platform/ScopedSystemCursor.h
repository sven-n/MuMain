#pragma once

namespace Core::Platform
{
// The game hides the system mouse cursor and draws its own while it renders.
// While it does not render, e.g. on the loading screen or behind a dialog,
// no cursor would be visible; this shows the system cursor for as long as it
// exists and hides it again afterwards.
class ScopedSystemCursor
{
public:
    ScopedSystemCursor();
    ~ScopedSystemCursor();

    ScopedSystemCursor(const ScopedSystemCursor&) = delete;
    ScopedSystemCursor& operator=(const ScopedSystemCursor&) = delete;

private:
    // How often the cursor display counter was raised.
    int m_raisedCount = 0;
};
} // namespace Core::Platform
