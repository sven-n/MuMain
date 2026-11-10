#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "Core/Platform/ProcessLocale.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/Translation/ItemReferenceChecks.h"
#include "Data/Translation/ItemTextExpansion.h"
#include "Engine/Object/ItemTooltipText.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "GameLogic/Quests/CSQuest.h"
#include "I18N/All.h"

#ifdef _EDITOR
#include "TestImGuiContext.h"
#include "UI/ItemTools/ItemSelection.h"
#include "UI/ItemTools/ItemTextCheckArea.h"
#include "UI/ItemTranslations/ItemNameFormsPanel.h"
#include "imgui.h"
#endif

#include <algorithm>
#include <clocale>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

// An NPC dialog's page, text and answer lines as CSQuest copies them (SceneCore.cpp).
extern int g_iCurrentDialogScript;
extern int g_iNumLineMessageBoxCustom;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern int g_iNumAnswer;
extern wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

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
int observedLocaleChanges = 0;

void CountChange(void*) noexcept
{
    ++observedChanges;
}

void CountLocaleChange(void*) noexcept
{
    ++observedLocaleChanges;
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

TEST_CASE("Elpis's menu button names the Jewel of Harmony as the item data does, translated or not [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());

    // The button shares the jewel window's text, so it follows the item data too: Spanish and Portuguese show the
    // inventory's name, not the text's former "Joya de la armonía" and "Jóia da Harmonia".
    I18N::SetLocale("es");
    CHECK(std::wstring_view(I18N::Game::JewelOfHarmony) == L"Joya de la Armonía");
    I18N::SetLocale("pt");
    CHECK(std::wstring_view(I18N::Game::JewelOfHarmony) == L"Joia da harmonia");
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(std::wstring_view(I18N::Game::JewelOfHarmony) == L"Jewel of Harmony");
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

TEST_CASE("Turning translated item names off tells the texts-changed observers once, and no locale observer "
          "[data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");
    Data::Translation::ItemTexts::RegisterTextsChangedObserver(&CountChange, nullptr);
    I18N::RegisterLocaleObserver(&CountLocaleChange, nullptr);
    observedChanges = 0;
    observedLocaleChanges = 0;

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(observedChanges == 1);
    // Nothing changes when it is set to what it is.
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(observedChanges == 1);
    // The locale stayed: monster names, the map name and the other locale observers are left alone.
    CHECK(observedLocaleChanges == 0);

    I18N::UnregisterLocaleObserver(&CountLocaleChange, nullptr);
    Data::Translation::ItemTexts::UnregisterTextsChangedObserver(&CountChange, nullptr);
}

TEST_CASE("An open NPC dialog's copied answer follows Translate item names [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    // The game's process locale: macOS writes the answer line (mu_swprintf) through LC_CTYPE, and in the test's "C"
    // locale it leaves "Pójdę ..." unwritten.
    const std::string previousLocale = std::setlocale(LC_ALL, nullptr);
    Core::Platform::ApplyProcessLocale();
    I18N::SetLocale("pl");

    // The Tear of Elf quest's answer, "{item:14,25:acc}" in Polish, as the dialog shows it.
    constexpr int TearOfElfQuestPage = 65;
    g_csQuest.ShowDialogText(TearOfElfQuestPage);
    CHECK(std::wstring_view(g_lpszDialogAnswer[0][0]).find(L"Łzę Elfa") != std::wstring_view::npos);

    // CSQuest's texts-changed observer copies it again: the English name.
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(std::wstring_view(g_lpszDialogAnswer[0][0]).find(L"Tear of Elf") != std::wstring_view::npos);

    // No dialog open for the tests after this one.
    g_iCurrentDialogScript = -1;
    g_iNumLineMessageBoxCustom = 0;
    std::memset(g_lpszMessageBoxCustom, 0, sizeof g_lpszMessageBoxCustom);
    g_iNumAnswer = 0;
    std::memset(g_lpszDialogAnswer, 0, sizeof g_lpszDialogAnswer);
    std::setlocale(LC_ALL, previousLocale.c_str());
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

TEST_CASE("The texts that name items follow the editor's name and form edits and their undo [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    const Data::Items::ItemDefinition* bless = g_ItemDatabase.Find(14, 13);
    const Data::Items::ItemDefinition* tear = g_ItemDatabase.Find(14, 25);
    REQUIRE(bless != nullptr);
    REQUIRE(tear != nullptr);
    const Data::Items::ItemDefinition originalBless = *bless;
    const Data::Items::ItemDefinition originalTear = *tear;

    // A name edit shows in the texts at the editor's next frame, which refreshes them.
    I18N::SetLocale("de");
    Data::Items::ItemDefinition renamed = originalBless;
    renamed.names.Set("de", "Segensjuwel");
    g_ItemDatabase.Set(renamed);
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Juwel des Segens");
    g_ItemDataHandler.RefreshItemTextsIfNamesChanged();
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Segensjuwel");

    // So does a form, in the case the text asks for.
    I18N::SetLocale("pl");
    Data::Items::ItemDefinition reformed = originalTear;
    reformed.nameForms.Set("pl", "acc", "Lze Elfki");
    g_ItemDatabase.Set(reformed);
    g_ItemDataHandler.RefreshItemTextsIfNamesChanged();
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Lze Elfki'!");

    // Undo puts the items back, and the texts with them.
    g_ItemDatabase.Set(originalBless);
    g_ItemDatabase.Set(originalTear);
    g_ItemDataHandler.RefreshItemTextsIfNamesChanged();
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Łzę Elfa'!");

#ifdef _EDITOR
    // Without the accusative in Polish's cases (Cases..., an editor edit), the text shows the name itself.
    const Data::Items::ItemNameCases cases = g_ItemDatabase.GetNameCases();
    Data::Items::ItemNameCases withoutAccusative = cases;
    withoutAccusative.SetCases("pl", {"gen", "dat", "ins", "loc"});
    g_ItemDataHandler.SetNameCases(withoutAccusative);
    g_ItemDataHandler.RefreshItemTextsIfNamesChanged();
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Łza Elfa'!");
    g_ItemDataHandler.SetNameCases(cases);
    g_ItemDataHandler.RefreshItemTextsIfNamesChanged();
    CHECK(std::wstring_view(I18N::Dialog::Answer65Slot0) == L"Pójdę odnaleźć 'Łzę Elfa'!");
#endif // _EDITOR

    I18N::SetLocale("de");
    CHECK(std::wstring_view(I18N::Game::JewelOfBless) == L"Juwel des Segens");
}

TEST_CASE("Data Checks finds no problem in the shipped texts that name items [data][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Translation::CheckUiItemTexts(g_ItemDatabase, issues);
    Data::Translation::CheckItemDataTexts(g_ItemDatabase, "Tooltip.json", issues);
    std::string found;
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        found += issue.ToString() + "\n";
    }
    INFO(found);
    CHECK(issues.empty());
}

TEST_CASE("Data Checks names a UI text whose item the item data lacks by its file and key [data][texts]")
{
    // No items: every UI text names an item no one has.
    Data::Items::ItemDatabase empty;
    empty.Build(std::vector<Data::Items::ItemDefinition>{});
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Translation::CheckUiItemTexts(empty, issues);
    const auto polishBless = std::find_if(issues.begin(), issues.end(),
                                          [](const Data::Items::ItemDataIssue& issue)
                                          { return issue.source == "Game.pl.resx" && issue.field == "Jewel of Bless"; });
    REQUIRE(polishBless != issues.end());
    CHECK(polishBless->severity == Data::Items::ItemDataIssueSeverity::Error);
    // A click selects the item the text names.
    CHECK(polishBless->group == 14);
    CHECK(polishBless->number == 13);
    // Dialog has no Russian file: Russian shows the English text, which is checked once, as English.
    CHECK(std::none_of(issues.begin(), issues.end(),
                       [](const Data::Items::ItemDataIssue& issue) { return issue.source == "Dialog.ru.resx"; }));
}

TEST_CASE("A tooltip text names an item in the item name language and the rest in the UI language [data][items][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    I18N::SetLocale("de");

    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de") == L"Combine Goldschlüssel at level 0");
    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(Tooltip("Combine {item:14,113} at level {level}", "de") == L"Combine Gold Key at level 0");
    // No item 14,511: the reference stays as written, and the rest of the line is filled in.
    CHECK(Tooltip("Combine {item:14,511} at level {level}", "en") == L"Combine {item:14,511} at level 0");
    // A placeholder without a value stays as written, and so does the rest, with the items' names put in.
    CHECK(Tooltip("{noSuchValue} with {item:14,13}", "en") == L"{noSuchValue} with Jewel of Bless");
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

#ifdef _EDITOR
TEST_CASE("Name forms leaves the rest of Item Translations to the names grid [editor][items][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    TestImGuiContext context;
    CItemNameFormsPanel panel;
    constexpr float WindowHeight = 700.0f;
    // The names grid needs room for some rows below the panel.
    constexpr float RoomForTheGrid = 300.0f;

    // The Tear of Elf has German and Polish names, and both languages decline names: the table has rows.
    float left = 0.0f;
    for (int frame = 0; frame < 2; ++frame)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(900.0f, WindowHeight));
        ImGui::Begin("Name forms###NameFormsTest");
        ImGui::SetNextItemOpen(true);
        panel.Render(Data::Items::MakeItemType(14, 25));
        left = ImGui::GetContentRegionAvail().y;
        ImGui::End();
        ImGui::Render();
    }
    CHECK(left > RoomForTheGrid);
}

