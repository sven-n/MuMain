#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundValidation.h"
#include "Data/GameData/MonsterData/MonsterSoundsJson.h"

#include <algorithm>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSoundEvent;
using Data::Monsters::MonsterSoundMoment;
using Data::Monsters::MonsterSoundSet;

namespace
{
struct ReadResult
{
    std::vector<MonsterSoundSet> sets;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Monsters::ReadMonsterSoundsJson(text, "Sounds.json", result.sets, result.issues);
    return result;
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
                   const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue)
                       {
                           return issue.severity == severity && issue.field == field &&
                                  issue.message.find(message) != std::string::npos;
                       });
}

std::string SoundsFile(const std::string& sets)
{
    return R"({ "formatVersion": 1, "soundSets": [ )" + sets + " ] }";
}

// One set named Hound with the fields given.
std::string Set(const std::string& fields)
{
    return R"({ "name": "Hound")" + (fields.empty() ? "" : ", " + fields) + " }";
}

MonsterSoundSet FullSet()
{
    MonsterSoundSet set;
    set.name = "Dark Knight";
    set.sounds = {{"Data/Sound/mDarkKnight1.wav", "SOUND_MONSTER_DARKKNIGHT1", 2, true},
                  {"Data/Sound/mDarkKnightDie.wav", "", 1, false}};
    MonsterSoundEvent step;
    step.cues = {"step 2", "step 1"};
    step.sounds = {"SOUND_MONSTER_DARKKNIGHT1"};
    MonsterSoundEvent breath;
    breath.animations = {MONSTER01_STOP1, MONSTER01_STOP2};
    breath.chance = Data::Monsters::MonsterSoundChance{100, Data::Monsters::MonsterSoundDraw::Rand};
    breath.pick = Data::Monsters::MonsterSoundDraw::RandFps;
    breath.sounds = {"SOUND_MONSTER_DARKKNIGHT1", "Data/Sound/mDarkKnightDie.wav"};
    MonsterSoundEvent die;
    die.animations = {MONSTER01_DIE};
    die.deathEffectArmed = true;
    die.sounds = {"Data/Sound/mDarkKnightDie.wav", "SOUND_MONSTER_DARKKNIGHT1"};
    MonsterSoundEvent idle;
    idle.moments = {MonsterSoundMoment::Stand, MonsterSoundMoment::Walk};
    idle.heardAnywhere = true;
    idle.atMonster = true;
    idle.chance = Data::Monsters::MonsterSoundChance{16, Data::Monsters::MonsterSoundDraw::RandFps};
    idle.pick = Data::Monsters::MonsterSoundDraw::Rand;
    idle.sounds = {"SOUND_MONSTER_DARKKNIGHT1"};
    set.soundEvents = {step, breath, die, idle};
    set.maps = {{69}, {70, 2}};
    return set;
}

MonsterDefinition Monster(int number, const std::string& model, const std::string& soundSet)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", "Monster " + std::to_string(number));
    monster.model = model;
    monster.soundSet = soundSet;
    return monster;
}

MonsterSoundSet SetOf(const std::string& name, std::vector<MonsterSoundEvent> events = {})
{
    MonsterSoundSet set;
    set.name = name;
    set.sounds = {{"Data/Sound/a.wav", "", 2, true}};
    set.soundEvents = std::move(events);
    return set;
}

MonsterSoundEvent OnCue(const std::string& cue)
{
    MonsterSoundEvent event;
    event.cues = {cue};
    event.sounds = {"Data/Sound/a.wav"};
    return event;
}
} // namespace

TEST_CASE("Monster sounds: a written set reads back the same (Q-MN78) [data][monsters]")
{
    const MonsterSoundSet set = FullSet();
    const std::string text = Data::Monsters::WriteMonsterSoundsJson(std::vector<MonsterSoundSet>{set});
    const ReadResult read = Read(text);
    INFO(text);
    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    REQUIRE(read.sets.size() == 1);
    CHECK(read.sets[0] == set);
}

