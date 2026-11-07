#include "stdafx.h"

#include "doctest.h"

// The monster tools with monsters of several numbers (D61, PR 3a): the
// Browser's tree, shared and own values (Q-MN95, Q-MN96), the Translations
// rows and the checks of Data Checks.
#ifdef _EDITOR

#include "ClientDataScope.h"
#include "TemporaryMonsterFolder.h"
#include "TestImGuiContext.h"

#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/MonsterTools/MonsterCheckArea.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterGroupChecks.h"
#include "UI/MonsterTools/MonsterNameRows.h"
#include "UI/MonsterTools/MonsterNpcNameFiles.h"
#include "UI/MonsterTools/MonsterOwnValues.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MonsterShownValues.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MonsterTreeModel.h"
#include "Core/MuEditorCore.h"
#include "UI/MonsterTools/MuMonsterAttacksUI.h"
#include "UI/MonsterTools/MuMonsterBrowserUI.h"
#include "UI/MonsterTools/MuMonsterSoundsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

using Data::Items::ItemDataIssue;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using Data::Monsters::MonsterValueField;
using MuEditor::Monsters::MonsterTreeModel;
using Row = MuEditor::Monsters::MonsterTreeModel::Row;
namespace OwnValues = MuEditor::Monsters::OwnValues;
namespace ShownValues = MuEditor::Monsters::ShownValues;

namespace
{
// Budge Dragon (2) is a monster of its own; Death Centurion has 145, 175 and
// 336, which attacks with the generic pattern as its own value.
constexpr char GroupedMonstersJson[] = R"({ "formatVersion": 2, "monsters": [
    { "name": "Budge Dragon", "kind": "monster", "numbers": [{"number": 2, "maps": [0]}] },
    { "name": {"en": "Death Centurion", "pt": "Centurião da morte"}, "kind": "monster",
      "model": "MONSTER_MODEL_DEATH_CENTURION",
      "numbers": [{"number": 145, "maps": [24]}, {"number": 175, "maps": [25]},
                  {"number": 336, "maps": [36], "attacks": {"otherwise": "generic", "clearSkill": "never"}}] } ] })";

constexpr char NumberDialog[] = "###MonsterNumberDialog";

// The grouped monsters read by the handler, with an empty history and every
// field locked.
class GroupedTools
{
public:
    GroupedTools() : m_scope(m_folder.Path())
    {
        m_folder.WriteMonsters(GroupedMonstersJson);
        std::vector<ItemDataIssue> issues;
        REQUIRE(g_MonsterDataHandler.Revert(issues));
        g_MonsterEdits.OnDataReplaced();
        g_MonsterEdits.GetHistory().MarkSaved();
        OwnValues::LockAll();
        g_MonsterSelection.Select(2);
    }

    const std::filesystem::path& Folder() const
    {
        return m_folder.Path();
    }

private:
    TemporaryMonsterFolder m_folder;
    ClientDataScope m_scope;
};

MonsterDefinition Entry(int number)
{
    const MonsterDefinition* entry = g_MonsterDatabase.Find(number);
    REQUIRE(entry != nullptr);
    return *entry;
}

void RenderBrowser()
{
    bool open = true;
    ImGui::NewFrame();
    g_MuMonsterBrowserUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

ImGuiID WidgetId(const char* window, const char* label)
{
    const ImGuiWindow* found = ImGui::FindWindowByName(window);
    REQUIRE(found != nullptr);
    return ImGui::GetIDWithSeed(label, nullptr, found->ID);
}

void Press(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderBrowser();
}

void Press(const char* window, const char* label)
{
    Press(WidgetId(window, label));
}

// Activation selects all of the field's text; the typed text replaces it. A
// field still active from typing before (a press of a disabled button leaves
// it so) is let go first, or the text would be added to it.
void Type(const char* window, const char* label, const char* text)
{
    ImGui::ClearActiveID();
    RenderBrowser();
    Press(window, label);
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderBrowser();
}

// The kind combo of a row: a number's, or a monster's (its first number with
// ShownValues::WholeMonsterKey).
ImGuiID RowKindCombo(int rowId)
{
    const ImGuiID table = WidgetId(MuEditor::WindowIds::MonsterBrowser, "##Monsters");
    return ImGui::GetIDWithSeed("##Kind", nullptr, ImGui::GetIDWithSeed(rowId, table));
}

std::vector<Row> Rows(MonsterTreeModel& tree, const char* search = "")
{
    MuEditor::Monsters::MonsterListModel::Filter filter;
    filter.search = search;
    return tree.GetRows(filter);
}
} // namespace

