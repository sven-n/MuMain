#pragma once

#include <string>
#include <string_view>

// Where a sound comes from, for the editor's Sound Mixer (Q-SM2): the data's
// players name the kind and source of the sounds they play while a
// ScopedSoundSource lives. In the game's own builds it does nothing.
namespace Audio
{
enum class SoundKind
{
    Music,
    MapAmbience,
    MapObject,
    Monster,
    Other,
};

constexpr int SoundKindCount = 5;

#ifdef _EDITOR
struct SoundSource
{
    SoundKind kind = SoundKind::Other;
    // A monster sound set's name, or "" (a map object sound).
    std::string name;
    // The monster's number or the map object's type; -1 for none.
    int number = -1;
};

// Names the source of the sounds played while it lives; the innermost wins.
class ScopedSoundSource
{
public:
    ScopedSoundSource(SoundKind kind, std::string_view name, int number = -1);
    ~ScopedSoundSource();

    ScopedSoundSource(const ScopedSoundSource&) = delete;
    ScopedSoundSource& operator=(const ScopedSoundSource&) = delete;

private:
    SoundSource m_source;
    const SoundSource* m_outer = nullptr;
};

// The innermost living source; nullptr for none.
const SoundSource* CurrentSoundSource();
#else
class ScopedSoundSource
{
public:
    ScopedSoundSource(SoundKind, std::string_view, int = -1) {}
    // Not trivial, so that a scope nobody reads draws no unused-variable
    // warning.
    ~ScopedSoundSource() {}
};
#endif
} // namespace Audio
