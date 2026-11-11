#include "stdafx.h"

#include "MonsterCreationScope.h"

#include "Core/Globals/_enum.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "GameLogic/Monsters/UnknownMonsters.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "Network/Server/WSclient.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GM_Raklion.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "World/MapInfra/w_MapProcess.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <span>
#include <vector>

extern int HeroKey;
extern int HeroIndex;

void ReceiveCreateMonsterViewport(const BYTE* ReceiveBuffer);
void ReceiveCreateSummonViewport(const BYTE* ReceiveBuffer);
void ReceiveCreateTransformViewport(std::span<const BYTE> ReceiveBuffer);
void ReceivePreviewPort(std::span<const BYTE> ReceiveBuffer);
void ReceiveSkillStatus(const BYTE* ReceiveBuffer);
void ReceiveMagicFinish(const BYTE* ReceiveBuffer);

namespace MonsterRecorder
{
namespace
{
constexpr int HeroTestKey = CreationScope::TestHeroKey;
constexpr int MonsterTestKey = CreationScope::TestMonsterKey;
constexpr unsigned char TileX = 100;
constexpr unsigned char TileY = 100;
// The server's direction of a packet's monster: 90 by 45 steps.
constexpr BYTE Direction = 3;
// The flags of a packet's key: just spawned (0x13, 0x1F, 0x45), teleported (0x13).
constexpr BYTE NewFlag = 0x80;
constexpr BYTE TeleportFlag = 0x40;

// The buffs the next packets carry (SetPacketBuffs).
std::vector<int> g_packetBuffs;

template <typename Entry> void SetBuffs(Entry& entry)
{
    entry.s_BuffCount = static_cast<BYTE>(g_packetBuffs.size());
    for (size_t i = 0; i < g_packetBuffs.size(); ++i)
        entry.s_BuffEffectState[i] = static_cast<BYTE>(g_packetBuffs[i]);
}

// A packet with the one entry, as the handlers read it.
template <typename Entry> std::vector<BYTE> Packet(const Entry& entry)
{
    PWHEADER_DEFAULT_WORD header{};
    header.Value = 1;
    std::vector<BYTE> packet(sizeof(header) + sizeof(entry));
    std::memcpy(packet.data(), &header, sizeof(header));
    std::memcpy(packet.data() + sizeof(header), &entry, sizeof(entry));
    return packet;
}

template <typename Entry> void SetKey(Entry& entry, int key, BYTE flags)
{
    entry.KeyH = static_cast<BYTE>(((key >> 8) & 0x7F) | flags);
    entry.KeyL = static_cast<BYTE>(key & 0xFF);
}

void SendViewport(int number, BYTE flags)
{
    PCREATE_MONSTER entry{};
    SetKey(entry, CreationScope::TestMonsterKey, flags);
    entry.TypeH = static_cast<BYTE>((number >> 8) & 0x03);
    entry.TypeL = static_cast<BYTE>(number & 0xFF);
    entry.PositionX = entry.TargetX = TileX + 2;
    entry.PositionY = entry.TargetY = TileY;
    entry.Path = static_cast<BYTE>(Direction << 4);
    SetBuffs(entry);
    ReceiveCreateMonsterViewport(Packet(entry).data());
}

void SendSummon(int number, BYTE flags)
{
    PCREATE_SUMMON entry{};
    SetKey(entry, CreationScope::TestMonsterKey, flags);
    entry.TypeH = static_cast<BYTE>(number >> 8);
    entry.TypeL = static_cast<BYTE>(number & 0xFF);
    entry.PositionX = entry.TargetX = TileX + 2;
    entry.PositionY = entry.TargetY = TileY;
    entry.Path = static_cast<BYTE>(Direction << 4);
    std::memcpy(entry.ID, "Owner", 5);
    SetBuffs(entry);
    ReceiveCreateSummonViewport(Packet(entry).data());
}

void SendTransform(int number, BYTE flags)
{
    PCREATE_TRANSFORM_EXTENDED entry{};
    SetKey(entry, CreationScope::TestMonsterKey, flags);
    entry.TypeH = static_cast<BYTE>(number >> 8);
    entry.TypeL = static_cast<BYTE>(number & 0xFF);
    entry.PositionX = entry.TargetX = TileX + 2;
    entry.PositionY = entry.TargetY = TileY;
    entry.Path = static_cast<BYTE>(Direction << 4);
    std::memcpy(entry.ID, "Player", 6);
    std::fill(std::begin(entry.Equipment), std::end(entry.Equipment), BYTE{0xFF});
    SetBuffs(entry);
    const std::vector<BYTE> packet = Packet(entry);
    ReceiveCreateTransformViewport(packet);
}

void SendPreviewPort(int number)
{
    PRECEIVE_PREVIEW_PORT_EXTENDED entry{};
    // A monster (3: an NPC, the same).
    entry.m_byObjType = 2;
    entry.m_byTypeH = static_cast<BYTE>(number >> 8);
    entry.m_byTypeL = static_cast<BYTE>(number & 0xFF);
    entry.m_byKeyH = static_cast<BYTE>(CreationScope::TestMonsterKey >> 8);
    entry.m_byKeyL = static_cast<BYTE>(CreationScope::TestMonsterKey & 0xFF);
    entry.m_byPosX = TileX + 2;
    entry.m_byPosY = TileY;
    std::fill(std::begin(entry.m_byEquipment), std::end(entry.m_byEquipment), BYTE{0xFF});
    SetBuffs(entry);
    const std::vector<BYTE> packet = Packet(entry);
    ReceivePreviewPort(packet);
}

void SummonInCutscene(int number)
{
    g_Direction.stl_Monster.clear();
    g_Direction.SummonCreateMonster(static_cast<EMonsterType>(number), TileX + 2, TileY, 0.0f, false, true);
    g_Direction.stl_Monster.clear();
}
} // namespace

const char* Describe(CreationPath path)
{
    switch (path)
    {
    case CreationPath::CreateMonster:
        return "CreateMonster";
    case CreationPath::Viewport:
        return "0x13";
    case CreationPath::ViewportNew:
        return "0x13 new";
    case CreationPath::ViewportTeleport:
        return "0x13 teleported";
    case CreationPath::Summon:
        return "0x1F";
    case CreationPath::SummonNew:
        return "0x1F new";
    case CreationPath::Transform:
        return "0x45";
    case CreationPath::TransformNew:
        return "0x45 new";
    case CreationPath::PreviewPort:
        return "0x68";
    case CreationPath::Cinematic:
        return "cutscene";
    }
    return "";
}

struct CreationScope::State
{
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT + 1]{}};
    EGameScene previousScene = WEBZEN_SCENE;
    std::vector<float> previousGround;
    CHARACTER* previousCharacters = nullptr;
    CHARACTER* previousHero = nullptr;
    int previousHeroKey = 0;
    int previousHeroIndex = 0;
    MapProcessPtr previousMapProcess;
    PetProcessPtr previousPetProcess;
    bool madeBuffSystem = false;
};

