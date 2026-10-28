#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ItemTooltipText.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "Engine/Object/ZzzInfomation.h"

#include <cwchar>
#include <filesystem>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

namespace TooltipText = Engine::Items::TooltipText;
using Bindings = std::vector<std::pair<std::string, std::string>>;

// The text compiled (in English) and written for the item at `level`, into a row of `capacity` characters. The texts
// here aren't the item data's, so each one is compiled anew: a compiled text is found by where its text is.
std::wstring Fill(const std::string& text, const ItemTooltipValues& values, int level = 0, const Bindings& bound = {},
                  const Data::Items::ItemDefinition* definition = nullptr, size_t capacity = 100)
{
    TooltipText::Clear();
    std::vector<wchar_t> row(capacity, L'?');
    const size_t length =
        TooltipText::Write(TooltipText::Compile(text, bound, definition, "en"), level, values, row.data(), capacity);
    CHECK(length < capacity);
    CHECK(row[length] == L'\0');
    return std::wstring(row.data(), length);
}
} // namespace

TEST_CASE("A tooltip text fills its placeholders once compiled [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipText::Clear();

    ItemTooltipValues values;
    values.defense = 42;
    values.strength.missing = 7;
    values.name = L"Kris";

    // Built-in numbers and texts, {level}, and a placeholder bound by the line's "values".
    CHECK(Fill("Defense: {defense}", values) == L"Defense: 42");
    CHECK(Fill("{name} +{level}", values, 9) == L"Kris +9");
    CHECK(Fill("(lacking {missing})", values, 0, {{"missing", "missingStrength"}}) == L"(lacking 7)");
    CHECK(Fill("No placeholders", values) == L"No placeholders");
    CHECK(Fill("", values).empty());

    // A table of the item stat file, at the item's level, as the data's own lookup gives it.
    const Data::Items::ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    for (const int level : {0, 7, 15})
    {
        const std::optional<int> bonus = Data::Items::GetPlaceholderValue("table:capeDamageIncrease", level, profiles);
        REQUIRE(bonus.has_value());
        CHECK(Fill("+{table:capeDamageIncrease}%", values, level) == L"+" + std::to_wstring(*bonus) + L"%");
    }

    // A placeholder that names nothing, or a brace without its pair, leaves the rest as written.
    CHECK(Fill("{defense} then {sparkle} {defense}", values) == L"42 then {sparkle} {defense}");
    CHECK(Fill("{defense} {oops", values) == L"42 {oops");
    CHECK(Fill("{defense} oops}", values) == L"42 oops}");
}

TEST_CASE("A tooltip text's level value follows the item's level [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipText::Clear();

    // The Fruits' {stat}: ENG at +0, STA at +1; past its levels, the text from the placeholder on as written.
    const Data::Items::ItemDefinition* fruits = g_ItemDatabase.Find(Data::Items::MakeItemType(13, 15));
    REQUIRE(fruits != nullptr);
    REQUIRE(fruits->FindLevelValues("stat") != nullptr);
    const ItemTooltipValues values;
    CHECK(Fill("Adds {stat}", values, 0, {}, fruits) == L"Adds ENG");
    CHECK(Fill("Adds {stat}", values, 1, {}, fruits) == L"Adds STA");
    CHECK(Fill("Adds {stat}", values, 99, {}, fruits) == L"Adds {stat}");
}

TEST_CASE("A tooltip text stops at the row's end [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipText::Clear();

    ItemTooltipValues values;
    values.defense = 123456;
    // Six characters fit a row of seven, with its terminating 0: the text, then the number, cut at the end.
    CHECK(Fill("Def {defense}", values, 0, {}, nullptr, 7) == L"Def 12");
    CHECK(Fill("Defense", values, 0, {}, nullptr, 7) == L"Defens");
    CHECK(Fill("{defense}", values, 0, {}, nullptr, 1).empty());
}

TEST_CASE("A tooltip text is compiled again when the item data changes [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipText::Clear();

    const std::string text = "{table:capeDamageIncrease}";
    const TooltipText::Compiled* first = &TooltipText::Compile(text, {}, nullptr, "en");
    // The same text, bindings, item and language: the compiled text kept.
    CHECK(&TooltipText::Compile(text, {}, nullptr, "en") == first);
    // Another language is another entry.
    CHECK(&TooltipText::Compile(text, {}, nullptr, "de") != first);

    // A new stat file is a version change: its tables are looked up again.
    const int version = g_ItemDatabase.GetVersion();
    g_ItemDatabase.SetStatProfiles(g_ItemDatabase.GetStatProfiles());
    CHECK(g_ItemDatabase.GetVersion() != version);
    ItemTooltipValues values;
    const std::optional<int> bonus =
        Data::Items::GetPlaceholderValue("table:capeDamageIncrease", 5, g_ItemDatabase.GetStatProfiles());
    REQUIRE(bonus.has_value());
    CHECK(Fill(text, values, 5) == std::to_wstring(*bonus));

    // So is a new tooltip file.
    const int before = g_ItemDatabase.GetVersion();
    g_ItemDatabase.SetTooltipFile(g_ItemDatabase.GetTooltipFile());
    CHECK(g_ItemDatabase.GetVersion() != before);
}

TEST_CASE("A tooltip text's skill name is looked up when the text is written [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipText::Clear();

    // Compiled while the skill list isn't there: written then, the rest of the line stays as written.
    SKILL_ATTRIBUTE* const previous = SkillAttribute;
    SkillAttribute = nullptr;
    // A skill's name is a line's value, as the wings' skill lines bind it.
    const std::string text = "Skill: {skill} (wings)";
    const Bindings bound = {{"skill", "skillName:30"}};
    const TooltipText::Compiled& compiled = TooltipText::Compile(text, bound, nullptr, "en");
    const ItemTooltipValues values;
    wchar_t row[100];
    CHECK(std::wstring(row, TooltipText::Write(compiled, 0, values, row, std::size(row))) == L"Skill: {skill} (wings)");

    // Once the skill list is there, the same compiled text writes the skill's name.
    std::vector<SKILL_ATTRIBUTE> skills(MAX_SKILLS);
    std::wcscpy(skills[30].Name, L"Twister");
    SkillAttribute = skills.data();
    CHECK(std::wstring(row, TooltipText::Write(compiled, 0, values, row, std::size(row))) == L"Skill: Twister (wings)");
    SkillAttribute = previous;
}
