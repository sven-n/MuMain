#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Engine/Object/ItemTooltipText.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "I18N/All.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Loads the shipped item data for the test and leaves "Translate item names" on, as the game starts.
class ItemTextScope
{
public:
    ItemTextScope() : m_client(ClientDirectory)
    {
        std::string errorMessage;
        m_loaded = g_ItemDataHandler.Load(errorMessage);
    }

    ~ItemTextScope()
    {
        g_ItemDataHandler.SetTranslateItemNames(true);
    }

    bool Loaded() const
    {
        return m_loaded;
    }

private:
    ClientDataScope m_client;
    bool m_loaded = false;
};

int observedChanges = 0;

void CountChange(void*) noexcept
{
    ++observedChanges;
}

std::wstring Tooltip(const std::string& text, std::string_view language, std::string_view textLanguage = {})
{
    Engine::Items::TooltipText::Clear();
    wchar_t row[100]{};
    const ItemTooltipValues values;
    Engine::Items::TooltipText::Write(Engine::Items::TooltipText::Compile(text, {}, nullptr, language, textLanguage), 0,
                                      values, row, std::size(row));
    return row;
}
} // namespace

TEST_CASE("The texts that name items are listed with their group and key [data][texts]")
{
    const auto texts = I18N::GetItemTexts();
    const auto has = [&](std::string_view group, std::string_view key)
    {
        return std::any_of(texts.begin(), texts.end(),
                           [&](const I18N::ItemText& text) { return group == text.group && key == text.key; });
    };
    CHECK(has("Game", "Jewel of Bless"));
    CHECK(has("Game", "Jewel of Soul"));
    CHECK(has("Dialog", "Answer_65_Slot_0"));
    CHECK_FALSE(has("Game", "Jewel of Bless and Jewel of Soul"));
}

TEST_CASE("The jewel window names the jewels as the item data does, translated or not [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());

    // With translated item names on, the names read as the texts spelled them before, except where the item data
    // names the item otherwise: "Jewel stone", "Juwelenstein" and "Kamień klejnotu" are "Gemstone", "Edelstein" and
    // "Kamień szlachetny", and the English refining stones lost their capitals.
    struct Jewel
    {
        const wchar_t* const* text;
        std::wstring_view english;
        std::wstring_view german;
        std::wstring_view polish;
    };
    const Jewel jewels[] = {
        {&I18N::Game::JewelOfBless, L"Jewel of Bless", L"Juwel des Segens", L"Klejnot Błogosławieństwa"},
        {&I18N::Game::JewelOfSoul, L"Jewel of Soul", L"Juwel der Seele", L"Klejnot Duszy"},
        {&I18N::Game::JewelOfLife, L"Jewel of Life", L"Juwel des Lebens", L"Klejnot Życia"},
        {&I18N::Game::JewelOfCreation, L"Jewel of Creation", L"Juwel der Schöpfung", L"Klejnot Stworzenia"},
        {&I18N::Game::JewelOfGuardian, L"Jewel of Guardian", L"Juwel des Wächters", L"Klejnot Strażnika"},
        {&I18N::Game::JewelStone, L"Gemstone", L"Edelstein", L"Kamień szlachetny"},
        {&I18N::Game::JewelOfHarmony, L"Jewel of Harmony", L"Juwel der Harmonie", L"Klejnot Harmonii"},
        {&I18N::Game::JewelOfChaos, L"Jewel of Chaos", L"Juwel des Chaos", L"Klejnot Chaosu"},
        {&I18N::Game::LowerRefiningStone, L"Lower refining stone", L"Niederer Verfeinerungsstein",
         L"Niższy kamień rafinacji"},
        {&I18N::Game::HigherRefiningStone, L"Higher refining stone", L"Höherer Verfeinerungsstein",
         L"Wyższy kamień rafinacji"},
    };

    I18N::SetLocale("en");
    for (const Jewel& jewel : jewels)
    {
        CHECK(std::wstring_view(*jewel.text) == jewel.english);
    }
    I18N::SetLocale("de");
    for (const Jewel& jewel : jewels)
    {
        CHECK(std::wstring_view(*jewel.text) == jewel.german);
    }
    I18N::SetLocale("pl");
    for (const Jewel& jewel : jewels)
    {
        CHECK(std::wstring_view(*jewel.text) == jewel.polish);
    }
    g_ItemDataHandler.SetTranslateItemNames(false);
    for (const Jewel& jewel : jewels)
    {
        CHECK(std::wstring_view(*jewel.text) == jewel.english);
    }
}

TEST_CASE("A text naming an item reads the name in the UI language, or English while translated item names are off "
          "[data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());

    I18N::SetLocale("en");
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Jewel of Bless");
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L" I will go find the 'Tear of Elf'!");

    // As written before: the German texts spelled the German item names.
    I18N::SetLocale("de");
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Juwel des Segens");
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L" Ich werde die 'Träne der Elfe' suchen!");

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Jewel of Bless");
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L" Ich werde die 'Tear of Elf' suchen!");

    g_ItemDataHandler.SetTranslateItemNames(true);
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Juwel des Segens");
}

TEST_CASE("A Polish text names the item in the case it asks for [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());

    // {item:14,25:acc}: the item data's accusative form, as the text spelled it before.
    I18N::SetLocale("pl");
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Łzę Elfa'!");

    // English has no cases: the name itself.
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Tear of Elf'!");
}

TEST_CASE("Once a test's item data is gone, the texts that name items are as written again [data][texts]")
{
    {
        ItemTextScope scope;
        REQUIRE(scope.Loaded());
        I18N::SetLocale("de");
        CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Juwel des Segens");
    }
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"{item:14,13}");
}

TEST_CASE("Turning translated item names off tells the locale observers once [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");
    I18N::RegisterLocaleObserver(&CountChange, nullptr);
    observedChanges = 0;

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(observedChanges == 1);
    // Nothing changes when it is set to what it is.
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(observedChanges == 1);

    I18N::UnregisterLocaleObserver(&CountChange, nullptr);
}

TEST_CASE("A language without a text of its own names the items as the English text does [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());

    // Dialog has no Russian file: the English answer, with the English name.
    I18N::SetLocale("ru");
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L" I will go find the 'Tear of Elf'!");
}

TEST_CASE("An earlier expansion of a text stays readable after the names change [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");
    const wchar_t* german = I18N::Game::JewelOfBless;

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(I18N::Game::JewelOfBless != german);
    CHECK(std::wstring_view(german) == L"Juwel des Segens");

    // The same expansion again is the same text.
    g_ItemDataHandler.SetTranslateItemNames(true);
    CHECK(I18N::Game::JewelOfBless == german);
}

TEST_CASE("A tooltip text names an item in the item name language and the rest in the UI language [data][items][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");

    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de") == L"Combine Goldschlüssel at level 0");
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de") == L"Combine Gold Key at level 0");
    // No item 14,511: the text stays as written from the reference on.
    CHECK(Tooltip("Combine {item:14,511} now", "en") == L"Combine {item:14,511} now");
}

TEST_CASE("A tooltip line without a translation of its own names the items in English, as the UI texts do "
          "[data][items][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");

    // The English text shown for German: English names, also with translated item names on.
    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de", "en") == L"Combine Gold Key at level 0");
    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de", "de") == L"Combine Goldschlüssel at level 0");
}
