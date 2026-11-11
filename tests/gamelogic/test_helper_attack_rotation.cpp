#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Skills/SkillManager.h"
#include "GameLogic/Automation/AttackSkillRotation.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/SkillCast.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "MUHelper/MuHelperData.h"
#include "MUHelper/MuHelper.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/AI/ZzzAI.h"
#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Automation/Movement.h"
#include "GameLogic/Automation/Pickup.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include <memory>

#include <algorithm>
#include <array>
#include <cstring>

#ifdef MU_TEST_COMBAT_AOT_TRANSPORT
#include <winsock2.h>
#include "Dotnet/Connection.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Textures/TextureFailure.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Options/OptionWindow.h"
#include "World/MapInfra/MapManager.h"
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>
extern DWORD g_dwLatestMagicTick;
extern int CurrentSkill;
extern int MouseUpdateTime;
extern int MouseUpdateTimeMax;
#endif

extern MovementSkill g_MovementSkill;
extern int TargetX;
extern int TargetY;
extern int Attacking;

namespace
{
constexpr ActionSkillType Primary = static_cast<ActionSkillType>(41);
constexpr ActionSkillType ExtraOne = static_cast<ActionSkillType>(42);
constexpr ActionSkillType ExtraTwo = static_cast<ActionSkillType>(43);
constexpr int KnightSkillDelay = 500;
using GameLogic::Automation::SkillResult;

// A skill condition as the helper's settings hold it: when the skill is cast, what is near and how many.
uint32_t SkillCondition(MUHelper::ESkillActivationBase when, MUHelper::ESkillActivationPreCon what,
                        MUHelper::ESkillActivationSubCon count)
{
    return static_cast<uint32_t>(when) | static_cast<uint32_t>(what) | static_cast<uint32_t>(count);
}

struct SkillFixture
{
    CHARACTER hero;
    CHARACTER_MACHINE machine{};
    std::array<SKILL_ATTRIBUTE, MAX_SKILLS> attributes{};
    CHARACTER* previousHero = Hero;
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    CHARACTER_ATTRIBUTE* previousCharacter = CharacterAttribute;
    SKILL_ATTRIBUTE* previousAttributes = SkillAttribute;
    EGameScene previousScene = SceneFlag;
    MovementSkill previousMovementSkill = g_MovementSkill;
    int previousSelectedCharacter = SelectedCharacter;
    int previousTargetX = TargetX;
    int previousTargetY = TargetY;
    int previousAttacking = Attacking;
    MUHelper::ConfigData config;
    GameLogic::Automation::AttackSkillRotation rotation;

    SkillFixture()
    {
        Hero = &hero;
        SceneFlag = MAIN_SCENE;
        hero.Class = CLASS_KNIGHT;
        CharacterMachine = &machine;
        CharacterAttribute = &machine.Character;
        SkillAttribute = attributes.data();
        machine.Character.Mana = 100;
        machine.Character.SkillMana = 100;
        machine.Character.Strength = 500;
        machine.Character.Level = 100;
        machine.Character.SkillNumber = 3;
        config.aiSkill = {Primary, ExtraOne, ExtraTwo};
        for (std::size_t slot = 0; slot < config.aiSkill.size(); ++slot)
            machine.Character.Skill[slot] = static_cast<ActionSkillType>(config.aiSkill[slot]);
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    ~SkillFixture()
    {
        Hero = previousHero;
        CharacterMachine = previousMachine;
        CharacterAttribute = previousCharacter;
        SkillAttribute = previousAttributes;
        SceneFlag = previousScene;
        g_MovementSkill = previousMovementSkill;
        SelectedCharacter = previousSelectedCharacter;
        TargetX = previousTargetX;
        TargetY = previousTargetY;
        Attacking = previousAttacking;
        gSkillManager.InvalidateSkillAttributeRequirementsCache();
    }

    void PrepareKnightCast(ActionSkillType skill)
    {
        hero.Object.Type = MODEL_PLAYER;
        hero.Object.CurrentAction = PLAYER_STOP_MALE;
        hero.Movement = false;
        hero.SkillSuccess = true;
        hero.CurrentSkill = 0;
        machine.Character.Skill[0] = skill;
        attributes[skill].Delay = KnightSkillDelay;
        for (auto& item : machine.Equipment)
            item.Type = -1;
        SelectedCharacter = -1;
        g_MovementSkill.m_iSkill = hero.CurrentSkill;
        g_MovementSkill.m_iTarget = -1;
    }

    ITEM& EquipCycloneSword()
    {
        auto& weapon = machine.Equipment[EQUIPMENT_WEAPON_RIGHT];
        weapon.Type = ITEM_SHORT_SWORD;
        weapon.SpecialNum = 1;
        weapon.Special[0] = AT_SKILL_CYCLONE;
        return weapon;
    }

    ActionSkillType Select(int seconds = 0, std::size_t nearby = 0, std::size_t attacking = 0)
    {
        return rotation.Select(config, seconds, nearby, attacking);
    }
};

constexpr int MovementStartTile = 100;
constexpr int MovementDropIndex = MAX_ITEMS - 1;

struct DropFixture
{
    bool previousLive = Items[MovementDropIndex].Object.Live;
    std::array<float, 3> previousPosition;

