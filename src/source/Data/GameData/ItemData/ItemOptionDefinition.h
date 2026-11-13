#pragma once

#include "Core/Platform/WinCompat.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The option groups an item can have (luck, the additional option, excellent, ...) as data: the definitions of
// Data/Items/Options/*.json, each with its options, as OpenMU's ItemOptionDefinition. An item links the definitions
// it can have by name ("options" in its item file); which byte of the server's item data turns an option on is the
// definition's type. See docs/item-data.md, "Item options".
namespace Data::Items
{
// What a definition is: which byte of the server's item data holds its options and what their number means. The
// names are OpenMU's ItemOptionTypes in camelCase. Each type comes with the pull request that reads its byte.
enum class ItemOptionType : BYTE
{
    // The luck flag; one option.
    Luck,
    // The additional option's level (ITEM::OptionLevel, the option byte's low four bits: 1 to 15, Season 6 sends 1 to
    // 4); its options numbered by the option type the server sends (ITEM::OptionType), their value by that level.
    Option,
    // The excellent byte (ITEM::ExcellentFlags); its options numbered 1 to 6, each on with the byte's bit number - 1.
    Excellent,
};

// An excellent option's number - 1 is its bit of the excellent byte, whose bits 0 to 5 hold them.
constexpr int MaxExcellentOptionNumber = 6;
// The excellent byte with every excellent option on.
constexpr BYTE AllExcellentOptionFlags = (1 << MaxExcellentOptionNumber) - 1;

// How an option's value adds to its attribute: as it is, after the other additions (OpenMU's AddFinal), or in per
// cent.
enum class ItemOptionAdd : BYTE
{
    Add,
    AddFinal,
    Percent,
};

// The hero attributes a term can read, named in ItemOptionTermAttributes (ItemOptionEffects.h); Unread for a name the
// client doesn't read, which adds nothing.
enum class ItemOptionTermAttribute : BYTE
{
    Unread,
    // The hero's level.
    Level,
    // The level plus the master level.
    TotalLevel,
    MasterLevel,
};

// A term an option's value adds on an attribute of the hero ("level" / 20): the attribute times `times`, or divided by
// `divisor`, rounded down on its own. Exactly one of the two is above 0. The attributes the client reads are
// ItemOptionTermAttributes (ItemOptionEffects.h).
struct ItemOptionTerm
{
    std::string attribute;
    int times = 0;
    int divisor = 0;
    // The attribute of that name, which the option database resolves when it loads (ResolveTermAttributes), so the
    // value computed every frame never compares a name.
    ItemOptionTermAttribute heroAttribute = ItemOptionTermAttribute::Unread;

    // The data only: heroAttribute follows from the name, so a loaded term equals an edited one with the same data.
    bool operator==(const ItemOptionTerm& other) const
    {
        return attribute == other.attribute && times == other.times && divisor == other.divisor;
    }
};

// An option's value: a constant, or a list with one value per level from level 1, whose last entry also counts above
// it; plus its terms on hero attributes, computed when the value is asked (GameLogic::Items::ComputeOptionValue).
struct ItemOptionValue
{
    std::vector<int> values;
    // Written as a list, also with one entry; else values holds the constant.
    bool isList = false;
    std::vector<ItemOptionTerm> per;

    // The value at `level` without its terms: the constant, or the list's entry (0 below level 1, the last entry above
    // the list).
    int At(int level) const
    {
        if (values.empty() || (isList && level < 1))
        {
            return 0;
        }
        if (!isList)
        {
            return values.front();
        }
        const size_t index = static_cast<size_t>(level - 1);
        return index < values.size() ? values[index] : values.back();
    }

    bool operator==(const ItemOptionValue& other) const = default;
};

struct ItemOption
{
    // OpenMU's ItemOption.Number; 0 for the types with one option.
    int number = 0;
    // The attribute it raises, OpenMU's Stats name in camelCase ("criticalDamageChance").
    std::string attribute;
    ItemOptionAdd add = ItemOptionAdd::Add;
    ItemOptionValue value;
    // The names of its tooltip lines in Data/Items/Tooltips/OptionTexts.json, in their order.
    std::vector<std::string> texts;

    bool operator==(const ItemOption& other) const = default;
};

// OpenMU's labels of the same definition, which the exchange matches by: its GUID where OpenMU's is fixed, and its
// English name, by which OpenMU's code looks it up. The game doesn't read them.
struct ItemOptionOpenMuLabels
{
    std::string id;
    std::string name;

    bool IsEmpty() const
    {
        return id.empty() && name.empty();
    }

    bool operator==(const ItemOptionOpenMuLabels& other) const = default;
};

// OpenMU's values the server alone reads: whether and how often the definition adds its options to a dropped item.
// The game doesn't read them; the exchange carries them.
struct ItemOptionServerValues
{
    bool addsRandomly = false;
    // 0 to 1.
    double addChance = 0.0;
    int maximumOptionsPerItem = 0;

    bool operator==(const ItemOptionServerValues& other) const = default;
};

struct ItemOptionDefinition
{
    // Unique across all option files; what items link to.
    std::string name;
    ItemOptionType type = ItemOptionType::Luck;
    LocalizedString displayName;
    ItemOptionOpenMuLabels openMu;
    std::optional<ItemOptionServerValues> server;
    std::vector<ItemOption> options;
    // The file of Data/Items/Options it is read from and written to ("Luck.json"); not in the file itself.
    std::string file;

    bool operator==(const ItemOptionDefinition& other) const = default;
};

// The texts of the options' tooltip lines (Data/Items/Tooltips/OptionTexts.json), by name; {value} is the option's
// value.
struct ItemOptionTexts
{
    std::map<std::string, LocalizedString, std::less<>> texts;

    const LocalizedString* Find(std::string_view name) const
    {
        const auto found = texts.find(name);
        return found != texts.end() ? &found->second : nullptr;
    }

    bool operator==(const ItemOptionTexts& other) const = default;
};
} // namespace Data::Items
