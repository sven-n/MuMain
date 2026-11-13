#include "stdafx.h"

#include "doctest.h"

// The Monster Translations window driven through ImGui: a name edited in the
// grid is an undo step, and Import names reads the files of the client
// folder (MN1).
#ifdef _EDITOR

#include "TemporaryMonsterFolder.h"
#include "TestImGuiContext.h"

#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/UndoTarget.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterNpcNameFiles.h"
#include "UI/MonsterTools/MonsterTalkTextRows.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterTranslationsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

using Data::Monsters::MonsterDefinition;

namespace
{
constexpr char TableId[] = "##MonsterTranslations";
// Wide enough for the columns of the first languages.
constexpr ImVec2 WindowSize(1200.0f, 700.0f);

MonsterDefinition Monster(int number, const char* names)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names = Data::LocalizedString::Parse(names);
    return monster;
}

// The monster table with these monsters, and an empty history that counts as
// saved.
void StartWith(const std::vector<MonsterDefinition>& monsters)
{
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterDatabase.SetTalkTexts({});
    g_MonsterEdits.OnDataReplaced();
    g_MonsterEdits.GetHistory().MarkSaved();
}

const Data::LocalizedString& NamesOf(int number)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    REQUIRE(monster != nullptr);
    return monster->names;
}

void RenderTranslations(const char* typed = nullptr)
{
    if (typed != nullptr)
    {
        ImGui::GetIO().AddInputCharactersUTF8(typed);
    }
    bool open = true;
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(WindowSize);
    g_MuMonsterTranslationsUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* TranslationsWindow()
{
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::MonsterTranslations);
    REQUIRE(window != nullptr);
    return window;
}

int ColumnOf(std::string_view locale)
{
    const auto locales = I18N::GetAvailableLocales();
    for (size_t index = 0; index < locales.size(); ++index)
    {
        if (locale == locales[index])
        {
            return static_cast<int>(index);
        }
    }
    FAIL("no such UI language");
    return -1;
}

// The name field of a monster in a language's column.
ImGuiID NameCellId(int number, std::string_view locale)
{
    // The names are in the window's Names tab.
    const ImGuiID tabBar = ImGui::GetIDWithSeed("##Texts", nullptr, TranslationsWindow()->ID);
    const ImGuiID tab = ImGui::GetIDWithSeed(I18N::Editor::Names, nullptr, tabBar);
    const ImGuiID table = ImGui::GetIDWithSeed(TableId, nullptr, tab);
    const ImGuiID row = ImGui::GetIDWithSeed(number, table);
    return ImGui::GetIDWithSeed("##Name", nullptr, ImGui::GetIDWithSeed(ColumnOf(locale), row));
}

// Types the text into the name field and leaves it.
void EditName(int number, std::string_view locale, const char* text)
{
    // Activation selects the whole text; the typed text replaces it.
    ImGui::ActivateItemByID(NameCellId(number, locale));
    RenderTranslations();
    RenderTranslations(text);
    ImGui::ClearActiveID();
    RenderTranslations();
}

// As a click on a button of the window: pressed in the next frame.
void Press(const char* label)
{
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(label, nullptr, TranslationsWindow()->ID));
    RenderTranslations();
}

// The client folder as the working directory while it lives: the window reads
// and writes the files there, as the game does.
class WorkingDirectoryScope
{
public:
    explicit WorkingDirectoryScope(const std::filesystem::path& directory) : m_previous(std::filesystem::current_path())
    {
        std::filesystem::current_path(directory);
    }

    ~WorkingDirectoryScope()
    {
        std::error_code ignored;
        std::filesystem::current_path(m_previous, ignored);
    }

    WorkingDirectoryScope(const WorkingDirectoryScope&) = delete;
    WorkingDirectoryScope& operator=(const WorkingDirectoryScope&) = delete;

private:
    std::filesystem::path m_previous;
};
} // namespace

TEST_CASE("Monster Translations: a name edited in the grid is an undo step [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon"), Monster(3, "100%s ##Spider")});
    TestImGuiContext context;
    RenderTranslations();
    RenderTranslations();

    EditName(2, "de", "Lahmer Drache");

    CHECK(NamesOf(2).GetOwn("de") == "Lahmer Drache");
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepMonsterName, {"Budge Dragon (2)", I18N::GetLanguageDisplayName("de")}));
    MuEditor::Monsters::ToolWindow::GetUndoTarget().Undo();
    RenderTranslations();
    CHECK_FALSE(NamesOf(2).HasOwn("de"));
}