    DropFixture()
    {
        auto& object = Items[MovementDropIndex].Object;
        std::copy_n(object.Position, previousPosition.size(), previousPosition.begin());
        object.Live = true;
        object.Position[0] = (MovementStartTile + 3.5f) * TERRAIN_SCALE;
        object.Position[1] = (MovementStartTile + 0.5f) * TERRAIN_SCALE;
    }

    ~DropFixture()
    {
        auto& object = Items[MovementDropIndex].Object;
        object.Live = previousLive;
        std::copy(previousPosition.begin(), previousPosition.end(), object.Position);
    }
};

void CheckActionsWaitForWalk()
{
    using namespace GameLogic::Automation;
    const auto currentPath = Hero->Path.CurrentPath;
    const auto currentFraction = Hero->Path.CurrentPathFloat;
    const int targetX = TargetX;
    const int targetY = TargetY;
    CHECK(WalkTo(MovementStartTile - 3, MovementStartTile) == MoveResult::Walking);
    CHECK(AttackObject(123, false, 6) == AttackResult::Busy);
    CHECK(CastSkill(Primary, true, 123, true, 6) == SkillResult::Busy);
    CHECK(PickUpItem(MovementDropIndex, 8) == PickupResult::Approaching);
    CHECK(Hero->Path.CurrentPath == currentPath);
    CHECK(Hero->Path.CurrentPathFloat == currentFraction);
    CHECK(TargetX == targetX);
    CHECK(TargetY == targetY);
}

void CheckCastRefused()
{
    CHECK_FALSE(Hero->SkillSuccess);
    CHECK(Hero->Object.CurrentAction == PLAYER_STOP_MALE);
    CHECK_FALSE(Hero->Movement);
}

#ifdef MU_TEST_COMBAT_AOT_TRANSPORT
// Reuses the public Cursed Temple test's real loopback/AOT transport, without its UI fixture.
class SkillPacketPeer
{
public:
    SkillPacketPeer()
    {
        try
        {
            Open();
        }
        catch (...)
        {
            Close();
            throw;
        }
    }

    ~SkillPacketPeer()
    {
        Close();
    }

    void Accept()
    {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(m_listener, &readable);
        constexpr long AcceptTimeoutSeconds = 2;
        timeval wait{AcceptTimeoutSeconds, 0};
        REQUIRE(select(0, &readable, nullptr, nullptr, &wait) == 1);
        m_peer = accept(m_listener, nullptr, nullptr);
        REQUIRE(m_peer != INVALID_SOCKET);
    }

    std::vector<BYTE> Collect()
    {
        std::vector<BYTE> bytes;
        constexpr auto CollectTimeout = std::chrono::milliseconds(300);
        constexpr long PollMicroseconds = 10000;
        constexpr std::size_t ReceiveCapacity = 256;
        const auto until = std::chrono::steady_clock::now() + CollectTimeout;
        while (std::chrono::steady_clock::now() < until)
        {
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(m_peer, &readable);
            timeval wait{0, PollMicroseconds};
            const int ready = select(0, &readable, nullptr, nullptr, &wait);
            REQUIRE(ready != SOCKET_ERROR);
            if (ready == 0)
                continue;
            char buffer[ReceiveCapacity];
            const int count = recv(m_peer, buffer, sizeof(buffer), 0);
            REQUIRE(count > 0);
            REQUIRE(bytes.size() + static_cast<std::size_t>(count) <= ReceiveCapacity);
            bytes.insert(bytes.end(), buffer, buffer + count);
        }
        return bytes;
    }

