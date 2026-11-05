#include "stdafx.h"

#include "doctest.h"

// The Monster Browser window driven through ImGui: adding, copying and
// removing monsters and setting their kind, with undo and redo (MN1).
#ifdef _EDITOR

#include "TestImGuiContext.h"

#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/UndoTarget.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterBrowserUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
constexpr char AddDialog[] = "###MonsterAddDialog";
constexpr char RemoveDialog[] = "###MonsterRemoveDialog";
// The window of an open combo (ImGui::BeginCombo).
constexpr char ComboWindow[] = "##Combo_00";

// Monsters 0 to 109 are taken: the next free number, 110, is a trap by the
// old kind rules, 111 a monster.
constexpr int TakenNumbers = 110;
constexpr int TrapNumber = 110;
constexpr int MonsterNumber = 111;

MonsterDefinition Monster(int number, const char* names, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names = Data::LocalizedString::Parse(names);
    monster.kind = kind;
    return monster;
}

// The monster table with these monsters, no history, and the monster selected.
void StartWith(const std::vector<MonsterDefinition>& monsters, int selected)
{
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.OnDataReplaced();
    g_MonsterEdits.GetHistory().MarkSaved();
    g_MonsterSelection.Select(selected);
}

std::vector<MonsterDefinition> TakenMonsters()
{
    std::vector<MonsterDefinition> monsters;
    for (int number = 0; number < TakenNumbers; ++number)
    {
        monsters.push_back(Monster(number, ("Monster " + std::to_string(number)).c_str()));
    }
    return monsters;
}

void RenderBrowser()
{
    bool open = true;
    ImGui::NewFrame();
    g_MuMonsterBrowserUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

const ImGuiWindow* Window(const char* name)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(name);
    REQUIRE(window != nullptr);
    return window;
}

// The id of a widget in the window (the browser, a dialog, an open combo).
ImGuiID WidgetId(const char* window, const char* label)
{
    return ImGui::GetIDWithSeed(label, nullptr, Window(window)->ID);
}

// As a click: pressed in the next frame.
void Press(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderBrowser();
}

void Press(const char* window, const char* label)
{
    Press(WidgetId(window, label));
}

// The kind combo of a monster's row in the browser's list.
ImGuiID RowKindCombo(int number)
{
    const ImGuiID table = WidgetId(MuEditor::WindowIds::MonsterBrowser, "##Monsters");
    return ImGui::GetIDWithSeed("##Kind", nullptr, ImGui::GetIDWithSeed(number, table));
}

// Activation selects all of the field's text; the typed text replaces it.
void Type(const char* window, const char* label, const char* text)
{
    Press(window, label);
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderBrowser();
}

std::string StepAdd(const char* logName)
{
    return I18N::Format(I18N::Editor::StepAdd, {logName});
}
} // namespace

TEST_CASE("Monster Browser: an added monster takes the next free number and the kind of the old rules, and "
          "is undone and redone [editor][monsters]")
{
    StartWith(TakenMonsters(), 0);
    TestImGuiContext context;
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::AddMonster);
    Type(AddDialog, I18N::Editor::EnglishName, "Golem");
    Press(AddDialog, I18N::Editor::Add);
    RenderBrowser();

    const MonsterDefinition* golem = g_MonsterDatabase.Find(TrapNumber);
    REQUIRE(golem != nullptr);
    CHECK(golem->names.GetNeutral() == "Golem");
    CHECK(golem->names.GetTranslations().empty());
    CHECK(golem->kind == MonsterKind::Trap);
    CHECK(g_MonsterSelection.GetNumber() == TrapNumber);
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == StepAdd("Golem (110)"));

    MuEditor::UndoTarget& undo = MuEditor::Monsters::ToolWindow::GetUndoTarget();
    undo.Undo();
    RenderBrowser();
    CHECK(g_MonsterDatabase.Find(TrapNumber) == nullptr);
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());

    undo.Redo();
    RenderBrowser();
    REQUIRE(g_MonsterDatabase.Find(TrapNumber) != nullptr);
    CHECK(g_MonsterDatabase.Find(TrapNumber)->names.GetNeutral() == "Golem");
    CHECK(g_MonsterSelection.GetNumber() == TrapNumber);
}