TEST_CASE("Monster Browser tree: a monster with several numbers is a row that opens to its numbers "
          "[editor][monsters]")
{
    GroupedTools tools;
    MonsterTreeModel tree;
    CHECK(Rows(tree) == std::vector<Row>{{2, Row::Kind::Single, 1, false}, {145, Row::Kind::Monster, 3, false}});

    tree.SetOpen(336, true);
    CHECK(tree.IsOpen(145));
    CHECK(Rows(tree) == std::vector<Row>{{2, Row::Kind::Single, 1, false},
                                         {145, Row::Kind::Monster, 3, true},
                                         {145, Row::Kind::Number, 3, false},
                                         {175, Row::Kind::Number, 3, false},
                                         {336, Row::Kind::Number, 3, false}});

    // A search opens the monsters to the numbers it finds.
    tree.SetOpen(145, false);
    CHECK(Rows(tree, "336") ==
          std::vector<Row>{{145, Row::Kind::Monster, 3, true}, {336, Row::Kind::Number, 3, false}});
    CHECK(Rows(tree, "Centuri") == std::vector<Row>{{145, Row::Kind::Monster, 3, true},
                                                    {145, Row::Kind::Number, 3, false},
                                                    {175, Row::Kind::Number, 3, false},
                                                    {336, Row::Kind::Number, 3, false}});

    // An open monster stays open when its first number moves out or a lower
    // one moves in.
    tree.SetOpen(145, true);
    REQUIRE(g_MonsterEdits.MoveNumber("move", 145, 2));
    CHECK(tree.IsOpen(175));
    REQUIRE(g_MonsterEdits.Undo());
    tree.SetOpen(145, false);

    // The rows follow a move into another monster.
    REQUIRE(g_MonsterEdits.MoveNumber("move", 2, 145));
    CHECK(Rows(tree) == std::vector<Row>{{2, Row::Kind::Monster, 4, false}});
}

TEST_CASE("Monster Browser tree: a number is shown on its own row, a monster as a whole on its monster's row "
          "[editor][monsters]")
{
    GroupedTools tools;
    MonsterTreeModel tree;
    const auto shows = [](const std::vector<Row>& rows, int number, bool whole)
    {
        return std::ranges::any_of(rows, [&](const Row& row)
                                   { return MonsterTreeModel::ShowsSelection(row, number, whole); });
    };
    // The search finds 175 only: the monster's row and 175's.
    const std::vector<Row> found = Rows(tree, "175");
    REQUIRE(found == std::vector<Row>{{145, Row::Kind::Monster, 3, true}, {175, Row::Kind::Number, 3, false}});
    CHECK(shows(found, 175, false));
    CHECK(shows(found, 145, true));
    CHECK(shows(found, 336, true));
    // The monster's row is not the row of its first number, so the Browser
    // clears the search for 145 selected elsewhere.
    CHECK_FALSE(shows(found, 145, false));
    CHECK_FALSE(shows(found, 2, false));

    // A monster with one number is on its row either way.
    const std::vector<Row> all = Rows(tree);
    CHECK(shows(all, 2, false));
    CHECK(shows(all, 2, true));
    // A closed monster shows itself as a whole, not its numbers.
    CHECK(shows(all, 175, true));
    CHECK_FALSE(shows(all, 175, false));
}

