#include "stdafx.h"

// The setup at spawn from the monster data (monsters design, MN4): what the
// moves of the old creators and of the packets' lines brought into the data,
// checked as the game shows it. The moves were compared with the old code
// case by case (D50) before it went.

#include "MonsterCreationRecorder.h"
#include "../data/BuffTableScope.h"
#include "../data/LogCapture.h"

#include "doctest.h"

#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/BuffData/BuffStorage.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/UnknownMonsterLook.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "GameLogic/Monsters/UnknownMonsters.h"
#include "Render/Effects/StatDownDebuffLights.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/AI/GOBoid.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzOpenData.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "GameLogic/Monsters/MonsterFacing.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/BoneManager.h"
#include "Render/Models/ZzzBMD.h"
#include "GameLogic/NPCs/npcGateSwitch.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GM_Raklion.h"
#include "World/GameMaps/MonsterSpawnWorld.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "I18N/All.h"

#include "Data/GameData/MapData/MapRef.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
using MonsterRecorder::CreationPath;
using MonsterRecorder::CreationSlot;
} // namespace

namespace
{
// The facing 0x13 gives a monster it makes at the tile, for the server's
// direction (1 to 8).
float FacingOnArrival(int number, int tileX, int tileY, int direction)
{
    constexpr int Key = 0x0123;
    CHARACTER* character = CreateMonster(static_cast<EMonsterType>(number), tileX, tileY, Key);
    REQUIRE(character != nullptr);
    return GameLogic::Monsters::Facing::AngleOnArrival(*character, direction, World::Monsters::GetSpawnWorld());
}

// The facings the old 0x13 checks of the models gave.
float ExpectedFacing(int number, int tileX, int tileY, int direction)
{
    float step = 45.0f;
    switch (number)
    {
    case MONSTER_SANTA_CLAUSE:
        return 20.0f;
    case MONSTER_MARKET_UNION_MEMBER_JULIA:
        return 49.0f;
    case MONSTER_LITTLE_SANTA_GREEN:
    case MONSTER_LITTLE_SANTA_RED:
    case MONSTER_LITTLE_SANTA_BLUE:
    case MONSTER_LITTLE_SANTA_WHITE:
    case MONSTER_LITTLE_SANTA_ORANGE:
    case MONSTER_LITTLE_SANTA_PINK:
        return 90.0f;
    case MONSTER_GATEWAY_MACHINE:
        step = 90.0f;
        break;
    case MONSTER_BALLISTA:
        if (tileY == 90)
            step = 0.0f;
        if (tileX == 62)
            step = -135.0f;
        if (tileX == 183)
            step = 90.0f;
        break;
    default:
        break;
    }
    return (static_cast<float>(direction) - 1.0f) * step;
}

#ifdef _EDITOR
// The monster's entry as it was, put back when the test ends, also after a
// failed REQUIRE: a test's change of the monster list stays in the test.
class MonsterEntryScope
{
public:
    explicit MonsterEntryScope(int number)
    {
        const Data::Monsters::MonsterDefinition* entry = g_MonsterDatabase.Find(number);
        REQUIRE(entry != nullptr);
        m_kept = *entry;
    }
    ~MonsterEntryScope()
    {
        g_MonsterDatabase.Set(m_kept);
    }
    MonsterEntryScope(const MonsterEntryScope&) = delete;
    MonsterEntryScope& operator=(const MonsterEntryScope&) = delete;

    const Data::Monsters::MonsterDefinition& Original() const
    {
        return m_kept;
    }

private:
    Data::Monsters::MonsterDefinition m_kept;
};
#endif // _EDITOR

// The temple's points as the server sends them (0xBF 0x01): a point that
// changes shows its storage's point effect.
void SendTemplePoints(int allied, int illusion)
{
    PMSG_CURSED_TAMPLE_STATE state{};
    state.btUserIndex = 0xffff;
    state.btAlliedPoint = static_cast<BYTE>(allied);
    state.btIllusionPoint = static_cast<BYTE>(illusion);
    g_CursedTemple->ReceiveCursedTempleInfo(reinterpret_cast<const BYTE*>(&state));
}
} // namespace

// The facing a monster gets when the server sends it (0x13) comes from its
// entry (Q-MN38): a fixed facing, a step of its own, the Ballista's steps by
// tile, else 45 by direction.
TEST_CASE("Monster setup: a monster the server sends faces by its entry (Q-MN38) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    struct Tile
    {
        int x;
        int y;
    };
    constexpr int Numbers[] = {0,
                               MONSTER_BALLISTA,
                               MONSTER_GATEWAY_MACHINE,
                               MONSTER_SANTA_CLAUSE,
                               MONSTER_LITTLE_SANTA_GREEN,
                               MONSTER_LITTLE_SANTA_RED,
                               MONSTER_LITTLE_SANTA_BLUE,
                               MONSTER_LITTLE_SANTA_WHITE,
                               MONSTER_LITTLE_SANTA_ORANGE,
                               MONSTER_LITTLE_SANTA_PINK,
                               MONSTER_MARKET_UNION_MEMBER_JULIA,
                               MONSTER_ANGELA_THE_SUPPLIER,
                               MONSTER_GATEKEEPER};
    for (const int number : Numbers)
    {
        // The Ballista and the Gateway Machine on their own worlds (Crywolf
        // 1st, Kanturu 2nd); the others on world 0.
        for (const int world : {number == MONSTER_BALLISTA ? 34 : number == MONSTER_GATEWAY_MACHINE ? 38 : 0})
        {
            scope.SetServerMap(world, 0);
            for (const Tile tile : {Tile{102, 100}, Tile{62, 100}, Tile{183, 100}, Tile{102, 90}})
            {
                for (const int direction : {1, 3, 6, 8})
                {
                    INFO("monster " << number << " on world " << world << " at " << tile.x << ", " << tile.y
                                    << " facing " << direction);
                    CHECK(FacingOnArrival(number, tile.x, tile.y, direction) ==
                          ExpectedFacing(number, tile.x, tile.y, direction));
                }
            }
        }
    }
    scope.SetServerMap(0, 0);
}

// The monster list changes for them only in the editor build.
#ifdef _EDITOR
// A facing a state variant sets is the facing on arrival while the state is
// on, as the spawn takes the state's fields.
TEST_CASE("Monster setup: a monster faces by its state variant on arrival (Q-MN38) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    const std::string state(Data::Monsters::GetMonsterStates().front());
    Data::Monsters::MonsterDefinition changed = original;
    Data::Monsters::MonsterStateVariant variant;
    variant.state = state;
    variant.setup.facing = 90.0f;
    changed.setup.stateVariants.push_back(variant);
    g_MonsterDatabase.Set(changed);
    bool on = true;
    const GameLogic::Monsters::Setup::SpawnWorld world{[&](std::string_view asked) { return on && asked == state; },
                                                       nullptr};
    using GameLogic::Monsters::Facing::AngleOnArrival;

    CHECK(AngleOnArrival(MONSTER_BUDGE_DRAGON, 100, 100, 4, world) == 90.0f);
    on = false;
    CHECK(AngleOnArrival(MONSTER_BUDGE_DRAGON, 100, 100, 4, world) == 135.0f);
}

// 0x13 faces by the server's tile, not the one a tile offset moves the body to.
TEST_CASE("Monster setup: a monster with a tile offset faces by the server's tile [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.tileOffset = std::array<int, 2>{1, 0};
    Data::Monsters::MonsterTileFacing tile;
    tile.tileX = 102;
    tile.step = 10.0f;
    changed.setup.facingByTile = {tile};
    g_MonsterDatabase.Set(changed);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    // The packet's monster stands on tile 102 and has the direction 3.
    const CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON, CreationPath::Viewport);
    REQUIRE(budge != nullptr);
    CHECK(budge->PositionX == 102);
    CHECK(budge->Object.Angle[2] == 20.0f);
}

// A start action the model does not have is left out: the monster starts
// with its first action, as without one.
TEST_CASE("Monster setup: a start action past the model's actions is left out [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    Data::Monsters::MonsterDefinition changed = original;
    for (const int action : {-1, 5000})
    {
        INFO("start action " << action);
        changed.setup.startAction = action;
        g_MonsterDatabase.Set(changed);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        const CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
        REQUIRE(budge != nullptr);
        CHECK(budge->Object.CurrentAction < Models[budge->Object.Type].NumActions);
    }
}
#endif // _EDITOR

// A player transformed into the Giant is smaller (Q-MN38d), and only then.
TEST_CASE("Monster setup: a player transformed into the Giant is smaller [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* transformed = scope.Create(MONSTER_GIANT, CreationPath::Transform);
    REQUIRE(transformed != nullptr);
    CHECK(transformed->Object.Scale == doctest::Approx(0.8f));
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* monster = scope.Create(MONSTER_GIANT, CreationPath::Viewport);
    REQUIRE(monster != nullptr);
    CHECK(monster->Object.Scale == doctest::Approx(1.6f));
}

