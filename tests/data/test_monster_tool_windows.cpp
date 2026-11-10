#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/MuEditorCore.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Engine/Object/MonsterModelBody.h"
#include "I18N/All.h"
#include "UI/Common/MuEditorLayout.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/Common/ToolGroups.h"
#include "UI/Common/UndoTarget.h"
#include "UI/ItemTools/ItemToolWindow.h"
#include "UI/MonsterTools/MonsterAttackPreview.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"
#include "UI/MonsterTools/MonsterShownModel.h"
#include "UI/MonsterTools/MonsterSoundEventsEditor.h"
#include "UI/MonsterTools/MonsterToolWindow.h"
#include "UI/MonsterTools/MuMonsterAttacksUI.h"
#include "UI/MonsterTools/MuMonsterBrowserUI.h"
#include "UI/MonsterTools/MuMonsterModelUI.h"
#include "UI/MonsterTools/MuMonsterPreviewUI.h"
#include "UI/MonsterTools/MonsterSetupFields.h"
#include "UI/MonsterTools/MonsterSetupLists.h"
#include "UI/MonsterTools/MuMonsterSetupUI.h"
#include "UI/MonsterTools/MuMonsterEquipmentUI.h"
#include "UI/Common/MapPicker.h"
#include "UI/MonsterTools/MuMonsterSoundsUI.h"
#include "UI/MonsterTools/MuMonsterTranslationsUI.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <array>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

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
    g_MuMonsterSetupUI.Render(&open);
    g_MuMonsterEquipmentUI.Render(&open);
    g_MuMonsterTranslationsUI.Render(&open);
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
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterModel) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterPreview) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterAttacks) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterSetup) != nullptr);
    CHECK(ImGui::FindWindowByName(WindowIds::MonsterEquipment) != nullptr);
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
    selupan.attacks->otherwise = {MONSTER01_ATTACK4};
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

// The id of an item of Monster Attacks' map row (`variant` nullopt) or of the
// attacks shown for a map row choice (all maps or a variant's index).
constexpr int AllMaps = -1;
ImGuiID AttacksItemId(int number, std::optional<int> variant, const char* label)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterAttacks);
    REQUIRE(window != nullptr);
    ImGuiID scope = ImGui::GetIDWithSeed(number, window->ID);
    if (variant)
    {
        scope = ImGui::GetIDWithSeed(*variant, scope);
    }
    return ImGui::GetIDWithSeed(label, nullptr, scope);
}

// The id of an item in a row of Monster Attacks' monster skills (`list`
// "MonsterSkills") or player skills ("Skills").
ImGuiID AttacksRowItemId(int number, int variant, const char* list, int row, const char* label)
{
    const ImGuiID rows = AttacksItemId(number, variant, list);
    return ImGui::GetIDWithSeed(label, nullptr, ImGui::GetIDWithSeed(row, rows));
}