TEST_CASE("Monster tools: a number's shared value is locked until it is made its own, an own value goes back "
          "(Q-MN95, Q-MN96) [editor][monsters]")
{
    GroupedTools tools;
    CHECK(OwnValues::GetNumberState(145, MonsterValueField::Attacks) == OwnValues::State::Shared);
    CHECK(OwnValues::GetNumberState(336, MonsterValueField::Attacks) == OwnValues::State::Own);
    CHECK(OwnValues::GetNumberState(336, MonsterValueField::Names) == OwnValues::State::Shared);
    CHECK(OwnValues::GetNumberState(2, MonsterValueField::Names) == OwnValues::State::Plain);

    OwnValues::Unlock(145, MonsterValueField::Attacks);
    CHECK(OwnValues::GetNumberState(145, MonsterValueField::Attacks) == OwnValues::State::Unlocked);
    CHECK(OwnValues::GetNumberState(175, MonsterValueField::Attacks) == OwnValues::State::Shared);

    // Unlocked in one monster, a number moved into another is locked there,
    // and stays locked once the move is undone; the other numbers keep theirs.
    OwnValues::Unlock(175, MonsterValueField::Kind);
    REQUIRE(g_MonsterEdits.MoveNumber("move", 175, 2));
    CHECK(OwnValues::GetNumberState(175, MonsterValueField::Kind) == OwnValues::State::Shared);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(OwnValues::GetNumberState(175, MonsterValueField::Kind) == OwnValues::State::Shared);
    CHECK(OwnValues::GetNumberState(145, MonsterValueField::Attacks) == OwnValues::State::Unlocked);

    // Split out and moved back into the monster, which kept its other two
    // numbers, the number is locked too.
    OwnValues::Unlock(175, MonsterValueField::Kind);
    REQUIRE(g_MonsterEdits.SplitNumber("split", 175));
    REQUIRE(g_MonsterEdits.MoveNumber("move back", 175, 145));
    CHECK(OwnValues::GetNumberState(175, MonsterValueField::Kind) == OwnValues::State::Shared);
    REQUIRE(g_MonsterEdits.Undo());
    REQUIRE(g_MonsterEdits.Undo());

    // "Back to..." offers the monster's value and the other numbers' own ones.
    std::vector<OwnValues::Source> sources = OwnValues::FindSources(336, MonsterValueField::Attacks);
    REQUIRE(sources.size() == 1);
    CHECK(sources[0].label == I18N::Editor::TheMonsterSValue);
    MonsterDefinition other = Entry(175);
    other.attacks.emplace();
    REQUIRE(g_MonsterEdits.SetMonster("other", other));
    sources = OwnValues::FindSources(336, MonsterValueField::Attacks);
    REQUIRE(sources.size() == 2);
    CHECK(sources[1].label == I18N::Format(I18N::Editor::LikeNumber0, {"175"}));
    // A number like the one asked for is offered once.
    CHECK(OwnValues::FindSources(145, MonsterValueField::Attacks).size() == 2);

    REQUIRE(OwnValues::TakeValue(336, MonsterValueField::Attacks, sources[1]));
    CHECK(Entry(336).attacks == other.attacks);
    REQUIRE(OwnValues::TakeValue(336, MonsterValueField::Attacks, sources[0]));
    CHECK_FALSE(Entry(336).attacks);
    CHECK(OwnValues::GetNumberState(336, MonsterValueField::Attacks) == OwnValues::State::Shared);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepBackTo,
                       {"Death Centurion (336)", I18N::Editor::Attacks, I18N::Editor::TheMonsterSValue}));
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(Entry(336).attacks == other.attacks);
}

