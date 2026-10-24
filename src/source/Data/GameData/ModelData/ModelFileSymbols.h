#pragma once

#include <span>
#include <string_view>

namespace Data::Models
{
// A model as the code names it ("MODEL_ELITE_YETI") and its index in Models[].
struct ModelFileSymbol
{
    std::string_view code;
    int type = 0;
};

// The models that Data/Models/ModelFiles.json may name (the monster models),
// one symbol per number, sorted by number.
std::span<const ModelFileSymbol> GetModelFileSymbols();

// The symbol with that code name, or nullptr.
const ModelFileSymbol* FindModelFileSymbol(std::string_view code);
} // namespace Data::Models
