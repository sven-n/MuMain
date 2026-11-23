#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// A buff's value or duration as OpenMU writes it (buffs design B17): a decimal constant, decimal terms on the hero's
// attributes and an optional maximum, summed as decimals and cut only where a number is shown. The item options keep
// their whole numbers (ItemOptionValue); this format is the buffs' own. See docs/buff-data.md, "Values".
namespace Data::Buffs
{
// The hero attributes a buff value's term can read, as the caster has them (OpenMU's Stats names in camelCase). A
// term on another name adds nothing (Buffs.json warns about it).
enum class BuffTermAttribute : std::uint8_t
{
    Unread,
    Level,
    TotalLevel,
    MasterLevel,
    TotalStrength,
    TotalAgility,
    TotalVitality,
    TotalEnergy,
    TotalLeadership,
    TotalStrengthAndAgility,
    MaximumMana,
    NearbyPartyMemberCount,
};

// How many attributes BuffTermAttribute has, Unread included: the size of a table indexed by it.
constexpr size_t BuffTermAttributeCount = static_cast<size_t>(BuffTermAttribute::NearbyPartyMemberCount) + 1;

struct BuffTermAttributeName
{
    std::string_view name;
    BuffTermAttribute attribute;
};

constexpr std::array<BuffTermAttributeName, BuffTermAttributeCount - 1> BuffTermAttributes = {{
    {"level", BuffTermAttribute::Level},
    {"totalLevel", BuffTermAttribute::TotalLevel},
    {"masterLevel", BuffTermAttribute::MasterLevel},
    {"totalStrength", BuffTermAttribute::TotalStrength},
    {"totalAgility", BuffTermAttribute::TotalAgility},
    {"totalVitality", BuffTermAttribute::TotalVitality},
    {"totalEnergy", BuffTermAttribute::TotalEnergy},
    {"totalLeadership", BuffTermAttribute::TotalLeadership},
    {"totalStrengthAndAgility", BuffTermAttribute::TotalStrengthAndAgility},
    {"maximumMana", BuffTermAttribute::MaximumMana},
    {"nearbyPartyMemberCount", BuffTermAttribute::NearbyPartyMemberCount},
}};

// The attribute of that name; Unread for a name the client doesn't read.
constexpr BuffTermAttribute FindBuffTermAttribute(std::string_view name)
{
    for (const BuffTermAttributeName& entry : BuffTermAttributes)
    {
        if (entry.name == name)
        {
            return entry.attribute;
        }
    }
    return BuffTermAttribute::Unread;
}

// A term the value adds: the attribute times `times`, or divided by `divisor`. Exactly one of the two is above 0.
struct BuffTerm
{
    std::string attribute;
    double times = 0;
    double divisor = 0;
    // The attribute of that name, which the buff table resolves when it loads (BuffDatabase::Build), so a value
    // computed every frame never compares a name.
    BuffTermAttribute heroAttribute = BuffTermAttribute::Unread;

    // The data only: heroAttribute follows from the name.
    bool operator==(const BuffTerm& other) const
    {
        return attribute == other.attribute && times == other.times && divisor == other.divisor;
    }
};

struct BuffValue
{
    double values = 0;
    std::vector<BuffTerm> per;
    // The highest the constant and the terms reach together; the master skills' parts come after it.
    std::optional<double> maximum;

    bool operator==(const BuffValue& other) const = default;
};

// Gives each of the value's terms the attribute its name names.
inline void ResolveTermAttributes(BuffValue& value)
{
    for (BuffTerm& term : value.per)
    {
        term.heroAttribute = FindBuffTermAttribute(term.attribute);
    }
}
} // namespace Data::Buffs