// A cutscene sets the walk speed of the model of the monster it summons: the
// models its own list named before (MN4), on the cutscenes' maps; it puts
// the speed back when it removes its monsters (P27).
TEST_CASE("Monster setup: a cutscene's walk speed goes to the summoned monster's model [monsters][creation]")
{
    struct Summoned
    {
        int number;
        int world;
        int model;
    };
    constexpr Summoned List[] = {
        {MONSTER_BALRAM, WD_34CRYWOLF_1ST, MODEL_BALRAM},
        {MONSTER_SORAM, WD_34CRYWOLF_1ST, MODEL_SORAM},
        {MONSTER_DARKELF, WD_34CRYWOLF_1ST, MODEL_DARK_ELF_1},
        {MONSTER_DARK_ELF, WD_34CRYWOLF_1ST, MODEL_DARK_ELF_1},
        {MONSTER_DEATH_SPIRIT, WD_34CRYWOLF_1ST, MODEL_DEATH_SPIRIT},
        {MONSTER_BALLISTA, WD_34CRYWOLF_1ST, MODEL_BALLISTA},
        {MONSTER_BALGASS, WD_34CRYWOLF_1ST, MODEL_BALGASS},
        {MONSTER_NIGHTMARE, WD_39KANTURU_3RD, MODEL_DARK_SKULL_SOLDIER_5},
    };
    for (const Summoned& summoned : List)
    {
        INFO("monster " << summoned.number);
        MonsterRecorder::CreationScope scope(summoned.world);
        scope.SetServerMap(summoned.world, 0);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        g_Direction.stl_Monster.clear();
        // One without a speed opens the model with its own.
        g_Direction.SummonCreateMonster(static_cast<EMonsterType>(summoned.number), 100, 100, 0.0f, false, false);
        REQUIRE(Models[summoned.model].Actions != nullptr);
        const float own = Models[summoned.model].Actions[MONSTER01_WALK].PlaySpeed;
        constexpr float Speed = 0.5f;
        g_Direction.SummonCreateMonster(static_cast<EMonsterType>(summoned.number), 102, 100, 0.0f, false, false,
                                        Speed);
        CHECK(Models[summoned.model].Actions[MONSTER01_WALK].PlaySpeed == Speed);
        // Another of the model with another speed; the own speed comes back.
        g_Direction.SummonCreateMonster(static_cast<EMonsterType>(summoned.number), 104, 100, 0.0f, false, false,
                                        0.25f);
        g_Direction.DeleteMonster();
        CHECK(Models[summoned.model].Actions[MONSTER01_WALK].PlaySpeed == own);
        scope.SetServerMap(0, 0);
    }
}