    int port = 0;

private:
    void Open()
    {
        WSADATA data{};
        REQUIRE(WSAStartup(MAKEWORD(2, 2), &data) == 0);
        m_started = true;
        m_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        REQUIRE(m_listener != INVALID_SOCKET);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        REQUIRE(bind(m_listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
        REQUIRE(listen(m_listener, 1) == 0);
        int size = sizeof(address);
        REQUIRE(getsockname(m_listener, reinterpret_cast<sockaddr*>(&address), &size) == 0);
        port = ntohs(address.sin_port);
    }

    void Close()
    {
        if (m_peer != INVALID_SOCKET)
            closesocket(m_peer);
        if (m_listener != INVALID_SOCKET)
            closesocket(m_listener);
        if (m_started)
            WSACleanup();
    }
    SOCKET m_listener = INVALID_SOCKET;
    SOCKET m_peer = INVALID_SOCKET;
    bool m_started = false;
};

std::string* activeTextureErrors = nullptr;

void RecordTextureError(const std::string& message)
{
    if (activeTextureErrors)
    {
        activeTextureErrors->append(message).push_back('\n');
    }
}

struct CombatGlobals
{
    std::unique_ptr<Connection>& connection;
    BMD* models = Models;
    CHARACTER* characters = CharactersClient;
    Connection* socket = SocketClient;
    int world = gMapManager.WorldActive;
    DWORD magicTick = g_dwLatestMagicTick;
    int currentSkill = CurrentSkill;
    int mouseTime = MouseUpdateTime;
    int mouseTimeMax = MouseUpdateTimeMax;
    std::wstring language = g_strSelectedML;
    std::filesystem::path directory = std::filesystem::current_path();
    Render::Textures::TextureErrorCollector textureCollector = Render::Textures::GetTextureErrorCollector();
    std::string textureErrors;
    bool ownsHeadlessServices = false;
    bool ownsUi = false;

    ~CombatGlobals()
    {
        if (ownsUi)
            g_pNewUISystem->Release();
        if (ownsHeadlessServices)
        {
            Render::Textures::SetTextureErrorCollector(textureCollector);
            activeTextureErrors = nullptr;
            // This isolated binary owns the upload-skip test service, as does TempleSkillFixture.
            Render::Sprites::SetGpuUploadsSkipped(false);
        }
        connection.reset();
        SocketClient = socket;
        Models = models;
        CharactersClient = characters;
        gMapManager.WorldActive = world;
        g_dwLatestMagicTick = magicTick;
        CurrentSkill = currentSkill;
        MouseUpdateTime = mouseTime;
        MouseUpdateTimeMax = mouseTimeMax;
        g_strSelectedML = language;
        std::filesystem::current_path(directory);
    }
};

struct AcceptedCastFixture : SkillFixture
{
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    std::unique_ptr<BMD[]> models = std::make_unique<BMD[]>(MODEL_PLAYER + 1);
    SkillPacketPeer peer;
    std::unique_ptr<Connection> connection;
    // Tears down UI and connection before restoring globals and freeing their storage, even if setup throws.
    CombatGlobals restore{connection};

    AcceptedCastFixture()
    {
        REQUIRE(g_pNewUIMng == nullptr);
        REQUIRE(activeTextureErrors == nullptr);
        std::filesystem::current_path(MU_BIN_DIR);
        REQUIRE(std::filesystem::is_regular_file("Data/Local/Eng/Slide_Eng.bmd"));
        g_strSelectedML = L"Eng";
        Models = models.get();
        Models[MODEL_PLAYER].Actions = new Action_t[MAX_PLAYER_ACTION]{};
        Models[MODEL_PLAYER].NumActions = MAX_PLAYER_ACTION;
        CharactersClient = characters.get();
        gMapManager.WorldActive = WD_0LORENCIA;
        hero.Helper.Type = -1;
        constexpr short HeroKey = 512;
        hero.Key = HeroKey;
        hero.PositionX = hero.PositionY = MovementStartTile;
        hero.Object.Position[0] = hero.Object.Position[1] = (MovementStartTile + 0.5f) * TERRAIN_SCALE;
        SelectedCharacter = 0;
        // CheckTarget gets valid coordinates, while CheckAttack refuses this dead target before combat UI access.
        characters[0].Dead = 1;
        characters[0].Object.Position[0] = hero.Object.Position[0];
        characters[0].Object.Position[1] = hero.Object.Position[1];
        LoadManagedLibrary();
        REQUIRE(munique_client_library_handle != nullptr);
        connection = std::make_unique<Connection>(u"127.0.0.1", peer.port, false, nullptr);
        SocketClient = connection.get();
        REQUIRE(connection->IsConnected());
        peer.Accept();
        restore.ownsHeadlessServices = true;
        Render::Sprites::SetGpuUploadsSkipped(true);
        activeTextureErrors = &restore.textureErrors;
        Render::Textures::SetTextureErrorCollector(&RecordTextureError);
        restore.ownsUi = true;
        REQUIRE(g_pNewUISystem->Create());
        INFO(restore.textureErrors);
        REQUIRE(restore.textureErrors.empty());
        g_pOption->SetAutoAttack(false);
    }

    std::vector<BYTE> ExpectSkillPacket(BYTE code, ActionSkillType skill, std::size_t size)
    {
        // Packet package 0.11.0: unencrypted connection preserves C3, opcode at 2, big-endian skill at 3.
        constexpr BYTE SkillHeader = 0xC3;
        constexpr std::size_t CodeOffset = 2;
        constexpr std::size_t SkillOffset = 3;
        const auto bytes = peer.Collect();
        REQUIRE(bytes.size() == size);
        REQUIRE(size >= SkillOffset + sizeof(WORD));
        CHECK(bytes[0] == SkillHeader);
        CHECK(bytes[1] == size);
        CHECK(bytes[CodeOffset] == code);
        CHECK(((static_cast<WORD>(bytes[SkillOffset]) << 8) | bytes[SkillOffset + 1]) == skill);
        return bytes;
    }
};
#endif
} // namespace

TEST_CASE("Helper actions finish a walk across slow frames before replanning [helper_movement]")
{
    SkillFixture fixture;
    DropFixture drop;
    constexpr float WalkSpeed = 15.f;
    constexpr double HelperIntervalMs = 250.0;
    for (const double fps : {5.0, 25.0, 60.0})
    {
        CAPTURE(fps);
        Hero->PositionX = Hero->PositionY = MovementStartTile;
        Hero->Object.Position[0] = Hero->Object.Position[1] = (MovementStartTile + 0.5f) * TERRAIN_SCALE;
        Hero->Object.CurrentAction = PLAYER_WALK_MALE;
        Hero->Movement = true;
        auto& path = Hero->Path;
        path.PathNum = 2;
        path.CurrentPath = path.CurrentPathFloat = 0;
        path.PathX[0] = path.PathY[0] = path.PathY[1] = MovementStartTile;
        path.PathX[1] = MovementStartTile + 1;
        double nextTickMs = 0.0;
        for (int frame = 0; frame < 120 && Hero->Movement; ++frame)
        {
            if (frame * 1000.0 / fps >= nextTickMs)
            {
                CheckActionsWaitForWalk();
                nextTickMs += HelperIntervalMs;
            }
            if (MovePath(Hero, false))
                Hero->Movement = false;
            else
                Hero->Object.Position[0] += WalkSpeed * std::min(static_cast<float>(REFERENCE_FPS / fps), 1.f);
        }
        CHECK_FALSE(Hero->Movement);
        CHECK_FALSE(GameLogic::Automation::IsWalkInProgress());
        CHECK(Hero->PositionX == MovementStartTile + 1);
        CHECK(Hero->Object.Position[0] == doctest::Approx((MovementStartTile + 1.5f) * TERRAIN_SCALE));
    }
}

TEST_CASE("Stopped and completed paths do not hold automation busy [helper_movement]")
{
    SkillFixture fixture;
    Hero->Movement = true;
    Hero->Path.PathNum = 2;
    Hero->Path.CurrentPath = 0;
    CHECK(GameLogic::Automation::IsWalkInProgress());
    Hero->Movement = false;
    CHECK_FALSE(GameLogic::Automation::IsWalkInProgress());
    Hero->Movement = true;
    Hero->Path.CurrentPath = 1;
    CHECK_FALSE(GameLogic::Automation::IsWalkInProgress());
    Hero->Path.PathNum = 0;
    Hero->Path.CurrentPath = 0;
    CHECK_FALSE(GameLogic::Automation::IsWalkInProgress());
}

TEST_CASE("Unconditioned additional attacks rotate with the primary [helper_rotation]")
{
    SkillFixture fixture;
    for (int cycle = 0; cycle < 3; ++cycle)
    {
        for (const auto expected : {Primary, ExtraOne, ExtraTwo})
        {
            CHECK(fixture.Select() == expected);
            fixture.rotation.Complete(SkillResult::Cast);
        }
    }
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    fixture.rotation.Reset();
    CHECK(fixture.Select() == Primary);
}

TEST_CASE("Automatic selection reads real cooldowns without consuming them [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Delay = 500;
    CharacterAttribute->SkillDelay[1] = 50;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 50);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK(CharacterAttribute->SkillDelay[1] == 50);

