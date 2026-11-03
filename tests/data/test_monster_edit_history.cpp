#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "ClientDataScope.h"
#include "TemporaryMonsterFolder.h"

#include "UI/MonsterTools/MonsterEditHistory.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterSelection.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"

#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

using namespace MuEditor::Monsters;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterModelSnapshot;
using Data::Monsters::MonsterSnapshot;

namespace
{
MonsterDefinition Monster(int number, const char* english, MonsterKind kind = MonsterKind::Monster)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    monster.kind = kind;
    return monster;
}

// Monsters kept in a map, standing in for the monster table; it remembers
// the order the numbers were restored in.
class FakeMonsters
{
public:
    MonsterEditHistory::Hooks MakeHooks()
    {
        return {[this](int number)
                {
                    const auto found = monsters.find(number);
                    return found != monsters.end() ? std::optional(found->second) : std::nullopt;
                },
                [this](std::span<const MonsterSnapshot> snapshots)
                {
                    for (const MonsterSnapshot& snapshot : snapshots)
                    {
                        restored.push_back(snapshot.number);
                        if (snapshot.monster)
                        {
                            monsters[snapshot.number] = *snapshot.monster;
                        }
                        else
                        {
                            monsters.erase(snapshot.number);
                        }
                    }
                },
                [this](int slot)
                {
                    const auto found = models.find(slot);
                    return found != models.end() ? std::optional(found->second) : std::nullopt;
                },
                [this](std::span<const MonsterModelSnapshot> snapshots)
                {
                    for (const MonsterModelSnapshot& snapshot : snapshots)
                    {
                        restoredModels.push_back(snapshot.slot);
                        if (snapshot.model)
                        {
                            models[snapshot.slot] = *snapshot.model;
                        }
                        else
                        {
                            models.erase(snapshot.slot);
                        }
                    }
                }};
    }

    std::map<int, MonsterDefinition> monsters;
    std::vector<int> restored;
    std::map<int, MonsterModelDefinition> models;
    std::vector<int> restoredModels;
};

MonsterModelDefinition Model(const char* slot, const char* file)
{
    MonsterModelDefinition model;
    model.slot = slot;
    model.load.file = file;
    model.load.textureFolders = {"Monster"};
    return model;
}

MonsterModelDefinition Hound()
{
    return Model("MONSTER_MODEL_HOUND", "Data/Monster/Monster02.bmd");
}

MonsterEditEntry Step(const std::string& label, int number, std::optional<MonsterDefinition> before,
                      std::optional<MonsterDefinition> after)
{
    return {label, {{number, std::move(before), std::move(after)}}};
}