// A cutscene monster whose model has no walk (the empty model `none`) gets
// no walk speed and keeps none to put back.
TEST_CASE("Monster setup: a cutscene's walk speed skips a model without a walk [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(WD_34CRYWOLF_1ST);
    scope.SetServerMap(69, 0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    g_Direction.stl_Monster.clear();
    REQUIRE(Models[Data::Monsters::NoModelSlot].Actions == nullptr);
    g_Direction.SummonCreateMonster(static_cast<EMonsterType>(523), 100, 100, 0.0f, false, false, 0.5f);
    CHECK(Models[Data::Monsters::NoModelSlot].Actions == nullptr);
    g_Direction.DeleteMonster();
    scope.SetServerMap(0, 0);
}

// The Dark Elf 412 brings a crater, two blue blurs and five red shock waves
// at its bone 27 at its first frame on screen (Q-MN38c), once, whichever way
// it came.
TEST_CASE("Monster setup: the Dark Elf's crater shows at its first frame [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(WD_42CHANGEUP3RD_2ND);
    scope.SetServerMap(WD_42CHANGEUP3RD_2ND, 0);
    for (const CreationPath path : {CreationPath::Viewport, CreationPath::ViewportNew})
    {
        INFO("by " << std::string(MonsterRecorder::Describe(path)));
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        CHARACTER* elf = scope.Create(MONSTER_DARK_ELF_TRAINEE_SOLDIER, path);
        REQUIRE(elf != nullptr);
        const MonsterRecorder::Record first = MonsterRecorder::RecordVisualFrame(scope, *elf, 11u);
        const MonsterRecorder::Record second = MonsterRecorder::RecordVisualFrame(scope, *elf, 12u);
        const auto count = [](const MonsterRecorder::Record& record, const char* pool, int type)
        {
            return std::ranges::count_if(record,
                                         [&](const MonsterRecorder::RecordedValue& value)
                                         {
                                             return value.path.starts_with(pool) && value.path.ends_with("].Type") &&
                                                    value.value == std::to_string(type);
                                         });
        };
        CHECK(count(first, "Effects[", BITMAP_CRATER) == 1);
        CHECK(count(first, "Particles[", BITMAP_BLUE_BLUR) == 2);
        CHECK(count(first, "Effects[", BITMAP_SHOCK_WAVE) == 5);
        CHECK(count(second, "Effects[", BITMAP_CRATER) == 0);
        CHECK(count(second, "Effects[", BITMAP_SHOCK_WAVE) == 0);
    }
    scope.SetServerMap(0, 0);
}

// The entries' appear and teleport are what the packets run (Q-MN38c): the
// Golden Dragon's stand and sound, Crywolf's thunder unless the battle is
// about to start, the Nightmare's sparks.
TEST_CASE("Monster setup: a new monster comes in by its entry's appear [monsters][creation]")
{
    const GameLogic::Monsters::Setup::SpawnWorld& world = World::Monsters::GetSpawnWorld();
    MonsterRecorder::CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* dragon = scope.Create(MONSTER_GOLDEN_DRAGON);
    REQUIRE(dragon != nullptr);
    GameLogic::Monsters::Setup::Appear(*dragon, world);
    CHECK(dragon->Object.CurrentAction == MONSTER01_STOP2);
    CHECK(dragon->Object.Alpha == 1.0f);

    scope.SetServerMap(WD_34CRYWOLF_1ST, 0);
    for (const bool ready : {false, true})
    {
        INFO((ready ? "Crywolf ready" : "Crywolf not ready"));
        scope.SetSpawnState("crywolfReady", ready);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        CHARACTER* balram = scope.Create(MONSTER_BALRAM);
        REQUIRE(balram != nullptr);
        GameLogic::Monsters::Setup::Appear(*balram, world);
        CHECK(balram->Object.Alpha == (ready ? 1.0f : 0.0f));
    }
    scope.SetSpawnState("crywolfReady", false);

    scope.SetServerMap(WD_39KANTURU_3RD, 0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* nightmare = scope.Create(MONSTER_NIGHTMARE);
    REQUIRE(nightmare != nullptr);
    scope.Load().TakePlayedSounds();
    GameLogic::Monsters::Setup::Teleport(*nightmare, world);
    const std::vector<MonsterRecorder::PlayedSound> sounds = scope.Load().TakePlayedSounds();
    REQUIRE(sounds.size() == 1);
    CHECK(sounds[0].id == SOUND_KANTURU_3RD_NIGHTMARE_TELE);
    scope.SetServerMap(0, 0);
}

// The monster list changes for them only in the editor build.
#ifdef _EDITOR
// The castle asks for a gate alone: it is the one the whole setup has, on the
// map and with a state on or off.
TEST_CASE("Monster setup: a character's gate is its setup's gate [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    scope.SetServerMap(WD_30BATTLECASTLE, 0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    const std::string state(Data::Monsters::GetMonsterStates().front());
    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.gate = Data::Monsters::MonsterGate{"stop1", "attack1", "stop2", true};
    Data::Monsters::MonsterMapVariant map;
    map.maps = {Data::Maps::MapRef{WD_30BATTLECASTLE, 0}};
    map.setup.gate = Data::Monsters::MonsterGate{"stop2", "attack2", "walk", false};
    Data::Monsters::MonsterStateVariant variant;
    variant.state = state;
    variant.setup.gate = Data::Monsters::MonsterGate{"walk", "stop1", "attack1", false};
    map.setup.stateVariants.push_back(variant);
    changed.mapVariants.push_back(map);
    g_MonsterDatabase.Set(changed);
    CHARACTER character;
    character.MonsterIndex = MONSTER_BUDGE_DRAGON;
    using GameLogic::Monsters::Setup::GateOf;
    using GameLogic::Monsters::Setup::SetupNow;

    for (const int serverMap : std::array<int, 2>{WD_30BATTLECASTLE, 0})
    {
        for (const bool on : {false, true})
        {
            INFO("map " << serverMap << (on ? " with the state on" : ""));
            scope.SetServerMap(serverMap, 0);
            const GameLogic::Monsters::Setup::SpawnWorld world{[&](std::string_view asked)
                                                               { return on && asked == state; }, nullptr};
            const std::optional<Data::Monsters::MonsterGate> whole = SetupNow(changed, world).gate;
            const Data::Monsters::MonsterGate* alone = GateOf(character, world);
            REQUIRE(whole.has_value());
            REQUIRE(alone != nullptr);
            CHECK(*alone == *whole);
        }
    }
    scope.SetServerMap(0, 0);
}
#endif // _EDITOR

// Castle Gate 277 shows what its buff says (Q-MN38c), from its entry's gate:
// when it comes into view (0x13, and 0x68, which closes only a gate without
// buffs), when the switch opens or closes it, and when one of its animations
// ends.
TEST_CASE("Monster setup: a castle gate opens and closes by its entry's gate [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(WD_30BATTLECASTLE);
    scope.SetServerMap(WD_30BATTLECASTLE, 0);
    struct Arrival
    {
        CreationPath path;
        std::vector<int> buffs;
        int action;
    };
    // Closed and open; another buff leaves 0x68's gate at the action it
    // starts with.
    const Arrival Arrivals[] = {
        {CreationPath::Viewport, {}, 0},
        {CreationPath::Viewport, {eBuff_CastleGateIsOpen}, 1},
        {CreationPath::Viewport, {eBuff_CastleRegimentDefense}, 0},
        {CreationPath::PreviewPort, {}, 0},
        {CreationPath::PreviewPort, {eBuff_CastleGateIsOpen}, 1},
    };
    for (const Arrival& arrival : Arrivals)
    {
        INFO("by " << std::string(MonsterRecorder::Describe(arrival.path)) << " with " << arrival.buffs.size()
                   << " buffs");
        scope.SetPacketBuffs(arrival.buffs);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        const CHARACTER* gate = scope.Create(MONSTER_CASTLE_GATE1, arrival.path);
        REQUIRE(gate != nullptr);
        CHECK(gate->Object.CurrentAction == arrival.action);
    }
    scope.SetPacketBuffs({});

    // The switch and the end of an animation, open and closed.
    for (const bool open : {false, true})
    {
        INFO((open ? "open" : "closed"));
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        CHARACTER* gate = scope.Create(MONSTER_CASTLE_GATE1);
        REQUIRE(gate != nullptr);
        if (open)
            npcGateSwitch::GateOpen(gate, &gate->Object);
        else
            npcGateSwitch::GateClose(gate, &gate->Object);
        CHECK(gate->Object.CurrentAction == (open ? 1 : 0));
        CHECK(battleCastle::StopBattleCastleMonster(gate, &gate->Object));
        CHECK(gate->Object.CurrentAction == (open ? 2 : 0));
        CHECK(gate->m_bIsSelected == !open);
    }

    // An open gate that stands open as it comes into view. The switch opens
    // it in view (ReceiveGateCurrentState: GateOpen registers the buff by the
    // macro, which NoteServerBuff does not see); Respawn keeps it.
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* gate = scope.Create(MONSTER_CASTLE_GATE1);
    REQUIRE(gate != nullptr);
    npcGateSwitch::GateOpen(gate, &gate->Object);
    battleCastle::GateLooks standing;
    standing.arrivalOpening = false;
    battleCastle::ShowGateOnArrival(*gate, standing, 102, 100, false);
    CHECK(gate->Object.CurrentAction == 2);

    // Respawn makes the open gate again as 0x13 shows it: open, not closed.
    CHECK(World::Monsters::RespawnLive(MONSTER_CASTLE_GATE1, false) >= 1);
    CHECK(gate->Object.m_BuffMap.isBuff(eBuff_CastleGateIsOpen));
    CHECK(gate->Object.CurrentAction == 1);
    scope.SetServerMap(0, 0);
}

// 0x07 for a buff a character shows already runs the Crywolf hero contract
// rule only for the altars and the NPCs it names: on another character it
// keeps its Crywolf states (the rule's condition was always true).
TEST_CASE("Monster setup: a buff sent again keeps a monster's Crywolf states [monsters][creation]")
{
    using MonsterRecorder::CreationScope;
    CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    CreationScope::SendBuffState(*budge, eDeBuff_Poison, true);
    REQUIRE(budge->Object.m_BuffMap.isBuff(eDeBuff_Poison));
    g_CharacterRegisterBuff((&budge->Object), eBuff_CrywolfNPCHide);

    CreationScope::SendBuffState(*budge, eDeBuff_Poison, true);
    CHECK(budge->Object.m_BuffMap.isBuff(eDeBuff_Poison));
    CHECK(budge->Object.m_BuffMap.isBuff(eBuff_CrywolfNPCHide));

    // A Crywolf altar still runs it: its altar state goes with the token list.
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* altar = scope.Create(MONSTER_WOLF_ALTAR1);
    REQUIRE(altar != nullptr);
    REQUIRE(altar->Object.Type == MODEL_CRYWOLF_ALTAR1);
    g_CharacterRegisterBuff((&altar->Object), eBuff_CrywolfAltarEnable);
    CreationScope::SendBuffState(*altar, eBuff_CrywolfAltarEnable, true);
    CHECK_FALSE(altar->Object.m_BuffMap.isBuff(eBuff_CrywolfAltarEnable));
}

// 0x07 for a stat-down debuff gives a monster its pair of lights, and taking the debuff removes them. Monsters get
// these lights only from this packet. The shipped buff table says it is a debuff, which the packet's handler needs
// before it draws anything.
TEST_CASE("Monster setup: 0x07 gives a monster the stat-down lights and takes them away [monsters][creation]")
{
    using MonsterRecorder::CreationScope;
    const Data::Buffs::BuffsLoadResult shipped =
        Data::Buffs::LoadBuffsFile(std::filesystem::path(MU_TEST_DATA_DIR) / "Buffs" / "Buffs.json");
    REQUIRE_FALSE(shipped.buffs.empty());
    const BuffTableScope table(shipped.buffs, "en");
    CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    OBJECT* o = &budge->Object;
    auto lights = [o]
    {
        int count = 0;
        for (int i = 0; i < MAX_EFFECTS; ++i)
        {
            const OBJECT& effect = Effects[i];
            if (effect.Live && effect.Owner == o && effect.SubType == 1 &&
                (effect.Type == BITMAP_SHINY + 6 || effect.Type == BITMAP_PIN_LIGHT))
                ++count;
        }
        return count;
    };

    CreationScope::SendBuffState(*budge, eDeBuff_AttackDown, true);
    CHECK(lights() == 2);
    // A debuff the monster shows already adds nothing: ReceiveSkillStatus doesn't register it again. This doesn't reach
    // the check in StatDownDebuffLights::Create, which guards the in-place re-create instead.
    CreationScope::SendBuffState(*budge, eDeBuff_AttackDown, true);
    CHECK(lights() == 2);

    CreationScope::SendBuffState(*budge, eDeBuff_AttackDown, false);
    CHECK(lights() == 0);
    Render::Effects::StatDownDebuffLights::Delete(o, eDeBuff_AttackDown); // none outlives the monster
}

// Molt's model file is missing: it is tried once a map, not at each spawn,
// and the map change forgets it (Q-MN45c).
TEST_CASE("Monster setup: a missing model file is tried once a map [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const int molt = MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_MOLT);
    REQUIRE_FALSE(Data::Monsters::ModelLoader::IsFileMissing(molt));
    REQUIRE(scope.Create(MONSTER_MOLT) != nullptr);
    CHECK(Data::Monsters::ModelLoader::IsFileMissing(molt));
    REQUIRE(scope.Create(MONSTER_MOLT) != nullptr);
    CHECK(Data::Monsters::ModelLoader::IsFileMissing(molt));
    DeleteMonsters();
    CHECK_FALSE(Data::Monsters::ModelLoader::IsFileMissing(molt));
}

// Maya 364 is an invisible stand-in for the giant Maya of Kanturu's 3rd map
// scene: her model has no file, and she no longer opens the Bull Fighter's
// (Q-MN45b; her creator did).
TEST_CASE("Monster setup: Maya opens no Bull Fighter model [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(WD_39KANTURU_3RD);
    scope.SetServerMap(WD_39KANTURU_3RD, 0);
    REQUIRE(Models[MODEL_BULL_FIGHTER].NumMeshs <= 0);
    CHARACTER* maya = scope.Create(MONSTER_MAYA);
    REQUIRE(maya != nullptr);
    CHECK(maya->Object.Type == MODEL_MAYA);
    CHECK(Models[MODEL_BULL_FIGHTER].NumMeshs <= 0);
    scope.SetServerMap(0, 0);
}

namespace
{
// The unknown monsters' look the test sets, until the test ends.
class UnknownLookScope
{
public:
    explicit UnknownLookScope(const Data::Monsters::UnknownMonsterLook& look)
        : m_kept(g_MonsterDatabase.GetUnknownLook())
    {
        g_MonsterDatabase.SetUnknownLook(look);
    }
    ~UnknownLookScope()
    {
        g_MonsterDatabase.SetUnknownLook(m_kept);
    }
    UnknownLookScope(const UnknownLookScope&) = delete;
    UnknownLookScope& operator=(const UnknownLookScope&) = delete;

private:
    Data::Monsters::UnknownMonsterLook m_kept;
};
} // namespace

// A monster the game cannot show as itself has the unknown monsters' look
// (D64): a number without an entry (named "Unknown monster 700"), an entry
// that names no model (76, with its own name), a model whose file is missing
// (Molt 68). The look's body is the Ghost at scale 1, or the look's own. A
// reason the look is not for keeps the Bull Fighter's body, and a missing
// file no body.
TEST_CASE("Monster setup: the monsters the game cannot show as themselves have the bug look (D64) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const int ghost = MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_GHOST);
    const int molt = MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_MOLT);
    REQUIRE(g_MonsterDatabase.Find(700) == nullptr);
    REQUIRE(g_MonsterDatabase.Find(76) != nullptr);
    REQUIRE(g_MonsterDatabase.GetUnknownLook() == Data::Monsters::UnknownMonsterLook{});
    std::wstring unknownName = I18N::Game::UnknownMonsterName;
    REQUIRE(unknownName.find(L"%d") != std::wstring::npos);
    unknownName.replace(unknownName.find(L"%d"), 2, L"700");

    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* noEntry = scope.Create(700);
    REQUIRE(noEntry != nullptr);
    CHECK(noEntry->Object.Type == ghost);
    CHECK(noEntry->Object.Scale == 1.0f);
    CHECK(Models[ghost].NumMeshs > 0);
    CHECK(std::wstring(noEntry->ID) == unknownName);
    CHECK(noEntry->MonsterIndex == 700);

    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* noModel = scope.Create(76);
    REQUIRE(noModel != nullptr);
    CHECK(noModel->Object.Type == ghost);
    CHECK(std::wstring(noModel->ID) == g_MonsterDatabase.FindDisplayName(76));
    CHECK(noModel->Object.Kind == KIND_MONSTER);

    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* missingFile = scope.Create(MONSTER_MOLT);
    REQUIRE(missingFile != nullptr);
    CHECK(missingFile->Object.Type == ghost);
    CHECK(Data::Monsters::ModelLoader::IsFileMissing(molt));

    {
        Data::Monsters::UnknownMonsterLook own;
        own.model = "MONSTER_MODEL_HOUND";
        own.scale = 1.5f;
        const UnknownLookScope set(own);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        const CHARACTER* hound = scope.Create(700);
        REQUIRE(hound != nullptr);
        CHECK(hound->Object.Type == MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_HOUND));
        CHECK(hound->Object.Scale == 1.5f);
    }

    Data::Monsters::UnknownMonsterLook off;
    off.forNoEntry = false;
    off.forNoModel = false;
    off.forMissingFile = false;
    const UnknownLookScope set(off);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* bullFighter = scope.Create(700);
    REQUIRE(bullFighter != nullptr);
    CHECK(bullFighter->Object.Type == MODEL_BULL_FIGHTER);
    CHECK(std::wstring(bullFighter->ID) != unknownName);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* bullFighterEntry = scope.Create(76);
    REQUIRE(bullFighterEntry != nullptr);
    CHECK(bullFighterEntry->Object.Type == MODEL_BULL_FIGHTER);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* noBody = scope.Create(MONSTER_MOLT);
    REQUIRE(noBody != nullptr);
    CHECK(noBody->Object.Type == molt);
    CHECK(Models[molt].NumMeshs <= 0);
}

