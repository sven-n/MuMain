#pragma once

#include "Data/GameData/EffectData/EffectCreateParams.h"
#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/EffectData/EffectTypesJson.h"

#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Effects
{
// The names of the effect types of all kinds, built once on the loading
// screen from the catalogue files. Data that names types looks them up while
// loading; logs and the editor show names for numbers. The effect code keeps
// using the numbers. The creation values of the effects go into the effect
// registry (Render::Effects::BuildRegistry).
class EffectTypeCatalogue
{
public:
    static EffectTypeCatalogue& GetInstance();

    // Replaces the types of `kind`; `types` must be valid (ValidateEffectTypes).
    void Build(EffectKind kind, std::span<const EffectTypeEntry> types);

    // The number of the type with that name, or nullopt for unknown names.
    std::optional<int> FindType(EffectKind kind, std::string_view name) const;

    // The name of a type number, or an empty text when the kind has no type
    // with that number.
    std::string_view GetName(EffectKind kind, int type) const;

    size_t GetTypeCount(EffectKind kind) const;

    // The effect types with creation values, sorted by number.
    std::span<const EffectTypeCreateParams> GetCreateParams() const;

private:
    struct NamedType
    {
        std::string name;
        int type = 0;
    };

    std::array<std::vector<NamedType>, EffectKindCount> m_byName;
    std::array<std::vector<NamedType>, EffectKindCount> m_byType;
    std::vector<EffectTypeCreateParams> m_createParams;
};
} // namespace Data::Effects

#define g_EffectTypeCatalogue Data::Effects::EffectTypeCatalogue::GetInstance()