// The problems, one per line, for a failed check.
std::string Lines(const std::vector<Data::Items::ItemDataIssue>& issues)
{
    std::string text;
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

// A monster table and a model table for the editor's edits, with an empty
// history that counts as saved.
void BuildMonsters()
{
    const std::array monsters = {Monster(2, "Budge Dragon"), Monster(252, "", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");
    const std::array models = {Hound()};
    g_MonsterModelDatabase.Build(models);
    g_MonsterEdits.GetHistory().Clear();
    g_MonsterEdits.GetHistory().MarkSaved();
}
} // namespace

TEST_CASE("The monster edit history undoes and redoes what the tools changed [editor][monsters]")
{
    FakeMonsters fake;
    MonsterEditHistory history(fake.MakeHooks());
    const MonsterDefinition before = Monster(2, "Budge Dragon");
    MonsterDefinition after = before;
    after.kind = MonsterKind::Npc;
    fake.monsters[2] = after;
    history.Record(Step("Kind", 2, before, after));

    REQUIRE(history.CanUndo());
    CHECK(history.GetUndoLabel() == "Kind");
    CHECK(history.IsModifiedSinceSave());
    REQUIRE(history.Undo());
    CHECK(fake.monsters[2].kind == MonsterKind::Monster);
    CHECK(history.GetLastChangedMonster() == 2);
    CHECK_FALSE(history.IsModifiedSinceSave());
    CHECK(history.CanRedo());
    CHECK(history.GetRedoLabel() == "Kind");

    REQUIRE(history.Redo());
    CHECK(fake.monsters[2].kind == MonsterKind::Npc);
    CHECK_FALSE(history.Redo());
    CHECK(history.GetLastRefusal() == MonsterEditHistory::Refusal::NothingToDo);

    // A new entry drops the ones that were undone.
    history.Undo();
    history.Record(Step("Name", 2, before, before));
    CHECK_FALSE(history.CanRedo());
}

TEST_CASE("The monster edit history refuses an undo when the monster changed past it [editor][monsters]")
{
    FakeMonsters fake;
    MonsterEditHistory history(fake.MakeHooks());
    fake.monsters[2] = Monster(2, "Budge Dragon 2");
    history.Record(Step("Name", 2, Monster(2, "Budge Dragon"), fake.monsters[2]));

    // Changed by something that did not record it.
    fake.monsters[2] = Monster(2, "Budge Dragon 3");
    CHECK_FALSE(history.Undo());
    CHECK(history.GetLastRefusal() == MonsterEditHistory::Refusal::MonstersChanged);
    CHECK(fake.monsters[2].names.GetNeutral() == "Budge Dragon 3");
    CHECK(history.CanUndo());
    CHECK(fake.restored.empty());
}

TEST_CASE("The monster edit history adds and removes entries, several numbers as one step [editor][monsters]")
{
    FakeMonsters fake;
    MonsterEditHistory history(fake.MakeHooks());
    fake.monsters[2] = Monster(2, "Budge Dragon");
    // Removes 2, adds 3.
    const MonsterEditEntry entry{"Import",
                                 {{2, fake.monsters[2], std::nullopt}, {3, std::nullopt, Monster(3, "Larva")}}};
    fake.monsters.erase(2);
    fake.monsters[3] = Monster(3, "Larva");
    history.Record(entry);

    REQUIRE(history.Undo());
    CHECK(fake.monsters.contains(2));
    CHECK_FALSE(fake.monsters.contains(3));
    // Last step first.
    CHECK(fake.restored == std::vector<int>{3, 2});
    CHECK(history.GetLastChangedMonster() == 2);

    fake.restored.clear();
    REQUIRE(history.Redo());
    CHECK_FALSE(fake.monsters.contains(2));
    CHECK(fake.monsters.contains(3));
    CHECK(fake.restored == std::vector<int>{2, 3});

    // An entry without steps is no entry.
    history.Record({"Nothing", {}});
    CHECK(history.GetUndoLabel() == "Import");
}

TEST_CASE("The monster edit history keeps a limited number of entries and knows the saved state [editor][monsters]")
{
    FakeMonsters fake;
    MonsterEditHistory history(fake.MakeHooks());
    MonsterDefinition state = Monster(2, "Budge Dragon");
    fake.monsters[2] = state;
    for (size_t entry = 0; entry < MonsterEditHistory::MaxEntries + 10; ++entry)
    {
        MonsterDefinition next = Monster(2, ("Budge Dragon " + std::to_string(entry)).c_str());
        fake.monsters[2] = next;
        history.Record(Step("Name", 2, state, next));
        state = next;
        if (entry == 5)
        {
            history.MarkSaved();
        }
    }
    size_t undone = 0;
    while (history.Undo())
    {
        ++undone;
    }
    CHECK(undone == MonsterEditHistory::MaxEntries);
    // The saved state was among the entries dropped first.
    CHECK(history.IsModifiedSinceSave());

    // Cleared for data that replaced all monsters: it is not saved yet.
    history.Clear();
    CHECK_FALSE(history.CanUndo());
    CHECK(history.IsModifiedSinceSave());
    history.MarkSaved();
    CHECK_FALSE(history.IsModifiedSinceSave());
}

TEST_CASE("Selecting a monster counts, also the same monster again [editor][monsters]")
{
    MonsterSelection& selection = MonsterSelection::GetInstance();
    const int serial = selection.GetSerial();
    selection.Select(2);
    CHECK(selection.GetNumber() == 2);
    selection.Select(2);
    CHECK(selection.GetSerial() == serial + 2);
}

TEST_CASE("The monster edits refuse what the monster list cannot hold [editor][monsters]")
{
    BuildMonsters();
    const int version = g_MonsterDatabase.GetVersion();

    // Names.
    CHECK_FALSE(g_MonsterEdits.SetMonsterName("Name", 3, "en", "Larva"));
    CHECK_FALSE(g_MonsterEdits.SetMonsterName("Name", 2, "en", ""));
    CHECK_FALSE(g_MonsterEdits.SetMonsterName("Name", 2, "pt", "Dragão||Lento"));
    CHECK_FALSE(g_MonsterEdits.SetMonsterName("Name", 2, "p=t", "Dragão Lento"));
    // Entries.
    CHECK_FALSE(g_MonsterEdits.AddMonster("Add", Monster(1024, "Nobody")));
    CHECK_FALSE(g_MonsterEdits.AddMonster("Add", Monster(-1, "Nobody")));
    CHECK_FALSE(g_MonsterEdits.AddMonster("Add", Monster(252, "Taken")));
    CHECK_FALSE(g_MonsterEdits.AddMonster("Add", Monster(3, "")));
    CHECK_FALSE(g_MonsterEdits.AddMonster("Add", Monster(3, "Lar||va")));
    CHECK_FALSE(g_MonsterEdits.SetMonster("Kind", Monster(3, "Larva", MonsterKind::Npc)));
    CHECK_FALSE(g_MonsterEdits.RemoveMonster("Remove", 3));

    CHECK(g_MonsterDatabase.GetVersion() == version);
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());

    // An English name that is empty already stays so; its translations can
    // be set.
    CHECK(g_MonsterEdits.SetMonsterName("Name", 252, "pt", "Sem nome"));
    CHECK(g_MonsterDatabase.Find(252)->names.GetOwn("pt") == "Sem nome");
    CHECK(g_MonsterDatabase.Find(252)->names.GetNeutral().empty());
}

TEST_CASE("The monster edits go through the monster table, and undo selects the monster [editor][monsters]")
{
    BuildMonsters();

    REQUIRE(g_MonsterEdits.SetMonsterName("Name", 2, "pt", "Dragão Lento"));
    CHECK(g_MonsterDatabase.Find(2)->names.GetOwn("pt") == "Dragão Lento");
    REQUIRE(g_MonsterEdits.SetMonster("Kind", Monster(2, "Budge Dragon", MonsterKind::Npc)));
    // The whole entry is stored: the name set before is gone with it.
    CHECK(g_MonsterDatabase.Find(2)->kind == MonsterKind::Npc);
    CHECK_FALSE(g_MonsterDatabase.Find(2)->names.HasOwn("pt"));
    REQUIRE(g_MonsterEdits.AddMonster("Add", Monster(3, "Larva")));
    REQUIRE(g_MonsterEdits.RemoveMonster("Remove", 252));
    CHECK(g_MonsterDatabase.Find(252) == nullptr);
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "Remove");

    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.Find(252) != nullptr);
    CHECK(g_MonsterSelection.GetNumber() == 252);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.Find(3) == nullptr);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDatabase.Find(2)->kind == MonsterKind::Monster);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK_FALSE(g_MonsterDatabase.Find(2)->names.HasOwn("pt"));
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());

    REQUIRE(g_MonsterEdits.Redo());
    CHECK(g_MonsterDatabase.Find(2)->names.GetOwn("pt") == "Dragão Lento");
    CHECK(g_MonsterSelection.GetNumber() == 2);

    // A change that changes nothing is no step.
    REQUIRE(g_MonsterEdits.SetMonsterName("Same", 2, "pt", "Dragão Lento"));
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "Name");
}