namespace
{
std::uint32_t g_unknownNow = 0;
std::vector<std::wstring> g_unknownSaid;

std::uint32_t UnknownNow()
{
    return g_unknownNow;
}

void RememberUnknownSaid(CHARACTER&, const wchar_t* text)
{
    g_unknownSaid.emplace_back(text);
}

// The test's clock and speaker for the unknown monsters, until the test ends,
// in the client folder (the marks' texture loads from there).
class UnknownShowScope
{
public:
    UnknownShowScope() : m_previousFolder(std::filesystem::current_path())
    {
        std::filesystem::current_path(MonsterRecorder::ClientFolder());
        GameLogic::Monsters::Unknown::SetClock(UnknownNow);
        GameLogic::Monsters::Unknown::SetSpeaker(RememberUnknownSaid);
        g_unknownSaid.clear();
    }
    ~UnknownShowScope()
    {
        GameLogic::Monsters::Unknown::SetClock(nullptr);
        GameLogic::Monsters::Unknown::SetSpeaker(nullptr);
        std::filesystem::current_path(m_previousFolder);
    }
    UnknownShowScope(const UnknownShowScope&) = delete;
    UnknownShowScope& operator=(const UnknownShowScope&) = delete;

private:
    std::filesystem::path m_previousFolder;
};

struct ShownSprites
{
    int glows = 0;
    int marks = 0;
    vec3_t glowLight{};
};

// Shows the look at the time, in a frame of its own, and counts its sprites.
ShownSprites ShowAt(CHARACTER& character, std::uint32_t now)
{
    g_unknownNow = now;
    ClearSprites();
    GameLogic::Monsters::Unknown::ShowOnScreen(character);
    ShownSprites shown;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        const OBJECT& sprite = Sprites[i];
        if (!sprite.Live || sprite.Owner != &character.Object)
            continue;
        if (sprite.Type == BITMAP_LIGHT)
        {
            ++shown.glows;
            VectorCopy(sprite.Light, shown.glowLight);
        }
        else
        {
            ++shown.marks;
        }
    }
    return shown;
}
} // namespace

// A number the game cannot show as itself is logged once, also when the look
// is not for its reason and it gets the Bull Fighter's body (D64).
TEST_CASE("Monster setup: the Bull Fighter's body for an unknown number is logged (D64) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    Data::Monsters::UnknownMonsterLook off;
    off.forNoEntry = false;
    const UnknownLookScope set(off);
    const LogCapture log("data");
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    REQUIRE(scope.Create(709) != nullptr);
    CHECK(log.LineWith("Monster 709 has the Bull Fighter's body").find("no entry") != std::string::npos);
}

// The bug look shows on screen (D64): a red glow that pulses, red X marks
// rising, and "I am a bug: 700" when it first is on screen and again every 10
// seconds; without marks and with messageSeconds 0 neither marks nor a second
// message. A character without the look, or whose slot holds another monster
// now, shows none.
TEST_CASE("Monster setup: the bug look glows, sends up marks and says it is a bug (D64) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const UnknownShowScope show;
    std::wstring message = I18N::Game::UnknownMonsterMessage;
    REQUIRE(message.find(L"%d") != std::wstring::npos);
    message.replace(message.find(L"%d"), 2, L"700");

    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* bug = scope.Create(700);
    REQUIRE(bug != nullptr);

    const ShownSprites first = ShowAt(*bug, 5000);
    CHECK(first.glows == 1);
    CHECK(first.glowLight[0] > 0.0f);
    CHECK(first.glowLight[0] > first.glowLight[1]);
    CHECK(first.glowLight[0] > first.glowLight[2]);
    CHECK(first.marks == 1);
    REQUIRE(g_unknownSaid.size() == 1);
    CHECK(g_unknownSaid[0] == message);

    // The marks rise in turn, at most 8; it says it again after 10 seconds.
    int mostMarks = 0;
    float dimmest = first.glowLight[0];
    float brightest = first.glowLight[0];
    for (std::uint32_t now = 5100; now < 15000; now += 100)
    {
        const ShownSprites frame = ShowAt(*bug, now);
        CHECK(frame.glows == 1);
        mostMarks = std::max(mostMarks, frame.marks);
        dimmest = std::min(dimmest, frame.glowLight[0]);
        brightest = std::max(brightest, frame.glowLight[0]);
    }
    CHECK(mostMarks > 1);
    CHECK(mostMarks <= 8);
    CHECK(brightest > dimmest + 0.3f);
    CHECK(g_unknownSaid.size() == 1);
    ShowAt(*bug, 15000);
    CHECK(g_unknownSaid.size() == 2);

    {
        Data::Monsters::UnknownMonsterLook quiet;
        quiet.marks = false;
        quiet.messageSeconds = 0;
        const UnknownLookScope set(quiet);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        CHARACTER* still = scope.Create(700);
        REQUIRE(still != nullptr);
        g_unknownSaid.clear();
        CHECK(ShowAt(*still, 20000).marks == 0);
        CHECK(ShowAt(*still, 60000).glows == 1);
        CHECK(g_unknownSaid.size() == 1);
    }

    // The slot gets a monster with its own body.
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    g_unknownSaid.clear();
    CHECK(ShowAt(*budge, 70000).glows == 0);
    CHECK(g_unknownSaid.empty());

    // The Bull Fighter's body for a reason the look is not for.
    Data::Monsters::UnknownMonsterLook off;
    off.forNoEntry = false;
    const UnknownLookScope set(off);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* bullFighter = scope.Create(700);
    REQUIRE(bullFighter != nullptr);
    CHECK(ShowAt(*bullFighter, 80000).glows == 0);
    ClearSprites();
}