// The talk texts of the NPCs (Q-MN47): a row per text, its missing
// languages listed, an edit stored as an undo step.
TEST_CASE("Monster Translations: an NPC's talk texts are rows of their own [editor][monsters]")
{
    MonsterDefinition gatekeeper = Monster(408, "Gatekeeper");
    Data::Monsters::MonsterTalkBlock dialog;
    dialog.kind = Data::Monsters::MonsterTalkBlock::Kind::Dialog;
    dialog.dialog.title.Set("en", "Gatekeeper");
    dialog.dialog.lines.resize(2);
    dialog.dialog.lines[0].Set("en", "Who are you?");
    dialog.dialog.lines[0].Set("de", "Wer bist du?");
    dialog.dialog.lines[1].Set("en", "Go on.");
    gatekeeper.setup.onTalk = {dialog};
    StartWith({Monster(2, "Budge Dragon"), gatekeeper});

    MuEditor::Monsters::MonsterTalkTextRows rows;
    REQUIRE(rows.List("", "").size() == 3);
    char id[96];
    rows.FormatId(1, id, sizeof(id));
    CHECK(std::string(id) == "408 onTalk[0].dialog.lines[0]");
    CHECK(rows.FindNames(1)->GetOwn("de") == "Wer bist du?");
    // Two texts miss their German.
    CHECK(rows.List("", "de").size() == 2);
    CHECK(rows.CountMissing(std::array<const char*, 1>{"de"})[0] == 2);

    rows.CommitName(2, "de", "Geh weiter.");
    CHECK(g_MonsterDatabase.Find(408)->setup.onTalk[0].dialog.lines[1].GetOwn("de") == "Geh weiter.");
    MuEditor::Monsters::ToolWindow::GetUndoTarget().Undo();
    CHECK_FALSE(g_MonsterDatabase.Find(408)->setup.onTalk[0].dialog.lines[1].HasOwn("de"));
}

// The shared talk texts (the list's talkTexts) are rows of their own, first;
// an edit is an undo step.
TEST_CASE("Monster Translations: the shared talk texts are rows [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon")});
    Data::Monsters::MonsterTalkTexts texts;
    for (Data::LocalizedString* text : Data::Monsters::AltarContractTexts(texts["crywolfAltar"]))
        text->Set("en", "Text");
    g_MonsterDatabase.SetTalkTexts(texts);

    MuEditor::Monsters::MonsterTalkTextRows rows;
    REQUIRE(rows.List("", "").size() == 6);
    char id[96];
    rows.FormatId(0, id, sizeof(id));
    CHECK(std::string(id) == "talkTexts.crywolfAltar.canContract");
    rows.FormatId(5, id, sizeof(id));
    CHECK(std::string(id) == "talkTexts.crywolfAltar.ongoing");

    rows.CommitName(1, "de", "Fortfahren?");
    CHECK(g_MonsterDatabase.GetTalkTexts().at("crywolfAltar").proceed.GetOwn("de") == "Fortfahren?");
    MuEditor::Monsters::ToolWindow::GetUndoTarget().Undo();
    CHECK_FALSE(g_MonsterDatabase.GetTalkTexts().at("crywolfAltar").proceed.HasOwn("de"));
    g_MonsterDatabase.SetTalkTexts({});
}

// The texts of a map variant's talk and of a state variant's are rows too.
TEST_CASE("Monster Translations: the talk texts of map and state variants are rows [editor][monsters]")
{
    Data::Monsters::MonsterTalkBlock message;
    message.kind = Data::Monsters::MonsterTalkBlock::Kind::Message;
    message.message.Set("en", "On Crywolf.");
    MonsterDefinition npc = Monster(408, "Gatekeeper");
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {Data::Maps::MapRef{34, 0}};
    variant.setup.onTalk = {message};
    npc.mapVariants.push_back(variant);
    Data::Monsters::MonsterStateVariant state;
    state.state = "crywolfReady";
    state.setup.onTalk = {message};
    state.setup.onTalk[0].message.Set("en", "Ready.");
    npc.setup.stateVariants.push_back(state);
    StartWith({npc});

    MuEditor::Monsters::MonsterTalkTextRows rows;
    REQUIRE(rows.List("", "").size() == 2);
    char id[96];
    rows.FormatId(0, id, sizeof(id));
    CHECK(std::string(id) == "408 stateVariants[0].onTalk[0].message");
    rows.FormatId(1, id, sizeof(id));
    CHECK(std::string(id) == "408 mapVariants[0].onTalk[0].message");

    rows.CommitName(1, "de", "Auf Crywolf.");
    CHECK(g_MonsterDatabase.Find(408)->mapVariants[0].setup.onTalk[0].message.GetOwn("de") == "Auf Crywolf.");
}

TEST_CASE("Monster Translations: a name with the separator is not stored [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon")});
    TestImGuiContext context;
    RenderTranslations();
    RenderTranslations();

    EditName(2, "de", "Lahmer||Drache");

    CHECK_FALSE(NamesOf(2).HasOwn("de"));
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
}

TEST_CASE("Monster Translations: Import names reads the files of the client folder [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon")});
    const TemporaryMonsterFolder folder;
    std::string error;
    REQUIRE(Data::WriteTextFile(
        MuEditor::Monsters::NpcNameFiles::GetFile(folder.Path(), Data::GetLegacyLanguages().front()),
        "2\t1\t\"Red Dragon\"\r\nend\r\n", error));
    const WorkingDirectoryScope workingDirectory(folder.Path());
    TestImGuiContext context;
    RenderTranslations();

    Press(I18N::Editor::ImportNamesFromNpcNameTxt);

    CHECK(NamesOf(2).GetNeutral() == "Red Dragon");
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == I18N::Editor::ImportNamesFromNpcNameTxt);
}

#endif // _EDITOR