// Monster Attacks' choices for any other attack, clicked one after another.
TEST_CASE("Monster Attacks: any other attack plays an animation, a rotation, the generic pattern or a draw (Q-MN69) "
          "[editor][monsters]")
{
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    budge.attacks.emplace();
    budge.attacks->otherwise = {MONSTER01_ATTACK2};
    const std::array monsters = {budge};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(2);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    // As a click on the attacks of all maps: pressed in the next frame, drawn
    // as it is after that.
    const auto click = [&](const char* label)
    {
        ImGui::ActivateItemByID(AttacksItemId(2, AllMaps, label));
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(2)->attacks; };
    render();

    click(I18N::Editor::ARotation);
    CHECK(stored().otherwise == std::vector<int>{MONSTER01_ATTACK2, MONSTER01_ATTACK2});
    click(I18N::Editor::AddAnAnimation);
    CHECK(stored().otherwise == std::vector<int>{MONSTER01_ATTACK2, MONSTER01_ATTACK2, MONSTER01_ATTACK2});
    click(I18N::Editor::TheGenericPattern);
    CHECK(stored().otherwiseGeneric);
    // Back to the rotation: all three turns are still there.
    click(I18N::Editor::ARotation);
    CHECK_FALSE(stored().otherwiseGeneric);
    CHECK(stored().otherwise == std::vector<int>{MONSTER01_ATTACK2, MONSTER01_ATTACK2, MONSTER01_ATTACK2});
    click(I18N::Editor::ADraw);
    CHECK_FALSE(stored().otherwiseGeneric);
    CHECK(stored().otherwiseRandom == Data::Monsters::MonsterAttackRandom::Rand);
    click(I18N::Editor::AnAnimation);
    CHECK_FALSE(stored().otherwiseRandom);
    CHECK_FALSE(stored().otherwiseGeneric);
    CHECK(stored().otherwise == std::vector<int>{MONSTER01_ATTACK2});
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

// Monster Attacks' map row: a map variant starts with the monster's attacks,
// its attacks are edited apart, and it goes as a whole.
TEST_CASE("Monster Attacks: a map variant is added with the monster's attacks, edited apart and removed (Q-MN65) "
          "[editor][monsters]")
{
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    budge.attacks.emplace();
    budge.attacks->otherwise = {MONSTER01_ATTACK2};
    const std::array monsters = {budge};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(2);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(2); };
    render();

    click(AttacksItemId(2, std::nullopt, I18N::Editor::AddAMapVariant));
    REQUIRE(stored().mapVariants.size() == 1);
    CHECK(stored().mapVariants[0].maps.empty());
    CHECK(stored().mapVariants[0].attacks == budge.attacks);

    // The new variant is shown: its own attacks change.
    click(AttacksItemId(2, 0, I18N::Editor::TheGenericPattern));
    REQUIRE(stored().mapVariants[0].attacks);
    CHECK(stored().mapVariants[0].attacks->otherwiseGeneric);
    CHECK(stored().attacks == budge.attacks);

    // Data Checks shows the attacks a problem is about: all maps, then the
    // variant again.
    g_MuMonsterAttacksUI.ShowMapVariant(2, std::nullopt);
    render();
    click(AttacksItemId(2, AllMaps, I18N::Editor::ADraw));
    CHECK(stored().attacks->otherwiseRandom.has_value());
    CHECK_FALSE(stored().mapVariants[0].attacks->otherwiseRandom.has_value());
    g_MuMonsterAttacksUI.ShowMapVariant(2, 0);
    render();
    click(AttacksItemId(2, 0, I18N::Editor::ADraw));
    CHECK(stored().mapVariants[0].attacks->otherwiseRandom.has_value());

    click(AttacksItemId(2, 0, I18N::Editor::RemoveTheMapVariant));
    CHECK(stored().mapVariants.empty());
    CHECK(stored().attacks->otherwiseRandom.has_value());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

// The first rows of the monster skills and of the player skills are items of
// their own: before, they had one id, and the second row's Remove did nothing.
TEST_CASE("Monster Attacks: a player skill row is removed while the monster skills have a row too [editor][monsters]")
{
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    budge.attacks.emplace();
    budge.attacks->monsterSkills = {{34, MONSTER01_ATTACK2}};
    budge.attacks->skills = {{12, MONSTER01_ATTACK3}};
    const std::array monsters = {budge};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(2);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(2)->attacks; };
    render();

    click(AttacksRowItemId(2, AllMaps, "Skills", 0, I18N::Editor::Remove));
    CHECK(stored().skills.empty());
    CHECK(stored().monsterSkills == budge.attacks->monsterSkills);
    click(AttacksRowItemId(2, AllMaps, "MonsterSkills", 0, I18N::Editor::Remove));
    CHECK(stored().monsterSkills.empty());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

TEST_CASE("Monster Attacks: Play next attack plays a rotation in turn and the generic pattern as the game counts "
          "(D48) [editor][monsters]")
{
    using MuEditor::Monsters::AttackPreview::Counts;
    using MuEditor::Monsters::AttackPreview::NextOtherwise;
    Data::Monsters::MonsterAttacks attacks;
    attacks.otherwise = {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK3};
    // Skill entries are no other attacks.
    attacks.monsterSkills = {{0, MONSTER01_ATTACK4}};
    attacks.skills = {{0, MONSTER01_ATTACK5}};
    Counts counts;
    for (const int expected : {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK3, MONSTER01_ATTACK2})
    {
        CHECK(NextOtherwise(attacks, counts) == expected);
    }
    CHECK(counts.rotation == 1);

    // SetPlayerAttack counts each attack, the pattern its own: attack 1, 2, 2
    // as the Fire Golem plays in Devil Square.
    attacks.otherwiseGeneric = true;
    counts = {};
    for (const int expected :
         {MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK2})
    {
        CHECK(NextOtherwise(attacks, counts) == expected);
    }
    CHECK(counts.swordCount == 10);
    CHECK(counts.rotation == 0);

    // A draw picks one of its animations.
    attacks.otherwiseGeneric = false;
    attacks.otherwiseRandom = Data::Monsters::MonsterAttackRandom::Bali;
    for (int draw = 0; draw < 20; ++draw)
    {
        const int animation = NextOtherwise(attacks, counts);
        CHECK(animation >= MONSTER01_ATTACK1);
        CHECK(animation <= MONSTER01_ATTACK4);
    }
    attacks.otherwiseRandom.reset();
    attacks.otherwise = {Data::Monsters::NoAttackAnimation};
    CHECK(NextOtherwise(attacks, counts) == Data::Monsters::NoAttackAnimation);
}

// The test binary opens no models: Play opens the Monster Preview, which says
// so, beside Monster Attacks; Play next attack says what it played.
TEST_CASE("Monster Attacks: Play opens the Monster Preview beside it, Play next attack goes through the rotation and "
          "starts again (D48) [editor][monsters]")
{
    const ToolWindowsScope windows;
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.attacks.emplace();
    hound.attacks->skills = {{12, MONSTER01_ATTACK3}, {13, Data::Monsters::NoAttackAnimation}};
    hound.attacks->otherwise = {MONSTER01_ATTACK2, MONSTER01_ATTACK1};
    const std::array monsters = {hound};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    TestImGuiContext context;
    g_MonsterSelection.Select(3);
    // In the editor window's first layout, as the editor draws it.
    const auto render = []
    {
        ImGui::NewFrame();
        BeginEditorWindow();
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        bool open = true;
        if (g_MuEditorCore.GetToolWindows().monsterPreview)
        {
            g_MuMonsterPreviewUI.Render(&open);
        }
        g_MuMonsterAttacksUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    render();
    click(AttacksItemId(3, AllMaps, I18N::Editor::FromTheStart));
    CHECK_FALSE(g_MuMonsterAttacksUI.GetLastPlayed());

    // No animation, nothing to play.
    click(AttacksRowItemId(3, AllMaps, "Skills", 1, I18N::Editor::Play));
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().monsterPreview);
    click(AttacksRowItemId(3, AllMaps, "Skills", 0, I18N::Editor::Play));
    CHECK(g_MuEditorCore.GetToolWindows().monsterPreview);
    // In the first layout both are tabs of one node: the preview goes beside,
    // shown from its second frame.
    render();
    CHECK(DockIdOf(WindowIds::MonsterPreview) != DockIdOf(WindowIds::MonsterAttacks));
    CHECK_FALSE(ImGui::FindWindowByName(WindowIds::MonsterAttacks)->Hidden);
    CHECK_FALSE(ImGui::FindWindowByName(WindowIds::MonsterPreview)->Hidden);

    for (const int expected : {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK2})
    {
        click(AttacksItemId(3, AllMaps, I18N::Editor::PlayNextAttack));
        CHECK(g_MuMonsterAttacksUI.GetLastPlayed() == expected);
    }
    click(AttacksItemId(3, AllMaps, I18N::Editor::FromTheStart));
    CHECK_FALSE(g_MuMonsterAttacksUI.GetLastPlayed());
    click(AttacksItemId(3, AllMaps, I18N::Editor::PlayNextAttack));
    CHECK(g_MuMonsterAttacksUI.GetLastPlayed() == MONSTER01_ATTACK2);

    // Playing stores nothing.
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
}

// The model tools with a monster whose model has an entry with cues and
// whose sound set has events at moments, in animations and on cues, a monster
// with a random variant (Q-MN83), one whose model is outside the model list,
// and one without a model. The test binary opens no models: the animations and
// the preview say that the model is not open.
TEST_CASE("The Monster Model, Monster Preview and Monster Sounds windows draw without ImGui errors (Q-MN78) "
          "[editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    MonsterDefinition elf = Monster(163, "Chaos Castle 2");
    elf.model = "MONSTER_MODEL_CHAOSCASTLE_ELF";
    elf.soundSet = "Hound";
    elf.randomVariants = {{"MONSTER_MODEL_CHAOSCASTLE_WIZARD", std::string("Wizard")}};
    MonsterDefinition player = Monster(4, "Guard");
    player.model = "player";
    const std::array monsters = {Monster(2, "Budge Dragon"), hound, player, elf};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.load.speeds = {{2, 0.34f}};
    model.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    model.timers = {{"hair swing", MONSTER01_WALK, 500}};
    Data::Monsters::MonsterModelDefinition wizard = model;
    wizard.slot = "MONSTER_MODEL_CHAOSCASTLE_WIZARD";
    wizard.cues.clear();
    Data::Monsters::MonsterSoundSet set;
    set.name = "Hound";
    set.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}, {"Data/Sound/mHound2.wav", "", 2, true}};
    Data::Monsters::MonsterSoundEvent idle;
    idle.moments = {Data::Monsters::MonsterSoundMoment::Stand, Data::Monsters::MonsterSoundMoment::Walk};
    idle.heardAnywhere = true;
    idle.atMonster = true;
    idle.chance = Data::Monsters::MonsterSoundChance{16, Data::Monsters::MonsterSoundDraw::RandFps};
    idle.pick = Data::Monsters::MonsterSoundDraw::Rand;
    idle.sounds = {"Data/Sound/mHound1.wav", "Data/Sound/mHound2.wav"};
    Data::Monsters::MonsterSoundEvent step;
    step.cues = {"step 1"};
    step.chance = Data::Monsters::MonsterSoundChance{20, Data::Monsters::MonsterSoundDraw::Rand};
    step.sounds = {"Data/Sound/mHound1.wav"};
    // A cue the elf's model lacks.
    Data::Monsters::MonsterSoundEvent missing;
    missing.cues = {"step 9"};
    missing.sounds = {"Data/Sound/mHound2.wav"};
    Data::Monsters::MonsterSoundEvent attack;
    attack.animations = {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    attack.pick = Data::Monsters::MonsterSoundDraw::Rand;
    attack.sounds = {"Data/Sound/mHound1.wav", "SOUND_THUNDERS01"};
    set.soundEvents = {idle, step, missing, attack};
    set.maps = {{69}, {70}};
    Data::Monsters::MonsterSoundSet wizardSet = set;
    wizardSet.name = "Wizard";
    const std::array models = {model, wizard};
    g_MonsterModelDatabase.Build(models);
    const std::array sets = {set, wizardSet};
    g_MonsterSoundSetDatabase.Build(sets);
    TestImGuiContext context;

    constexpr int Frames = 5;
    for (const int number : {3, 163, 4, 2, MonsterSelection::NoMonster})
    {
        g_MonsterSelection.Select(number);
        for (const int variant : {-1, 0})
        {
            MuEditor::Monsters::ShownModel::SetShownVariant(number, variant);
            for (int frame = 0; frame < Frames; ++frame)
            {
                ImGui::NewFrame();
                bool open = true;
                g_MuMonsterModelUI.Render(&open);
                g_MuMonsterPreviewUI.Render(&open);
                g_MuMonsterSoundsUI.Render(&open);
                ImGui::Render();
                INFO("monster " << number << ", variant " << variant << ", frame " << frame);
                CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
            }
        }
    }
    // The tools show the entries: nothing was stored.
    CHECK(g_MonsterModelDatabase.Find(MODEL_HOUND)->load.speeds == model.load.speeds);
    CHECK(*g_MonsterSoundSetDatabase.Find("Hound") == set);
    g_MonsterModelDatabase.Build({});
    g_MonsterSoundSetDatabase.Build({});
}

TEST_CASE("Monster Sounds gives Monster Preview the model it edits in the frame it showed it and the next, as "
          "Monster Preview draws first (Q-MN81) [editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    const std::array monsters = {hound};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    Data::Monsters::MonsterSoundSet set;
    set.name = "Hound";
    const std::array sets = {set};
    g_MonsterSoundSetDatabase.Build(sets);
    TestImGuiContext context;
    g_MonsterSelection.Select(3);

    ImGui::NewFrame();
    bool open = true;
    g_MuMonsterSoundsUI.Render(&open);
    const Data::Monsters::MonsterModelDefinition* shown = g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND);
    REQUIRE(shown != nullptr);
    CHECK(shown->cues == model.cues);
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_BULL_FIGHTER) == nullptr);
    ImGui::Render();
    // The next frame, before the window draws again: Monster Preview draws
    // first and marks the draft's cues.
    ImGui::NewFrame();
    REQUIRE(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) == shown);
    CHECK(g_MuMonsterPreviewUI.GetCues(MODEL_HOUND).size() == 1);
    ImGui::Render();
    // A frame later without the window: its draft is old.
    ImGui::NewFrame();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) == nullptr);
    ImGui::Render();
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
    g_MonsterSoundSetDatabase.Build({});
}

TEST_CASE("Monster Sounds and Monster Preview show a frame alike, as a new cue is named [editor][monsters]")
{
    using MuEditor::Monsters::SoundEventsEditor::FrameText;
    CHECK(FrameText(4.5f) == "4.50");
    CHECK(FrameText(0.0f) == "0.00");
    CHECK(FrameText(12.126f) == "12.13");
}

TEST_CASE("Monster Sounds and Monster Preview keep a cue's frames at 0 or later and 0.1 frames long "
          "[editor][monsters]")
{
    using MuEditor::Monsters::SoundEventsEditor::KeepCueFrames;
    std::pair<float, float> frames{-1.0f, 0.5f};
    KeepCueFrames(frames);
    CHECK(frames == std::pair(0.0f, 0.5f));
    frames = {2.0f, 2.05f};
    KeepCueFrames(frames);
    CHECK(frames.first == 2.0f);
    CHECK(frames.second == doctest::Approx(2.1f));
    frames = {3.0f, 1.0f};
    KeepCueFrames(frames);
    CHECK(frames.second == doctest::Approx(3.1f));
    frames = {3.0f, 5.0f};
    KeepCueFrames(frames);
    CHECK(frames == std::pair(3.0f, 5.0f));
}

