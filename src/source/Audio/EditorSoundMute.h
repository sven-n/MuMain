#pragma once

#ifdef _EDITOR

// While muted, PlayBuffer starts no sound effect played once. Music, looped
// sounds and sounds already playing go on. The editor mutes the game this way
// while a muted effect preview runs.
namespace Audio::EditorMute
{
void SetMuted(bool muted);
[[nodiscard]] bool IsMuted();

// While it lives, muted; afterwards as before.
class Scope
{
public:
    Scope() : m_wasMuted(IsMuted())
    {
        SetMuted(true);
    }
    ~Scope()
    {
        SetMuted(m_wasMuted);
    }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    bool m_wasMuted;
};
} // namespace Audio::EditorMute

#endif // _EDITOR
