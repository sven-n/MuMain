#include "stdafx.h"

#include "doctest.h"

#include "MonsterCreationScope.h"
#include "MonsterLookRecorder.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/w_CharacterInfo.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using Data::Maps::MapRef;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterSetup;

bool NamesLook(const MonsterSetup& setup)
{
    if (setup.renderStyle || setup.effect)
        return true;
    return std::ranges::any_of(setup.stateVariants, [](const auto& state) { return NamesLook(state.setup); });
}

// The maps the number is drawn on: those its entry places it on (D61), and those of its map variants that name a
// look (Crywolf's green chrome on monsters placed elsewhere). A number is also drawn on MU_MONSTER_LOOK_MAP=<map>
// while its lines are written or compared (Doppelganger's numbers, which the server places without a map in the
// data), then on the maps its baseline lines name. Without any of them, on map 0.
std::vector<MapRef> MapsOf(const MonsterDefinition& monster,
                           const std::map<std::string, std::string>* baseline = nullptr)
{
    std::vector<MapRef> maps;
    const auto add = [&maps](const MapRef& map)
    {
        if (std::ranges::find(maps, map) == maps.end())
            maps.push_back(map);
    };
    for (const MapRef& map : monster.maps)
        add(map);
    for (const auto& variant : monster.mapVariants)
    {
        if (NamesLook(variant.setup))
        {
            for (const MapRef& map : variant.maps)
                add(map);
        }
    }
    if (const char* map = std::getenv("MU_MONSTER_LOOK_MAP"))
        add(MapRef{.map = std::stoi(map)});
    if (baseline != nullptr)
    {
        const std::string prefix = std::to_string(monster.number) + " on map ";
        for (auto line = baseline->lower_bound(prefix); line != baseline->end() && line->first.starts_with(prefix);
             ++line)
        {
            const std::string rest = line->first.substr(prefix.size());
            MapRef map{.map = std::stoi(rest)};
            if (const std::size_t stage = rest.find(" stage "); stage != std::string::npos && stage < rest.find(','))
                map.stage = std::stoi(rest.substr(stage + 7));
            add(map);
        }
    }
    return maps.empty() ? std::vector<MapRef>{MapRef{}} : maps;
}

// Whether the number draws with a look of the data: its entry, a map variant, a state variant or its model names one.
bool HasLook(const MonsterDefinition& monster)
{
    if (NamesLook(monster.setup) ||
        std::ranges::any_of(monster.mapVariants, [](const auto& variant) { return NamesLook(variant.setup); }))
        return true;
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(monster.model);
    const Data::Monsters::MonsterModelDefinition* model = slot ? g_MonsterModelDatabase.Find(*slot) : nullptr;
    return model != nullptr && (!model->renderStyle.empty() || !model->effect.empty());
}

std::set<int> SplitNumbers(const char* text)
{
    std::set<int> numbers;
    std::stringstream in(text);
    std::string number;
    while (std::getline(in, number, ','))
        numbers.insert(std::stoi(number));
    return numbers;
}

// The baseline lines of a number as it draws now, on each of its maps; `code`: without its looks, as its old code
// draws it.
std::map<std::string, std::string> RecordNumber(MonsterRecorder::CreationScope& scope, const MonsterDefinition& monster,
                                                const std::map<std::string, std::string>& baseline, bool code = false)
{
    std::map<std::string, std::string> lines;
    for (const MapRef& map : MapsOf(monster, &baseline))
    {
        scope.SetServerMap(map.map, map.stage);
        CHARACTER* character = scope.Create(monster.number);
        REQUIRE(character != nullptr);
        if (code)
        {
            character->Object.RenderStyleLook = 0;
            character->Object.EffectLook = 0;
        }
        for (const MonsterRecorder::DrawSituation& situation : MonsterRecorder::DrawSituations(character->Object.Type))
        {
            lines[MonsterRecorder::BaselineKey(monster.number, map, situation)] =
                LookRecorder::Digest(MonsterRecorder::RecordMonster(*character, situation));
        }
    }
    scope.SetServerMap(0, 0);
    return lines;
}
} // namespace

TEST_CASE("The monster look recorder draws a monster the same way twice, and its draws reach the renderer "
          "[monsters][looks][recorder]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterRecorder::ShippedLooks looks;
    CHARACTER* character = scope.Create(MONSTER_BALROG);
    REQUIRE(character != nullptr);
    for (const MonsterRecorder::DrawSituation& situation : MonsterRecorder::DrawSituations(character->Object.Type))
    {
        CAPTURE(situation.Describe());
        const LookRecorder::Record first = MonsterRecorder::RecordMonster(*character, situation);
        const LookRecorder::Record second = MonsterRecorder::RecordMonster(*character, situation);
        CHECK_FALSE(first.meshDraws.empty());
        CHECK_FALSE(first.gpuCalls.empty());
        CHECK(LookRecorder::FirstDifference(first, second).empty());
    }
}

// Frozen and hit by Blow of Destruction, a monster draws its body and the second pass over it in the same blue tint at
// its opacity, whatever light its look's layers leave (ML-P19): the old code drew some frozen monsters' second pass
// in the light their drawing left behind.
TEST_CASE("A frozen monster draws as one hit by Blow of Destruction (ML-P19) [monsters][looks][recorder]")
{
    const std::map<std::string, std::string> baseline = MonsterRecorder::ReadBaseline();
    int frozen = 0;
    for (const auto& [key, digest] : baseline)
    {
        if (!key.ends_with(", buff 56"))
            continue;
        ++frozen;
        CAPTURE(key);
        const auto blow = baseline.find(key.substr(0, key.size() - 2) + "86");
        REQUIRE(blow != baseline.end());
        CHECK(digest == blow->second);
    }
    CHECK(frozen > 0);
}

