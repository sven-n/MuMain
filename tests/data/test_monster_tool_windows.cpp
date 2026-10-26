#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/MuEditorCore.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "UI/Common/MuEditorLayout.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/ToolGroups.h"
#include "UI/Common/UndoTarget.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterAttacksUI.h"
#include "UI/MonsterTools/MuMonsterBrowserUI.h"
#include "UI/MonsterTools/MuMonsterChecksUI.h"
#include "UI/MonsterTools/MuMonsterModelUI.h"
#include "UI/MonsterTools/MuMonsterPreviewUI.h"
#include "UI/MonsterTools/MuMonsterTranslationsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <array>

using Data::Monsters::MonsterDefinition;
using MuEditor::Monsters::MonsterSelection;
namespace WindowIds = MuEditor::WindowIds;

namespace
{
constexpr ImVec2 EditorWindowSize(1200.0f, 800.0f);

MonsterDefinition Monster(int number, const char* english)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    return monster;
}

void RenderMonsterTools()
{
    bool open = true;
    g_MuMonsterBrowserUI.Render(&open);
    g_MuMonsterModelUI.Render(&open);
    g_MuMonsterPreviewUI.Render(&open);
    g_MuMonsterAttacksUI.Render(&open);
    g_MuMonsterTranslationsUI.Render(&open);
    g_MuMonsterChecksUI.Render(&open);
}

// The editor window, which holds the dock space of the tool windows.
void BeginEditorWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(EditorWindowSize);
    ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoDocking);
}

ImGuiID DockIdOf(const char* windowId)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(windowId);
    REQUIRE(window != nullptr);
    return window->DockId;
}
} // namespace

// Draws the monster tools for a few frames, with a monster, a name that holds
// a format and an ImGui id, a number without an entry and no selection: ImGui
// counts what the windows get wrong (Begin and End out of step) as errors.
TEST_CASE("The monster tools draw their frames without ImGui errors [editor][monsters]")
{
    const std::array monsters = {Monster(2, "Budge Dragon"), Monster(9, "100%s ##Lich")};
    g_MonsterDatabase.Build(monsters, "en");
    TestImGuiContext context;

    constexpr int Frames = 5;
    for (const int number : {2, 9, 76, MonsterSelection::NoMonster})
    {
        g_MonsterSelection.Select(number);
        for (int frame = 0; frame < Frames; ++frame)
        {
            ImGui::NewFrame();
            RenderMonsterTools();
            ImGui::Render();
            INFO("monster " << number << ", frame " << frame);
            CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        }
    }
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterBrowser) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterTranslations) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterChecks) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterModel) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterPreview) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterAttacks) != nullptr);
}

// Monster Attacks with attacks of each kind (skill tables, a draw, a skill
// 0x69 plays no attack for), a monster without attacks, a player-shaped one
// and one that no handler reads. The test binary opens no models: the tool
// offers the named animations.
TEST_CASE("The Monster Attacks window draws attacks without ImGui errors and stores nothing [editor][monsters]")
{
    MonsterDefinition selupan = Monster(MONSTER_SELUPAN, "Selupan");
    selupan.model = "MONSTER_MODEL_SELUPAN";
    selupan.attacks.emplace();
    selupan.attacks->monsterSkills = {
        {34, MONSTER01_ATTACK2}, {41, Data::Monsters::NoAttackAnimation}, {90, MONSTER01_APEAR}};
    selupan.attacks->skills = {{12, MONSTER01_ATTACK3}};
    selupan.attacks->otherwise = MONSTER01_ATTACK4;
    selupan.attacks->clearSkill = Data::Monsters::MonsterSkillClear::AfterAttack;
    MonsterDefinition bali = Monster(MONSTER_BALI, "Bali");
    bali.model = "MONSTER_MODEL_BALI";
    bali.attacks.emplace();
    bali.attacks->otherwiseRandom = Data::Monsters::MonsterAttackRandom::Bali;
    bali.attacks->sound = "SOUND_TRAP01";
    bali.attacks->soundAtMonster = true;
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    budge.attacks.emplace();
    MonsterDefinition player = Monster(4, "Guard");
    player.model = "player";
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    const std::array monsters = {budge, hound, player, bali, selupan};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;

    constexpr int Frames = 5;
    for (const int number : std::array<int, 7>{MONSTER_SELUPAN, MONSTER_BALI, 2, 3, 4, 76, MonsterSelection::NoMonster})
    {
        g_MonsterSelection.Select(number);
        for (int frame = 0; frame < Frames; ++frame)
        {
            ImGui::NewFrame();
            bool open = true;
            g_MuMonsterAttacksUI.Render(&open);
            ImGui::Render();
            INFO("monster " << number << ", frame " << frame);
            CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        }
    }
    CHECK(g_MonsterDatabase.Find(MONSTER_SELUPAN)->attacks == selupan.attacks);
    CHECK_FALSE(g_MonsterDatabase.Find(3)->attacks.has_value());
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
}