TEST_CASE("Monster tools: a monster as a whole shows and stores the values its numbers share [editor][monsters]")
{
    GroupedTools tools;
    g_MonsterSelection.SelectMonster(175);
    CHECK(g_MonsterSelection.IsWholeMonster());
    const std::optional<ShownValues::ShownMonster> shown = ShownValues::ReadSelected();
    REQUIRE(shown);
    CHECK(shown->wholeMonster);
    CHECK(shown->numberCount == 3);
    CHECK(shown->monster.number == 175);
    CHECK_FALSE(shown->monster.attacks);
    CHECK(OwnValues::GetState(*shown, MonsterValueField::Attacks) == OwnValues::State::Plain);
    CHECK(ShownValues::LogName(*shown) == I18N::Format(I18N::Editor::MonsterLogName01, {"Death Centurion", "3"}));

    ShownValues::ShownMonster changed = *shown;
    changed.monster.kind = MonsterKind::Npc;
    REQUIRE(ShownValues::Store("kind", changed));
    CHECK(Entry(145).kind == MonsterKind::Npc);
    CHECK(Entry(336).kind == MonsterKind::Npc);
    CHECK(Entry(175).maps == std::vector<Data::Maps::MapRef>{{25, 0}});

    // Its undo and redo select the monster as a whole again.
    g_MonsterSelection.Select(2);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(Entry(145).kind == MonsterKind::Monster);
    CHECK(g_MonsterSelection.IsWholeMonster());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(g_MonsterSelection.GetNumber()).size() == 3);
    g_MonsterSelection.Select(2);
    REQUIRE(g_MonsterEdits.Redo());
    CHECK(g_MonsterSelection.IsWholeMonster());
    // A number's own edit selects the number.
    MonsterDefinition own = Entry(336);
    own.kind = MonsterKind::Trap;
    REQUIRE(g_MonsterEdits.SetMonster("own kind", own));
    REQUIRE(g_MonsterEdits.Undo());
    CHECK_FALSE(g_MonsterSelection.IsWholeMonster());
    CHECK(g_MonsterSelection.GetNumber() == 336);

    // A number shows its entry; a monster with one number too.
    g_MonsterSelection.Select(336);
    CHECK_FALSE(ShownValues::ReadSelected()->wholeMonster);
    CHECK(ShownValues::ReadSelected()->monster.attacks);
    g_MonsterSelection.SelectMonster(2);
    CHECK_FALSE(ShownValues::ReadSelected()->wholeMonster);
}

TEST_CASE("Monster Attacks: a number's shared attacks are locked for edits, not for the previews [editor][monsters]")
{
    GroupedTools tools;
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_DEATH_CENTURION";
    model.load.file = "Data/Monster/Monster11.bmd";
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.attacks.emplace();
    values.attacks->otherwise = {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    REQUIRE(g_MonsterEdits.SetSharedValues("attacks", 145, values));
    REQUIRE(OwnValues::GetNumberState(145, MonsterValueField::Attacks) == OwnValues::State::Shared);
    TestImGuiContext context;
    g_MonsterSelection.Select(145);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    // The items of the attacks of all maps of 145.
    const auto item = [](const char* label)
    {
        constexpr int AllMaps = -1;
        const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::MonsterAttacks);
        REQUIRE(window != nullptr);
        return ImGui::GetIDWithSeed(label, nullptr,
                                    ImGui::GetIDWithSeed(AllMaps, ImGui::GetIDWithSeed(145, window->ID)));
    };
    const auto click = [&](const char* label)
    {
        ImGui::ActivateItemByID(item(label));
        render();
        render();
    };
    render();

    click(I18N::Editor::PlayNextAttack);
    CHECK(g_MuMonsterAttacksUI.GetLastPlayed().has_value());
    click(I18N::Editor::TheGenericPattern);
    CHECK_FALSE(Entry(145).attacks->otherwiseGeneric);

    g_MuEditorCore.GetToolWindows().monsterPreview = false;
    g_MonsterModelDatabase.Build({});
}

TEST_CASE("Monster Attacks: a monster as a whole and its first number each start on all maps [editor][monsters]")
{
    GroupedTools tools;
    Data::Monsters::MonsterMapVariant variant;
    variant.attacks.emplace();
    variant.maps = {{24, 0}};
    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.attacks.emplace();
    values.mapVariants = {variant, variant};
    values.mapVariants[1].maps = {{36, 0}};
    REQUIRE(g_MonsterEdits.SetSharedValues("shared variants", 145, values));
    MonsterDefinition own = Entry(145);
    own.mapVariants[1].maps = {{30, 0}};
    REQUIRE(g_MonsterEdits.SetMonster("own variants", own));
    OwnValues::Unlock(145, MonsterValueField::Attacks);
    TestImGuiContext context;
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };

    // The whole monster shows its second variant; its first number, selected
    // next, shows its own attacks of all maps, not its own second variant.
    g_MonsterSelection.SelectMonster(145);
    g_MuMonsterAttacksUI.ShowMapVariant(ShownValues::GetSelectedKey(), 1);
    render();
    g_MonsterSelection.Select(145);
    render();
    constexpr int AllMaps = -1;
    const ImGuiWindow* window = ImGui::FindWindowByName(MuEditor::WindowIds::MonsterAttacks);
    REQUIRE(window != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(I18N::Editor::ADraw, nullptr,
                                                 ImGui::GetIDWithSeed(AllMaps, ImGui::GetIDWithSeed(145, window->ID))));
    render();
    render();
    CHECK(Entry(145).attacks->otherwiseRandom.has_value());
    CHECK_FALSE(Entry(145).mapVariants[1].attacks->otherwiseRandom.has_value());
}