// The baseline (tests/monsters/baseline/MonsterLookDraws.txt) holds every situation of every number with a look of
// the data, and of every number whose drawing code was cleaned up, on each map the number is placed on, as its old
// code drew it. MU_MONSTER_LOOK_WRITE=<numbers,
// comma-separated> records those numbers as their code draws them without their looks: run it while their old code
// is still there.
TEST_CASE("Monster looks moved into data draw as their old code did (the baseline) [monsters][looks][recorder]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterRecorder::ShippedLooks looks;
    std::map<std::string, std::string> baseline = MonsterRecorder::ReadBaseline();
    if (const char* write = std::getenv("MU_MONSTER_LOOK_WRITE"))
    {
        for (const int number : SplitNumbers(write))
        {
            const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
            REQUIRE(monster != nullptr);
            const std::string prefix = std::to_string(number) + " on map ";
            std::erase_if(baseline, [&](const auto& line) { return line.first.rfind(prefix, 0) == 0; });
            baseline.merge(RecordNumber(scope, *monster, baseline, true));
        }
        MonsterRecorder::WriteBaseline(baseline);
        MESSAGE("Wrote the monster look baseline");
        return;
    }

    std::set<int> withLines;
    for (const auto& [key, digest] : baseline)
        withLines.insert(std::stoi(key.substr(0, key.find(' '))));
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        // A number with lines draws as they say whether or not it names a look: its old code was cleaned up (a case
        // that drew as the default goes) or moved.
        if (monster == nullptr || (!HasLook(*monster) && !withLines.contains(number)))
            continue;
        CAPTURE(number);
        CHECK(withLines.contains(number));
        for (const auto& [key, digest] : RecordNumber(scope, *monster, baseline))
        {
            CAPTURE(key);
            const auto line = baseline.find(key);
            REQUIRE(line != baseline.end());
            INFO(digest << " drawn, " << line->second << " kept");
            CHECK(LookRecorder::SameDigest(digest, line->second));
        }
    }
}

// While a look moves: MU_MONSTER_LOOK_COMPARE=<numbers, comma-separated> draws each number in each situation with its
// looks and with its looks taken off (its old code, while that is there), and reports the first difference.
TEST_CASE("Monster looks draw as the old code still there draws without them [monsters][looks][recorder]")
{
    const char* compare = std::getenv("MU_MONSTER_LOOK_COMPARE");
    if (compare == nullptr)
        return;
    MonsterRecorder::CreationScope scope(0);
    const MonsterRecorder::ShippedLooks looks;
    for (const int number : SplitNumbers(compare))
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        REQUIRE(monster != nullptr);
        for (const MapRef& map : MapsOf(*monster))
        {
            scope.SetServerMap(map.map, map.stage);
            CHARACTER* character = scope.Create(number);
            REQUIRE(character != nullptr);
            for (const MonsterRecorder::DrawSituation& situation :
                 MonsterRecorder::DrawSituations(character->Object.Type))
            {
                CAPTURE(MonsterRecorder::BaselineKey(number, map, situation));
                const LookRecorder::Record data = MonsterRecorder::RecordMonster(*character, situation);
                const unsigned short style = character->Object.RenderStyleLook;
                const unsigned short effect = character->Object.EffectLook;
                character->Object.RenderStyleLook = 0;
                character->Object.EffectLook = 0;
                const LookRecorder::Record code = MonsterRecorder::RecordMonster(*character, situation);
                character->Object.RenderStyleLook = style;
                character->Object.EffectLook = effect;
                const std::string difference = LookRecorder::FirstDifference(code, data);
                CHECK_MESSAGE(difference.empty(), difference);
            }
        }
        scope.SetServerMap(0, 0);
    }
}

// MU_MONSTER_LOOK_DUMP=<numbers> writes each mesh draw of those numbers as they draw now, for writing their looks.
TEST_CASE("Monster looks: the mesh draws of numbers, for writing their looks [monsters][looks][recorder]")
{
    const char* dump = std::getenv("MU_MONSTER_LOOK_DUMP");
    if (dump == nullptr)
        return;
    MonsterRecorder::CreationScope scope(0);
    const MonsterRecorder::ShippedLooks looks;
    for (const int number : SplitNumbers(dump))
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        REQUIRE(monster != nullptr);
        for (const MapRef& map : MapsOf(*monster))
        {
            scope.SetServerMap(map.map, map.stage);
            CHARACTER* character = scope.Create(number);
            REQUIRE(character != nullptr);
            for (const MonsterRecorder::DrawSituation& situation :
                 MonsterRecorder::DrawSituations(character->Object.Type))
            {
                const LookRecorder::Record record = MonsterRecorder::RecordMonster(*character, situation);
                std::string text = MonsterRecorder::BaselineKey(number, map, situation) + " model " +
                                   std::to_string(character->Object.Type) + "\n";
                for (const auto& draw : record.meshDraws)
                {
                    for (const auto& [name, value] : draw)
                        text += std::string(" ") + name + "=" + value;
                    text += "\n";
                }
                for (const auto& [name, value] : record.modelAfter)
                    text += std::string(" after ") + name + "=" + value;
                MESSAGE(text);
            }
        }
        scope.SetServerMap(0, 0);
    }
}
