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
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterTranslationsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

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
    const ImGuiID table = ImGui::GetIDWithSeed(TableId, nullptr, TranslationsWindow()->ID);
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