TEST_CASE("Monster tools: a change of only the shared values is a change of the tools' data [editor][monsters]")
{
    GroupedTools tools;
    // Every number has a set of its own: the shared set changes no entry.
    for (const int number : {145, 175, 336})
    {
        MonsterDefinition own = Entry(number);
        own.soundSet = "Own" + std::to_string(number);
        REQUIRE(g_MonsterEdits.SetMonster("own set", own));
    }
    const int before = MuEditor::Monsters::ToolWindow::GetDataVersion();
    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.soundSet = "Centurion";
    REQUIRE(g_MonsterEdits.SetSharedValues("shared set", 145, values));
    CHECK(Entry(145).soundSet == "Own145");
    // A whole monster's draft (Monster Attacks) takes the new shared set
    // before its next store.
    CHECK(MuEditor::Monsters::ToolWindow::GetDataVersion() != before);
}

TEST_CASE("Monster Sounds: a monster as a whole shows the set its numbers share, a number its own [editor][monsters]")
{
    GroupedTools tools;
    std::vector<Data::Monsters::MonsterSoundSet> sets(2);
    sets[0].name = "Centurion";
    sets[1].name = "Rogue";
    g_MonsterSoundSetDatabase.Build(sets);
    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.soundSet = "Centurion";
    REQUIRE(g_MonsterEdits.SetSharedValues("shared set", 145, values));
    MonsterDefinition own = Entry(145);
    own.soundSet = "Rogue";
    REQUIRE(g_MonsterEdits.SetMonster("own set", own));
    TestImGuiContext context;
    const auto draw = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSoundsUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };

    g_MonsterSelection.SelectMonster(145);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownSet() == "Centurion");
    g_MonsterSelection.Select(145);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownSet() == "Rogue");

    // Every number has a set of its own: a change of only the shared values
    // reaches the whole monster shown, which is read again then.
    for (const int number : {175, 336})
    {
        MonsterDefinition other = Entry(number);
        other.soundSet = "Rogue";
        REQUIRE(g_MonsterEdits.SetMonster("own set", other));
    }
    g_MonsterSelection.SelectMonster(145);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownSet() == "Centurion");
    values.soundSet = "Rogue";
    REQUIRE(g_MonsterEdits.SetSharedValues("shared set", 145, values));
    g_MonsterSelection.SelectMonster(145);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownSet() == "Rogue");
    g_MonsterSoundSetDatabase.Build({});
}

TEST_CASE("Monster Browser: a monster row's kind is the shared one, a number's shared kind is locked "
          "[editor][monsters]")
{
    GroupedTools tools;
    TestImGuiContext context;
    // Selecting a number elsewhere opens its monster in the Browser.
    g_MonsterSelection.Select(175);
    RenderBrowser();
    RenderBrowser();

    Press(RowKindCombo(175));
    CHECK(ImGui::FindWindowByName("##Combo_00") == nullptr);

    Press(RowKindCombo(145 + ShownValues::WholeMonsterKey));
    Press("##Combo_00", I18N::Editor::KindNPC);
    RenderBrowser();
    CHECK(Entry(145).kind == MonsterKind::Npc);
    CHECK(Entry(175).kind == MonsterKind::Npc);
    CHECK(Entry(336).kind == MonsterKind::Npc);
    CHECK(g_MonsterSelection.IsWholeMonster());
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepKind, {"Death Centurion (3 numbers)"}));
}