CreationScope::CreationScope(int world) : m_load(std::make_unique<LoadScope>(world)), m_state(std::make_unique<State>())
{
    m_state->previousCharacters = CharactersClient;
    m_state->previousHero = Hero;
    m_state->previousHeroKey = HeroKey;
    m_state->previousHeroIndex = HeroIndex;
    m_state->previousMapProcess = g_MapProcess;

    // The game's scene, on a sloped ground: monsters appear in the main scene,
    // where RequestTerrainHeight reads the ground (in the others it is 0), so
    // a creator that keeps or adds to the ground height shows.
    m_state->previousScene = SceneFlag;
    SceneFlag = MAIN_SCENE;
    m_state->previousGround.assign(BackTerrainHeight, BackTerrainHeight + TERRAIN_SIZE * TERRAIN_SIZE);
    for (int y = 0; y < TERRAIN_SIZE; ++y)
    {
        for (int x = 0; x < TERRAIN_SIZE; ++x)
            BackTerrainHeight[y * TERRAIN_SIZE + x] = 150.0f + static_cast<float>(x) * 3.0f + static_cast<float>(y);
    }

    CharactersClient = m_state->characters.get();
    Hero = &CharactersClient[0];
    HeroIndex = 0;
    HeroKey = HeroTestKey;
    CreateCharacterPointer(Hero, MODEL_PLAYER, TileX, TileY, 0.0f);
    Hero->Key = HeroTestKey;
    g_MapProcess = MapProcess::Make();
    // 0x45 reads the player's equipment, which reaches the pets; 0x13 finds
    // the monster's path on the map's walls.
    m_state->previousPetProcess = g_petProcess;
    g_petProcess = PetProcess::Make();
    InitPath();
    // The buffs a packet carries are registered with the buff system's
    // classes (RegisterBuff); it reads the English buff list relative to the
    // client folder.
    if (!g_BuffSystem)
    {
        const std::filesystem::path previousDirectory = std::filesystem::current_path();
        const std::wstring previousLanguage = g_strSelectedML;
        std::filesystem::current_path(ClientFolder());
        g_strSelectedML = L"Eng";
        g_BuffSystem = BuffStateSystem::Make();
        g_strSelectedML = previousLanguage;
        std::filesystem::current_path(previousDirectory);
        m_state->madeBuffSystem = true;
    }
}

