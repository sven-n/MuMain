#pragma once

#ifdef _EDITOR

// While muted, PlayBuffer starts no sound effect played once. Music, looped
// sounds and sounds already playing go on. The editor mutes the game this way
// while a muted effect preview runs.
namespace Audio::EditorMute
{
void SetMuted(bool muted);
[[nodiscard]] bool IsMuted();
} // namespace Audio::EditorMute

#endif // _EDITOR