TEST_CASE("Monster Browser: Add number and Split number out work on the selection [editor][monsters]")
{
    GroupedTools tools;
    TestImGuiContext context;
    g_MonsterSelection.SelectMonster(145);
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::AddNumber);
    RenderBrowser();
    Press(NumberDialog, I18N::Editor::Add);
    RenderBrowser();
    // The next free number after the monster's last one.
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145) == std::vector{145, 175, 336, 337});
    CHECK(g_MonsterSelection.GetNumber() == 337);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepAddNumber, {"337", "Death Centurion"}));

    g_MonsterSelection.Select(175);
    RenderBrowser();
    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::SplitNumberOut);
    RenderBrowser();
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{175});
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{145, 175, 336, 337});
}

TEST_CASE("Monster Browser: Move number to... lists the other monsters and moves the number into one "
          "[editor][monsters]")
{
    GroupedTools tools;
    TestImGuiContext context;
    g_MonsterSelection.Select(175);
    RenderBrowser();
    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::MoveNumberTo);
    RenderBrowser();
    const ImVector<ImGuiWindow*>& windows = ImGui::GetCurrentContext()->Windows;
    const auto targets =
        std::find_if(windows.begin(), windows.end(), [](const ImGuiWindow* window)
                     { return std::string_view(window->Name).find("##Targets") != std::string_view::npos; });
    REQUIRE(targets != windows.end());
    const ImGuiID budgeDragon = ImGui::GetIDWithSeed("Budge Dragon (2)##2", nullptr, (*targets)->ID);
    Press(budgeDragon);

    // A search that hides the picked monster drops it: Move moves nothing.
    Type(NumberDialog, "##Search", "Centurion");
    Press(NumberDialog, I18N::Editor::Move);
    RenderBrowser();
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{145, 175, 336});

    Type(NumberDialog, "##Search", "Budge");
    Press(budgeDragon);
    Press(NumberDialog, I18N::Editor::Move);
    RenderBrowser();
    CHECK(g_MonsterEdits.ReadMonsterNumbers(175) == std::vector{2, 175});
}

TEST_CASE("Monster Browser: Remove monster names the monster by its shared name [editor][monsters]")
{
    GroupedTools tools;
    TestImGuiContext context;
    MonsterDefinition own = Entry(145);
    own.names.Set("en", "Rogue Centurion");
    REQUIRE(g_MonsterEdits.SetMonster("own name", own));
    g_MonsterSelection.SelectMonster(145);
    RenderBrowser();

    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::RemoveMonster);
    RenderBrowser();
    Press("###MonsterRemoveDialog", I18N::Editor::Remove);
    RenderBrowser();
    CHECK(g_MonsterEdits.ReadMonsterNumbers(145).empty());
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepRemoveMonsterWithNumbers, {"Death Centurion", "3"}));
}

TEST_CASE("Monster Browser: Remove monster follows a change of the monster's numbers while it is open "
          "[editor][monsters]")
{
    GroupedTools tools;
    TestImGuiContext context;
    g_MonsterSelection.SelectMonster(145);
    RenderBrowser();
    Press(MuEditor::WindowIds::MonsterBrowser, I18N::Editor::RemoveMonster);
    RenderBrowser();

    // 336 leaves the monster while the dialog is open (an undo elsewhere):
    // the dialog lists and removes the two numbers left.
    REQUIRE(g_MonsterEdits.MoveNumber("move", 336, 2));
    RenderBrowser();
    Press("###MonsterRemoveDialog", I18N::Editor::Remove);
    RenderBrowser();
    CHECK(g_MonsterDatabase.Find(145) == nullptr);
    CHECK(g_MonsterDatabase.Find(175) == nullptr);
    CHECK(g_MonsterEdits.ReadMonsterNumbers(336) == std::vector{2, 336});
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() ==
          I18N::Format(I18N::Editor::StepRemoveMonsterWithNumbers, {"Death Centurion", "2"}));
}

