#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemOptionDatabase.h"

#include <array>
#include <string>
#include <string_view>

// The option controls Item Tooltip's preview offers for an item, and the option bytes the preview gives the item from
// them. A setting whose control the item does not show stays out of the preview, so a value set for another item does
// not change this one's tooltip.
namespace MuEditor::Items
{
// The option byte's option type as a server can send it.
constexpr int MaxPreviewOptionType = 15;

// What the item's code reads from the excellent byte: its excellent options, the wings' options, or a kind its name
// lines name (the Horn of Fenrir's).
enum class ExcellentByteUse
{
    Excellent,
    WingOptions,
    Kinds,
};

struct PreviewOptionControls
{
    // The item's first additional (option) and excellent definitions, or nullptr.
    const Data::Items::ItemOptionDefinition* additional = nullptr;
    const Data::Items::ItemOptionDefinition* excellent = nullptr;
    // The highest option type the additional definition numbers its options by (wings); 0 when it numbers none.
    int highestType = 0;
    // The excellent bits that have a checkbox: the numbers of the excellent definition's options; without one, the
    // bits the item's name lines name as a value of the byte ("excellentFlags=4", kindLines), else all six on wings,
    // whose own code reads the byte, else none.
    BYTE excellentBits = 0;
    ExcellentByteUse byteUse = ExcellentByteUse::Excellent;
    // By the bit's number - 1, the first name line shown when the byte is that one bit: what the kind is called.
    std::array<const Data::Items::ItemNameLine*, Data::Items::MaxExcellentOptionNumber> kindLines{};
};

PreviewOptionControls FindPreviewOptionControls(const Data::Items::ItemDefinition& item,
                                                const Data::Items::ItemOptionDatabase& options);

// The option bytes the preview gives the item: the settings its controls show.
struct PreviewOptionBytes
{
    int optionLevel = 0;
    int optionType = 0;
    BYTE excellentFlags = 0;
};

PreviewOptionBytes ResolvePreviewOptions(const PreviewOptionControls& controls, int optionLevel, int optionType,
                                         BYTE excellentFlags);

// The attribute of the excellent definition's option with the bit's number; empty when there is none.
std::string_view DescribeExcellentBit(const Data::Items::ItemOptionDefinition* excellent, int number);

// The label of the kind checkbox with the bit's number (ExcellentByteUse::Kinds): its name line's text in `locale`
// (else English) without its placeholders ("{name} +Destroy" -> "+Destroy"), or the number when nothing is left.
std::string DescribeKind(const PreviewOptionControls& controls, int number, std::string_view locale);
} // namespace MuEditor::Items

#endif // _EDITOR
