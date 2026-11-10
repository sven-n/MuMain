#pragma once

#include <array>
#include <string>

// How the game shows a monster it cannot show as itself (monsters design, D64,
// Q-MN42e): a "bug look" instead of the Bull Fighter's body, so that a missing
// entry or file shows in the game. The settings section "unknownMonsters" of
// Data/Monsters/Monsters.json holds it; the name ("Unknown monster {number}")
// and what it says ("I am a bug: {number}") are game texts.
namespace Data::Monsters
{
// The longest wait between its messages the reader takes.
inline constexpr int MaxUnknownMessageSeconds = 3600;

struct UnknownMonsterLook
{
    // Its body, as an entry's "model" names one (the Ghost), and its scale.
    std::string model = "MONSTER_MODEL_GHOST";
    float scale = 1.0f;
    // The colour of its pulsing glow.
    std::array<float, 3> glow{1.0f, 0.15f, 0.1f};
    // Red X marks rising from it.
    bool marks = true;
    // It says what it is when it appears and again every so many seconds; 0:
    // only when it appears.
    int messageSeconds = 10;
    // Who gets it: a number the list has no entry for, an entry that names no
    // model (76, 252, 296, 585), a model whose file is missing (Molt 68).
    // Another keeps the Bull Fighter's body, and one with a missing file no
    // body.
    bool forNoEntry = true;
    bool forNoModel = true;
    bool forMissingFile = true;

    bool operator==(const UnknownMonsterLook&) const = default;
};

// Why the game cannot show a monster as itself: the reasons "for" names.
enum class UnknownReason
{
    NoEntry,
    NoModel,
    MissingFile,
};

// Whether the look is for monsters of the reason.
inline bool IsFor(const UnknownMonsterLook& look, UnknownReason reason)
{
    switch (reason)
    {
    case UnknownReason::NoEntry:
        return look.forNoEntry;
    case UnknownReason::NoModel:
        return look.forNoModel;
    case UnknownReason::MissingFile:
        return look.forMissingFile;
    }
    return false;
}
} // namespace Data::Monsters