TEST_CASE("A group of monster edits is one step, with one change per number [editor][monsters]")
{
    BuildMonsters();

    g_MonsterEdits.BeginGroup("Import names");
    REQUIRE(g_MonsterEdits.SetMonsterName("pt", 2, "pt", "Dragão"));
    REQUIRE(g_MonsterEdits.SetMonsterName("pt", 2, "pt", "Dragão Lento"));
    REQUIRE(g_MonsterEdits.SetMonsterName("es", 252, "es", "Sin nombre"));
    g_MonsterEdits.EndGroup();

    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "Import names");
    REQUIRE(g_MonsterEdits.Undo());
    CHECK_FALSE(g_MonsterDatabase.Find(2)->names.HasOwn("pt"));
    CHECK_FALSE(g_MonsterDatabase.Find(252)->names.HasOwn("es"));
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());

    REQUIRE(g_MonsterEdits.Redo());
    CHECK(g_MonsterDatabase.Find(2)->names.GetOwn("pt") == "Dragão Lento");
    CHECK(g_MonsterDatabase.Find(252)->names.GetOwn("es") == "Sin nombre");
}

TEST_CASE("The monster edits refuse an undo after a change past them [editor][monsters]")
{
    BuildMonsters();
    REQUIRE(g_MonsterEdits.SetMonsterName("Name", 2, "en", "Budge Dragon 2"));

    g_MonsterDatabase.Set(Monster(2, "Budge Dragon 3"));

    CHECK_FALSE(g_MonsterEdits.Undo());
    CHECK(g_MonsterEdits.GetHistory().GetLastRefusal() == MonsterEditHistory::Refusal::MonstersChanged);
    CHECK(g_MonsterDatabase.Find(2)->names.GetNeutral() == "Budge Dragon 3");
}