#ifdef _EDITOR
// The live changes come from the monster editor (MonsterDatabase::Set).
// Living monsters take a change of the fields they take at once (D55,
// Q-MN39): their look, equipment and speed now, from the values they had
// before their setup, so a field the entry no longer has goes back to the
// game's; a field of the next spawn waits.
TEST_CASE("Monster setup: living monsters take a change of their look at once, and lose it again (D55) "
          "[monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    const float scale = budge->Object.Scale;
    const float outline = budge->Object.m_fEdgeScale;
    const short weapon = budge->Weapon[0].Type;
    const unsigned short action = budge->Object.CurrentAction;

    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.scale = scale + 0.5f;
    changed.setup.outline = 2.0f;
    changed.setup.moveSpeed = 20;
    changed.setup.equipment.weapons[0] = Data::Monsters::MonsterItem{0, 0, 7};
    changed.setup.startAction = 5;
    g_MonsterDatabase.Set(changed);
    GameLogic::Monsters::Setup::ApplyLive(MONSTER_BUDGE_DRAGON, World::Monsters::GetSpawnWorld());
    CHECK(budge->Object.Scale == doctest::Approx(scale + 0.5f));
    CHECK(budge->Object.m_fEdgeScale == 2.0f);
    CHECK(budge->MoveSpeed == 20);
    CHECK(budge->Weapon[0].Type == MODEL_ITEM);
    CHECK(budge->Weapon[0].Level == 7);
    CHECK(budge->Object.CurrentAction == action);

    // Another number's change leaves it alone.
    GameLogic::Monsters::Setup::ApplyLive(MONSTER_SPIDER, World::Monsters::GetSpawnWorld());
    CHECK(budge->Object.m_fEdgeScale == 2.0f);

    g_MonsterDatabase.Set(original);
    GameLogic::Monsters::Setup::ApplyLive(-1, World::Monsters::GetSpawnWorld());
    CHECK(budge->Object.Scale == doctest::Approx(scale));
    CHECK(budge->Object.m_fEdgeScale == outline);
    CHECK(budge->Weapon[0].Type == weapon);
}

// Respawn live monsters (D55): a field of the next spawn waits until the
// monster appears again; Respawn makes it again in its slot with its key and
// turn, with the new value.
TEST_CASE("Monster setup: Respawn makes the living monsters again with the next spawn's fields (D55) "
          "[monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    const int key = budge->Key;
    budge->Object.Angle[2] = 135.0f;
    const auto& world = World::Monsters::GetSpawnWorld();
    // How many wait for the field, as Monster Setup reads the counts.
    const auto waitingFor = [](int number, std::string_view key, const auto& spawnWorld)
    {
        GameLogic::Monsters::Setup::WaitingCounts waiting;
        GameLogic::Monsters::Setup::CountWaiting(number, spawnWorld, waiting);
        const auto found = waiting.find(key);
        return found != waiting.end() ? found->second : 0;
    };
    CHECK(waitingFor(MONSTER_BUDGE_DRAGON, "startAction", world) == 0);

    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.startAction = 5;
    g_MonsterDatabase.Set(changed);
    CHECK(waitingFor(MONSTER_BUDGE_DRAGON, "startAction", world) == 1);
    CHECK(waitingFor(MONSTER_BUDGE_DRAGON, "scale", world) == 0);
    CHECK(waitingFor(MONSTER_BUDGE_DRAGON, "model", world) == 0);

    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
    CHECK(budge->Object.Live);
    CHECK(budge->Key == key);
    CHECK(budge->MonsterIndex == MONSTER_BUDGE_DRAGON);
    CHECK(budge->Object.CurrentAction == 5);
    CHECK(budge->Object.Angle[2] == 135.0f);
    CHECK(waitingFor(MONSTER_BUDGE_DRAGON, "startAction", world) == 0);
}

// Respawn keeps what the server gave the living monster (an open Castle
// Gate's buff, a Life Stone's build time, the bars), not the buffs of its
// last spawn's setup, and runs its spawn hooks no more.
TEST_CASE("Monster setup: Respawn keeps the server's buffs and runs no hooks again (D55) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    changed.setup.spawnBuffs = {"poison"};
    g_MonsterDatabase.Set(changed);
    const std::optional<int> poison = Data::Monsters::FindMonsterBuff("poison");
    REQUIRE(poison.has_value());
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    REQUIRE(budge->Object.m_BuffMap.isBuff(static_cast<eBuffState>(*poison)));
    // A buff of another path than the noted packets (a macro registration).
    g_CharacterRegisterBuff((&budge->Object), eBuff_CastleGateIsOpen);
    budge->Object.m_byBuildTime = 3;
    budge->HealthStatus = 0.5f;
    budge->ShieldStatus = 0.25f;

    changed.setup.spawnBuffs.clear();
    g_MonsterDatabase.Set(changed);
    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
    CHECK(budge->Object.m_BuffMap.isBuff(eBuff_CastleGateIsOpen));
    CHECK_FALSE(budge->Object.m_BuffMap.isBuff(static_cast<eBuffState>(*poison)));
    CHECK(budge->Object.m_byBuildTime == 3);
    CHECK(budge->HealthStatus == 0.5f);
    CHECK(budge->ShieldStatus == 0.25f);

    // A hook ran at its spawn; a respawn runs it no more.
    CHECK_FALSE(World::Monsters::GetSpawnWorld().isRespawn());
    changed.setup.hooks = {Data::Monsters::MonsterHook{"RaklionBossDrop", {}}};
    int hooksRun = 0;
    bool respawn = false;
    const GameLogic::Monsters::Setup::SpawnWorld world{nullptr, [&](const Data::Monsters::MonsterHook&, CHARACTER&)
                                                       { ++hooksRun; }, [&] { return respawn; }};
    const int key = budge->Key;
    REQUIRE(GameLogic::Monsters::Setup::Create(changed, budge->PositionX, budge->PositionY, key, world, nullptr) !=
            nullptr);
    CHECK(hooksRun == 1);
    respawn = true;
    REQUIRE(GameLogic::Monsters::Setup::Create(changed, budge->PositionX, budge->PositionY, key, world, nullptr) !=
            nullptr);
    CHECK(hooksRun == 1);
}

// Respawn takes off only the last spawn's setup buffs the server did not
// give too: a spawn buff the server gives too stays (a player poisons a Bull
// Fighter that spawns poisoned; 0x07 for a buff it shows already), and so
// does one the server gives again after it ended it (0x1B); one the client
// put on by itself goes with the setup's (a poison cast the server resisted).
TEST_CASE("Monster setup: Respawn keeps the buffs the server gave (D55) [monsters][creation]")
{
    using MonsterRecorder::CreationScope;
    CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BULL_FIGHTER);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    changed.setup.spawnBuffs = {"poison"};
    g_MonsterDatabase.Set(changed);
    REQUIRE(Data::Monsters::FindMonsterBuff("poison") == static_cast<int>(eDeBuff_Poison));
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* bull = scope.Create(MONSTER_BULL_FIGHTER);
    REQUIRE(bull != nullptr);
    const OBJECT& object = bull->Object;
    REQUIRE(object.m_BuffMap.isBuff(eDeBuff_Poison));
    const auto respawn = [&](std::vector<std::string> spawnBuffs)
    {
        changed.setup.spawnBuffs = std::move(spawnBuffs);
        g_MonsterDatabase.Set(changed);
        CHECK(World::Monsters::RespawnLive(MONSTER_BULL_FIGHTER, false) == 1);
    };

    // Only the setup's poison: it goes with the setup.
    respawn({});
    CHECK_FALSE(object.m_BuffMap.isBuff(eDeBuff_Poison));

    // A player poisons the poisoned Bull Fighter: the server's poison stays.
    respawn({"poison"});
    CreationScope::SendBuffState(*bull, eDeBuff_Poison, true);
    respawn({});
    CHECK(object.m_BuffMap.isBuff(eDeBuff_Poison));

    // The server ends it and poisons it again: still the server's.
    respawn({"poison"});
    CreationScope::SendMagicFinish(*bull, AT_SKILL_POISON);
    CHECK_FALSE(object.m_BuffMap.isBuff(eDeBuff_Poison));
    CreationScope::SendBuffState(*bull, eDeBuff_Poison, true);
    respawn({});
    CHECK(object.m_BuffMap.isBuff(eDeBuff_Poison));

    // The setup poisons it too and the server ends its poison; the client
    // poisons it by itself (a cast the server resisted): not the server's,
    // so it goes with the setup's.
    respawn({"poison"});
    CreationScope::SendBuffState(*bull, eDeBuff_Poison, false);
    g_CharacterRegisterBuff((&bull->Object), eDeBuff_Poison);
    respawn({});
    CHECK_FALSE(object.m_BuffMap.isBuff(eDeBuff_Poison));

    // A note of a poison the client took off by itself (the macro, not the
    // server) ends at the next Respawn: the setup's poison after it goes.
    respawn({"poison"});
    CreationScope::SendBuffState(*bull, eDeBuff_Poison, true);
    g_CharacterUnRegisterBuff((&bull->Object), eDeBuff_Poison);
    respawn({"poison"});
    respawn({});
    CHECK_FALSE(object.m_BuffMap.isBuff(eDeBuff_Poison));
}

