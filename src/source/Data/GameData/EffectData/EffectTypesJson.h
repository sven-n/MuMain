#pragma once

#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The effect catalogue: one file per kind in Data/Effects names every type
// that the code of that kind uses (docs/effect-data.md):
//
//   { "formatVersion": 1, "kind": "particle",
//     "types": [ { "name": "smoke", "code": "BITMAP_SMOKE" }, { "name": "smoke2", "code": "BITMAP_SMOKE+1" } ] }
//
// `code` is the type as the code writes it (EffectTypeSymbols.h). Names have
// letters and digits, start with a letter and are unique in their kind.
namespace Data::Effects
{
struct EffectTypeEntry
{
    std::string name;
    std::string code;

    bool operator==(const EffectTypeEntry&) const = default;
};

constexpr int EffectTypesFormatVersion = 1;

// "EffectTypes.json", "ParticleTypes.json", "JointTypes.json", "SpriteTypes.json".
std::string_view GetEffectTypesFileName(EffectKind kind);

// Reads the types of the catalogue file of `kind` into `types`.
void ReadEffectTypesJson(std::string_view text, const std::string& source, EffectKind kind,
                         std::vector<EffectTypeEntry>& types, std::vector<Items::ItemDataIssue>& issues);

// Checks the types of a kind: valid and unique names, known codes, and one
// name for every type the code of the kind uses.
void ValidateEffectTypes(EffectKind kind, std::span<const EffectTypeEntry> types, const std::string& source,
                         std::vector<Items::ItemDataIssue>& issues);

// The text of the catalogue file of `kind`: sorted by name, fields in a fixed
// order.
std::string WriteEffectTypesJson(EffectKind kind, std::span<const EffectTypeEntry> types);
} // namespace Data::Effects