TEST_CASE("A Texts problem of a UI or tooltip file text naming a number no item has leaves the selection "
          "[editor][items][texts]")
{
    ItemTextScope scope;
    REQUIRE(scope.Loaded());
    REQUIRE(g_ItemDatabase.Find(14, 511) == nullptr);
    MuEditor::Items::ItemTextCheckArea area;
    const int tear = Data::Items::MakeItemType(14, 25);
    // The selection is the editor's, shared by the tests: put back as it was.
    const int selected = g_ItemSelection.GetItemType();
    g_ItemSelection.Select(tear);

    area.Open({Data::Items::ItemDataIssueSeverity::Error, "Game.pl.resx", 14, 511, "Tear", "names no item"}, false);
    CHECK(g_ItemSelection.GetItemType() == tear);
    area.Open({Data::Items::ItemDataIssueSeverity::Error, "Tooltip.json", 14, 511, "texts.talisman", "names no item"},
              false);
    CHECK(g_ItemSelection.GetItemType() == tear);
    // An item the text names is selected.
    area.Open({Data::Items::ItemDataIssueSeverity::Warning, "Game.pl.resx", 14, 13, "Bless", "missing form"}, false);
    CHECK(g_ItemSelection.GetItemType() == Data::Items::MakeItemType(14, 13));
    g_ItemSelection.Select(selected);
}
#endif // _EDITOR