// The model tools with a monster whose model has an entry with sounds and
// sound slots, one whose model is outside the model list, and one without a
// model. The test binary opens no models: the animations and the preview
// say that the model is not open.
TEST_CASE("The Monster Model and Monster Preview windows draw a model entry without ImGui errors [editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    MonsterDefinition player = Monster(4, "Guard");
    player.model = "player";
    const std::array monsters = {Monster(2, "Budge Dragon"), hound, player};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.load.speeds = {{2, 0.34f}};
    model.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}, {"Data/Sound/mHound2.wav", "", 2, true}};
    model.soundSlots.emplace();
    model.soundSlots->first = {"Data/Sound/mHound1.wav", "Data/Sound/mHound2.wav", "", "", ""};
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    walk.frames = std::pair<float, float>{1.0f, 2.0f};
    walk.chance = Data::Monsters::MonsterSoundChance{20, Data::Monsters::MonsterSoundDraw::Rand};
    walk.sounds = {"Data/Sound/mHound1.wav"};
    Data::Monsters::MonsterSoundEvent attack;
    attack.animations = {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    attack.pick = Data::Monsters::MonsterSoundDraw::Rand;
    attack.sounds = {"Data/Sound/mHound1.wav", "SOUND_THUNDERS01"};
    model.soundEvents = {walk, attack};
    model.soundEventWorlds = {69, 70};
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    TestImGuiContext context;

    constexpr int Frames = 5;
    for (const int number : {3, 4, 2, MonsterSelection::NoMonster})
    {
        g_MonsterSelection.Select(number);
        for (int frame = 0; frame < Frames; ++frame)
        {
            ImGui::NewFrame();
            bool open = true;
            g_MuMonsterModelUI.Render(&open);
            g_MuMonsterPreviewUI.Render(&open);
            ImGui::Render();
            INFO("monster " << number << ", frame " << frame);
            CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        }
    }
    // The model tool shows the entry: nothing was stored.
    CHECK(g_MonsterModelDatabase.Find(MODEL_HOUND)->load.speeds == model.load.speeds);
    g_MonsterModelDatabase.Build({});
}

TEST_CASE("The Monster Model window leaves a sound event's frame range under 0.1 as it is until it is edited "
          "[editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    const std::array monsters = {hound};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}};
    Data::Monsters::MonsterSoundEvent walk;
    walk.animations = {MONSTER01_WALK};
    // Narrower than the inputs keep a range they edit; the file allows it.
    walk.frames = std::pair<float, float>{7.0f, 7.05f};
    walk.sounds = {"Data/Sound/mHound1.wav"};
    model.soundEvents = {walk};
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    TestImGuiContext context;
    g_MonsterSelection.Select(3);

    for (int frame = 0; frame < 3; ++frame)
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterModelUI.Render(&open);
        ImGui::Render();
        INFO("frame " << frame);
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        // Before, the draft held [7, 7.1] from the first frame on: the
        // preview played it, and the next stored change of the model saved it.
        const Data::Monsters::MonsterModelDefinition* draft = g_MuMonsterModelUI.GetShownDraft(MODEL_HOUND);
        REQUIRE(draft != nullptr);
        REQUIRE(draft->soundEvents.size() == 1);
        CHECK(draft->soundEvents[0].frames == walk.frames);
    }
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
}