TEST_CASE("Saving the monster edits marks them saved, a revert drops them [editor][monsters]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    BuildMonsters();
    std::vector<Data::Items::ItemDataIssue> issues;

    REQUIRE(g_MonsterEdits.AddMonster("Add", Monster(3, "Larva")));
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    // 252's empty English name is a warning, which saves.
    const bool saved = g_MonsterEdits.Save(issues);
    INFO(Lines(issues));
    REQUIRE(saved);
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    CHECK(g_MonsterEdits.GetHistory().CanUndo());

    REQUIRE(g_MonsterEdits.RemoveMonster("Remove", 3));
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    REQUIRE(g_MonsterEdits.Revert(issues));
    CHECK(g_MonsterDatabase.Find(3) != nullptr);
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
}

TEST_CASE("The monster edit history undoes and redoes model steps and selects their monster [editor][monsters]")
{
    FakeMonsters fake;
    MonsterEditHistory history(fake.MakeHooks());
    const MonsterModelDefinition before = Hound();
    MonsterModelDefinition after = before;
    after.load.speeds = {{2, 0.5f}};
    fake.models[MODEL_HOUND] = after;
    MonsterEditEntry entry{"Speed", {}, {}};
    entry.models.push_back({MODEL_HOUND, before, after, 3});
    history.Record(entry);

    REQUIRE(history.CanUndo());
    REQUIRE(history.Undo());
    CHECK(fake.models[MODEL_HOUND] == before);
    CHECK(fake.restoredModels == std::vector<int>{MODEL_HOUND});
    CHECK(fake.restored.empty());
    CHECK(history.GetLastChangedMonster() == 3);
    REQUIRE(history.Redo());
    CHECK(fake.models[MODEL_HOUND] == after);

    // Changed past the history: the undo refuses.
    fake.models[MODEL_HOUND] = before;
    CHECK_FALSE(history.Undo());
    CHECK(history.GetLastRefusal() == MonsterEditHistory::Refusal::MonstersChanged);
}