    CharacterAttribute->SkillDelay[1] = 0;
    fixture.rotation.Reset();
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 0);
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraOne);
    CHECK(CharacterAttribute->SkillDelay[1] == 0);

    SkillAttribute[AT_SKILL_NOVA].Delay = 500;
    CharacterAttribute->Skill[2] = AT_SKILL_NOVA;
    CharacterAttribute->SkillDelay[2] = 5;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_NOVA_BEGIN));
    CharacterAttribute->SkillDelay[2] = 0;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_NOVA_BEGIN));
    CHECK(CharacterAttribute->SkillDelay[2] == 0);
}

// These four cases call Knight dispatch directly, without ExecuteSkill routing or accepted casts.
TEST_CASE("Knight cooldown refusal cannot complete a cast [helper_execution]")
{
    constexpr int RemainingDelay = 50;
    constexpr float SkillDistance = 2.f;
    for (const auto skill : {AT_SKILL_CYCLONE, AT_SKILL_TWISTING_SLASH, AT_SKILL_RAGEFUL_BLOW})
    {
        for (const bool previousCast : {false, true})
        {
            CAPTURE(skill);
            CAPTURE(previousCast);
            SkillFixture fixture;
            fixture.PrepareKnightCast(skill);
            Hero->SkillSuccess = previousCast;
            CharacterAttribute->SkillDelay[0] = RemainingDelay;
            auto& weapon = fixture.EquipCycloneSword();

            // Exercise weapon dispatch first; ExecuteSkill itself is not called here.
            if (skill == AT_SKILL_CYCLONE)
                CHECK_FALSE(GameLogic::Combat::SkillWarrior(Hero, &weapon));
            GameLogic::Combat::AttackKnight(Hero, skill, SkillDistance);

            CheckCastRefused();
            CHECK(CharacterAttribute->SkillDelay[0] == RemainingDelay);
        }
    }
}

TEST_CASE("Knight range refusal cannot complete a cast [helper_execution]")
{
    SkillFixture fixture;
    fixture.PrepareKnightCast(AT_SKILL_RAGEFUL_BLOW);
    fixture.machine.Equipment[EQUIPMENT_WEAPON_RIGHT].Type = ITEM_SHORT_SWORD;
    constexpr float SkillDistance = 2.f;
    constexpr int TargetOffset = 3;
    Hero->PositionX = Hero->PositionY = MovementStartTile;
    Hero->Object.Position[0] = Hero->Object.Position[1] = (MovementStartTile + 0.5f) * TERRAIN_SCALE;
    TargetX = MovementStartTile + TargetOffset;
    TargetY = MovementStartTile;
    Hero->TargetPosition[0] = (TargetX + 0.5f) * TERRAIN_SCALE;
    Hero->TargetPosition[1] = (TargetY + 0.5f) * TERRAIN_SCALE;
    REQUIRE_FALSE(CheckTile(Hero, &Hero->Object, SkillDistance));

    GameLogic::Combat::AttackKnight(Hero, AT_SKILL_RAGEFUL_BLOW, SkillDistance);

    CheckCastRefused();
}