TEST_CASE("The first layout docks the monster browser beside the item browser, its tools with the others "
          "[editor][monsters]")
{
    TestImGuiContext context;
    const auto frame = []
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics, WindowIds::MonsterBrowser,
                                   WindowIds::MonsterModel, WindowIds::MonsterPreview, WindowIds::MonsterAttacks,
                                   WindowIds::MonsterTranslations, WindowIds::MonsterChecks})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::MonsterBrowser) == DockIdOf(WindowIds::ItemBrowser));
    CHECK(DockIdOf(WindowIds::MonsterTranslations) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterChecks) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterModel) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterPreview) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterAttacks) == DockIdOf(WindowIds::ItemBasics));
    CHECK(DockIdOf(WindowIds::MonsterBrowser) != DockIdOf(WindowIds::ItemBasics));
}

// A layout saved before the monster tools: the item browser beside the item
// tools above the console. A monster window opened the first time docks with
// the item windows: the browser with the item browser also after a monster
// tool found its place.
TEST_CASE("A monster window opened the first time in a layout from before the monster tools docks with the item "
          "windows [editor][monsters]")
{
    TestImGuiContext context;
    ImGuiID browser = 0;
    ImGuiID tools = 0;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        const ImGuiID dockSpace = ImGui::GetID("ToolWindows");
        if (ImGui::DockBuilderGetNode(dockSpace) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpace, ImVec2(EditorWindowSize.x, EditorWindowSize.y - 20.0f));
            ImGuiID above = 0;
            const ImGuiID console = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Down, 0.25f, nullptr, &above);
            browser = ImGui::DockBuilderSplitNode(above, ImGuiDir_Left, 0.35f, nullptr, &tools);
            ImGui::DockBuilderDockWindow(WindowIds::Console, console);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBrowser, browser);
            ImGui::DockBuilderDockWindow(WindowIds::ItemBasics, tools);
            ImGui::DockBuilderFinish(dockSpace);
        }
        // The node is there: it is not built again.
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        for (const char* window : {WindowIds::ItemBrowser, WindowIds::ItemBasics})
        {
            ImGui::Begin(window);
            ImGui::End();
        }
        for (const char* window : {WindowIds::MonsterTranslations, WindowIds::MonsterBrowser})
        {
            MuEditor::Layout::DockInGroup(window, MuEditor::GetMonsterTools());
            ImGui::Begin(window);
            ImGui::End();
        }
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    frame();

    CHECK(DockIdOf(WindowIds::MonsterBrowser) == browser);
    CHECK(DockIdOf(WindowIds::MonsterTranslations) == tools);
}

TEST_CASE("The editor bar's undo is the monster tools' while a monster tool is open [editor][monsters]")
{
    ToolWindowsScope windows;
    MuEditor::UndoTarget& target = MuEditor::Monsters::ToolWindow::GetUndoTarget();
    const std::array monsters = {Monster(2, "Budge Dragon")};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    REQUIRE(g_MonsterEdits.SetMonsterName("Name", 2, "en", "Budge Dragon 2"));

    CHECK_FALSE(target.IsOpen());
    g_MuEditorCore.GetToolWindows().monsterChecks = true;
    CHECK(target.IsOpen());
    CHECK(target.CanUndo());
    CHECK(target.GetUndoLabel() == "Name");

    // Changed past the history: the undo is refused (and logged).
    g_MonsterDatabase.Set(Monster(2, "Budge Dragon 3"));
    target.Undo();
    CHECK(g_MonsterDatabase.Find(2)->names.GetNeutral() == "Budge Dragon 3");
    CHECK(target.CanUndo());
}

#endif // _EDITOR