TEST_CASE("Monster sounds: the writer sorts by name and puts each sound and event on one line [data][monsters]")
{
    MonsterSoundSet hound;
    hound.name = "Hound";
    hound.sounds = {{"Data/Sound/mHound1.wav", "SOUND_MONSTER_HOUND1", 2, true}};
    MonsterSoundEvent idle;
    idle.moments = {MonsterSoundMoment::Stand};
    idle.sounds = {"SOUND_MONSTER_HOUND1"};
    MonsterSoundEvent step;
    step.cues = {"step 1"};
    step.sounds = {"SOUND_MONSTER_HOUND1"};
    hound.soundEvents = {idle, step};
    hound.maps = {{80}, {81}};
    MonsterSoundSet empty;
    empty.name = "Elf Lala";

    const std::string text = Data::Monsters::WriteMonsterSoundsJson(std::vector<MonsterSoundSet>{hound, empty});
    CHECK(text == R"({
  "formatVersion": 1,
  "soundSets": [
    {
      "name": "Elf Lala"
    },
    {
      "name": "Hound",
      "sounds": [
        {"file": "Data/Sound/mHound1.wav", "id": "SOUND_MONSTER_HOUND1", "channels": 2, "spatial": true}
      ],
      "soundEvents": [
        {"on": ["stand"], "sounds": ["SOUND_MONSTER_HOUND1"]},
        {"cues": ["step 1"], "sounds": ["SOUND_MONSTER_HOUND1"]}
      ],
      "maps": [80, 81]
    }
  ]
}
)");
}

TEST_CASE("Monster sounds: moments are sorted when read, cues keep their order [data][monsters]")
{
    const ReadResult read = Read(SoundsFile(Set(
        R"("sounds": [{"file": "Data/Sound/a.wav"}], "soundEvents": [{"on": ["walk", "attack"], "sounds": ["Data/Sound/a.wav"]}, {"cues": ["b", "a"], "sounds": ["Data/Sound/a.wav"]}])")));
    INFO(Messages(read.issues));
    REQUIRE(read.sets.size() == 1);
    CHECK(read.sets[0].soundEvents[0].moments ==
          std::vector<MonsterSoundMoment>{MonsterSoundMoment::Attack, MonsterSoundMoment::Walk});
    CHECK(read.sets[0].soundEvents[1].cues == std::vector<std::string>{"b", "a"});
    // A sound without channels and 3D gets LoadWaveFile's defaults.
    CHECK(read.sets[0].sounds[0].channels == Data::Monsters::DefaultSoundChannels);
    CHECK_FALSE(read.sets[0].sounds[0].spatial);
}

TEST_CASE("Monster sounds: an event names a sound of the set or a SOUND_ id [data][monsters]")
{
    MonsterSoundSet set;
    set.name = "Hound";
    set.sounds = {{"Data/Sound/mHound1.wav", "", 2, true}, {"Data/Sound/mHound2.wav", "SOUND_NOT_AN_ID", 2, true}};
    MonsterSoundEvent event;
    event.animations = {MONSTER01_ATTACK1};
    // The set's sound by its file, an id the data can name, and neither.
    event.sounds = {"Data/Sound/mHound1.wav", "SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01", "Data/Sound/mNobody.wav"};
    set.soundEvents = {event};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterSoundSetNames(std::vector<MonsterSoundSet>{set}, "Sounds.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 2);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "sound set Hound sounds[1].id", "is not a SOUND_ id"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "sound set Hound soundEvents[0].sounds[2]",
                        "is no sound of the set"));
}