TEST_CASE("The monster edits store a model entry, and its undo puts it back [editor][monsters]")
{
    BuildMonsters();
    MonsterModelDefinition faster = Hound();
    faster.load.speeds = {{2, 0.5f}};

    REQUIRE(g_MonsterEdits.SetModel("Speed", faster, 2));
    CHECK(g_MonsterDataHandler.ReadModel(MODEL_HOUND) == faster);
    // The game opens it again between frames.
    CHECK(g_MonsterDataHandler.HasModelsToReopen());
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == "Speed");
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());

    g_MonsterSelection.Select(MonsterSelection::NoMonster);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK(g_MonsterDataHandler.ReadModel(MODEL_HOUND) == Hound());
    CHECK(g_MonsterSelection.GetNumber() == 2);
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    g_MonsterDataHandler.ReopenChangedModels();
    CHECK_FALSE(g_MonsterDataHandler.HasModelsToReopen());

    // A change that changes nothing is no step.
    REQUIRE(g_MonsterEdits.SetModel("Same", Hound(), 2));
    CHECK(g_MonsterEdits.GetHistory().GetRedoLabel() == "Speed");
}

TEST_CASE("The monster edits refuse a model that the model list cannot hold [editor][monsters]")
{
    BuildMonsters();
    const int version = g_MonsterModelDatabase.GetVersion();

    // A slot without an entry, and no slot.
    CHECK_FALSE(g_MonsterEdits.SetModel("File", Model("MONSTER_MODEL_BULL_FIGHTER", "Data/Monster/Monster01.bmd"), 0));
    CHECK_FALSE(g_MonsterEdits.SetModel("File", Model("MONSTER_MODEL_NOBODY", "Data/Monster/Monster01.bmd"), 0));
    // A sound slot that names no sound of the model and no SOUND_ id.
    MonsterModelDefinition wrongSlot = Hound();
    wrongSlot.soundSlots.emplace();
    wrongSlot.soundSlots->first[0] = "Data/Sound/mNobody.wav";
    CHECK_FALSE(g_MonsterEdits.SetModel("Sound", wrongSlot, 2));
    // What the file cannot hold: a model without a texture folder.
    MonsterModelDefinition noFolder = Hound();
    noFolder.load.textureFolders.clear();
    CHECK_FALSE(g_MonsterEdits.SetModel("Folders", noFolder, 2));

    CHECK(g_MonsterModelDatabase.GetVersion() == version);
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
}

TEST_CASE("Saving the monster edits writes the model list too, a revert reads it [editor][monsters]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    BuildMonsters();
    std::vector<Data::Items::ItemDataIssue> issues;
    MonsterModelDefinition faster = Hound();
    faster.load.speeds = {{2, 0.5f}};
    REQUIRE(g_MonsterEdits.SetModel("Speed", faster, 2));

    // The Hound's file is not in this client folder: a warning, which saves.
    const bool written = g_MonsterEdits.Save(issues);
    INFO(Lines(issues));
    REQUIRE(written);
    CHECK_FALSE(Data::Items::HasFatal(issues));
    const Data::Monsters::MonsterModelsLoadResult saved =
        Data::Monsters::LoadMonsterModelsFile(folder.Path() / Data::Monsters::GetMonsterModelsFile());
    CHECK(saved.models == std::vector{faster});
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());

    MonsterModelDefinition slower = Hound();
    slower.load.speeds = {{2, 0.1f}};
    REQUIRE(g_MonsterEdits.SetModel("Speed", slower, 2));
    REQUIRE(g_MonsterEdits.Revert(issues));
    CHECK(g_MonsterDataHandler.ReadModel(MODEL_HOUND) == faster);
    CHECK(g_MonsterDataHandler.HasModelsToReopen());
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
    CHECK_FALSE(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
    g_MonsterDataHandler.ReopenChangedModels();
}