TEST_CASE("Monster Model: a cue that no sound event of the model's monsters plays on can be removed "
          "[editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    // A set that no monster of the model plays, on the second cue.
    MonsterDefinition other = Monster(4, "Budge Dragon");
    other.model = "MONSTER_MODEL_BUDGE_DRAGON";
    other.soundSet = "Budge Dragon";
    const std::array monsters = {hound, other};
    g_MonsterDatabase.Build(monsters, "en");
    std::vector<Data::Monsters::MonsterSoundSet> sets(2);
    sets[0].name = "Hound";
    sets[0].soundEvents.resize(1);
    sets[0].soundEvents[0].cues = {"step 1"};
    sets[1].name = "Budge Dragon";
    sets[1].soundEvents.resize(1);
    sets[1].soundEvents[0].cues = {"walk 4.50"};
    g_MonsterSoundSetDatabase.Build(sets);
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}},
                  {"walk 4.50", MONSTER01_WALK, {4.5f, 5.5f}},
                  {"step 2", MONSTER01_WALK, {7.0f, 8.0f}}};

    CHECK(MuEditor::Monsters::SoundEventsEditor::FindUnusedCues(model, MODEL_HOUND) ==
          std::vector<std::string>{"walk 4.50", "step 2"});
    sets[0].soundEvents[0].cues = {"step 2", "step 1"};
    g_MonsterSoundSetDatabase.Build(sets);
    CHECK(MuEditor::Monsters::SoundEventsEditor::FindUnusedCues(model, MODEL_HOUND) ==
          std::vector<std::string>{"walk 4.50"});
    g_MonsterSoundSetDatabase.Build({});
    g_MonsterDatabase.Build({}, "en");
}

TEST_CASE("Monster Sounds: Add a file says why it cannot add a sound, also for an id that is not a SOUND_ id "
          "[editor][monsters]")
{
    using MuEditor::Monsters::FindNewSoundProblem;
    Data::Monsters::MonsterSoundSet set;
    set.name = "Hound";
    set.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}};
    const auto sound = [](const char* file, const char* id)
    {
        Data::Monsters::MonsterSound added;
        added.file = file;
        added.id = id;
        return added;
    };
    CHECK(FindNewSoundProblem(set, sound("Data/Sound/mHound2.wav", "")).empty());
    CHECK(FindNewSoundProblem(set, sound("Data/Sound/mHound2.wav", "SOUND_THUNDERS01")).empty());
    CHECK(FindNewSoundProblem(set, sound("", "")) == I18N::Editor::SoundFileMissing);
    CHECK(FindNewSoundProblem(set, sound("../mHound2.wav", "")) != "");
    CHECK(FindNewSoundProblem(set, sound("Data/Sound/mHound1.wav", "")) == I18N::Editor::SoundFileTwice);
    CHECK(FindNewSoundProblem(set, sound("Data/Sound/mHound2.wav", "SOUND_HOUND_TYPO")) ==
          I18N::Editor::SoundIdUnknown);
    set.sounds.push_back(sound("Data/Sound/mHound3.wav", "SOUND_THUNDERS01"));
    CHECK(FindNewSoundProblem(set, sound("Data/Sound/mHound4.wav", "SOUND_THUNDERS01")) ==
          I18N::Editor::SoundNameTwice);
}

TEST_CASE("Monster Sounds' drafts keep the rows' events and cues for the rest of the frame after a store, and "
          "take the data back the next frame [editor][monsters]")
{
    using namespace MuEditor::Monsters::SoundEventsEditor;
    TestImGuiContext context;
    Data::Monsters::MonsterSoundSet storedSet;
    storedSet.name = "Hound";
    storedSet.soundEvents.resize(2);
    int setStores = 0;
    SetDraft setDraft = MakeSetDraft(
        [&](int id) { return id == 0 ? std::optional<Data::Monsters::MonsterSoundSet>(storedSet) : std::nullopt; },
        [&](const std::string&, const Data::Monsters::MonsterSoundSet& set)
        {
            storedSet = set;
            ++setStores;
        });
    Data::Monsters::MonsterModelDefinition storedModel;
    storedModel.slot = "MONSTER_MODEL_HOUND";
    storedModel.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}, {"step 2", MONSTER01_WALK, {7.0f, 8.0f}}};
    ModelDraft modelDraft = MakeModelDraft(
        [&](int slot)
        {
            return slot == MODEL_HOUND ? std::optional<Data::Monsters::MonsterModelDefinition>(storedModel)
                                       : std::nullopt;
        },
        [&](const std::string&, const Data::Monsters::MonsterModelDefinition& model) { storedModel = model; });

    ImGui::NewFrame();
    Data::Monsters::MonsterSoundSet* set = setDraft.Begin(0, 1);
    Data::Monsters::MonsterModelDefinition* model = modelDraft.Begin(MODEL_HOUND, 1);
    REQUIRE(set != nullptr);
    REQUIRE(model != nullptr);
    // A row ticks a box and draws on with its event; a cue row picks an
    // animation and draws on with its cue.
    Data::Monsters::MonsterSoundEvent& event = set->soundEvents[1];
    event.heardAnywhere = true;
    setDraft.AfterChange(true, "Hound: sound events");
    Data::Monsters::MonsterModelCue& cue = model->cues[1];
    cue.animation = MONSTER01_STOP1;
    modelDraft.AfterChange(true, "Hound: cue step 2");
    CHECK(setStores == 1);
    CHECK(storedSet.soundEvents[1].heardAnywhere);
    CHECK(storedModel.cues[1].animation == MONSTER01_STOP1);
    REQUIRE(&set->soundEvents[1] == &event);
    REQUIRE(&model->cues[1] == &cue);
    CHECK(event.heardAnywhere);
    CHECK(cue.animation == MONSTER01_STOP1);
    setDraft.End();
    modelDraft.End();
    ImGui::Render();

    // The next frame takes the data back, also what the store changed.
    storedSet.soundEvents[0].atMonster = true;
    ImGui::NewFrame();
    set = setDraft.Begin(0, 1);
    REQUIRE(set != nullptr);
    CHECK(set->soundEvents[0].atMonster);
    CHECK(set->soundEvents[1].heardAnywhere);
    setDraft.End();
    ImGui::Render();
}

TEST_CASE("Monster Sounds edits the cues of the shown look's model only with the set the look plays "
          "[editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    MonsterDefinition elf = Monster(163, "Chaos Castle 2");
    elf.model = "MONSTER_MODEL_CHAOSCASTLE_ELF";
    elf.soundSet = "Elf";
    elf.randomVariants = {{"MONSTER_MODEL_CHAOSCASTLE_WIZARD", std::string("Wizard")}};
    const std::array monsters = {hound, elf};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    Data::Monsters::MonsterModelDefinition elfModel = model;
    elfModel.slot = "MONSTER_MODEL_CHAOSCASTLE_ELF";
    const std::array models = {model, elfModel};
    g_MonsterModelDatabase.Build(models);
    std::vector<Data::Monsters::MonsterSoundSet> sets(3);
    sets[0].name = "Hound";
    sets[1].name = "Elf";
    sets[2].name = "Wizard";
    g_MonsterSoundSetDatabase.Build(sets);
    TestImGuiContext context;
    const auto draw = [&]
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSoundsUI.Render(&open);
        ImGui::Render();
    };

    g_MonsterSelection.Select(3);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) != nullptr);
    SUBCASE("another monster's set picked in the list")
    {
        g_MuMonsterSoundsUI.ShowEvent("Elf", std::nullopt);
        draw();
        CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) == nullptr);
        g_MuMonsterSoundsUI.ShowEvent("Hound", std::nullopt);
        draw();
        CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) != nullptr);
    }
    SUBCASE("a variant's set opened while the entry's look is shown")
    {
        g_MonsterSelection.Select(163);
        MuEditor::Monsters::ShownModel::SetShownVariant(163, -1);
        draw();
        const int elfSlot = MODEL_CHAOS_CASTLE_ELF;
        CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(elfSlot) != nullptr);
        g_MuMonsterSoundsUI.ShowEvent("Wizard", std::nullopt);
        draw();
        CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(elfSlot) == nullptr);
    }
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
    g_MonsterSoundSetDatabase.Build({});
    g_MonsterDatabase.Build({}, "en");
}