TEST_CASE("Monster sounds: wrong values are errors that name the set and the field [data][monsters]")
{
    struct Case
    {
        std::string set;
        std::string field;
        std::string message;
    };
    const std::vector<Case> cases = {
        {Set(R"("sounds": [{"file": "Data/Sound/a.wav", "channels": 5}])"), "sound set Hound sounds[0].channels",
         "1 to 4"},
        {Set(R"("sounds": [{"channels": 2}])"), "sound set Hound sounds[0].file", "missing"},
        {Set(R"("sounds": [{"file": "Data\\Sound\\a.wav"}])"), "sound set Hound sounds[0].file", "/"},
        {Set(R"("soundEvents": [{"sounds": ["a"]}])"), "sound set Hound soundEvents[0]",
         "one of animations, on and cues"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "on": ["walk"], "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0]", "one of animations, on and cues"},
        {Set(R"("soundEvents": [{"cues": ["step 1"], "on": ["walk"], "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0]", "one of animations, on and cues"},
        {Set(R"("soundEvents": [{"on": [], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].on",
         "one moment or more"},
        {Set(R"("soundEvents": [{"on": ["jump"], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].on[0]",
         "not a moment"},
        {Set(R"("soundEvents": [{"on": ["hit", "hit"], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].on",
         "hit more than once"},
        {Set(R"("soundEvents": [{"cues": [], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].cues",
         "one cue or more"},
        {Set(R"("soundEvents": [{"cues": ["a", "a"], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].cues",
         "more than once"},
        {Set(R"("soundEvents": [{"cues": [""], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].cues[0]",
         "must be a text"},
        // SubType conditions and frames are gone (Q-MN78, Q-MN79).
        {Set(R"("soundEvents": [{"on": ["hit"], "subType": 9, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].subType", "names its own sound set"},
        {Set(R"("soundEvents": [{"on": ["hit"], "notSubType": 9, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].notSubType", "names its own sound set"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "frames": [7, 8], "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].frames", "cues of the monster's model"},
        {Set(R"("soundEvents": [{"on": ["hit"], "heardAnywhere": 1, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].heardAnywhere", "true or false"},
        {Set(R"("soundEvents": [{"on": ["hit"], "atMonster": "yes", "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].atMonster", "true or false"},
        {Set(R"("soundEvents": [{"animations": [], "sounds": ["a"]}])"), "sound set Hound soundEvents[0].animations",
         "one animation or more"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "sounds": []}])"), "sound set Hound soundEvents[0].sounds",
         "one sound or more"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "chance": {"oneIn": 0, "draw": "rand"}, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].chance.oneIn", "1 to 100000"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "chance": {"oneIn": 2, "draw": "dice"}, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].chance.draw", "randFps or rand"},
        {Set(R"("soundEvents": [{"animations": ["walk"], "pick": "rand", "sounds": ["a", "b", "c"]}])"),
         "sound set Hound soundEvents[0].sounds", "one or two sounds with pick"},
        {Set(R"("soundEvents": [{"animations": ["die"], "deathEffectArmed": 1, "sounds": ["a"]}])"),
         "sound set Hound soundEvents[0].deathEffectArmed", "true or false"},
        {Set(R"("maps": [69, -1])"), "sound set Hound maps[1]", "from 0 to 255"},
        {Set(R"("maps": [{"map": 9, "stage": 256}])"), "sound set Hound maps[0]", "\"stage\": 1 to 255"},
    };
    for (const Case& test : cases)
    {
        const ReadResult read = Read(SoundsFile(test.set));
        INFO(test.set);
        INFO(Messages(read.issues));
        CHECK(read.sets.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, test.field, test.message));
    }
}

TEST_CASE("Monster sounds: a set needs a name, once; unknown fields are warnings [data][monsters]")
{
    ReadResult read = Read(SoundsFile(R"({ "sounds": [] })"));
    CHECK(read.sets.empty());
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "soundSets[0].name", "missing"));

    read = Read(SoundsFile(Set("") + ", " + Set("")));
    CHECK(read.sets.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "sound set Hound name", "more than once"));

    read = Read(SoundsFile(Set(R"("volume": 1, "sounds": [{"file": "Data/Sound/a.wav", "loop": true}])")));
    INFO(Messages(read.issues));
    CHECK_FALSE(HasFatal(read.issues));
    REQUIRE(read.sets.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "sound set Hound volume", "unknown"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "sound set Hound sounds[0].loop", "unknown"));

    read = Read(R"({ "formatVersion": 2, "soundSets": [] })");
    CHECK(HasFatal(read.issues));
}

TEST_CASE("Monster sounds: a monster plays its own set, a random variant's with the variant's model (Q-MN83) "
          "[data][monsters]")
{
    MonsterDefinition elf = Monster(163, "MONSTER_MODEL_CHAOSCASTLE_ELF", "Chaos Castle");
    elf.randomVariants = {{"MONSTER_MODEL_CHAOSCASTLE_WIZARD", std::string("Chaos Castle Wizard")}};
    CHECK(Data::Monsters::GetSoundSetWithModel(elf, MODEL_CHAOS_CASTLE_ELF) == "Chaos Castle");
    CHECK(Data::Monsters::GetSoundSetWithModel(elf, MODEL_CHAOS_CASTLE_WIZARD) == "Chaos Castle Wizard");
    // Another model: the entry's set.
    CHECK(Data::Monsters::GetSoundSetWithModel(elf, MODEL_BULL_FIGHTER) == "Chaos Castle");
    // A variant without a set changes the model only.
    elf.randomVariants[0].soundSet.reset();
    CHECK(Data::Monsters::GetSoundSetWithModel(elf, MODEL_CHAOS_CASTLE_WIZARD) == "Chaos Castle");
}

TEST_CASE("Monster sounds: the links between monsters, models and sets are checked (Q-MN78, Q-MN79, Q-MN83) "
          "[data][monsters]")
{
    MonsterModelDefinition raymond;
    raymond.slot = "MONSTER_MODEL_RAYMOND";
    raymond.cues = {{"step 1", MONSTER01_WALK, {1.0f, 2.0f}}};
    MonsterModelDefinition hound;
    hound.slot = "MONSTER_MODEL_HOUND";
    const std::vector<MonsterModelDefinition> models = {raymond, hound};

    MonsterDefinition stepper = Monster(506, "MONSTER_MODEL_RAYMOND", "Raymond");
    // The Raymond set on a model without its cue: the event never plays for it.
    MonsterDefinition borrower = Monster(1, "MONSTER_MODEL_HOUND", "Raymond");
    MonsterDefinition lost = Monster(5, "MONSTER_MODEL_HOUND", "Nobody");
    MonsterDefinition elf = Monster(163, "MONSTER_MODEL_CHAOSCASTLE_ELF", "Raymond");
    elf.randomVariants = {{"MONSTER_MODEL_CHAOSCASTLE_WIZARD", std::string("Nobody either")}};
    // A random variant on a monster whose look the code does not draw.
    MonsterDefinition drawnNot = Monster(2, "MONSTER_MODEL_HOUND", "Raymond");
    drawnNot.randomVariants = {{"MONSTER_MODEL_BUDGE_DRAGON", std::nullopt}};
    const std::vector<MonsterDefinition> monsters = {borrower, drawnNot, lost, elf, stepper};
    const std::vector<MonsterSoundSet> sets = {SetOf("Raymond", {OnCue("step 1")}), SetOf("Unused")};
    std::vector<ItemDataIssue> issues;

    Data::Monsters::ValidateMonsterSoundLinks(monsters, models, sets, "Monsters.json", "Sounds.json", issues);

    INFO(Messages(issues));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "monster 5 soundSet", "\"Nobody\" is no sound set"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "monster 163 randomVariants[0].soundSet",
                        "\"Nobody either\" is no sound set"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "monster 2 randomVariants[0].model",
                        "never shows the monster with MONSTER_MODEL_BUDGE_DRAGON"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "sound set Raymond soundEvents[0].cues",
                        "the model of monster 1, 2, 163 has no cue \"step 1\""));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "sound set Unused name", "no monster plays the set"));
    CHECK(issues.size() == 5);
    // The Error of a monster has its number, so Data Checks selects it.
    const auto lostIssue =
        std::find_if(issues.begin(), issues.end(), [](const ItemDataIssue& issue) { return issue.number == 5; });
    REQUIRE(lostIssue != issues.end());
    CHECK(lostIssue->source == "Monsters.json");
}

TEST_CASE("Monster sounds: the code draws the looks of the ranged Chaos Castle soldiers only (Q-MN83) "
          "[data][monsters]")
{
    for (const int number : {163, 165, 167, 169, 171, 173, 427})
    {
        INFO(number);
        CHECK(Data::Monsters::GetDrawnLookModels(number).size() == 2);
    }
    CHECK(Data::Monsters::GetDrawnLookModels(162).empty());
    CHECK(Data::Monsters::GetDrawnLookModels(0).empty());
}
