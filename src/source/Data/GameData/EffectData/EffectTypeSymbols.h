#pragma once

#include "Data/GameData/EffectData/EffectKind.h"

#include <span>
#include <string_view>

namespace Data::Effects
{
// A type as the code writes it, and its number: an enum name ("MODEL_ARROW"),
// an enum name with an offset ("BITMAP_SMOKE+1", without spaces) or a number
// ("9").
struct EffectTypeSymbol
{
    std::string_view code;
    int type = 0;
};

// The types that the code of a kind uses, one symbol per number, sorted by
// number. The catalogue files name each of them once.
std::span<const EffectTypeSymbol> GetEffectTypeSymbols(EffectKind kind);
} // namespace Data::Effects