TEST_CASE("Monster Sounds shows the set Data Checks or a cue tag asks for, also after the selection changed while it "
          "was closed [editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    const std::array monsters = {hound};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    std::vector<Data::Monsters::MonsterSoundSet> sets(2);
    sets[0].name = "Hound";
    sets[1].name = "Raymond";
    g_MonsterSoundSetDatabase.Build(sets);
    TestImGuiContext context;
    const auto draw = [&]
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSoundsUI.Render(&open);
        ImGui::Render();
    };

    // The Hound is selected while the tool is not drawn, then a problem of
    // Raymond's set is clicked: the tool shows Raymond's set, not the Hound's
    // (whose model draft it would show).
    g_MonsterSelection.Select(3);
    g_MuMonsterSoundsUI.ShowEvent("Raymond", std::nullopt);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) == nullptr);
    // A selection after that shows the monster's set again.
    g_MonsterSelection.Select(3);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) != nullptr);

    // A set that is gone (its adding undone) gives way to the monster's set.
    // The step names no monster, so the undo selects none.
    g_MonsterEdits.GetHistory().Clear();
    Data::Monsters::MonsterSoundSet added;
    added.name = "New sound set";
    REQUIRE(g_MonsterEdits.SetSoundSet("Add", added, -1));
    g_MuMonsterSoundsUI.ShowEvent(added.name, std::nullopt);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) == nullptr);
    REQUIRE(g_MonsterEdits.Undo());
    REQUIRE(g_MonsterSoundSetDatabase.Find(added.name) == nullptr);
    draw();
    CHECK(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND) != nullptr);
    g_MonsterEdits.GetHistory().Clear();

    // Monster Preview does not show the Hound's model (it is not open), and
    // a frame later not at all: its animation and frame are no cue's to take.
    ImGui::NewFrame();
    bool open = true;
    g_MuMonsterPreviewUI.Render(&open);
    ImGui::Render();
    CHECK_FALSE(g_MuMonsterPreviewUI.ShowsSlot(MODEL_HOUND));
    draw();
    draw();
    CHECK_FALSE(g_MuMonsterPreviewUI.ShowsSlot(MODEL_HOUND));
    CHECK_FALSE(g_MuMonsterPreviewUI.ShowsSlot(-1));

    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
    g_MonsterSoundSetDatabase.Build({});
    g_MonsterDatabase.Build({}, "en");
}

TEST_CASE("Monster Preview marks the cues of Monster Sounds' draft, also while Monster Model shows the model "
          "(Q-MN81) [editor][monsters]")
{
    MonsterDefinition hound = Monster(3, "Hound");
    hound.model = "MONSTER_MODEL_HOUND";
    hound.soundSet = "Hound";
    const std::array monsters = {hound};
    g_MonsterDatabase.Build(monsters, "en");
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_HOUND";
    model.load.file = "Data/Monster/Monster02.bmd";
    model.load.textureFolders = {"Monster"};
    model.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    const std::array models = {model};
    g_MonsterModelDatabase.Build(models);
    Data::Monsters::MonsterSoundSet set;
    set.name = "Hound";
    const std::array sets = {set};
    g_MonsterSoundSetDatabase.Build(sets);
    TestImGuiContext context;
    g_MonsterSelection.Select(3);

    ImGui::NewFrame();
    bool open = true;
    g_MuMonsterModelUI.Render(&open);
    g_MuMonsterSoundsUI.Render(&open);
    // A cue frame typed in Monster Sounds and not stored yet, as its field
    // leaves the draft while it is edited.
    auto* draft =
        const_cast<Data::Monsters::MonsterModelDefinition*>(g_MuMonsterSoundsUI.GetShownModelDraft(MODEL_HOUND));
    REQUIRE(draft != nullptr);
    draft->cues[0].frames = {4.5f, 5.5f};
    const std::span<const Data::Monsters::MonsterModelCue> cues = g_MuMonsterPreviewUI.GetCues(MODEL_HOUND);
    REQUIRE(cues.size() == 1);
    CHECK(cues[0].frames == std::pair(4.5f, 5.5f));
    ImGui::Render();
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
    g_MonsterSoundSetDatabase.Build({});
    g_MonsterDatabase.Build({}, "en");
}

TEST_CASE("The shown look of a monster with random variants is the entry's until a variant is picked (Q-MN83) "
          "[editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle 2");
    elf.model = "MONSTER_MODEL_CHAOSCASTLE_ELF";
    elf.randomVariants = {{"MONSTER_MODEL_CHAOSCASTLE_WIZARD", std::nullopt}};
    Data::Monsters::MonsterModelDefinition elfModel;
    elfModel.slot = "MONSTER_MODEL_CHAOSCASTLE_ELF";
    Data::Monsters::MonsterModelDefinition wizard;
    wizard.slot = "MONSTER_MODEL_CHAOSCASTLE_WIZARD";
    const std::array models = {elfModel, wizard};
    g_MonsterModelDatabase.Build(models);
    using namespace MuEditor::Monsters::ShownModel;

    SetShownVariant(-1, -1);
    CHECK(GetShownVariant(elf) == -1);
    CHECK(FindShownSlot(elf) == MODEL_CHAOS_CASTLE_ELF);
    SetShownVariant(163, 0);
    CHECK(GetShownVariant(elf) == 0);
    CHECK(FindShownSlot(elf) == MODEL_CHAOS_CASTLE_WIZARD);
    // Another monster, or a variant that is gone: the entry's look.
    MonsterDefinition other = elf;
    other.number = 165;
    CHECK(GetShownVariant(other) == -1);
    SetShownVariant(163, 3);
    CHECK(FindShownSlot(elf) == MODEL_CHAOS_CASTLE_ELF);

    // Another monster selected: the entry's look also when the monster is
    // selected again; the same one selected again (an undo) keeps the look.
    g_MonsterSelection.Select(163);
    SetShownVariant(163, 0);
    g_MonsterSelection.Select(163);
    CHECK(GetShownVariant(elf) == 0);
    g_MonsterSelection.Select(165);
    g_MonsterSelection.Select(163);
    CHECK(GetShownVariant(elf) == -1);
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    SetShownVariant(-1, -1);
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
                                   WindowIds::MonsterTranslations, WindowIds::DataChecks})
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
    CHECK(DockIdOf(WindowIds::DataChecks) == DockIdOf(WindowIds::ItemBasics));
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
    g_MuEditorCore.GetToolWindows().monsterTranslations = true;
    CHECK(target.IsOpen());
    CHECK(target.CanUndo());
    CHECK(target.GetUndoLabel() == "Name");

    // Changed past the history: the undo is refused (and logged).
    g_MonsterDatabase.Set(Monster(2, "Budge Dragon 3"));
    target.Undo();
    CHECK(g_MonsterDatabase.Find(2)->names.GetNeutral() == "Budge Dragon 3");
    CHECK(target.CanUndo());
}

// Data Checks has no undo of its own: its Ctrl+Z acts on the target the
// editor bar shows, the tool worked in last while it is open.
TEST_CASE("The bar's undo target is the tool worked in last while it is open [editor][monsters]")
{
    ToolWindowsScope windows;
    MuEditor::UndoTarget& monsters = MuEditor::Monsters::ToolWindow::GetUndoTarget();
    MuEditor::UndoTarget& items = MuEditor::Items::ToolWindow::GetUndoTarget();
    MuEditor::UndoTargets::Activate(monsters);

    // No monster tool is open.
    CHECK(&MuEditor::UndoTargets::GetCurrent(items) == &items);

    g_MuEditorCore.GetToolWindows().monsterTranslations = true;
    CHECK(&MuEditor::UndoTargets::GetCurrent(items) == &monsters);
}

TEST_CASE("Monster Sounds: a sound event plays at moments or in animations, one of the other kind switches it (Q-MN76) "
          "[editor][monsters]")
{
    using Data::Monsters::MonsterSoundMoment;
    using namespace MuEditor::Monsters::SoundEventsEditor;
    Data::Monsters::MonsterSoundEvent event;
    event.animations = {MONSTER01_ATTACK1};
    CHECK(WhenText(event) == "attack1");

    // A moment makes an event of animations one of the moment.
    CHECK(ToggleMoment(event, MonsterSoundMoment::Hit));
    CHECK(event.animations.empty());
    CHECK(event.moments == std::vector{MonsterSoundMoment::Hit});
    CHECK(ToggleMoment(event, MonsterSoundMoment::Attack));
    CHECK(event.moments == std::vector{MonsterSoundMoment::Attack, MonsterSoundMoment::Hit});
    CHECK(WhenText(event) == "on attack, hit");
    CHECK(ToggleMoment(event, MonsterSoundMoment::Hit));
    CHECK(event.moments == std::vector{MonsterSoundMoment::Attack});
    // The last moment stays.
    CHECK_FALSE(ToggleMoment(event, MonsterSoundMoment::Attack));
    CHECK(event.moments == std::vector{MonsterSoundMoment::Attack});

    // An animation makes it one of animations again.
    CHECK(ToggleAnimation(event, MONSTER01_DIE));
    CHECK(event.moments.empty());
    CHECK(event.animations == std::vector{static_cast<int>(MONSTER01_DIE)});
    CHECK_FALSE(ToggleAnimation(event, MONSTER01_DIE));
}