// The Crywolf hero contract rule of 0x07 is the entries' crywolfContract: a
// monster given it runs the rule too.
TEST_CASE("Monster setup: a monster in the Crywolf hero contract loses its states to a buff sent again "
          "[monsters][creation]")
{
    using MonsterRecorder::CreationScope;
    CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    changed.setup.crywolfContract = true;
    g_MonsterDatabase.Set(changed);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON);
    REQUIRE(budge != nullptr);
    g_CharacterRegisterBuff((&budge->Object), eBuff_CrywolfAltarEnable);
    CreationScope::SendBuffState(*budge, eBuff_CrywolfAltarEnable, true);
    CHECK_FALSE(budge->Object.m_BuffMap.isBuff(eBuff_CrywolfAltarEnable));
}

// The shipped entries with crywolfContract are those the old code named by
// model: the five altars, MODEL_SMITH, MODEL_NPC_SERBIS, MODEL_MERCHANT_MAN,
// MODEL_STORAGE and MODEL_NPC_BREEDER, on the entry and its map variants.
TEST_CASE("Monster setup: the shipped crywolfContract entries are the old code's models [monsters][creation]")
{
    constexpr std::array<std::string_view, 10> OldModels{
        "MODEL_CRYWOLF_ALTAR1", "MODEL_CRYWOLF_ALTAR2", "MODEL_CRYWOLF_ALTAR3", "MODEL_CRYWOLF_ALTAR4",
        "MODEL_CRYWOLF_ALTAR5", "MODEL_SMITH",          "MODEL_NPC_SERBIS",     "MODEL_MERCHANT_MAN",
        "MODEL_STORAGE",        "MODEL_NPC_BREEDER"};
    const auto oldRule = [&](const std::string& model)
    { return std::ranges::find(OldModels, model) != OldModels.end(); };
    const std::vector<Data::Monsters::MonsterDefinition> monsters =
        Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" / "Monsters" / "Monsters.json")
            .monsters;
    REQUIRE(monsters.size() > 500);
    int contracts = 0;
    for (const Data::Monsters::MonsterDefinition& monster : monsters)
    {
        INFO("monster " << monster.number);
        const bool own = monster.setup.crywolfContract.value_or(false);
        CHECK(own == oldRule(monster.model));
        contracts += own ? 1 : 0;
        for (const Data::Monsters::MonsterMapVariant& variant : monster.mapVariants)
        {
            CHECK(variant.setup.crywolfContract.value_or(own) == oldRule(variant.model.value_or(monster.model)));
        }
    }
    CHECK(contracts == 13);
}

// Respawn makes a monster with a tile offset (Elpis) on the server's tile, as
// 0x13 keeps it, so the body stands at the offset once and the tile stays
// the server's; one that walked comes again on the tile it walked to.
TEST_CASE("Monster setup: Respawn adds a tile offset once (D55) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.tileOffset = std::array<int, 2>{1, -1};
    g_MonsterDatabase.Set(changed);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON, CreationPath::Viewport);
    REQUIRE(budge != nullptr);
    const int x = budge->PositionX;
    const int y = budge->PositionY;
    const auto body = [&]
    {
        return std::array<int, 2>{static_cast<int>(budge->Object.Position[0] / TERRAIN_SCALE),
                                  static_cast<int>(budge->Object.Position[1] / TERRAIN_SCALE)};
    };
    CHECK(body() == std::array<int, 2>{x + 1, y - 1});

    for (int i = 0; i < 2; ++i)
    {
        CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
        CHECK(budge->PositionX == x);
        CHECK(budge->PositionY == y);
        CHECK(budge->TargetX == x);
        CHECK(body() == std::array<int, 2>{x + 1, y - 1});
    }
    // The server's moves have no offset.
    budge->PositionX = static_cast<BYTE>(x + 3);
    budge->PositionY = static_cast<BYTE>(y + 3);
    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
    CHECK(budge->PositionX == x + 3);
    CHECK(budge->PositionY == y + 3);
    CHECK(body() == std::array<int, 2>{x + 4, y + 2});
}

// A tile offset past the map's edge puts the body at the edge, not across
// the map (an unsigned char that wraps).
TEST_CASE("Monster setup: a tile offset stops at the map's edge [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON, CreationPath::Viewport);
    REQUIRE(budge != nullptr);
    const int x = budge->PositionX;
    const int y = budge->PositionY;
    changed.setup.tileOffset = std::array<int, 2>{-(x + 5), TERRAIN_SIZE};
    g_MonsterDatabase.Set(changed);
    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
    CHECK(budge->PositionX == x);
    CHECK(budge->PositionY == y);
    CHECK(static_cast<int>(budge->Object.Position[0] / TERRAIN_SCALE) == 0);
    CHECK(static_cast<int>(budge->Object.Position[1] / TERRAIN_SCALE) == TERRAIN_SIZE - 1);
}

// Every path keeps the server's tile, a cutscene's too: CreateMonster puts
// only the body at the tile offset, so Respawn moves no monster.
TEST_CASE("Monster setup: a cutscene monster with a tile offset keeps the server's tile [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    changed.setup.tileOffset = std::array<int, 2>{1, -1};
    g_MonsterDatabase.Set(changed);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* budge = scope.Create(MONSTER_BUDGE_DRAGON, CreationPath::Cinematic);
    REQUIRE(budge != nullptr);
    // The cutscene summons it on tile 102, 100.
    CHECK(budge->PositionX == 102);
    CHECK(budge->PositionY == 100);
    CHECK(budge->TargetX == 102);
    CHECK(static_cast<int>(budge->Object.Position[0] / TERRAIN_SCALE) == 103);
    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) == 1);
    CHECK(budge->PositionX == 102);
    CHECK(static_cast<int>(budge->Object.Position[0] / TERRAIN_SCALE) == 103);
}

// The Dark Horse a helper brings is equipment, which living monsters take at
// once (D55): it goes with the helper, comes back with it, takes a new look,
// and an edit of another field leaves it going.
TEST_CASE("Monster setup: living monsters get and lose their Dark Horse at once (D55) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    constexpr int Rider = 538;
    const MonsterEntryScope kept(Rider);
    const Data::Monsters::MonsterDefinition& original = kept.Original();
    REQUIRE(original.setup.equipment.helper.has_value());
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* rider = scope.Create(Rider);
    REQUIRE(rider != nullptr);
    const auto horses = [&]
    {
        std::vector<const OBJECT*> found;
        for (int i = 0; i < MAX_MOUNTS; ++i)
        {
            if (Mounts[i].Live && Mounts[i].Owner == &rider->Object && Mounts[i].Type == MODEL_DARK_HORSE)
                found.push_back(&Mounts[i]);
        }
        return found;
    };
    REQUIRE(horses().size() == 1);
    const OBJECT* horse = horses()[0];
    const auto& world = World::Monsters::GetSpawnWorld();

    Data::Monsters::MonsterDefinition changed = original;
    changed.setup.outline = 2.0f;
    g_MonsterDatabase.Set(changed);
    GameLogic::Monsters::Setup::ApplyLive(Rider, world);
    REQUIRE(horses().size() == 1);
    CHECK(horses()[0] == horse);

    changed.setup.equipment.helper.reset();
    g_MonsterDatabase.Set(changed);
    GameLogic::Monsters::Setup::ApplyLive(Rider, world);
    CHECK(horses().empty());

    changed.setup.equipment.helper = original.setup.equipment.helper;
    changed.setup.equipment.helper->mountLook.clear();
    g_MonsterDatabase.Set(changed);
    GameLogic::Monsters::Setup::ApplyLive(Rider, world);
    REQUIRE(horses().size() == 1);
    CHECK(horses()[0]->SubType == 0);

    g_MonsterDatabase.Set(original);
    GameLogic::Monsters::Setup::ApplyLive(Rider, world);
    REQUIRE(horses().size() == 1);
    CHECK(horses()[0]->SubType == Data::Monsters::FindMountLook("doppelganger"));
    GameLogic::Monsters::Setup::RemoveSpawnAttachments(*rider);
    DeleteMount(&rider->Object);
}

// A named bone of Models.json the model lacks is not registered: its users
// would read past the model's pose.
TEST_CASE("Monster setup: named bones the model lacks are left out [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* titan = scope.Create(MONSTER_GOLDEN_TITAN);
    REQUIRE(titan != nullptr);
    const int slot = titan->Object.Type;
    const Data::Monsters::MonsterModelDefinition* found = g_MonsterModelDatabase.Find(slot);
    REQUIRE(found != nullptr);
    const Data::Monsters::MonsterModelDefinition kept = *found;
    Data::Monsters::MonsterModelDefinition changed = kept;
    const int bones = Models[slot].NumBones;
    changed.namedBones = {{"Fine", 0}, {"Past", bones}, {"Far", 300}};
    g_MonsterModelDatabase.Set(changed);

    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    titan = scope.Create(MONSTER_GOLDEN_TITAN);
    g_MonsterModelDatabase.Set(kept);
    REQUIRE(titan != nullptr);
    CHECK(BoneManager::GetBoneNumber(&titan->Object, L"Fine") == 0);
    CHECK(BoneManager::GetBoneNumber(&titan->Object, L"Past") == -1);
    CHECK(BoneManager::GetBoneNumber(&titan->Object, L"Far") == -1);
}

