#include "stdafx.h"

#include "ScopedSystemCursor.h"

namespace Core::Platform
{
// The cursor shows while the display counter of ShowCursor is 0 or more. The
// game lowers it every time it hides the cursor, so it can be far below 0;
// it is raised until the cursor shows and lowered by the same amount after.
ScopedSystemCursor::ScopedSystemCursor()
{
    do
    {
        ++m_raisedCount;
    } while (ShowCursor(TRUE) < 0);
}

ScopedSystemCursor::~ScopedSystemCursor()
{
    for (int i = 0; i < m_raisedCount; ++i)
    {
        ShowCursor(FALSE);
    }
}
} // namespace Core::Platform