TEST_CASE("Monster Sounds: a sound event plays on cues of the model, one of another kind switches it (Q-MN79) "
          "[editor][monsters]")
{
    using Data::Monsters::MonsterSoundMoment;
    using namespace MuEditor::Monsters::SoundEventsEditor;
    Data::Monsters::MonsterSoundEvent event;
    event.moments = {MonsterSoundMoment::Walk};
    CHECK(ToggleCue(event, "step 2"));
    CHECK(event.moments.empty());
    CHECK(event.cues == std::vector<std::string>{"step 2"});
    // Cues keep the order they were picked in.
    CHECK(ToggleCue(event, "step 1"));
    CHECK(event.cues == std::vector<std::string>{"step 2", "step 1"});
    CHECK(WhenText(event) == "cue step 2, step 1");
    CHECK(ToggleCue(event, "step 2"));
    CHECK_FALSE(ToggleCue(event, "step 1"));
    CHECK(event.cues == std::vector<std::string>{"step 1"});
    // An animation or a moment makes it one of them again.
    CHECK(ToggleAnimation(event, MONSTER01_WALK));
    CHECK(event.cues.empty());
    CHECK(ToggleCue(event, "step 1"));
    CHECK(event.animations.empty());
    CHECK(ToggleMoment(event, MonsterSoundMoment::Death));
    CHECK(event.cues.empty());
}

TEST_CASE("Monster Sounds: a sound event plays each drawn frame in animations or once in them, one of another "
          "kind switches it (Q-MN84, Q-MN85, Q-MN88) [editor][monsters]")
{
    using Data::Monsters::MonsterSoundMoment;
    using namespace MuEditor::Monsters::SoundEventsEditor;
    Data::Monsters::MonsterSoundEvent event;
    event.animations = {MONSTER01_WALK};
    // Each frame in: the event becomes one of drawn frames.
    CHECK(ToggleDuring(event, MONSTER01_RUN));
    CHECK(event.animations.empty());
    CHECK(event.during == std::vector{static_cast<int>(MONSTER01_RUN)});
    CHECK(ToggleDuring(event, MONSTER01_WALK));
    CHECK(event.during == std::vector{static_cast<int>(MONSTER01_WALK), static_cast<int>(MONSTER01_RUN)});
    CHECK(WhenText(event) == "each frame: walk, run");
    CHECK(ToggleDuring(event, MONSTER01_RUN));
    CHECK_FALSE(ToggleDuring(event, MONSTER01_WALK));
    CHECK(event.during == std::vector{static_cast<int>(MONSTER01_WALK)});

    // Once until it stands: picking one switches the event again.
    CHECK(ToggleOnce(event, MONSTER01_ATTACK1));
    CHECK(event.during.empty());
    CHECK(event.once == std::vector{static_cast<int>(MONSTER01_ATTACK1)});
    CHECK(WhenText(event) == "once: attack1");

    // A moment, a cue or an animation makes it one of them.
    CHECK(ToggleMoment(event, MonsterSoundMoment::Death));
    CHECK(event.once.empty());
    CHECK(ToggleDuring(event, MONSTER01_WALK));
    CHECK(event.moments.empty());
    CHECK(ToggleCue(event, "step 1"));
    CHECK(event.during.empty());
    CHECK(ToggleOnce(event, MONSTER01_DIE));
    CHECK(event.cues.empty());
    CHECK(ToggleAnimation(event, MONSTER01_DIE));
    CHECK(event.once.empty());
    CHECK(event.animations == std::vector{static_cast<int>(MONSTER01_DIE)});

    // Each frame in a cue (Q-MN90).
    CHECK(ToggleDuringCue(event, "attack2 sound"));
    CHECK(event.animations.empty());
    CHECK(event.duringCues == std::vector<std::string>{"attack2 sound"});
    CHECK(WhenText(event) == "each frame in cue attack2 sound");
    CHECK_FALSE(ToggleDuringCue(event, "attack2 sound"));
    CHECK(ToggleCue(event, "step 1"));
    CHECK(event.duringCues.empty());

    // On a timer of the model (Q-MN91).
    CHECK(ToggleTimer(event, "hair swing"));
    CHECK(event.cues.empty());
    CHECK(event.timers == std::vector<std::string>{"hair swing"});
    CHECK(WhenText(event) == "on timer hair swing");
    CHECK_FALSE(ToggleTimer(event, "hair swing"));
    CHECK(ToggleDuring(event, MONSTER01_WALK));
    CHECK(event.timers.empty());
}

TEST_CASE("Monster Sounds: a new cue is one frame at the preview's frame, named after it once (Q-MN82) "
          "[editor][monsters]")
{
    using namespace MuEditor::Monsters::SoundEventsEditor;
    Data::Monsters::MonsterModelDefinition model;
    model.slot = "MONSTER_MODEL_RAYMOND";
    const Data::Monsters::MonsterModelCue first = MakeCue(model, MONSTER01_WALK, 4.5f);
    CHECK(first.name == "walk 4.50");
    CHECK(first.animation == MONSTER01_WALK);
    CHECK(first.frames == std::pair<float, float>{4.5f, 5.5f});
    model.cues = {first};
    CHECK(MakeCue(model, MONSTER01_WALK, 4.5f).name == "walk 4.50 (2)");
    model.cues.push_back(MakeCue(model, MONSTER01_WALK, 4.5f));
    CHECK(MakeCue(model, MONSTER01_WALK, 4.5f).name == "walk 4.50 (3)");
    CHECK(MakeCue(model, MONSTER01_ATTACK1, 0.0f).name == "attack1 0.00");
}

TEST_CASE("Monster Sounds: Play plays all of an event's sounds, or one of a pick's by the draw [editor][monsters]")
{
    using namespace MuEditor::Monsters::SoundEventsEditor;
    using Sounds = std::vector<std::string>;
    Data::Monsters::MonsterSoundEvent event;
    event.sounds = {"SOUND_MONSTER_DSIDLE1", "SOUND_MONSTER_DSIDLE2"};
    // Without a pick the game plays them together, whatever the draw.
    CHECK(PreviewSounds(event, 1) == event.sounds);

    event.pick = Data::Monsters::MonsterSoundDraw::Rand;
    CHECK(PreviewSounds(event, 0) == Sounds{"SOUND_MONSTER_DSIDLE1"});
    CHECK(PreviewSounds(event, 1) == Sounds{"SOUND_MONSTER_DSIDLE2"});

    // A pick of one sound: the game plays it one time in two, Play each time.
    event.sounds = {"SOUND_MONSTER_DSIDLE1"};
    CHECK(PreviewSounds(event, 1) == Sounds{"SOUND_MONSTER_DSIDLE1"});
}

TEST_CASE("Monster Sounds: the question whether a cue goes too removes only the event it was asked for (Q-MN82) "
          "[editor][monsters]")
{
    using MuEditor::Monsters::SoundEventsEditor::IsSameEventAt;
    Data::Monsters::MonsterSoundEvent step;
    step.cues = {"step 1"};
    step.sounds = {"SOUND_MONSTER_DSIDLE1"};
    Data::Monsters::MonsterSoundEvent attack;
    attack.animations = {MONSTER01_ATTACK1};
    attack.sounds = {"SOUND_MONSTER_DSIDLE2"};
    Data::Monsters::MonsterSoundSet set;
    set.name = "Raymond";
    set.soundEvents = {attack, step};
    CHECK(IsSameEventAt(set, 1, step));

    // An undo or another edit moved the events meanwhile: the answer removes
    // nothing.
    set.soundEvents = {step, attack};
    CHECK_FALSE(IsSameEventAt(set, 1, step));
    set.soundEvents = {attack};
    CHECK_FALSE(IsSameEventAt(set, 1, step));
    // The event changed meanwhile.
    set.soundEvents = {attack, step};
    set.soundEvents[1].sounds = {"SOUND_MONSTER_DSIDLE2"};
    CHECK_FALSE(IsSameEventAt(set, 1, step));
}

// Monster Setup shows every setup field once, in a section, with when a
// living monster takes it (D55, Q-MN39).
TEST_CASE("Monster Setup: every setup field has a row, a section and a time it takes effect [editor][monsters]")
{
    namespace SetupFields = MuEditor::Monsters::SetupFields;
    for (const Data::Monsters::MonsterSetupField& field : Data::Monsters::GetMonsterSetupFields())
    {
        INFO(field.key);
        const SetupFields::Field* row = SetupFields::Find(field.key);
        REQUIRE(row != nullptr);
        CHECK(std::string_view(*row->label) != field.key);
        CHECK(SetupFields::GetValueField(field.key).has_value());
    }
    CHECK(SetupFields::GetFields().size() == Data::Monsters::GetMonsterSetupFields().size());
    CHECK(SetupFields::Find("scale")->takes == SetupFields::Takes::AtOnce);
    CHECK(SetupFields::Find("spawnEffects")->takes == SetupFields::Takes::NextSpawn);
}

namespace
{
// The id of an item in a field row of Monster Setup, or of the Map row.
ImGuiID SetupItemId(int key, const char* row, const char* label)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed(row, nullptr, ImGui::GetIDWithSeed(key, window->ID));
    return ImGui::GetIDWithSeed(label, nullptr, scope);
}
} // namespace

