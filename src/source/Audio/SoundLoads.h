#pragma once

#include <optional>
#include <set>

// What the session saw of the code's sounds, for the Sound Editor (sound
// editor design, Q-SE1): when each loaded, and the ones played but never
// loaded (a load that names another id, a file without its extension). In
// the game's own builds it keeps nothing.
namespace Audio::SoundLoads
{
// Loaded at start (OpenSounds), before a map.
constexpr int AtStart = -1;

#ifdef _EDITOR
// Names where the loads while it lives happen: the world being entered
// (MapManager); the innermost wins, none is AtStart.
class ScopedLoadWhere
{
public:
    explicit ScopedLoadWhere(int world);
    ~ScopedLoadWhere();

    ScopedLoadWhere(const ScopedLoadWhere&) = delete;
    ScopedLoadWhere& operator=(const ScopedLoadWhere&) = delete;

private:
    std::optional<int> m_outer;
};

// LoadGameSound loaded the sound: kept with the living place, AtStart
// without one; its first load counts.
void NoteLoad(int number);

// Where the sound first loaded this session; nullopt for not yet.
std::optional<int> WhereLoaded(int number);

// PlayBuffer was asked for a sound that has nothing loaded.
void NotePlayedUnloaded(int number);

// The sounds played but not loaded so far.
const std::set<int>& PlayedUnloaded();

// Forgets everything (tests).
void Clear();
#else
class ScopedLoadWhere
{
public:
    explicit ScopedLoadWhere(int) {}
    // Not trivial, so that a scope nobody reads draws no unused-variable
    // warning.
    ~ScopedLoadWhere() {}
};
#endif
} // namespace Audio::SoundLoads
