#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/ItemTools/ItemPreviewOptions.h"

#include <string>
#include <vector>

using namespace MuEditor::Items;
using namespace Data::Items;

namespace
{
ItemOptionDefinition MakeDefinition(std::string name, ItemOptionType type, std::vector<int> numbers)
{
    ItemOptionDefinition definition{.name = std::move(name), .type = type, .file = "Option.json"};
    for (const int number : numbers)
    {
        ItemOption option{.number = number, .attribute = "attribute" + std::to_string(number)};
        option.value.values = {1};
        definition.options.push_back(std::move(option));
    }
    return definition;
}

ItemOptionFiles MakeFiles()
{
    ItemOptionFiles files;
    files.definitions.push_back(MakeDefinition("physicalDamageOption", ItemOptionType::Option, {0}));
    files.definitions.push_back(MakeDefinition("wingOption", ItemOptionType::Option, {1, 2, 3}));
    files.definitions.push_back(MakeDefinition("excellentWeapon", ItemOptionType::Excellent, {1, 2, 3, 4, 5, 6}));
    files.definitions.push_back(MakeDefinition("excellentPartial", ItemOptionType::Excellent, {2, 5}));
    return files;
}

ItemDefinition MakeItem(std::vector<std::string> options)
{
    ItemDefinition item;
    item.options = std::move(options);
    return item;
}
} // namespace

TEST_CASE("Item Tooltip's preview takes only the option settings the item has controls for "
          "[data][items][editor][options]")
{
    const ItemOptionDatabase options(MakeFiles());

    SUBCASE("a Kris: the option level and its excellent bits, no option type")
    {
        const PreviewOptionControls controls =
            FindPreviewOptionControls(MakeItem({"physicalDamageOption", "excellentWeapon"}), options);
        REQUIRE(controls.additional != nullptr);
        CHECK(controls.highestType == 0);
        CHECK(controls.excellentBits == AllExcellentOptionFlags);
        // A type set on wings before stays out.
        const PreviewOptionBytes bytes = ResolvePreviewOptions(controls, 7, 2, 0x05);
        CHECK(bytes.optionLevel == 7);
        CHECK(bytes.optionType == 0);
        CHECK(bytes.excellentFlags == 0x05);
        CHECK(DescribeExcellentBit(controls.excellent, 3) == "attribute3");
    }

    SUBCASE("an excellent definition with some numbers: only their bits")
    {
        const PreviewOptionControls controls = FindPreviewOptionControls(MakeItem({"excellentPartial"}), options);
        CHECK(controls.additional == nullptr);
        CHECK(controls.excellentBits == 0x12);
        const PreviewOptionBytes bytes = ResolvePreviewOptions(controls, 7, 0, 0x3F);
        CHECK(bytes.optionLevel == 0);
        CHECK(bytes.excellentFlags == 0x12);
        CHECK(DescribeExcellentBit(controls.excellent, 1).empty());
    }

    SUBCASE("wings: the option type up to the definition's highest number, and the six excellent bits")
    {
        ItemDefinition wings = MakeItem({"wingOption"});
        wings.slot = ItemSlot::Wings;
        const PreviewOptionControls controls = FindPreviewOptionControls(wings, options);
        CHECK(controls.highestType == 3);
        CHECK(controls.excellentBits == AllExcellentOptionFlags);
        CHECK(controls.byteUse == ExcellentByteUse::WingOptions);
        const PreviewOptionBytes bytes = ResolvePreviewOptions(controls, 4, 9, 0x01);
        CHECK(bytes.optionLevel == 4);
        CHECK(bytes.optionType == 3);
        CHECK(bytes.excellentFlags == 0x01);
        CHECK(DescribeExcellentBit(controls.excellent, 1).empty());
    }

    SUBCASE("the kinds an item's name lines name as the byte (the Fenrir's), not six excellent options")
    {
        ItemDefinition horn = MakeItem({});
        horn.tags.Set(ItemTag::HornMount);
        for (const char* when :
             {"excellentFlags=1", "excellentFlags=4", "excellentFlags=3", "excellentFlags!=2", "excellentFlags=1"})
        {
            ItemNameLine line;
            line.text = Data::LocalizedString::Parse(std::string("{name} ") + when);
            TooltipCondition condition;
            std::string error;
            REQUIRE(ReadTooltipCondition(when, condition, error));
            line.when.push_back(condition);
            horn.nameLines.push_back(std::move(line));
        }
        const PreviewOptionControls controls = FindPreviewOptionControls(horn, options);
        // Only whole-byte values of one bit; the first line of each.
        CHECK(controls.excellentBits == 0x05);
        CHECK(controls.byteUse == ExcellentByteUse::Kinds);
        REQUIRE(controls.kindLines[0] == &horn.nameLines[0]);
        CHECK(controls.kindLines[2] == &horn.nameLines[1]);
        CHECK(controls.kindLines[1] == nullptr);
        CHECK(ResolvePreviewOptions(controls, 0, 0, 0x3F).excellentFlags == 0x05);
    }

    SUBCASE("a kind is labelled by its name line without the placeholders, else by its number")
    {
        ItemDefinition horn = MakeItem({});
        const auto addLine = [&horn](const char* text, const char* when)
        {
            ItemNameLine line;
            line.text = Data::LocalizedString::Parse(text);
            TooltipCondition condition;
            std::string error;
            REQUIRE(ReadTooltipCondition(when, condition, error));
            line.when.push_back(condition);
            horn.nameLines.push_back(std::move(line));
        };
        addLine("{name} +Destroy||de={name} +Zerstören", "excellentFlags=1");
        addLine("{name} +Protect", "excellentFlags=2");
        addLine("{name}", "excellentFlags=4");
        addLine("{name} kind {level}", "excellentFlags=8");
        const PreviewOptionControls controls = FindPreviewOptionControls(horn, options);
        REQUIRE(controls.byteUse == ExcellentByteUse::Kinds);
        CHECK(DescribeKind(controls, 1, "en") == "+Destroy");
        CHECK(DescribeKind(controls, 1, "de") == "+Zerstören");
        // A language without a translation shows the English text.
        CHECK(DescribeKind(controls, 2, "de") == "+Protect");
        // Nothing left of the text, or no line: the number.
        CHECK(DescribeKind(controls, 3, "en") == "3");
        CHECK(DescribeKind(controls, 4, "en") == "kind");
        CHECK(DescribeKind(controls, 5, "en") == "5");
    }

    SUBCASE("a horn whose name lines name no kind (Uniria, Dinorant): no excellent controls")
    {
        ItemDefinition horn = MakeItem({});
        horn.tags.Set(ItemTag::HornMount);
        const PreviewOptionControls controls = FindPreviewOptionControls(horn, options);
        CHECK(controls.excellentBits == 0);
        CHECK(controls.byteUse == ExcellentByteUse::Excellent);
    }

    SUBCASE("a potion: no option controls, so nothing of them")
    {
        const PreviewOptionControls controls = FindPreviewOptionControls(MakeItem({}), options);
        CHECK(controls.additional == nullptr);
        CHECK(controls.excellentBits == 0);
        const PreviewOptionBytes bytes = ResolvePreviewOptions(controls, 7, 2, 0x3F);
        CHECK(bytes.optionLevel == 0);
        CHECK(bytes.optionType == 0);
        CHECK(bytes.excellentFlags == 0);
    }
}

#endif // _EDITOR