// A map variant's own state variants are in the State row while the map
// variant is picked; Set on one takes the map variant's value, and Remove
// removes it from the map variant.
TEST_CASE("Monster Setup: a map variant's state variants are edited under the map variant [editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle Elf");
    elf.model = "MONSTER_MODEL_CHAOS_CASTLE_ELF";
    elf.setup.outline = 1.0f;
    elf.mapVariants.emplace_back();
    elf.mapVariants[0].maps = {{18}};
    elf.mapVariants[0].setup.outline = 2.0f;
    const std::string state(Data::Monsters::GetMonsterStates().front());
    Data::Monsters::MonsterStateVariant variant;
    variant.state = state;
    elf.mapVariants[0].setup.stateVariants.push_back(variant);
    const std::array monsters = {elf};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(163); };
    render();

    const std::string map =
        MuEditor::MapPicker::Describe(elf.mapVariants[0].maps, I18N::Editor::NoMapYet) + "##Variant0";
    click(SetupItemId(163, "MapRow", map.c_str()));
    click(SetupItemId(163, "StateRow", (state + "##State0").c_str()));
    click(SetupItemId(163, "outline", I18N::Editor::Set));
    REQUIRE(stored().mapVariants[0].setup.stateVariants.size() == 1);
    CHECK(stored().mapVariants[0].setup.stateVariants[0].setup.outline == 2.0f);
    CHECK(stored().setup.stateVariants.empty());

    click(SetupItemId(163, "StateVariant", I18N::Editor::RemoveTheStateVariant));
    CHECK(stored().mapVariants[0].setup.stateVariants.empty());
    CHECK(stored().mapVariants.size() == 1);
    // The window keeps its picks for the monster: back to All maps.
    click(SetupItemId(163, "MapRow", I18N::Editor::AllMaps));
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// A map variant's own state variants replace the monster's on its maps, so
// its first one starts from the monster's: they keep applying there.
TEST_CASE("Monster Setup: a map variant's first state variant keeps the monster's [editor][monsters]")
{
    MonsterDefinition giant = Monster(7, "Giant");
    giant.model = "MONSTER_MODEL_GIANT";
    Data::Monsters::MonsterStateVariant transformed;
    transformed.state = "playerTransformed";
    transformed.setup.scale = 0.8f;
    giant.setup.stateVariants.push_back(transformed);
    giant.mapVariants.emplace_back();
    giant.mapVariants[0].maps = {{34}};
    const std::array monsters = {giant};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(7);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(7); };
    render();

    const std::string map =
        MuEditor::MapPicker::Describe(giant.mapVariants[0].maps, I18N::Editor::NoMapYet) + "##Variant0";
    click(SetupItemId(7, "MapRow", map.c_str()));
    CHECK(MuEditor::Monsters::SetupLists::MonsterStatesOnMap(stored(), 0) == "playerTransformed");
    click(SetupItemId(7, "StateRow", "##AddState"));
    const ImGuiWindow* combo = ImGui::FindWindowByName("##Combo_00");
    REQUIRE(combo != nullptr);
    click(ImGui::GetIDWithSeed("crywolfReady", nullptr, combo->ID));

    const std::vector<Data::Monsters::MonsterStateVariant> states = stored().mapVariants[0].setup.stateVariants;
    REQUIRE(states.size() == 2);
    CHECK(states[0] == transformed);
    CHECK(states[1].state == "crywolfReady");
    CHECK(stored().setup.stateVariants.size() == 1);
    CHECK(MuEditor::Monsters::SetupLists::MonsterStatesOnMap(stored(), 0).empty());
    click(SetupItemId(7, "MapRow", I18N::Editor::AllMaps));
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// Set gives a field a value, x leaves it to the game; on a map variant Set
// takes the monster's value and x the monster's value again.
TEST_CASE("Monster Setup: Set and x on the monster and on a map variant [editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle Elf");
    elf.model = "MONSTER_MODEL_CHAOS_CASTLE_ELF";
    elf.setup.scale = 1.2f;
    elf.mapVariants.emplace_back();
    elf.mapVariants[0].maps = {{18}};
    const std::array monsters = {elf};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(163); };
    render();

    click(SetupItemId(163, "outline", I18N::Editor::Set));
    CHECK(stored().setup.outline == 1.0f);
    click(SetupItemId(163, "outline", "x"));
    CHECK_FALSE(stored().setup.outline.has_value());
    CHECK(stored().setup.scale == 1.2f);

    const std::string variant =
        MuEditor::MapPicker::Describe(elf.mapVariants[0].maps, I18N::Editor::NoMapYet) + "##Variant0";
    click(SetupItemId(163, "MapRow", variant.c_str()));
    click(SetupItemId(163, "scale", I18N::Editor::Set));
    REQUIRE(stored().mapVariants.size() == 1);
    CHECK(stored().mapVariants[0].setup.scale == 1.2f);
    CHECK(stored().setup.scale == 1.2f);
    click(SetupItemId(163, "scale", "x"));
    CHECK_FALSE(stored().mapVariants[0].setup.scale.has_value());

    // The model of the variant: the monster's while it names none.
    CHECK_FALSE(stored().mapVariants[0].model.has_value());
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(stored().mapVariants[0].setup.scale == 1.2f);
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

TEST_CASE("Monster Setup: a list of numbers is read from its text [editor][monsters]")
{
    using MuEditor::Monsters::SetupLists::FormatNumbers;
    using MuEditor::Monsters::SetupLists::ParseNumbers;
    CHECK(ParseNumbers("0, 1, 2") == std::vector<int>{0, 1, 2});
    CHECK(ParseNumbers("7") == std::vector<int>{7});
    CHECK(ParseNumbers("") == std::vector<int>{});
    CHECK(ParseNumbers(" 3 ,4") == std::vector<int>{3, 4});
    CHECK_FALSE(ParseNumbers("1,,2").has_value());
    CHECK_FALSE(ParseNumbers("a").has_value());
    CHECK(FormatNumbers({0, 1, 2}) == "0, 1, 2");
}

// An appear's or a teleport's effects are joints, effects and particles: the
// reader refuses objects and eyes there, so their rows do not offer them.
TEST_CASE("Monster Setup: appear effects offer the kinds the reader takes [editor][monsters]")
{
    using Data::Monsters::MonsterSpawnEffect;
    using MuEditor::Monsters::SetupLists::EffectRows;
    using MuEditor::Monsters::SetupLists::IsEffectKindAllowed;
    for (const MonsterSpawnEffect::Kind kind :
         {MonsterSpawnEffect::Kind::Joint, MonsterSpawnEffect::Kind::Effect, MonsterSpawnEffect::Kind::Particle,
          MonsterSpawnEffect::Kind::Object, MonsterSpawnEffect::Kind::Eyes})
    {
        CHECK(IsEffectKindAllowed(EffectRows::Spawn, kind));
    }
    CHECK(IsEffectKindAllowed(EffectRows::Appear, MonsterSpawnEffect::Kind::Joint));
    CHECK(IsEffectKindAllowed(EffectRows::Appear, MonsterSpawnEffect::Kind::Effect));
    CHECK(IsEffectKindAllowed(EffectRows::Appear, MonsterSpawnEffect::Kind::Particle));
    CHECK_FALSE(IsEffectKindAllowed(EffectRows::Appear, MonsterSpawnEffect::Kind::Object));
    CHECK_FALSE(IsEffectKindAllowed(EffectRows::Appear, MonsterSpawnEffect::Kind::Eyes));
}

// A row switched to another kind keeps none of the old kind's fields (a
// joint's bone would make an object a bone row until the file is read again).
TEST_CASE("Monster Setup: an effect row switched to another kind starts fresh [editor][monsters]")
{
    using Data::Monsters::MonsterSpawnEffect;
    using MuEditor::Monsters::SetupLists::SwitchedKind;
    MonsterSpawnEffect joint;
    joint.kind = MonsterSpawnEffect::Kind::Joint;
    joint.type = "BITMAP_JOINT_ENERGY";
    joint.bone = 27;
    joint.scale = 30.0f;
    joint.count = 3;
    joint.offset = std::array<float, 3>{20.0f, 0.0f, 0.0f};
    joint.targetOffset = std::array<float, 3>{0.0f, 0.0f, 120.0f};

    const MonsterSpawnEffect object = SwitchedKind(joint, MonsterSpawnEffect::Kind::Object);
    CHECK(object.kind == MonsterSpawnEffect::Kind::Object);
    CHECK_FALSE(object.bone.has_value());
    CHECK_FALSE(object.offset.has_value());
    CHECK_FALSE(object.targetOffset.has_value());
    CHECK(object.count == 1);

    const MonsterSpawnEffect particle = SwitchedKind(joint, MonsterSpawnEffect::Kind::Particle);
    CHECK(particle.kind == MonsterSpawnEffect::Kind::Particle);
    CHECK_FALSE(particle.bone.has_value());
    CHECK_FALSE(particle.targetOffset.has_value());
    CHECK(particle.scale == MonsterSpawnEffect{}.scale);
    CHECK(particle.count == 3);
    CHECK(particle.offset == joint.offset);
    CHECK(std::ranges::any_of(Data::Effects::GetEffectTypeSymbols(Data::Effects::EffectKind::Particle),
                              [&](const Data::Effects::EffectTypeSymbol& symbol)
                              { return symbol.code == particle.type; }));
}