// A model whose file is missing has no bones: its named bones are not logged
// as missing one by one, which would blame the bones for the file.
TEST_CASE("Monster setup: a missing model file logs no missing named bones [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    const int molt = MODEL_MONSTER01 + static_cast<int>(MONSTER_MODEL_MOLT);
    const Data::Monsters::MonsterModelDefinition* found = g_MonsterModelDatabase.Find(molt);
    REQUIRE(found != nullptr);
    const Data::Monsters::MonsterModelDefinition kept = *found;
    Data::Monsters::MonsterModelDefinition changed = kept;
    changed.namedBones = {{"MissingFileBone", 3}};
    g_MonsterModelDatabase.Set(changed);
    const LogCapture log("data");
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* made = scope.Create(MONSTER_MOLT);
    g_MonsterModelDatabase.Set(kept);
    REQUIRE(made != nullptr);
    CHECK(Data::Monsters::ModelLoader::IsFileMissing(molt));
    CHECK(log.LineWith("MissingFileBone").empty());
    DeleteMonsters();
}
#endif // _EDITOR

// A Blood Castle's monsters are bigger by castle (P13, Q-MN43): castle 1 +0,
// castles 2 to 5 +0.05, castles 6 to 8 +0.10, Blood Castle 8 too, each castle
// number's own scale; the castle gate and the statues, which stand in every
// castle, by map.
TEST_CASE("Monster setup: a Blood Castle's monsters are bigger by castle (P13) [monsters][creation]")
{
    struct Sized
    {
        int number;
        int map;
        float scale;
    };
    constexpr Sized Sizes[] = {
        {88, 11, 1.19f},  {94, 12, 1.24f}, {123, 15, 1.24f}, {129, 16, 1.29f}, {142, 17, 1.29f},
        {432, 52, 1.29f}, {87, 11, 0.8f},  {119, 15, 1.15f}, {431, 52, 0.9f},  {131, 11, 0.8f},
        {131, 13, 0.85f}, {131, 17, 0.9f}, {132, 15, 0.85f}, {133, 52, 0.9f},  {134, 11, 0.8f},
    };
    MonsterRecorder::CreationScope scope(0);
    for (const Sized& sized : Sizes)
    {
        INFO("monster " << sized.number << " on map " << sized.map);
        scope.SetServerMap(sized.map, 0);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        const CHARACTER* monster = scope.Create(sized.number);
        REQUIRE(monster != nullptr);
        CHECK(monster->Object.Scale == doctest::Approx(sized.scale));
    }
    scope.SetServerMap(0, 0);
}