TEST_CASE("Saving the monster edits writes neither file when one has errors [editor][monsters]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    BuildMonsters();
    std::vector<Data::Items::ItemDataIssue> issues;
    REQUIRE(g_MonsterEdits.AddMonster("Add", Monster(3, "Larva")));
    // A model the model list cannot hold, put in past the edits' checks: a
    // sound slot that names no sound of the model and no SOUND_ id.
    MonsterModelDefinition wrongSlot = Hound();
    wrongSlot.soundSlots.emplace();
    wrongSlot.soundSlots->first[0] = "Data/Sound/mNobody.wav";
    const std::array models = {wrongSlot};
    g_MonsterModelDatabase.Build(models);

    CHECK_FALSE(g_MonsterEdits.Save(issues));

    CHECK(Data::Items::HasFatal(issues));
    // Before, the monster list was written and the model list was not.
    CHECK_FALSE(std::filesystem::exists(folder.MonstersFile()));
    CHECK_FALSE(std::filesystem::exists(folder.Path() / Data::Monsters::GetMonsterModelsFile()));
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
}

TEST_CASE("Saving the monster edits writes neither file when one cannot be written or replaced [editor][monsters]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    BuildMonsters();
    std::vector<Data::Items::ItemDataIssue> issues;
    // The Hound's file is not in this client folder: a warning, which saves.
    REQUIRE(g_MonsterEdits.Save(issues));
    const std::filesystem::path monstersFile = folder.MonstersFile();
    const std::filesystem::path modelsFile = folder.Path() / Data::Monsters::GetMonsterModelsFile();
    const std::optional<std::string> monsters = Data::ReadTextFile(monstersFile);
    REQUIRE(monsters);
    REQUIRE(std::filesystem::is_regular_file(modelsFile));
    REQUIRE(g_MonsterEdits.AddMonster("Add", Monster(3, "Larva")));

    // A folder where the model list's temporary file goes: it cannot be
    // written.
    REQUIRE(std::filesystem::create_directory(Data::GetTemporaryPath(modelsFile)));
    issues.clear();
    CHECK_FALSE(g_MonsterEdits.Save(issues));
    CHECK(Data::Items::HasFatal(issues));
    CHECK(Data::ReadTextFile(monstersFile) == monsters);
    CHECK_FALSE(std::filesystem::exists(Data::GetTemporaryPath(monstersFile)));
    std::filesystem::remove(Data::GetTemporaryPath(modelsFile));

    // A folder in the model list's place: its new text cannot replace it.
    // Before, the monster list was replaced already.
    std::filesystem::remove(modelsFile);
    REQUIRE(std::filesystem::create_directory(modelsFile));
    std::string error;
    REQUIRE(Data::WriteTextFile(modelsFile / "kept.txt", "kept", error));
    issues.clear();
    CHECK_FALSE(g_MonsterEdits.Save(issues));
    CHECK(Data::Items::HasFatal(issues));
    CHECK(Data::ReadTextFile(monstersFile) == monsters);
    CHECK_FALSE(std::filesystem::exists(Data::GetTemporaryPath(monstersFile)));
    CHECK_FALSE(std::filesystem::exists(Data::GetTemporaryPath(modelsFile)));
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
}

TEST_CASE("A revert of the monster edits changes nothing when one file has errors [editor][monsters]")
{
    TemporaryMonsterFolder folder;
    ClientDataScope scope(folder.Path());
    BuildMonsters();
    std::vector<Data::Items::ItemDataIssue> issues;
    // The Hound's file is not in this client folder: a warning, which saves.
    REQUIRE(g_MonsterEdits.Save(issues));
    REQUIRE(g_MonsterEdits.AddMonster("Add", Monster(3, "Larva")));
    std::string error;
    REQUIRE(Data::WriteTextFile(folder.Path() / Data::Monsters::GetMonsterModelsFile(), "{ broken", error));

    CHECK_FALSE(g_MonsterEdits.Revert(issues));

    CHECK(Data::Items::HasFatal(issues));
    // Before, the monster list was read again and the history cleared.
    CHECK(g_MonsterDatabase.Find(3) != nullptr);
    CHECK(g_MonsterEdits.GetHistory().CanUndo());
    CHECK(g_MonsterEdits.GetHistory().IsModifiedSinceSave());
}

#endif // _EDITOR