// The Spawn section's lists: an effect added and removed, a buff ticked, a
// tile added.
TEST_CASE("Monster Setup: the spawn lists add and remove rows [editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle Elf");
    elf.model = "MONSTER_MODEL_CHAOS_CASTLE_ELF";
    const std::array monsters = {elf};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(163); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    click(ImGui::GetIDWithSeed(I18N::Editor::Spawn, nullptr, ImGui::GetIDWithSeed(163, window->ID)));

    click(SetupItemId(163, "spawnEffects", I18N::Editor::AddAnEffect));
    REQUIRE(stored().setup.spawnEffects.size() == 1);
    CHECK(stored().setup.spawnEffects[0].kind == Data::Monsters::MonsterSpawnEffect::Kind::Joint);
    // A joint type the reader takes, not an empty one.
    CHECK(std::ranges::any_of(Data::Effects::GetEffectTypeSymbols(Data::Effects::EffectKind::Joint),
                              [&](const Data::Effects::EffectTypeSymbol& symbol)
                              { return symbol.code == stored().setup.spawnEffects[0].type; }));

    click(SetupItemId(163, "spawnBuffs", "poison"));
    CHECK(stored().setup.spawnBuffs == std::vector<std::string>{"poison"});

    click(SetupItemId(163, "facingByTile", I18N::Editor::AddATile));
    REQUIRE(stored().setup.facingByTile.size() == 1);
    CHECK(stored().setup.facingByTile[0].tileX == 0);

    const ImGuiWindow* setup = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    const ImGuiID row = ImGui::GetIDWithSeed(
        0, ImGui::GetIDWithSeed("spawnEffects", nullptr, ImGui::GetIDWithSeed(163, setup->ID)));
    click(ImGui::GetIDWithSeed("x", nullptr, row));
    CHECK(stored().setup.spawnEffects.empty());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

