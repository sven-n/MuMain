#pragma once

#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemTagSet.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// The warp list of the M window (Data/Maps/Warps.json): the warps a player
// can pick, in the order the window shows them, see docs/warp-list.md.
namespace Data::Maps
{
// The longest name the game holds (MOVEREQINFO's names): longer ones are cut,
// counted in UTF-16 units.
constexpr size_t MaxWarpNameLength = 31;

// An item a warp rule names: one item, the items of a tag (without those of
// another tag), or any wings.
struct WarpItemMatch
{
    enum class Kind : uint8_t
    {
        // The item `group`, `number`.
        Item,
        // The items with `tag` ("flying", as the item files spell the tags),
        // but not those with `exceptTag` (none when empty).
        Tag,
        // Every item of the wing slot.
        Wings,
    };

    Kind kind = Kind::Item;
    int group = 0;
    int number = 0;
    std::string tag;
    std::string exceptTag;
    // The tags as values, set by ResolveItemTags (WarpsJson.h) when the list is
    // read or the game takes it; none for an unknown tag, which matches no item.
    std::optional<Items::ItemTag> tagValue;
    std::optional<Items::ItemTag> exceptTagValue;

    // The values follow the names, so only the names count.
    bool operator==(const WarpItemMatch& other) const
    {
        return kind == other.kind && group == other.group && number == other.number && tag == other.tag &&
               exceptTag == other.exceptTag;
    }
};

// A class whose characters need less level for the warps: the level times
// numerator / denominator, cut to a whole number.
struct ClassLevelDiscount
{
    // The class of the first stage, as CLASS_TYPE counts them (0 Dark Wizard,
    // 3 Magic Gladiator, 4 Dark Lord, 6 Rage Fighter).
    int playerClass = 0;
    int numerator = 1;
    int denominator = 1;

    bool operator==(const ClassLevelDiscount&) const = default;
};

// One warp of the list.
struct Warp
{
    // The warp's number, which the client sends to the server to warp there.
    int index = 0;
    // The name the M window shows, per language; English is required, a
    // language without its own falls back to it.
    LocalizedString names;
    // The name /move and the warp chat command take besides the shown name.
    std::string command;
    // The character level it needs (the M window lowers it for some classes).
    int level = 0;
    // What it costs.
    int zen = 0;
    // The server's landing spot; the client does not use it.
    int gate = 0;
    // The level stays as it is for the classes of the class level discount.
    bool noClassDiscount = false;
    // The master level and resets it needs; 0 for none.
    int minMasterLevel = 0;
    int minResets = 0;
    // Closed on non-PvP servers.
    bool pvpServersOnly = false;
    // On PvP servers only for Gens members (the M window marks it).
    bool gensOnly = false;
    // At least one of these items has to be equipped, not broken; none: no rule.
    std::vector<WarpItemMatch> needsOneOf;
    // None of these items may be equipped.
    std::vector<WarpItemMatch> notWith;
    // The game text shown when the equipment rules block the warp (the key of
    // a warp message, see WarpMessages.h); empty for the general one.
    std::string blockedMessage;

    bool operator==(const Warp&) const = default;
};

// The whole file.
struct WarpList
{
    // The classes that need less level, for every warp.
    std::vector<ClassLevelDiscount> classLevelDiscounts;
    // In the order the M window shows them.
    std::vector<Warp> warps;

    bool operator==(const WarpList&) const = default;
};
} // namespace Data::Maps
