#include "stdafx.h"

#ifdef _EDITOR

#include "ItemPreviewOptions.h"

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"

#include <algorithm>

namespace MuEditor::Items
{
namespace
{
// The bits the item's name lines name as the whole byte ("excellentFlags=4": one bit), with their first line.
void FindKindLines(const Data::Items::ItemDefinition& item, PreviewOptionControls& controls)
{
    using Data::Items::TooltipCondition;
    for (const Data::Items::ItemNameLine& line : item.nameLines)
    {
        for (const TooltipCondition& condition : line.when)
        {
            if (condition.kind != TooltipCondition::Kind::Value ||
                condition.value != Data::Items::TooltipValue::ExcellentFlags || condition.negated ||
                condition.comparison != TooltipCondition::Comparison::Equal)
            {
                continue;
            }
            for (int number = 1; number <= Data::Items::MaxExcellentOptionNumber; ++number)
            {
                const BYTE bit = static_cast<BYTE>(1 << (number - 1));
                if (condition.number == bit && controls.kindLines[number - 1] == nullptr)
                {
                    controls.kindLines[number - 1] = &line;
                    controls.excellentBits |= bit;
                }
            }
        }
    }
}
} // namespace

PreviewOptionControls FindPreviewOptionControls(const Data::Items::ItemDefinition& item,
                                                const Data::Items::ItemOptionDatabase& options)
{
    PreviewOptionControls controls;
    for (const Data::Items::ItemOptionIndex index : options.ResolveLinks(item))
    {
        const Data::Items::ItemOptionDefinition& definition = options.Get(index);
        if (definition.type == Data::Items::ItemOptionType::Option && controls.additional == nullptr)
        {
            controls.additional = &definition;
        }
        else if ((definition.type == Data::Items::ItemOptionType::Excellent ||
                  definition.type == Data::Items::ItemOptionType::Wing) &&
                 controls.excellent == nullptr)
        {
            // A wing definition's options are the wing options of the excellent byte (second and third tier wings,
            // capes); of an item that links both types ResolveLinks keeps the one the item's byte holds.
            controls.excellent = &definition;
            if (definition.type == Data::Items::ItemOptionType::Wing)
            {
                controls.byteUse = ExcellentByteUse::WingOptions;
            }
        }
    }
    if (controls.additional != nullptr)
    {
        for (const Data::Items::ItemOption& option : controls.additional->options)
        {
            controls.highestType = std::max(controls.highestType, std::min(option.number, MaxPreviewOptionType));
        }
    }
    if (controls.excellent != nullptr)
    {
        for (const Data::Items::ItemOption& option : controls.excellent->options)
        {
            if (option.number >= 1 && option.number <= Data::Items::MaxExcellentOptionNumber)
            {
                controls.excellentBits |= static_cast<BYTE>(1 << (option.number - 1));
            }
        }
    }
    else
    {
        FindKindLines(item, controls);
        if (controls.excellentBits != 0)
        {
            controls.byteUse = ExcellentByteUse::Kinds;
        }
    }
    return controls;
}

PreviewOptionBytes ResolvePreviewOptions(const PreviewOptionControls& controls, int optionLevel, int optionType,
                                         BYTE excellentFlags)
{
    PreviewOptionBytes bytes;
    bytes.optionLevel = controls.additional != nullptr ? optionLevel : 0;
    bytes.optionType = std::min(optionType, controls.highestType);
    bytes.excellentFlags = static_cast<BYTE>(excellentFlags & controls.excellentBits);
    return bytes;
}

std::string_view DescribeExcellentBit(const Data::Items::ItemOptionDefinition* excellent, int number)
{
    if (excellent == nullptr)
    {
        return {};
    }
    const auto option = std::ranges::find(excellent->options, number, &Data::Items::ItemOption::number);
    return option != excellent->options.end() ? std::string_view(option->attribute) : std::string_view();
}

std::string DescribeKind(const PreviewOptionControls& controls, int number, std::string_view locale)
{
    const Data::Items::ItemNameLine* line =
        number >= 1 && number <= Data::Items::MaxExcellentOptionNumber ? controls.kindLines[number - 1] : nullptr;
    std::string text = line != nullptr ? line->text.Get(locale) : std::string();
    const Data::Items::PlaceholderSpans placeholders = Data::Items::FindPlaceholders(text);
    for (auto span = placeholders.spans.rbegin(); span != placeholders.spans.rend(); ++span)
    {
        text.erase(span->begin, span->end - span->begin);
    }
    const size_t first = text.find_first_not_of(' ');
    if (first == std::string::npos)
    {
        return std::to_string(number);
    }
    return text.substr(first, text.find_last_not_of(' ') - first + 1);
}
} // namespace MuEditor::Items

#endif // _EDITOR