// A box with only one corner keeps the other as the model's: drawing the
// Spawn section writes nothing, so an edit of another field saves no corner.
TEST_CASE("Monster Setup: a max-only box survives an edit of another field [editor][monsters]")
{
    MonsterDefinition dragon = Monster(42, "Red Dragon");
    dragon.setup.boxMax = std::array<float, 3>{100.0f, 100.0f, 200.0f};
    const std::array monsters = {dragon};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(42);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(42); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    click(ImGui::GetIDWithSeed(I18N::Editor::Spawn, nullptr, ImGui::GetIDWithSeed(42, window->ID)));
    render();

    click(SetupItemId(42, "scale", I18N::Editor::Set));
    REQUIRE(stored().setup.scale.has_value());
    CHECK_FALSE(stored().setup.boxMin.has_value());
    CHECK(stored().setup.boxMax == std::array<float, 3>{100.0f, 100.0f, 200.0f});

    // Its own min is ticked on, from the model's (a monster without a model:
    // the game's default box), and off again.
    click(SetupItemId(42, "boundingBox", I18N::Editor::BoxMin));
    CHECK(stored().setup.boxMin == std::array<float, 3>{-60.0f, -60.0f, 0.0f});
    click(SetupItemId(42, "boundingBox", I18N::Editor::BoxMin));
    CHECK_FALSE(stored().setup.boxMin.has_value());
    CHECK(stored().setup.boxMax.has_value());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// A player-shaped monster's box starts as the player model's character slot
// has it (40, 40, 120 at the top), not as the other models' default.
TEST_CASE("Monster Setup: a player-shaped monster's box starts as the player's [editor][monsters]")
{
    MonsterDefinition luke = Monster(258, "Luke the Helper");
    luke.model = "player";
    const std::array monsters = {luke};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(258);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(I18N::Editor::Spawn, nullptr, ImGui::GetIDWithSeed(258, window->ID)));
    render();
    render();
    ImGui::ActivateItemByID(SetupItemId(258, "boundingBox", I18N::Editor::Set));
    render();
    render();
    const MonsterDefinition& stored = *g_MonsterDatabase.Find(258);
    CHECK(stored.setup.boxMin == Engine::Object::MonsterBody::DefaultBoxMin);
    CHECK(stored.setup.boxMax == Engine::Object::MonsterBody::PlayerBoxMax);
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// A facing-by-tile row switched to the other axis has that axis only, with
// its tile, and the field shows that axis' tile.
// A typed value the reader would refuse is kept to its range: a class step
// is 0, 1 or 2, so a save is not refused for it.
TEST_CASE("Monster Setup: a typed class step is kept to the reader's range [editor][monsters]")
{
    const std::array monsters = {Monster(163, "Chaos Castle Elf")};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = [](const char* typed = nullptr)
    {
        if (typed != nullptr)
            ImGui::GetIO().AddInputCharactersUTF8(typed);
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    render();
    ImGui::ActivateItemByID(SetupItemId(163, "classStep", I18N::Editor::Set));
    render();
    render();
    REQUIRE(g_MonsterDatabase.Find(163)->setup.classStep == 0);

    const ImGuiID field = SetupItemId(163, "classStep", "##Value");
    ImGui::ActivateItemByID(field);
    render();
    REQUIRE(ImGui::GetActiveID() == field);
    render("7");
    ImGui::ClearActiveID();
    render();
    render();
    CHECK(g_MonsterDatabase.Find(163)->setup.classStep == Data::Monsters::MaxClassStep);
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// An eyes row takes the bones the reader takes: pairs, up to three, from 0.
TEST_CASE("Monster Setup: an eyes row takes two, four or six bones [editor][monsters]")
{
    using MuEditor::Monsters::SetupLists::ParseEyeBones;
    CHECK(ParseEyeBones("16, 17") == std::vector<int>{16, 17});
    CHECK(ParseEyeBones("1, 2, 3, 4, 5, 6") == std::vector<int>{1, 2, 3, 4, 5, 6});
    CHECK_FALSE(ParseEyeBones("16").has_value());
    CHECK_FALSE(ParseEyeBones("5, 6, 7").has_value());
    CHECK_FALSE(ParseEyeBones("1, 2, 3, 4, 5, 6, 7, 8").has_value());
    CHECK_FALSE(ParseEyeBones("-1, 2").has_value());
    CHECK_FALSE(ParseEyeBones("").has_value());
}

// The state variants that apply on a map variant's maps, which Monster
// Preview's State row lists: its own, else the monster's.
TEST_CASE("Monster Preview: the State row lists the state variants on the picked map [editor][monsters]")
{
    using MuEditor::Monsters::SetupLists::StatesOnMap;
    MonsterDefinition giant = Monster(7, "Giant");
    giant.setup.stateVariants.emplace_back().state = "playerTransformed";
    giant.mapVariants.resize(2);
    giant.mapVariants[0].setup.stateVariants.emplace_back().state = "crywolfReady";

    CHECK(&StatesOnMap(giant, -1) == &giant.setup.stateVariants);
    CHECK(&StatesOnMap(giant, 0) == &giant.mapVariants[0].setup.stateVariants);
    CHECK(&StatesOnMap(giant, 1) == &giant.setup.stateVariants);
}

// Monster Preview keeps its picked state by name: when the listed state
// variants change under it (a map variant gets its maps), it stays on that
// state or goes back to Always, never to another state.
TEST_CASE("Monster Preview: a picked state stays itself when the list changes [editor][monsters]")
{
    using MuEditor::Monsters::SetupLists::PickedState;
    std::vector<Data::Monsters::MonsterStateVariant> monsterStates(2);
    monsterStates[0].state = "playerTransformed";
    monsterStates[1].state = "raklionBossWaiting";
    std::vector<Data::Monsters::MonsterStateVariant> mapStates(2);
    mapStates[0].state = "castleSiegeRunning";
    mapStates[1].state = "crywolfReady";

    CHECK(PickedState(monsterStates, "raklionBossWaiting") == "raklionBossWaiting");
    CHECK(PickedState(mapStates, "raklionBossWaiting").empty());
    CHECK(PickedState(mapStates, "crywolfReady") == "crywolfReady");
    CHECK(PickedState(mapStates, "").empty());
}

TEST_CASE("Monster Setup: a facing-by-tile row switches its axis [editor][monsters]")
{
    using MuEditor::Monsters::SetupLists::SetTileAxis;
    using MuEditor::Monsters::SetupLists::TileOnAxis;
    Data::Monsters::MonsterTileFacing tile;
    tile.tileX = 62;

    SetTileAxis(tile, 1);
    CHECK_FALSE(tile.tileX.has_value());
    CHECK(tile.tileY == 62);
    TileOnAxis(tile) = 90;
    CHECK(tile.tileY == 90);
    CHECK_FALSE(tile.tileX.has_value());

    SetTileAxis(tile, 0);
    CHECK(tile.tileX == 90);
    CHECK_FALSE(tile.tileY.has_value());
    TileOnAxis(tile) = 183;
    CHECK(tile.tileX == 183);
    CHECK_FALSE(tile.tileY.has_value());
}

TEST_CASE("Monster Setup: every hook says what code it runs [editor][monsters]")
{
    for (const Data::Monsters::MonsterHookKind& kind : Data::Monsters::GetMonsterHookKinds())
    {
        INFO(kind.name);
        CHECK_FALSE(MuEditor::Monsters::SetupLists::GetHookCode(kind.name).empty());
    }
}

// Appear is set and left to the game again; a hook is added and removed.
TEST_CASE("Monster Setup: appear and hooks [editor][monsters]")
{
    MonsterDefinition gate = Monster(277, "Castle Gate");
    gate.model = "MONSTER_MODEL_CASTLE_GATE";
    const std::array monsters = {gate};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(277);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(277); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed(277, window->ID);
    click(ImGui::GetIDWithSeed(I18N::Editor::AppearAndTeleport, nullptr, scope));
    click(ImGui::GetIDWithSeed(I18N::Editor::GateAndHooks, nullptr, scope));

    click(SetupItemId(277, "appear", I18N::Editor::Set));
    REQUIRE(stored().setup.appear.has_value());
    CHECK(stored().setup.appear->effects.empty());
    click(SetupItemId(277, "appear", "x"));
    CHECK_FALSE(stored().setup.appear.has_value());

    click(SetupItemId(277, "gate", I18N::Editor::Set));
    REQUIRE(stored().setup.gate.has_value());
    CHECK(stored().setup.gate->arrivalOpening);

    click(SetupItemId(277, "hooks", I18N::Editor::AddAHook));
    REQUIRE(stored().setup.hooks.size() == 1);
    CHECK_FALSE(stored().setup.hooks[0].name.empty());
    const ImGuiID row = ImGui::GetIDWithSeed(0, ImGui::GetIDWithSeed("hooks", nullptr, scope));
    click(ImGui::GetIDWithSeed("x", nullptr, row));
    CHECK(stored().setup.hooks.empty());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

// Monster Equipment's paper doll: a slot picked and set to a body, then
// emptied; a random look takes the monster's equipment with Set and gives it
// back.
TEST_CASE("Monster Equipment: a slot of the doll, and a random look's equipment [editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle Elf");
    elf.model = "MONSTER_MODEL_CHAOS_CASTLE_ELF";
    elf.setup.equipment.weapons[0] = Data::Monsters::MonsterItem{4, 8};
    elf.randomVariants.emplace_back();
    elf.randomVariants[0].model = "MONSTER_MODEL_CHAOS_CASTLE_WIZARD";
    const std::array monsters = {elf};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterEquipmentUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(163); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterEquipment);
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed(163, window->ID);
    const auto in = [&](const char* group, const char* label)
    { return ImGui::GetIDWithSeed(label, nullptr, ImGui::GetIDWithSeed(group, nullptr, scope)); };

    click(in("Doll", "###Slot2"));
    click(in("Slot", I18N::Editor::Body));
    REQUIRE(stored().setup.equipment.helm.has_value());
    CHECK(stored().setup.equipment.helm->part == "MODEL_BODY_HELM");
    click(in("Slot", I18N::Editor::Empty));
    CHECK_FALSE(stored().setup.equipment.helm.has_value());
    CHECK(stored().setup.equipment.weapons[0].has_value());

    click(in("Rows", "MONSTER_MODEL_CHAOS_CASTLE_WIZARD##Look0"));
    click(ImGui::GetIDWithSeed(I18N::Editor::Set, nullptr, scope));
    REQUIRE(stored().randomVariants[0].equipment.has_value());
    CHECK(*stored().randomVariants[0].equipment == stored().setup.equipment);
    click(ImGui::GetIDWithSeed(I18N::Editor::UseTheMonsterValue, nullptr, scope));
    CHECK_FALSE(stored().randomVariants[0].equipment.has_value());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
}

// A monster model takes items in its weapon slots only: its other slots take
// a body (the merchants' part models) or nothing.
TEST_CASE("Monster Equipment: a monster model's other slots take a body, not an item [editor][monsters]")
{
    MonsterDefinition bull = Monster(0, "Bull Fighter");
    bull.model = "MONSTER_MODEL_BULL_FIGHTER";
    const std::array monsters = {bull};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(0);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterEquipmentUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(0); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterEquipment);
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed(0, window->ID);
    const auto in = [&](const char* group, const char* label)
    { return ImGui::GetIDWithSeed(label, nullptr, ImGui::GetIDWithSeed(group, nullptr, scope)); };

    click(in("Doll", "###Slot0"));
    click(in("Slot", I18N::Editor::Item));
    REQUIRE(stored().setup.equipment.weapons[0].has_value());
    click(in("Doll", "###Slot2"));
    click(in("Slot", I18N::Editor::Item));
    CHECK_FALSE(stored().setup.equipment.helm.has_value());
    click(in("Slot", I18N::Editor::Body));
    REQUIRE(stored().setup.equipment.helm.has_value());
    CHECK(stored().setup.equipment.helm->part == "MODEL_BODY_HELM");
    click(in("Slot", I18N::Editor::Empty));
    CHECK_FALSE(stored().setup.equipment.helm.has_value());
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// A map variant's own state variants are in the State row of Monster
// Equipment while the map variant is picked; Set takes the map variant's
// equipment.
TEST_CASE("Monster Equipment: a map variant's state variant wears the map variant's equipment [editor][monsters]")
{
    MonsterDefinition elf = Monster(163, "Chaos Castle Elf");
    elf.model = "MONSTER_MODEL_CHAOS_CASTLE_ELF";
    elf.setup.equipment.weapons[0] = Data::Monsters::MonsterItem{4, 8};
    elf.mapVariants.emplace_back();
    elf.mapVariants[0].maps = {{18}};
    elf.mapVariants[0].setup.equipment.weapons[0] = Data::Monsters::MonsterItem{4, 9};
    const std::string state(Data::Monsters::GetMonsterStates().front());
    Data::Monsters::MonsterStateVariant variant;
    variant.state = state;
    variant.setup.outline = 2.0f;
    elf.mapVariants[0].setup.stateVariants.push_back(variant);
    const std::array monsters = {elf};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(163);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterEquipmentUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    const auto stored = [] { return *g_MonsterDatabase.Find(163); };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterEquipment);
    REQUIRE(window != nullptr);
    const ImGuiID scope = ImGui::GetIDWithSeed(163, window->ID);
    const auto in = [&](const char* group, const char* label)
    { return ImGui::GetIDWithSeed(label, nullptr, ImGui::GetIDWithSeed(group, nullptr, scope)); };

    const std::string map = MuEditor::MapPicker::Describe(elf.mapVariants[0].maps, I18N::Editor::NoMapYet) + "##Map0";
    click(in("Rows", map.c_str()));
    click(in("Rows", (state + "##State0").c_str()));
    click(ImGui::GetIDWithSeed(I18N::Editor::Set, nullptr, scope));
    REQUIRE(stored().mapVariants[0].setup.stateVariants.size() == 1);
    CHECK(stored().mapVariants[0].setup.stateVariants[0].setup.equipment == stored().mapVariants[0].setup.equipment);
    // Back to the monster's own, as the window keeps its picks.
    click(in("Rows", I18N::Editor::AllMaps));
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterEdits.GetHistory().Clear();
}

// The unknown monsters' look in Monster Setup (D64): a box changes it as an
// undo step, and Preview as selects the number for Monster Preview, which
// shows the look's body for a number without an entry.
TEST_CASE("Monster Setup: the unknown monsters' look is edited, undone and previewed as a number [editor][monsters]")
{
    const std::array monsters = {Monster(2, "Budge Dragon")};
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterDatabase.SetUnknownLook({});
    Data::Monsters::MonsterModelDefinition ghost;
    ghost.slot = "MONSTER_MODEL_GHOST";
    const std::array models = {ghost};
    g_MonsterModelDatabase.Build(models);
    g_MonsterEdits.GetHistory().Clear();
    TestImGuiContext context;
    g_MonsterSelection.Select(2);
    const auto render = []
    {
        ImGui::NewFrame();
        bool open = true;
        g_MuMonsterSetupUI.Render(&open);
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    const auto click = [&](ImGuiID id)
    {
        ImGui::ActivateItemByID(id);
        render();
        render();
    };
    render();
    const ImGuiWindow* window = ImGui::FindWindowByName(WindowIds::MonsterSetup);
    REQUIRE(window != nullptr);
    click(ImGui::GetIDWithSeed(I18N::Editor::UnknownMonsters, nullptr, window->ID));
    const ImGuiID section = ImGui::GetIDWithSeed("UnknownMonsters", nullptr, window->ID);

    click(ImGui::GetIDWithSeed(I18N::Editor::ANumberWithoutAnEntry, nullptr, section));
    CHECK_FALSE(g_MonsterDatabase.GetUnknownLook().forNoEntry);
    CHECK_FALSE(MuEditor::Monsters::ShownModel::FindUnknownLookSlot(700, nullptr).has_value());
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.GetUnknownLook().forNoEntry);
    CHECK(MuEditor::Monsters::ShownModel::FindUnknownLookSlot(700, nullptr) ==
          MODEL_MONSTER01 + MONSTER_MODEL_GHOST);
    // An entry with a model shows its own.
    MonsterDefinition budge = Monster(2, "Budge Dragon");
    budge.model = "MONSTER_MODEL_BUDGE_DRAGON";
    CHECK_FALSE(MuEditor::Monsters::ShownModel::FindUnknownLookSlot(2, &budge).has_value());

    click(ImGui::GetIDWithSeed(I18N::Editor::ShowInMonsterPreview, nullptr, section));
    CHECK(g_MonsterSelection.GetNumber() == 700);
    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    g_MonsterModelDatabase.Build({});
}

#endif // _EDITOR