TEST_CASE("Knight weapon refusal cannot complete a cast [helper_execution]")
{
    SkillFixture fixture;
    fixture.PrepareKnightCast(AT_SKILL_RAGEFUL_BLOW);
    constexpr float SkillDistance = 2.f;

    GameLogic::Combat::AttackKnight(Hero, AT_SKILL_RAGEFUL_BLOW, SkillDistance);

    CheckCastRefused();
    CHECK(CharacterAttribute->SkillDelay[0] == 0);
}

TEST_CASE("Unsupported Knight dispatch cannot complete a cast [helper_execution]")
{
    SkillFixture fixture;
    fixture.PrepareKnightCast(AT_SKILL_CYCLONE);
    fixture.EquipCycloneSword();
    constexpr float SkillDistance = 2.f;

    GameLogic::Combat::AttackKnight(Hero, AT_SKILL_CYCLONE, SkillDistance);

    CheckCastRefused();
}

#ifdef MU_TEST_COMBAT_AOT_TRANSPORT
TEST_CASE("MG Inferno completes ExecuteSkill and emits its area packet [helper_accepted_transport]")
{
    AcceptedCastFixture fixture;
    fixture.PrepareKnightCast(AT_SKILL_INFERNO_STR_MG);
    Hero->Class = CLASS_DARK;
    // Unarmed Inferno avoids weapon classification from the unloaded item database.
    REQUIRE(fixture.machine.Equipment[EQUIPMENT_WEAPON_RIGHT].Type == -1);
    REQUIRE(fixture.machine.Equipment[EQUIPMENT_WEAPON_LEFT].Type == -1);
    // PrepareKnightCast resets selection; the non-attackable selected character avoids terrain rendering.
    SelectedCharacter = 0;
    constexpr float SkillDistance = 2.f;
    const int result = GameLogic::Combat::ExecuteSkill(Hero, AT_SKILL_INFERNO_STR_MG, SkillDistance);
    CHECK(result == 1);
    CHECK(Hero->SkillSuccess);
    CHECK(GameLogic::Combat::ExecuteSkillComplete(Hero));
    CHECK(Hero->Object.CurrentAction == PLAYER_SKILL_INFERNO);
    constexpr BYTE AreaSkillCode = 0x1E;
    constexpr std::size_t AreaSkillLength = 13;
    const auto packet = fixture.ExpectSkillPacket(AreaSkillCode, AT_SKILL_INFERNO_STR_MG, AreaSkillLength);
    constexpr std::size_t TargetXOffset = 5;
    constexpr std::size_t TargetYOffset = 6;
    CHECK(packet[TargetXOffset] == MovementStartTile);
    CHECK(packet[TargetYOffset] == MovementStartTile);
}

TEST_CASE("Shield Blocking completes after a refusal and emits its targeted packet [helper_accepted_transport]")
{
    AcceptedCastFixture fixture;
    fixture.PrepareKnightCast(AT_SKILL_TWISTING_SLASH);
    fixture.machine.Equipment[EQUIPMENT_WEAPON_RIGHT].Type = ITEM_SHORT_SWORD;
    constexpr int RemainingDelay = 50;
    CharacterAttribute->SkillDelay[0] = RemainingDelay;
    constexpr float SkillDistance = 2.f;
    GameLogic::Combat::AttackKnight(Hero, AT_SKILL_TWISTING_SLASH, SkillDistance);
    REQUIRE_FALSE(Hero->SkillSuccess);
    REQUIRE(CharacterAttribute->SkillDelay[0] == RemainingDelay);
    CharacterAttribute->Skill[0] = AT_SKILL_BLOCKING;
    CharacterAttribute->SkillDelay[0] = 0;
    auto& shield = fixture.machine.Equipment[EQUIPMENT_WEAPON_LEFT];
    shield.Type = ITEM_SMALL_SHIELD;
    shield.SpecialNum = 1;
    shield.Special[0] = AT_SKILL_BLOCKING;
    SelectedCharacter = 0;
    constexpr DWORD MagicThrottleMs = 300;
    g_dwLatestMagicTick = GetTickCount() - (MagicThrottleMs + 1);
    const int result = GameLogic::Combat::ExecuteSkill(Hero, AT_SKILL_BLOCKING, SkillDistance);
    CHECK(result == 1);
    CHECK(Hero->SkillSuccess);
    CHECK(GameLogic::Combat::ExecuteSkillComplete(Hero));
    CHECK(Hero->Object.CurrentAction == PLAYER_DEFENSE1);
    constexpr BYTE TargetedSkillCode = 0x19;
    constexpr std::size_t TargetedSkillLength = 7;
    const auto packet = fixture.ExpectSkillPacket(TargetedSkillCode, AT_SKILL_BLOCKING, TargetedSkillLength);
    constexpr std::size_t TargetOffset = 5;
    CHECK(((static_cast<WORD>(packet[TargetOffset]) << 8) | packet[TargetOffset + 1]) == Hero->Key);
}
#endif