CreationScope::~CreationScope()
{
    SetSpawnState("castleSiegeRunning", false);
    SetSpawnState("raklionBossWaiting", false);
    SetSpawnState("crywolfReady", false);
    for (int i = 0; i <= MAX_CHARACTERS_CLIENT; ++i)
    {
        // The lists that point at the characters forget them before the
        // array is freed, so no later test reads through them.
        GameLogic::Monsters::Setup::ForgetLive(CharactersClient[i]);
        GameLogic::Monsters::Unknown::Forget(CharactersClient[i]);
        delete[] CharactersClient[i].Object.BoneTransform;
        CharactersClient[i].Object.BoneTransform = nullptr;
    }
    g_Direction.stl_Monster.clear();
    g_packetBuffs.clear();
    if (m_state->madeBuffSystem)
        g_BuffSystem.reset();
    g_petProcess = m_state->previousPetProcess;
    g_MapProcess = m_state->previousMapProcess;
    std::copy(m_state->previousGround.begin(), m_state->previousGround.end(), BackTerrainHeight);
    SceneFlag = m_state->previousScene;
    HeroIndex = m_state->previousHeroIndex;
    HeroKey = m_state->previousHeroKey;
    Hero = m_state->previousHero;
    CharactersClient = m_state->previousCharacters;
}

void CreationScope::SetWorld(int world)
{
    gMapManager.WorldActive = world;
}

void CreationScope::SetSpawnState(std::string_view state, bool on)
{
    if (state == "castleSiegeRunning" || state == "castleSiegeNotRunning")
        battleCastle::SetBattleCastleStart(on == (state == "castleSiegeRunning"));
    else if (state == "crywolfReady")
        g_Direction.m_CMVP.m_iCryWolfState = on ? CRYWOLF_STATE_READY : CRYWOLF_STATE_NONE;
    else if (state == "raklionBossWaiting")
        g_Raklion.SetState(static_cast<BYTE>(on ? SEASON4A::CGM_Raklion::RAKLION_STATE_STANDBY
                                                : SEASON4A::CGM_Raklion::RAKLION_STATE_IDLE),
                           0);
}

void CreationScope::SetServerMap(int serverMap, int stage)
{
    gMapManager.SetServerMap(serverMap);
    gMapManager.SetServerStage(stage);
}

CHARACTER* CreationScope::Create(int number)
{
    return CreateMonster(static_cast<EMonsterType>(number), TileX + 2, TileY, MonsterTestKey);
}

void CreationScope::SetPacketBuffs(std::vector<int> buffs)
{
    g_packetBuffs = std::move(buffs);
}

void CreationScope::SendBuffState(const CHARACTER& character, int buff, bool add)
{
    PMSG_VIEWSKILLSTATE packet{};
    packet.State = add ? 1 : 0;
    packet.KeyH = static_cast<BYTE>((character.Key >> 8) & 0x7F);
    packet.KeyL = static_cast<BYTE>(character.Key & 0xFF);
    packet.BuffIndex = static_cast<BYTE>(buff);
    ReceiveSkillStatus(reinterpret_cast<const BYTE*>(&packet));
}

void CreationScope::SendMagicFinish(const CHARACTER& character, int skill)
{
    PHEADER_DEFAULT_VALUE_KEY packet{};
    packet.Value = static_cast<BYTE>(skill);
    packet.KeyH = static_cast<BYTE>((character.Key >> 8) & 0x7F);
    packet.KeyL = static_cast<BYTE>(character.Key & 0xFF);
    ReceiveMagicFinish(reinterpret_cast<const BYTE*>(&packet));
}

int CreationScope::KeyOf(CreationPath path)
{
    if (path == CreationPath::ViewportTeleport)
        return MonsterTestKey | (TeleportFlag << 8);
    if (path == CreationPath::Cinematic)
        return NUMOFMON;
    return MonsterTestKey;
}

CHARACTER* CreationScope::Create(int number, CreationPath path)
{
    switch (path)
    {
    case CreationPath::CreateMonster:
        return Create(number);
    case CreationPath::Viewport:
        SendViewport(number, 0);
        break;
    case CreationPath::ViewportNew:
        SendViewport(number, NewFlag);
        break;
    case CreationPath::ViewportTeleport:
        SendViewport(number, TeleportFlag);
        break;
    case CreationPath::Summon:
        SendSummon(number, 0);
        break;
    case CreationPath::SummonNew:
        SendSummon(number, NewFlag);
        break;
    case CreationPath::Transform:
        SendTransform(number, 0);
        break;
    case CreationPath::TransformNew:
        SendTransform(number, NewFlag);
        break;
    case CreationPath::PreviewPort:
        SendPreviewPort(number);
        break;
    case CreationPath::Cinematic:
        SummonInCutscene(number);
        break;
    }
    const int index = FindCharacterIndex(KeyOf(path));
    return index == MAX_CHARACTERS_CLIENT ? nullptr : &CharactersClient[index];
}

int CreationScope::CreatedModel(int number)
{
    CHARACTER* character = CreateMonster(static_cast<EMonsterType>(number), TileX + 2, TileY, MonsterTestKey);
    if (character == nullptr)
    {
        return -1;
    }
    const int model = character->Object.Type;
    character->Object.Live = false;
    return model;
}
} // namespace MonsterRecorder
