#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

// Where a sound comes from, for the editor's Sound Mixer (Q-SM2, Q-SM13): the
// data's players name the kind and source of the sounds they play while a
// ScopedSoundSource lives, the game's code the place it runs for while a
// ScopedSoundPlace lives. In the game's own builds they do nothing.
namespace Audio
{
// The mixer's types (Q-SM13), in the window's order.
enum class SoundKind
{
    Music,
    MapAmbience,
    MapObject,
    Monster,
    Npc,
    // The hero: your character.
    Hero,
    // Every other player.
    Player,
    // An effect nobody made: an effect counts for its maker.
    Effect,
    // An item on the ground.
    Item,
    // Birds, bats, mice, fish.
    Creature,
    Interface,
    Other,
};

constexpr int SoundKindCount = 12;

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

// What the game's own code that plays a sound runs for (Q-SM11, Q-SM13): a map
// object while it is moved or drawn, a character while it moves or is drawn,
// the caster of a skill the server sends, an effect as it moves (typed by its
// maker), the interface. Without a source, a sound's type is its place's, and
// the overlay marks it there; one without a position (the interface) goes to
// the overlay's log.
struct SoundPlace
{
    SoundKind kind = SoundKind::MapObject;
    // The map object's type, the monster's or NPC's number, the effect's type;
    // -1 for none.
    int number = -1;
    std::optional<std::array<float, 3>> position;
};

// Names the place of the sounds played while it lives; the innermost wins.
class ScopedSoundPlace
{
public:
    // `position`: three coordinates, or nullptr for none.
    ScopedSoundPlace(SoundKind kind, int number, const float* position);
    explicit ScopedSoundPlace(const SoundPlace& place);
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