TEST_CASE("Monster Browser: the kind of a new monster follows its number until one is picked [editor][monsters]")
{
    StartWith(TakenMonsters(), 0);
    TestImGuiContext context;
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::AddMonster);
    Press(AddDialog, I18N::Editor::NextFree);
    Type(AddDialog, I18N::Editor::EnglishName, "Ogre");
    Press(AddDialog, I18N::Editor::Add);

    const MonsterDefinition* ogre = g_MonsterDatabase.Find(MonsterNumber);
    REQUIRE(ogre != nullptr);
    CHECK(ogre->kind == MonsterKind::Monster);
    CHECK(g_MonsterDatabase.Find(TrapNumber) == nullptr);
}

TEST_CASE("Monster Browser: a copy has the kind and English name of its source, not its translations "
          "[editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o Lento", MonsterKind::Npc)}, 2);
    TestImGuiContext context;
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::CopyMonster);
    Press(AddDialog, I18N::Editor::Add);

    const MonsterDefinition* copy = g_MonsterDatabase.Find(3);
    REQUIRE(copy != nullptr);
    CHECK(copy->names.GetNeutral() == "Budge Dragon");
    CHECK(copy->names.GetTranslations().empty());
    CHECK(copy->kind == MonsterKind::Npc);
    CHECK(g_MonsterSelection.GetNumber() == 3);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == StepAdd("Budge Dragon (3)"));
}

TEST_CASE("Monster Browser: a row's kind combo changes that monster's kind as an undo step [editor][monsters]")
{
    // 2 is selected; the kind is changed in 7's row.
    StartWith({Monster(2, "Budge Dragon"), Monster(7, "Giant")}, 2);
    TestImGuiContext context;
    RenderBrowser();

    Press(RowKindCombo(7));
    Press(ComboWindow, I18N::Editor::KindNPC);
    RenderBrowser();

    REQUIRE(g_MonsterDatabase.Find(7) != nullptr);
    CHECK(g_MonsterDatabase.Find(7)->kind == MonsterKind::Npc);
    CHECK(g_MonsterDatabase.Find(2)->kind == MonsterKind::Monster);
    CHECK(g_MonsterSelection.GetNumber() == 7);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == I18N::Format(I18N::Editor::StepKind, {"Giant (7)"}));
    MuEditor::Monsters::ToolWindow::GetUndoTarget().Undo();
    CHECK(g_MonsterDatabase.Find(7)->kind == MonsterKind::Monster);
}

TEST_CASE("Monster Browser: picking the kind a monster has changes nothing [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon", MonsterKind::Npc)}, 2);
    TestImGuiContext context;
    RenderBrowser();

    Press(RowKindCombo(2));
    Press(ComboWindow, I18N::Editor::KindNPC);
    RenderBrowser();

    CHECK(g_MonsterDatabase.Find(2)->kind == MonsterKind::Npc);
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
}

TEST_CASE("Monster Browser: a removed monster comes back with undo [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o Lento")}, 2);
    TestImGuiContext context;
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::RemoveMonster);
    // The dialog lists what names the monster in the sources.
    RenderBrowser();
    Press(RemoveDialog, I18N::Editor::Remove);
    RenderBrowser();

    CHECK(g_MonsterDatabase.Find(2) == nullptr);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == I18N::Format(I18N::Editor::StepRemove, {"Budge Dragon (2)"}));
    MuEditor::Monsters::ToolWindow::GetUndoTarget().Undo();
    RenderBrowser();
    REQUIRE(g_MonsterDatabase.Find(2) != nullptr);
    CHECK(g_MonsterDatabase.Find(2)->names.Get("pt") == "Drag\xC3\xA3o Lento");
}

#endif // _EDITOR