TEST_CASE("Resource shortages reach execution while unlearned skills are skipped [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Mana = 10;
    SkillAttribute[ExtraOne].AbilityGuage = 5;
    CharacterAttribute->Mana = 9;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->Mana = 10;
    CharacterAttribute->SkillMana = 4;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->SkillMana = 5;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CharacterAttribute->Skill[1] = AT_SKILL_UNDEFINED;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(AT_SKILL_UNDEFINED));
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(static_cast<ActionSkillType>(MAX_SKILLS)));
}

TEST_CASE("Automatic eligibility retains attributes and skill-use conditions [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Strength = 501;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    SkillAttribute[ExtraOne].Strength = 500;
    gSkillManager.InvalidateSkillAttributeRequirementsCache();
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    SkillAttribute[ExtraOne].SkillUseType = SKILL_USE_TYPE_BRAND;
    CHECK_FALSE(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
}

TEST_CASE("Busy and approaching actions keep their slot while cast and refusal advance [helper_rotation]")
{
    SkillFixture fixture;
    CHECK(fixture.Select() == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    for (const auto result : {SkillResult::Busy, SkillResult::Approaching})
    {
        CHECK(fixture.Select() == ExtraOne);
        fixture.rotation.Complete(result);
        CHECK(fixture.Select() == ExtraOne);
    }
    fixture.rotation.Complete(SkillResult::Refused);
    CHECK(fixture.Select() == ExtraTwo);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == Primary);
}

TEST_CASE("Timers and monster conditions keep priority and OR semantics [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.aiSkillCondition[1] = MUHelper::ON_TIMER;
    fixture.config.aiSkillInterval[1] = 5;
    CHECK(fixture.Select(0) == Primary);
    fixture.config.aiSkillInterval[1] = 0;
    CHECK(fixture.Select(5) == Primary);
    fixture.config.aiSkillInterval[1] = 5;
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(1) == ExtraTwo);
    CHECK(fixture.Select(5) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(6) == ExtraTwo);
    fixture.rotation.Reset();
    // A timer starts when the rotation first sees it and stays due until a cast consumes it.
    CHECK(fixture.Select(5) == Primary);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(9) == ExtraTwo);
    CHECK(fixture.Select(10) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Busy);
    CHECK(fixture.Select(11) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(11) == ExtraTwo);
    CHECK(fixture.Select(15) == ExtraTwo);
    fixture.rotation.Complete(SkillResult::Cast);
    fixture.config.aiSkillCondition[1] |=
        SkillCondition(MUHelper::ON_CONDITION, MUHelper::ON_MOBS_NEARBY, MUHelper::ON_MORE_THAN_TWO_MOBS);
    CHECK(fixture.Select(1, 2) == ExtraOne);
    CHECK(fixture.Select(16, 0) == ExtraOne);
    fixture.config.aiSkillCondition[1] =
        SkillCondition(MUHelper::ON_CONDITION, MUHelper::ON_MOBS_ATTACKING, MUHelper::ON_MORE_THAN_THREE_MOBS);
    CHECK(fixture.Select(1, 5, 2) == Primary);
    CHECK(fixture.Select(1, 0, 3) == ExtraOne);
    fixture.config.aiSkillCondition[2] = MUHelper::ON_TIMER;
    fixture.config.aiSkillInterval[2] = 1;
    CHECK(fixture.Select(1, 0, 3) == ExtraOne);
    CHECK(fixture.Select(1, 0, 0) == Primary);
    CHECK(fixture.Select(2, 0, 0) == ExtraTwo);
}

TEST_CASE("Triggered skills that cannot be cast leave the rotation to usable ones [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.aiSkillCondition[1] =
        SkillCondition(MUHelper::ON_CONDITION, MUHelper::ON_MOBS_NEARBY, MUHelper::ON_MORE_THAN_TWO_MOBS);
    CharacterAttribute->Skill[1] = AT_SKILL_UNDEFINED;
    for (int tick = 0; tick < 3; ++tick)
    {
        CHECK(fixture.Select(0, 5) != ExtraOne);
        fixture.rotation.Complete(SkillResult::Cast);
    }
    CharacterAttribute->Skill[1] = ExtraOne;
    CHECK(fixture.Select(0, 5) == ExtraOne);
}

TEST_CASE("Primary fallback and primary-only behavior remain available [helper_rotation]")
{
    SkillFixture fixture;
    for (int slot = 0; slot < 3; ++slot)
        CharacterAttribute->SkillDelay[slot] = 100;
    CHECK(fixture.Select() == Primary);
    fixture.config.aiSkill = {Primary, 0, 0};
    CHECK(fixture.Select() == Primary);
    fixture.config.aiSkill = {0, 0, 0};
    CHECK(fixture.Select() == AT_SKILL_UNDEFINED);
}