TEST_CASE("Monster Translations rows: a monster with several numbers is translated once, numbers with names "
          "of their own have rows [editor][monsters]")
{
    GroupedTools tools;
    MuEditor::Monsters::MonsterNameRows rows;
    const int whole = 145 + ShownValues::WholeMonsterKey;
    CHECK(rows.List("", "") == std::vector{2, whole});
    // The monster's row is found by each of its numbers, with the other
    // filters still applied.
    CHECK(rows.List("336", "") == std::vector{whole});
    CHECK(rows.List("145", "") == std::vector{whole});
    CHECK(rows.List("336", "pt").empty());
    REQUIRE(rows.FindNames(whole) != nullptr);
    CHECK(rows.FindNames(whole)->Get("pt") == "Centurião da morte");
    char id[32];
    rows.FormatId(whole, id, sizeof(id));
    CHECK(std::string(id) == "145 (3)");

    rows.CommitName(whole, "de", "Todeszenturio");
    CHECK(Entry(145).names.Get("de") == "Todeszenturio");
    CHECK(Entry(336).names.Get("de") == "Todeszenturio");
    CHECK(rows.FindNames(whole)->Get("de") == "Todeszenturio");

    // A number that shares the names (selected in the Browser) is in its
    // monster's row.
    g_MonsterSelection.Select(175);
    CHECK(rows.GetSelected() == whole);

    // Names made 336's own (the Browser's row menu) get a row to edit them in.
    OwnValues::Unlock(336, MonsterValueField::Names);
    CHECK(rows.List("", "") == std::vector{2, whole, 336});
    OwnValues::LockAll();

    // 175 with a name of its own gets a row after its monster's.
    MonsterDefinition own = Entry(175);
    own.names.Set("en", "Rogue Centurion");
    REQUIRE(g_MonsterEdits.SetMonster("own", own));
    rows.Refresh();
    CHECK(rows.List("", "") == std::vector{2, whole, 175});
    CHECK(rows.List("Rogue", "") == std::vector{175});
    rows.CommitName(175, "de", "Schurkenzenturio");
    CHECK(Entry(175).names.Get("de") == "Schurkenzenturio");
    CHECK(Entry(145).names.Get("de") == "Todeszenturio");

    rows.Select(whole);
    CHECK(g_MonsterSelection.IsWholeMonster());
    CHECK(rows.GetSelected() == whole);
    rows.Select(175);
    CHECK(rows.GetSelected() == 175);

    // A monster's row still shown after its monster lost its other numbers
    // changes the number left.
    REQUIRE(g_MonsterEdits.SplitNumber("split", 175));
    REQUIRE(g_MonsterEdits.SplitNumber("split", 336));
    rows.CommitName(whole, "de", "Zenturio");
    CHECK(Entry(145).names.Get("de") == "Zenturio");
}

TEST_CASE("Monster Translations: a name the import gives every number of a monster is the monster's "
          "[editor][monsters]")
{
    GroupedTools tools;
    const auto languages = Data::GetLegacyLanguages();
    const auto portuguese = std::ranges::find(languages, std::string("pt"), &Data::LegacyLanguage::locale);
    REQUIRE(portuguese != languages.end());
    std::string error;
    REQUIRE(Data::WriteTextFile(MuEditor::Monsters::NpcNameFiles::GetFile(tools.Folder(), *portuguese),
                                "145\t1\t\"Centuriao\"\r\n175\t1\t\"Centuriao\"\r\n336\t1\t\"Centuriao\"\r\nend\r\n",
                                error));
    MuEditor::Monsters::NpcNameFiles::Import(tools.Folder(), "import", std::span(&*portuguese, 1));

    CHECK(g_MonsterEdits.ReadSharedValues(145)->names.Get("pt") == "Centuriao");
    for (const int number : {145, 175, 336})
    {
        CHECK(Entry(number).names.Get("pt") == "Centuriao");
        CHECK(OwnValues::GetNumberState(number, MonsterValueField::Names) == OwnValues::State::Shared);
    }
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "import");
}

