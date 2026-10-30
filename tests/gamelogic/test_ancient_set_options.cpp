#include "stdafx.h"
#include "doctest.h"

#include "GameLogic/Items/CSItemOption.h"

#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"

#include <cstdint>
#include <cwchar>
#include <initializer_list>
#include <string>
#include <vector>

namespace
{
constexpr std::uint8_t NoOption = 0xFF;

// A set with every option slot empty, the way the set data marks an empty slot.
ITEM_SET_OPTION EmptySet()
{
    ITEM_SET_OPTION set{};
    std::wcscpy(set.strSetName, L"Test");
    set.bySetItemCount = 4;
    set.byOptionCount = 4;
    for (auto& options : set.byStandardOption)
    {
        options.fill(NoOption);
    }
    set.byExtOption.fill(NoOption);
    set.byFullOption.fill(NoOption);
    return set;
}

// Sets with options, one per name, for the /<set name> lookup.
std::vector<ITEM_SET_OPTION> NamedSets(std::initializer_list<const wchar_t*> names)
{
    std::vector<ITEM_SET_OPTION> sets;
    for (const wchar_t* name : names)
    {
        ITEM_SET_OPTION set = EmptySet();
        std::wcscpy(set.strSetName, name);
        sets.push_back(set);
    }
    return sets;
}

// The name of the set `typed` finds among `sets`, "-" for none.
std::wstring FoundSet(const wchar_t* typed, const std::vector<ITEM_SET_OPTION>& sets)
{
    const int index = CSItemOption::FindSetByName(typed, sets);
    return index < 0 ? std::wstring(L"-") : std::wstring(sets[index].strSetName);
}

// SKILL_ATTRIBUTE::MasteryType of the skills of an element.
constexpr std::uint8_t IceMastery = 0;
constexpr std::uint8_t PoisonMastery = 1;
constexpr std::uint8_t LightningMastery = 2;
constexpr std::uint8_t WindMastery = 5;
constexpr std::uint8_t WaterMastery = 6;

// The option list `setOptions` gives a wearer of base class `firstClass` with `itemCount` of the
// set's four items.
SET_SEARCH_RESULT_OPT WornSet(const ITEM_SET_OPTION& setOptions, std::uint8_t itemCount, int firstClass,
                              int secondClass)
{
    SET_SEARCH_RESULT_OPT worn{};
    worn.CompleteSetItemCount = 4;
    worn.ItemCount = itemCount;
    CSItemOption::AddSetOptions(worn, setOptions, firstClass, secondClass);
    return worn;
}

// The ice skill bonus `setOptions` gives a wearer of base class `firstClass` with all four items.
int IceSkillBonus(const ITEM_SET_OPTION& setOptions, int firstClass, int secondClass)
{
    return CSItemOption::GetMasteryBonus(WornSet(setOptions, 4, firstClass, secondClass), IceMastery);
}

// The game texts in English for the scope, then the language before it again.
class EnglishTextsScope
{
public:
    EnglishTextsScope() : m_previous(I18N::GetCurrentLocale())
    {
        I18N::SetLocale("en");
    }
    ~EnglishTextsScope()
    {
        I18N::SetLocale(m_previous.c_str());
    }
    EnglishTextsScope(const EnglishTextsScope&) = delete;
    EnglishTextsScope& operator=(const EnglishTextsScope&) = delete;

private:
    std::string m_previous;
};

struct TableLine
{
    std::wstring text;
    int color;
};

// The table's lines with text, leaving out its empty spacing lines.
std::vector<TableLine> TextLines(int lineCount)
{
    std::vector<TableLine> lines;
    for (int i = 0; i < lineCount; ++i)
    {
        if (TextList[i][0] != L'\0' && TextList[i][0] != L'\n')
        {
            lines.push_back({TextList[i], TextListColor[i]});
        }
    }
    return lines;
}
} // namespace