TEST_CASE("Rotation is transient and preserves saved skill combo and pickup settings [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.bUseCombo = true;
    fixture.config.bPickAllItems = true;
    fixture.config.bPickSelectItems = true;
    fixture.config.aiSkillCondition[1] =
        SkillCondition(MUHelper::ON_TIMER, MUHelper::ON_MOBS_NEARBY, MUHelper::ON_MORE_THAN_TWO_MOBS);
    fixture.config.aiSkillInterval[1] = 5;
    fixture.config.aiSkillCondition[2] =
        SkillCondition(MUHelper::ON_CONDITION, MUHelper::ON_MOBS_NEARBY, MUHelper::ON_MORE_THAN_TWO_MOBS);
    PRECEIVE_MUHELPER_DATA before{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, before);
    fixture.Select(5);
    fixture.rotation.Complete(SkillResult::Cast);
    PRECEIVE_MUHELPER_DATA after{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, after);
    CHECK(std::memcmp(&before, &after, sizeof(before)) == 0);
    MUHelper::ConfigData restored;
    MUHelper::ConfigDataSerDe::Deserialize(after, restored);
    CHECK(restored.aiSkill == fixture.config.aiSkill);
    CHECK(restored.aiSkillCondition == fixture.config.aiSkillCondition);
    CHECK(restored.aiSkillInterval == fixture.config.aiSkillInterval);
    CHECK(restored.bUseCombo);
    CHECK(restored.bPickAllItems);
    CHECK(restored.bPickSelectItems);
}

TEST_CASE("Saved return delays keep their full value [helper_rotation]")
{
    MUHelper::ConfigData config;
    config.iMaxSecondsAway = 30;
    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(config, packet);
    MUHelper::ConfigData restored;
    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);
    CHECK(restored.iMaxSecondsAway == 30);
}

TEST_CASE("Saved unconditioned slots rotate despite inactive condition subfields [helper_rotation]")
{
    SkillFixture fixture;
    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(fixture.config, packet);
    MUHelper::ConfigDataSerDe::Deserialize(packet, fixture.config);
    for (const auto expected : {Primary, ExtraOne, ExtraTwo})
    {
        CHECK(fixture.Select() == expected);
        fixture.rotation.Complete(SkillResult::Cast);
    }
}

TEST_CASE("Selected extra retains its actual slot after a skipped primary [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[Primary].Delay = 500;
    CharacterAttribute->SkillDelay[0] = 50;
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Approaching);
    CharacterAttribute->SkillDelay[0] = 0;
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Busy);
    CHECK(fixture.Select() == ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select() == ExtraTwo);
}

TEST_CASE("Real target failures preserve the next attack slot [helper_rotation]")
{
    SkillFixture fixture;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    auto* previous = CharactersClient;
    CharactersClient = characters.get();
    auto& target = characters[0];
    target.Key = 123;
    target.Object.Live = true;
    target.Dead = 1;
    for (const auto expected : {Primary, ExtraOne, ExtraTwo})
    {
        CHECK(fixture.Select() == expected);
        const auto result = GameLogic::Automation::CastSkill(expected, true, target.Key, true, 6);
        CHECK(result == SkillResult::NotAttackable);
        fixture.rotation.Complete(result);
        CHECK(fixture.Select() == expected);
        target.Object.Live = false;
        const auto gone = GameLogic::Automation::CastSkill(expected, true, target.Key, true, 6);
        CHECK(gone == SkillResult::NotInView);
        fixture.rotation.Complete(gone);
        CHECK(fixture.Select() == expected);
        fixture.rotation.Complete(SkillResult::Cast);
        target.Object.Live = true;
    }
    CharactersClient = previous;
}

TEST_CASE("Long execution animations retain their selected turn [helper_rotation]")
{
    SkillFixture fixture;
    Hero->Object.Type = MODEL_PLAYER;
    for (const auto action : {PLAYER_SKILL_CHAIN_LIGHTNING, PLAYER_SKILL_THRUST, PLAYER_WALK_MALE})
    {
        fixture.rotation.Reset();
        Hero->Object.CurrentAction = action;
        for (int attempt = 0; attempt < 9; ++attempt)
        {
            REQUIRE(fixture.Select() == Primary);
            g_MovementSkill.m_iSkill = ExtraTwo;
            g_MovementSkill.m_bMagic = false;
            const auto result = GameLogic::Automation::CastSkill(Primary, false, -1, true, 6);
            CHECK(result == SkillResult::Busy);
            CHECK(g_MovementSkill.m_iSkill == ExtraTwo);
            CHECK_FALSE(g_MovementSkill.m_bMagic);
            fixture.rotation.Complete(result);
            CHECK(CharacterAttribute->SkillDelay[0] == 0);
        }
        CHECK(fixture.Select() == Primary);
    }
}

TEST_CASE("Ready skills that repeatedly produce no action yield a bounded turn [helper_rotation]")
{
    SkillFixture fixture;
    Hero->Object.Type = MODEL_PLAYER;
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        REQUIRE(fixture.Select() == Primary);
        CharacterAttribute->Skill[0] = AT_SKILL_UNDEFINED;
        const auto result = GameLogic::Automation::CastSkill(Primary, false, -1, true, 6);
        CHECK(result == SkillResult::NotReady);
        fixture.rotation.Complete(result);
        CharacterAttribute->Skill[0] = Primary;
    }
    CHECK(fixture.Select() == ExtraOne);
}