// The numbers without a creator that OpenMU sends (P2, Q-MN42): the Imperial
// Guardian's traps 523 have no body (model none, the trap kind), the renewed
// Illusion Temple's statues 658 to 668 the Stone Statue's look, 658 the
// monster kind (attackable), 659 to 668 the NPC kind.
TEST_CASE("Monster setup: the traps and statues OpenMU sends have their looks (P2) [monsters][creation]")
{
    MonsterRecorder::CreationScope scope(0);
    scope.SetServerMap(69, 0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    const CHARACTER* trap = scope.Create(523);
    REQUIRE(trap != nullptr);
    CHECK(trap->Object.Type == Data::Monsters::NoModelSlot);
    CHECK(trap->Object.Kind == KIND_TRAP);
    CHECK(Models[Data::Monsters::NoModelSlot].NumMeshs <= 0);

    scope.SetServerMap(45, 0);
    for (const int number : {658, 660, 668})
    {
        INFO("monster " << number);
        MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
        const CHARACTER* statue = scope.Create(number);
        REQUIRE(statue != nullptr);
        CHECK(statue->Object.Type == MODEL_CURSEDTEMPLE_STATUE);
        CHECK(statue->Object.m_fEdgeScale == doctest::Approx(1.03f));
        CHECK_FALSE(statue->Object.m_bRenderShadow);
        CHECK(statue->Object.Kind == (number == 658 ? KIND_MONSTER : KIND_NPC));
    }
    scope.SetServerMap(0, 0);
}

// The spawn hooks (Q-MN36d) hand over to the maps' own state.
TEST_CASE("Monster setup: the spawn hooks hand over to the maps (MN4b) [monsters][creation]")
{
    {
        MonsterRecorder::CreationScope scope(0);

        // Selupan drops down only from where he waits.
        REQUIRE(scope.Create(MONSTER_SELUPAN) != nullptr);
        CHECK_FALSE(g_Raklion.IsBossDropping());
        scope.SetSpawnState("raklionBossWaiting", true);
        REQUIRE(scope.Create(MONSTER_SELUPAN) != nullptr);
        CHECK(g_Raklion.IsBossDropping());
        scope.SetSpawnState("raklionBossWaiting", false);

        // A new storage shows no point effect its team had.
        SendTemplePoints(10, 10);
        SendTemplePoints(11, 10);
        REQUIRE(g_CursedTemple->IsStoragePointEffectShown(true));
        REQUIRE(scope.Create(MONSTER_ALLIANCE_ITEM_STORAGE) != nullptr);
        CHECK_FALSE(g_CursedTemple->IsStoragePointEffectShown(true));
        SendTemplePoints(11, 11);
        REQUIRE(g_CursedTemple->IsStoragePointEffectShown(false));
        REQUIRE(scope.Create(MONSTER_ILLUSION_ITEM_STORAGE) != nullptr);
        CHECK_FALSE(g_CursedTemple->IsStoragePointEffectShown(false));

        // The Gateway Machine goes to Kanturu's entrance window, which the
        // tests do not have: it is made without it.
        scope.SetServerMap(WD_38KANTURU_2ND, 0);
        CHECK(scope.Create(MONSTER_GATEWAY_MACHINE) != nullptr);
        scope.SetServerMap(0, 0);
    }
}

// Gayion throws five swords, models of their own: his model opens them as
// its parts (Q-MN45a).
TEST_CASE("Monster setup: Gayion's swords are open when he appears [monsters][creation]")
{
    constexpr int Swords[] = {
        MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_, MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_,
        MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_, MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_,
        MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_};
    MonsterRecorder::CreationScope scope(WD_72EMPIREGUARDIAN4);
    for (const int sword : Swords)
        REQUIRE(Models[sword].NumMeshs <= 0);
    REQUIRE(scope.Create(MONSTER_GAYION_THE_GLADIATOR) != nullptr);
    for (const int sword : Swords)
    {
        INFO("sword model " << sword);
        CHECK(Models[sword].NumMeshs > 0);
    }
}

// The setup Monster Preview shows (Q-MN40): the entry's; on a map of a map
// variant, with the variant's fields; with a state variant's fields over them.
TEST_CASE("Monster setup: the preview's setup is the entry's, by the map and the state picked [monsters][creation]")
{
    using GameLogic::Monsters::Setup::SetupFor;
    const std::string state(Data::Monsters::GetMonsterStates().front());
    Data::Monsters::MonsterDefinition entry;
    entry.setup.scale = 1.0f;
    entry.setup.outline = 1.5f;
    Data::Monsters::MonsterStateVariant stateVariant;
    stateVariant.state = state;
    stateVariant.setup.outline = 3.0f;
    entry.setup.stateVariants.push_back(stateVariant);
    Data::Monsters::MonsterMapVariant mapVariant;
    mapVariant.maps = {Data::Maps::MapRef{7, 0}};
    mapVariant.setup.scale = 2.0f;
    entry.mapVariants.push_back(mapVariant);

    const Data::Monsters::MonsterSetup plain = SetupFor(entry, std::nullopt, "");
    CHECK(plain.scale == 1.0f);
    CHECK(plain.outline == 1.5f);

    const Data::Monsters::MonsterSetup onMap = SetupFor(entry, Data::Maps::MapRef{7, 0}, "");
    CHECK(onMap.scale == 2.0f);
    CHECK(onMap.outline == 1.5f);

    const Data::Monsters::MonsterSetup inState = SetupFor(entry, std::nullopt, state);
    CHECK(inState.scale == 1.0f);
    CHECK(inState.outline == 3.0f);

    const Data::Monsters::MonsterSetup both = SetupFor(entry, Data::Maps::MapRef{7, 0}, state);
    CHECK(both.scale == 2.0f);
    CHECK(both.outline == 3.0f);

    // Another map or a state the entry has no variant for: the entry's.
    CHECK(SetupFor(entry, Data::Maps::MapRef{8, 0}, "").scale == 1.0f);
    CHECK(SetupFor(entry, std::nullopt, "no such state").outline == 1.5f);
}

// Monster Preview dresses its character as a spawn does, with the look and
// equipment of the setup; what the monster does is left out.
TEST_CASE("Monster setup: Dress gives a character the setup's look and equipment only [monsters][creation]")
{
    CHARACTER character{};
    Data::Monsters::MonsterSetup setup;
    setup.equipment.weapons[0] = Data::Monsters::MonsterItem{0, 3, 7, 1};
    setup.equipment.helm = Data::Monsters::MonsterItem{7, 2, 4};
    setup.scale = 1.4f;
    setup.outline = 2.0f;
    setup.pkColor = 6;
    setup.startAction = 5;
    setup.noTurnOnHit = true;

    GameLogic::Monsters::Setup::Dress(character, setup);
    CHECK(character.Weapon[0].Type == MODEL_ITEM + 3);
    CHECK(character.Weapon[0].Level == 7);
    CHECK(character.Weapon[0].ExcellentFlags == 1);
    CHECK(character.BodyPart[BODYPART_HELM].Type == MODEL_ITEM + 7 * MAX_ITEM_INDEX + 2);
    CHECK(character.BodyPart[BODYPART_HELM].Level == 4);
    CHECK(character.Object.Scale == doctest::Approx(1.4f));
    CHECK(character.Object.m_fEdgeScale == 2.0f);
    CHECK(character.PK == 6);
    CHECK(character.Object.CurrentAction == 0);
    CHECK_FALSE(character.NotRotateOnMagicHit);
}

namespace
{
int CountJointsOn(const OBJECT& object)
{
    return static_cast<int>(std::count_if(Joints, Joints + MAX_JOINTS,
                                          [&](const JOINT& joint) { return joint.Live && joint.Target == &object; }));
}

int CountLiveJoints(int type)
{
    return static_cast<int>(std::count_if(Joints, Joints + MAX_JOINTS,
                                          [&](const JOINT& joint) { return joint.Live && joint.Type == type; }));
}

int CountMapObjects()
{
    int count = 0;
    for (const OBJECT_BLOCK& block : ObjectBlock)
    {
        for (const OBJECT* object = block.Head; object != nullptr; object = object->Next)
            ++count;
    }
    return count;
}
} // namespace

// Monster Preview's replays (MN4): the spawn effects again on a character,
// the appear as a spawn fills it, its sound only when asked; neither puts an
// object into the map. RemoveEffectsOf ends what is on the character.
// The animation an appear plays, which the game's appear and Monster
// Preview's Replay appear share: its own, else a player's come-up by the model.
TEST_CASE("Monster setup: an appear's animation is its own, else the model's [monsters][creation]")
{
    using GameLogic::Monsters::Setup::AppearAction;
    Data::Monsters::MonsterAppear appear;
    CHECK(AppearAction(appear, MODEL_PLAYER, true) == PLAYER_COME_UP);
    CHECK_FALSE(AppearAction(appear, MODEL_PLAYER, false).has_value());
    CHECK_FALSE(AppearAction(appear, MODEL_MONSTER01, true).has_value());
    appear.action = "stop2";
    CHECK(AppearAction(appear, MODEL_PLAYER, true) == Data::Monsters::FindMonsterAnimation("stop2"));
    CHECK(AppearAction(appear, MODEL_MONSTER01, false) == Data::Monsters::FindMonsterAnimation("stop2"));
}

TEST_CASE("Monster setup: the preview replays a spawn and an appear without objects [monsters][creation]")
{
    using Data::Monsters::MonsterSpawnEffect;
    MonsterRecorder::CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* titan = scope.Create(MONSTER_GOLDEN_TITAN);
    REQUIRE(titan != nullptr);
    OBJECT& object = titan->Object;
    // Its entry's two eye trails (the slot may hold earlier ones).
    CHECK(CountJointsOn(object) >= 2);
    GameLogic::Monsters::Setup::RemoveEffectsOf(object);
    CHECK(CountJointsOn(object) == 0);

    Data::Monsters::MonsterSetup setup;
    setup.spawnEffects.push_back({MonsterSpawnEffect::Kind::Joint, "BITMAP_JOINT_ENERGY", 2, 30.0f});
    setup.spawnEffects.push_back({MonsterSpawnEffect::Kind::Object, "MODEL_BC_BOX", 0, 0.0f});
    const int objects = CountMapObjects();
    GameLogic::Monsters::Setup::ReplaySpawnEffects(*titan, setup);
    CHECK(CountJointsOn(object) == 1);
    CHECK(CountMapObjects() == objects);

    Data::Monsters::MonsterAppear appear;
    appear.fadeIn = true;
    appear.sound = "SOUND_ASSASSIN";
    appear.effects.push_back({MonsterSpawnEffect::Kind::Joint, "BITMAP_JOINT_THUNDER+1", 7, 0.0f});
    appear.effects.push_back({MonsterSpawnEffect::Kind::Object, "MODEL_BC_BOX", 0, 0.0f});
    setup.appear = appear;
    const int thunder = CountLiveJoints(BITMAP_JOINT_THUNDER + 1);
    scope.Load().TakePlayedSounds();
    object.Alpha = 1.0f;
    GameLogic::Monsters::Setup::ReplayAppear(*titan, setup, false);
    CHECK(object.Alpha == 0.0f);
    CHECK(CountLiveJoints(BITMAP_JOINT_THUNDER + 1) == thunder + 1);
    CHECK(CountMapObjects() == objects);
    CHECK(scope.Load().TakePlayedSounds().empty());
    GameLogic::Monsters::Setup::ReplayAppear(*titan, setup, true);
    const std::vector<MonsterRecorder::PlayedSound> sounds = scope.Load().TakePlayedSounds();
    REQUIRE(sounds.size() == 1);
    CHECK(sounds[0].id == SOUND_ASSASSIN);

    GameLogic::Monsters::Setup::RemoveEffectsOf(object);
    CHECK(CountJointsOn(object) == 0);
}

#ifdef _EDITOR
// Respawn runs as the game's own: CreateMonster in RespawnScope, so a hook
// (the temple storage's point effect) does not run again and an object row
// (a supply box) is not made again.
TEST_CASE("Monster setup: Respawn runs no hooks and makes no objects again (D55) [monsters][creation]")
{
    using Data::Monsters::MonsterSpawnEffect;
    MonsterRecorder::CreationScope scope(0);
    const MonsterEntryScope kept(MONSTER_BUDGE_DRAGON);
    Data::Monsters::MonsterDefinition changed = kept.Original();
    changed.setup.hooks = {Data::Monsters::MonsterHook{"IllusionTempleStorageReset", {{"storage", "allied"}}}};
    changed.setup.spawnEffects.push_back({MonsterSpawnEffect::Kind::Object, "MODEL_BC_BOX", 0, 0.0f});
    g_MonsterDatabase.Set(changed);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    int objects = CountMapObjects();
    REQUIRE(scope.Create(MONSTER_BUDGE_DRAGON) != nullptr);
    REQUIRE(CountMapObjects() == objects + 1);
    objects = CountMapObjects();
    CHECK_FALSE(World::Monsters::GetSpawnWorld().isRespawn());

    SendTemplePoints(10, 10);
    SendTemplePoints(11, 10);
    REQUIRE(g_CursedTemple->IsStoragePointEffectShown(true));
    CHECK(World::Monsters::RespawnLive(MONSTER_BUDGE_DRAGON, false) >= 1);
    CHECK(g_CursedTemple->IsStoragePointEffectShown(true));
    CHECK(CountMapObjects() == objects);
    CHECK_FALSE(World::Monsters::GetSpawnWorld().isRespawn());
}
#endif // _EDITOR

// A row's bone the model does not have (a negative one, or one past its
// bones, as a map variant's model can give) makes nothing and reads no bone.
TEST_CASE("Monster setup: spawn effects at bones the model lacks make nothing [monsters][creation]")
{
    using Data::Monsters::MonsterSpawnEffect;
    MonsterRecorder::CreationScope scope(0);
    MonsterRecorder::PrepareCreationSlot(CreationSlot::Clean);
    CHARACTER* titan = scope.Create(MONSTER_GOLDEN_TITAN);
    REQUIRE(titan != nullptr);
    OBJECT& object = titan->Object;
    REQUIRE(object.BoneTransform != nullptr);
    const int bones = Models[object.Type].NumBones;
    REQUIRE(bones > 0);
    const auto blurs = []
    {
        return std::count_if(Particles, Particles + MAX_PARTICLES, [](const PARTICLE& particle)
                             { return particle.Live && particle.Type == BITMAP_BLUE_BLUR; });
    };
    const auto before = blurs();

    Data::Monsters::MonsterSetup setup;
    for (const int bone : {-1, bones})
    {
        MonsterSpawnEffect blur{MonsterSpawnEffect::Kind::Particle, "BITMAP_BLUE_BLUR", 0, 0.0f};
        blur.bone = bone;
        setup.spawnEffects.push_back(blur);
    }
    MonsterSpawnEffect eyes{MonsterSpawnEffect::Kind::Eyes, "", 0, 0.0f};
    eyes.bones = {0, bones};
    setup.spawnEffects.push_back(eyes);
    Vector(1.0f, 2.0f, 3.0f, object.EyeRight);
    GameLogic::Monsters::Setup::ReplaySpawnEffects(*titan, setup);
    CHECK(blurs() == before);
    CHECK(object.EyeRight[0] == 1.0f);

    // The same rows with bones it has.
    setup.spawnEffects[0].bone = 0;
    setup.spawnEffects[2].bones = {0, bones - 1};
    GameLogic::Monsters::Setup::ReplaySpawnEffects(*titan, setup);
    CHECK(blurs() == before + 1);
    GameLogic::Monsters::Setup::RemoveEffectsOf(object);
}