TEST_CASE("Set options 24 to 28 have no description, the others keep theirs [gamelogic][items][sets]")
{
    for (int option = 0; option < 24; ++option)
    {
        CAPTURE(option);
        CHECK(CSItemOption::GetSetOptionFormat(static_cast<std::uint8_t>(option)) != nullptr);
    }
    for (int option = 24; option <= 28; ++option)
    {
        CAPTURE(option);
        CHECK(CSItemOption::GetSetOptionFormat(static_cast<std::uint8_t>(option)) == nullptr);
    }

    CHECK(CSItemOption::GetSetOptionFormat(0) == I18N::Game::IncreaseStrengthD);
    CHECK(CSItemOption::GetSetOptionFormat(23) == I18N::Game::IncreaseDamageWhenUsingTwoHandedWeaponsD);
    CHECK(CSItemOption::GetSetOptionFormat(MASTERY_OPTION) == I18N::Game::IceAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(29) == I18N::Game::IceAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(30) == I18N::Game::PoisonAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(31) == I18N::Game::LightningAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(32) == I18N::Game::FireAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(33) == I18N::Game::EarthAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(34) == I18N::Game::WindAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(35) == I18N::Game::WaterAttributeSkillIncreaseDamageD);
    CHECK(CSItemOption::GetSetOptionFormat(36) == nullptr);
    CHECK(CSItemOption::GetSetOptionFormat(0xFF) == nullptr);
}

TEST_CASE("The /SetName table shows each set option with its own value [gamelogic][items][sets]")
{
    EnglishTextsScope english;

    ITEM_SET_OPTION set = EmptySet();
    set.byStandardOption[0][0] = 0; // Increase strength
    set.byStandardOptionValue[0][0] = 10;
    set.byStandardOption[1][0] = 24; // not supported: no line
    set.byStandardOptionValue[1][0] = 2;
    set.byExtOption[0] = 15; // Increase critical damage rate
    set.byExtOptionValue[0] = 5;
    set.byFullOption[0] = 17; // Increase excellent damage rate
    set.byFullOptionValue[0] = 7;
    set.byFullOption[1] = 3; // Increase stamina
    set.byFullOptionValue[1] = 25;

    const auto lines = TextLines(CSItemOption::BuildSetOptionTextList(set));

    REQUIRE(lines.size() == 5); // the set's name line, then one line per supported option
    CHECK(lines[1].text == L"Increase strength +10");
    CHECK(lines[1].color == TEXT_COLOR_BLUE);
    CHECK(lines[2].text == L"Increase critical damage rate 5%");
    CHECK(lines[2].color == TEXT_COLOR_GREEN);
    CHECK(lines[3].text == L"Increase excellent damage rate 7%");
    CHECK(lines[3].color == TEXT_COLOR_YELLOW);
    CHECK(lines[4].text == L"Increase stamina +25");
    CHECK(lines[4].color == TEXT_COLOR_YELLOW);
}

TEST_CASE("A mastery set option counts only for the class the set names [gamelogic][items][sets]")
{
    ITEM_SET_OPTION set = EmptySet();
    set.byFullOption[0] = MASTERY_OPTION; // Ice attribute skill increase damage
    set.byFullOptionValue[0] = 20;

    // A set that names no class gives it to nobody, as in the original client.
    CHECK(IceSkillBonus(set, CLASS_WIZARD, 0) == 0);

    set.byRequireClass[CLASS_WIZARD] = 1; // Dark Wizards
    CHECK(IceSkillBonus(set, CLASS_WIZARD, 0) == 20);
    CHECK(IceSkillBonus(set, CLASS_WIZARD, 1) == 20);
    CHECK(IceSkillBonus(set, CLASS_KNIGHT, 0) == 0);

    set.byRequireClass[CLASS_WIZARD] = 2; // Soul Masters only
    CHECK(IceSkillBonus(set, CLASS_WIZARD, 0) == 0);
    CHECK(IceSkillBonus(set, CLASS_WIZARD, 1) == 20);
}