TEST_CASE("Helper timer feeds real execution outcomes back into rotation [helper_rotation]")
{
    SkillFixture fixture;
    auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    auto* previous = CharactersClient;
    CharactersClient = characters.get();
    auto& target = characters[0];
    target.Key = 123;
    target.Object.Live = true;
    target.Object.Kind = KIND_MONSTER;
    target.PositionX = target.TargetX = Hero->PositionX = 100;
    target.PositionY = target.TargetY = Hero->PositionY = 100;
    target.Object.Position[0] = target.Object.Position[1] = 100 * TERRAIN_SCALE;
    Hero->Object.Position[0] = Hero->Object.Position[1] = 100 * TERRAIN_SCALE;
    Hero->Object.Type = MODEL_PLAYER;
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    fixture.config.iHuntingRange = 6;
    fixture.config.bFallbackBasicAttack = false;
    for (auto skill : fixture.config.aiSkill)
        SkillAttribute[skill].Distance = 6;
    MUHelper::g_MuHelper.Load(fixture.config);
    MUHelper::g_MuHelper.Start();
    MUHelper::g_MuHelper.AddTarget(target.Key, false);
    for (int attempt = 0; attempt < 9; ++attempt)
    {
        MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
        CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
        CHECK(CharacterAttribute->SkillDelay[0] == 0);
    }
    // The real admission check refuses insufficient AG and advances once.
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    SkillAttribute[Primary].AbilityGuage = 101;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == ExtraOne);
    SkillAttribute[Primary].AbilityGuage = 0;
    MUHelper::g_MuHelper.Load(fixture.config);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    auto& replacement = characters[1];
    replacement.Key = 124;
    replacement.Object.Live = true;
    replacement.Object.Kind = KIND_MONSTER;
    replacement.PositionX = replacement.TargetX = 100;
    replacement.PositionY = replacement.TargetY = 100;
    replacement.Object.Position[0] = replacement.Object.Position[1] = 100 * TERRAIN_SCALE;
    target.Dead = 1;
    MUHelper::g_MuHelper.AddTarget(replacement.Key, false);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    replacement.Object.Live = false;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    target.Dead = 0;
    MUHelper::g_MuHelper.AddTarget(target.Key, false);
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    Hero->Object.CurrentAction = PLAYER_STOP_MALE;
    SkillAttribute[Primary].AbilityGuage = 101;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    Hero->Object.CurrentAction = PLAYER_SKILL_CHAIN_LIGHTNING;
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == ExtraOne);
    SkillAttribute[Primary].AbilityGuage = 0;
    MUHelper::g_MuHelper.Stop();
    MUHelper::g_MuHelper.Start();
    MUHelper::CMuHelper::TimerProc(nullptr, 0, 0, 0);
    CHECK(MUHelper::g_MuHelper.GetCurrentSkill() == Primary);
    MUHelper::g_MuHelper.Stop();
    MUHelper::g_MuHelper.DeleteAllTargets();
    CharactersClient = previous;
}

TEST_CASE("Unaffordable primary reaches the normal mana-potion path [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[Primary].Mana = 100;
    SkillAttribute[ExtraOne].Mana = 10;
    CharacterAttribute->Mana = 50;
    CHECK(fixture.Select() == Primary);
    CHECK(CharacterAttribute->Mana == 50);
    CHECK(CharacterAttribute->SkillDelay[0] == 0);
}

TEST_CASE("Zero-delay skills ignore stale delay counters as execution does [helper_rotation]")
{
    SkillFixture fixture;
    SkillAttribute[ExtraOne].Delay = 0;
    CharacterAttribute->SkillDelay[1] = 50;
    CHECK(GameLogic::Automation::IsAttackSkillAvailable(ExtraOne));
    CHECK(CharacterAttribute->SkillDelay[1] == 50);
}

TEST_CASE("A timed skill that keeps producing no action yields after bounded attempts [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.aiSkillCondition[1] = MUHelper::ON_TIMER;
    fixture.config.aiSkillInterval[1] = 5;
    CHECK(fixture.Select(0) == Primary);
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        REQUIRE(fixture.Select(5) == ExtraOne);
        fixture.rotation.Complete(SkillResult::NotReady);
    }
    REQUIRE(fixture.Select(5) == ExtraOne);
    fixture.rotation.Complete(SkillResult::NotReady);
    CHECK(fixture.Select(5) != ExtraOne);
    CHECK(fixture.Select(10) == ExtraOne);
}

TEST_CASE("A refused conditional skill yields to the rotation for a helper second [helper_rotation]")
{
    SkillFixture fixture;
    fixture.config.aiSkillCondition[1] =
        SkillCondition(MUHelper::ON_CONDITION, MUHelper::ON_MOBS_NEARBY, MUHelper::ON_MORE_THAN_TWO_MOBS);
    REQUIRE(fixture.Select(3, 5) == ExtraOne);
    fixture.rotation.Complete(SkillResult::Refused);
    CHECK(fixture.Select(3, 5) != ExtraOne);
    fixture.rotation.Complete(SkillResult::Cast);
    CHECK(fixture.Select(4, 5) == ExtraOne);
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        REQUIRE(fixture.Select(4, 5) == ExtraOne);
        fixture.rotation.Complete(SkillResult::NotReady);
    }
    CHECK(fixture.Select(4, 5) != ExtraOne);
}
