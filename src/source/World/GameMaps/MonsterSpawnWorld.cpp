#include "stdafx.h"

#include "MonsterSpawnWorld.h"

#include "Data/GameData/MonsterData/MonsterSetup.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GMCrywolf1st.h"
#include "World/GameMaps/GMEmpireGuardian1.h"
#include "World/GameMaps/GMEmpireGuardian2.h"
#include "World/GameMaps/GMEmpireGuardian3.h"
#include "World/GameMaps/GMEmpireGuardian4.h"
#include "World/GameMaps/GM_Raklion.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Events/KanturuEvent.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapHeaders.h"

namespace World::Monsters
{
namespace
{
bool g_playerTransformed = false;
bool g_respawning = false;

// While it lives, Create makes living monsters again (Respawn).
class RespawnScope
{
public:
    RespawnScope() : m_previous(g_respawning)
    {
        g_respawning = true;
    }
    ~RespawnScope()
    {
        g_respawning = m_previous;
    }

    RespawnScope(const RespawnScope&) = delete;
    RespawnScope& operator=(const RespawnScope&) = delete;

private:
    bool m_previous;
};
} // namespace

PlayerTransformScope::PlayerTransformScope() : m_previous(g_playerTransformed)
{
    g_playerTransformed = true;
}

PlayerTransformScope::~PlayerTransformScope()
{
    g_playerTransformed = m_previous;
}

bool IsSpawnStateOn(std::string_view state)
{
    if (state == "castleSiegeRunning")
        return battleCastle::IsBattleCastleStart();
    if (state == "castleSiegeNotRunning")
        return !battleCastle::IsBattleCastleStart();
    if (state == "raklionBossWaiting")
        return g_Raklion.IsBossWaiting();
    if (state == "playerTransformed")
        return g_playerTransformed;
    if (state == "crywolfReady")
        return g_Direction.m_CMVP.m_iCryWolfState == CRYWOLF_STATE_READY;
    if (state == "crywolfElfTime")
        return M34CryWolf1st::IsCyrWolf1st() && M34CryWolf1st::Get_State_Only_Elf();
    return false;
}

void RunSpawnHook(const Data::Monsters::MonsterHook& hook, CHARACTER& character)
{
    if (hook.name == "RaklionBossDrop")
    {
        g_Raklion.LetBossDrop();
    }
    else if (hook.name == "Kanturu2ndEntrance")
    {
        // The game has the window; the monster tests do not.
        if (mu::ui::window::CKanturu2ndEnterNpc* window = g_pKanturu2ndEnterNpc)
        {
            window->SetNpcObject(&character.Object);
            window->SetNpcAnimation(false);
        }
    }
    else if (hook.name == "EmpireGuardianRageReset")
    {
        const std::string_view boss = hook.Value("boss");
        if (!g_EmpireGuardian1.ResetRage(boss) && !g_EmpireGuardian2.ResetRage(boss) &&
            !g_EmpireGuardian3.ResetRage(boss))
            g_EmpireGuardian4.ResetRage(boss);
    }
    else if (hook.name == "IllusionTempleStorageReset")
    {
        g_CursedTemple->HideStoragePointEffect(hook.Value("storage") == "allied");
    }
}

const GameLogic::Monsters::Setup::SpawnWorld& GetSpawnWorld()
{
    static const GameLogic::Monsters::Setup::SpawnWorld world{IsSpawnStateOn, RunSpawnHook,
                                                              [] { return g_respawning; }};
    return world;
}

void ApplyLiveSetup(int number)
{
    GameLogic::Monsters::Setup::ApplyLive(number, GetSpawnWorld());
}

int RespawnLive(int number, bool playAppear)
{
    int made = 0;
    for (CHARACTER* character : GameLogic::Monsters::Setup::FindLive(number))
    {
        const int key = character->Key;
        // The server's tile, which CreateMonster keeps while it puts the body
        // at its tile offset.
        const BYTE x = character->PositionX;
        const BYTE y = character->PositionY;
        const float angle = character->Object.Angle[2];
        // The server's buffs (an open Castle Gate's), build time and bars
        // stay; CreateCharacter clears them.
        const GameLogic::Monsters::Setup::ServerState state = GameLogic::Monsters::Setup::KeepServerState(*character);
        GameLogic::Monsters::Setup::RemoveSpawnAttachments(*character);
        CHARACTER* again = nullptr;
        {
            const RespawnScope respawn;
            again = CreateMonster(static_cast<EMonsterType>(number), x, y, key);
        }
        if (again == nullptr)
            continue;
        GameLogic::Monsters::Setup::RestoreServerState(*again, state);
        again->Object.Angle[2] = angle;
        // A castle gate stands as its buff says, as 0x13 shows it.
        if (gMapManager.InBattleCastle())
        {
            if (const std::optional<battleCastle::GateLooks> gate = battleCastle::GateLooksOf(*again))
                battleCastle::ShowGateOnArrival(*again, *gate, x, y, false);
        }
        if (playAppear)
            GameLogic::Monsters::Setup::Appear(*again, GetSpawnWorld());
        ++made;
    }
    // A fixed facing turns them again.
    ApplyLiveSetup(number);
    return made;
}
} // namespace World::Monsters