TEST_CASE("/<set name> finds a set however its name is typed [gamelogic][items][sets]")
{
    const auto sets = NamedSets({L"Warrior's", L"Chrono\u2019s", L"Enis", L"Hyperion's"});

    CHECK(FoundSet(L"Warrior's", sets) == L"Warrior's");
    CHECK(FoundSet(L"Warrior", sets) == L"Warrior's");
    CHECK(FoundSet(L"warrior", sets) == L"Warrior's");
    CHECK(FoundSet(L"Warriors", sets) == L"Warrior's");
    CHECK(FoundSet(L"WARRIOR'S", sets) == L"Warrior's");
    CHECK(FoundSet(L"Warrior\u2019s", sets) == L"Warrior's");
    CHECK(FoundSet(L"Chrono's", sets) == L"Chrono\u2019s");
    CHECK(FoundSet(L"chronos", sets) == L"Chrono\u2019s");
    CHECK(FoundSet(L"Enis", sets) == L"Enis");

    CHECK(FoundSet(L"Warr", sets) == L"-");
    CHECK(FoundSet(L"Warrior's set", sets) == L"-");
    CHECK(FoundSet(L"Warriorss", sets) == L"-");
    CHECK(FoundSet(L"Hyon", sets) == L"-");
    CHECK(FoundSet(L"", sets) == L"-");
    CHECK(FoundSet(L"'s", sets) == L"-");

    auto unused = NamedSets({L"Warrior's"});
    unused[0].byOptionCount = 255; // the set data's mark for an unused set
    CHECK(FoundSet(L"Warrior", unused) == L"-");
}

TEST_CASE("/<set name> prefers the name typed as it is, then the first set [gamelogic][items][sets]")
{
    // "Ares's" and "Ares" both accept "aress"; "ares" and "Ares's" name one of them as it is.
    const auto sets = NamedSets({L"Ares's", L"Ares"});
    CHECK(FoundSet(L"aress", sets) == L"Ares's");
    CHECK(FoundSet(L"ares", sets) == L"Ares");
    CHECK(FoundSet(L"Ares's", sets) == L"Ares's");

    const auto reversed = NamedSets({L"Ares", L"Ares's"});
    CHECK(FoundSet(L"aress", reversed) == L"Ares");
    CHECK(FoundSet(L"Ares's", reversed) == L"Ares's");
}

TEST_CASE("A set's options leave out 24 to 28 [gamelogic][items][sets]")
{
    ITEM_SET_OPTION set = EmptySet();
    set.byRequireClass[CLASS_WIZARD] = 1;
    set.byStandardOption[0][0] = 0; // Increase strength
    set.byStandardOptionValue[0][0] = 10;
    set.byStandardOption[1][0] = 24;
    set.byStandardOptionValue[1][0] = 2;
    set.byStandardOption[2][0] = 28;
    set.byStandardOptionValue[2][0] = 3;
    set.byFullOption[0] = 26;
    set.byFullOptionValue[0] = 4;

    const SET_SEARCH_RESULT_OPT worn = WornSet(set, 4, CLASS_WIZARD, 0);

    REQUIRE(worn.SetOptionCount == 1);
    CHECK(worn.SetOption[0].OptionNumber == 0);
    CHECK(worn.SetOption[0].Value == 10);
    CHECK(worn.SetOption[0].IsActive);
}

TEST_CASE("Each mastery set option raises its own element's skills [gamelogic][items][sets]")
{
    ITEM_SET_OPTION set = EmptySet();
    set.byRequireClass[CLASS_WIZARD] = 1;
    set.byFullOption[0] = MASTERY_OPTION; // ice
    set.byFullOptionValue[0] = 5;
    set.byFullOption[1] = MASTERY_OPTION + 2; // lightning
    set.byFullOptionValue[1] = 7;
    set.byFullOption[2] = MASTERY_OPTION + 6; // water
    set.byFullOptionValue[2] = 9;

    const SET_SEARCH_RESULT_OPT complete = WornSet(set, 4, CLASS_WIZARD, 0);
    CHECK(CSItemOption::GetMasteryBonus(complete, IceMastery) == 5);
    CHECK(CSItemOption::GetMasteryBonus(complete, LightningMastery) == 7);
    CHECK(CSItemOption::GetMasteryBonus(complete, WaterMastery) == 9);
    CHECK(CSItemOption::GetMasteryBonus(complete, PoisonMastery) == 0);
    CHECK(CSItemOption::GetMasteryBonus(complete, WindMastery) == 0);

    // Full options need the complete set.
    const SET_SEARCH_RESULT_OPT threeOfFour = WornSet(set, 3, CLASS_WIZARD, 0);
    CHECK(CSItemOption::GetMasteryBonus(threeOfFour, IceMastery) == 0);
}