TEST_CASE("Monster checks: generic own attacks of a monster with attacks, and map variants where no number "
          "appears [editor][monsters]")
{
    GroupedTools tools;
    std::vector<ItemDataIssue> issues;
    MuEditor::Monsters::GroupChecks::Validate(issues);
    // The monster has no attacks: 336's generic pattern is no difference that
    // plays differently.
    CHECK(issues.empty());

    MonsterDefinition values = *g_MonsterEdits.ReadSharedValues(145);
    values.attacks.emplace();
    Data::Monsters::MonsterMapVariant variant;
    variant.maps = {{50, 0}};
    variant.attacks.emplace();
    values.mapVariants = {variant};
    REQUIRE(g_MonsterEdits.SetSharedValues("attacks", 145, values));
    MonsterDefinition own = Entry(175);
    variant.maps = {{25, 0}, {30, 0}};
    own.mapVariants = {variant};
    REQUIRE(g_MonsterEdits.SetMonster("own variant", own));

    MuEditor::Monsters::GroupChecks::Validate(issues);
    REQUIRE(issues.size() == 3);
    const auto about = [&](int number, const char* field)
    {
        return std::ranges::find_if(issues, [&](const ItemDataIssue& issue)
                                    { return issue.number == number && issue.field == field; });
    };
    REQUIRE(about(145, "monster 145 mapVariants[0].attacks") != issues.end());
    CHECK(about(145, "monster 145 mapVariants[0].attacks")->message.find("no number of Death Centurion") !=
          std::string::npos);
    REQUIRE(about(175, "monster 175 mapVariants[0].attacks") != issues.end());
    CHECK(about(175, "monster 175 mapVariants[0].attacks")->message.find("the number does not appear") !=
          std::string::npos);
    REQUIRE(about(336, "monster 336 attacks") != issues.end());
    CHECK(about(336, "monster 336 attacks")->message.find("(P26)") != std::string::npos);
    CHECK(std::ranges::all_of(issues, [](const ItemDataIssue& issue)
                              { return issue.severity == Data::Items::ItemDataIssueSeverity::Warning; }));

    // Data Checks opens the shared variant on the monster as a whole, where
    // it is changed, and a number's own one on the number.
    const std::unique_ptr<MuEditor::DataChecks::DataCheckArea> area = MuEditor::Monsters::CreateMonsterCheckArea();
    area->Open(*about(145, "monster 145 mapVariants[0].attacks"), false);
    CHECK(g_MonsterSelection.IsWholeMonster());
    area->Open(*about(175, "monster 175 mapVariants[0].attacks"), false);
    CHECK_FALSE(g_MonsterSelection.IsWholeMonster());
    CHECK(g_MonsterSelection.GetNumber() == 175);

    // With variants of its own on the first number, a number that plays the
    // shared ones names them.
    MonsterDefinition first = Entry(145);
    first.mapVariants.clear();
    REQUIRE(g_MonsterEdits.SetMonster("no variants", first));
    issues.clear();
    MuEditor::Monsters::GroupChecks::Validate(issues);
    CHECK(about(336, "monster 336 mapVariants[0].attacks") != issues.end());
    CHECK(about(145, "monster 145 mapVariants[0].attacks") == issues.end());
}

TEST_CASE("Monster checks: the shipped list warns about the 13 numbers of P26 and Erkanne's map variant "
          "[editor][monsters]")
{
    // Read only: the client folder of the sources.
    ClientDataScope scope(std::filesystem::path(MU_TEST_DATA_DIR).parent_path());
    std::vector<ItemDataIssue> issues;
    REQUIRE(g_MonsterDataHandler.Revert(issues));
    g_MonsterEdits.OnDataReplaced();
    issues.clear();
    MuEditor::Monsters::GroupChecks::Validate(issues);

    std::vector<int> generic;
    std::vector<int> variants;
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.number << ": " << issue.message);
        (issue.message.find("(P26)") != std::string::npos ? generic : variants).push_back(issue.number);
    }
    // In the order of their monsters' first numbers.
    std::ranges::sort(generic);
    CHECK(generic == std::vector{331, 332, 335, 336, 337, 338, 434, 435, 436, 437, 438, 439, 531});
    // On map 79, where OpenMU places it nowhere: the attacks PR kept the
    // code's case.
    CHECK(variants == std::vector{507});
}

#endif // _EDITOR
