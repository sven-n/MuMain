#include "stdafx.h"

#ifdef _EDITOR

#include "EditorSoundMute.h"

namespace Audio::EditorMute
{
namespace
{
bool g_muted = false;
}

void SetMuted(bool muted)
{
    g_muted = muted;
}

bool IsMuted()
{
    return g_muted;
}
} // namespace Audio::EditorMute

#endif // _EDITOR
