#pragma once

#include <array>
#include <optional>
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
    // Where it stands (a monster, a map object), for the overlay's flash.
    std::optional<std::array<float, 3>> position;
};

// Names the source of the sounds played while it lives; the innermost wins.
class ScopedSoundSource
{
public:
    // `position`: three coordinates, or nullptr for none.
    ScopedSoundSource(SoundKind kind, std::string_view name, int number = -1, const float* position = nullptr);
    ~ScopedSoundSource();

    ScopedSoundSource(const ScopedSoundSource&) = delete;
    ScopedSoundSource& operator=(const ScopedSoundSource&) = delete;

private:
    SoundSource m_source;
    const SoundSource* m_outer = nullptr;
};

// The innermost living source; nullptr for none.
const SoundSource* CurrentSoundSource();

// Where the game's own code that plays a sound stands: a map object while it
// is moved or drawn, a monster while it moves or is drawn (Q-SM11). Only the
// overlay reads it, to mark the sounds that no data names; a sound's kind
// still comes from its SoundSource.
struct SoundPlace
{
    // MapObject or Monster.
    SoundKind kind = SoundKind::MapObject;
    // The map object's type or the monster's number.
    int number = -1;
    std::array<float, 3> position{};
};

// Names the place of the sounds played while it lives; the innermost wins.
class ScopedSoundPlace
{
public:
    // `position`: three coordinates.
    ScopedSoundPlace(SoundKind kind, int number, const float* position);
    ~ScopedSoundPlace();

    ScopedSoundPlace(const ScopedSoundPlace&) = delete;
    ScopedSoundPlace& operator=(const ScopedSoundPlace&) = delete;

private:
    SoundPlace m_place;
    const SoundPlace* m_outer = nullptr;
};

// The innermost living place; nullptr for none.
const SoundPlace* CurrentSoundPlace();
#else
class ScopedSoundSource
{
public:
    ScopedSoundSource(SoundKind, std::string_view, int = -1, const float* = nullptr) {}
    // Not trivial, so that a scope nobody reads draws no unused-variable
    // warning.
    ~ScopedSoundSource() {}
};

class ScopedSoundPlace
{
public:
    ScopedSoundPlace(SoundKind, int, const float*) {}
    ~ScopedSoundPlace() {}
};
#endif
} // namespace Audio
