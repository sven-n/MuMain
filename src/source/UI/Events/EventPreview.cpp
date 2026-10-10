#include "stdafx.h"

#include "UI/Events/EventPreview.h"

#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameShop/InGameShop.h"
#include "GameShop/InGameShopSystem.h"
#include "GameLogic/Events/MatchEvent.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Events/BloodCastleTime.h"
#include "UI/Events/ChaosCastleTime.h"
#include "UI/Events/CursedTempleResult.h"
#include "UI/Events/CursedTempleSystem.h"
#include "UI/Events/CursedTempleUpdates.h"
#include "UI/Events/CryWolf.h"
#include "UI/Events/CryWolfUpdates.h"
#include "UI/Events/KanturuEvent.h"
#include "UI/Combat/SiegeUpdates.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Combat/SiegeWarfare.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/GameMaps/GMCrywolf1st.h"

#include <array>
#include <string_view>

// GMCrywolf1st.cpp's state, which the CryWolf window draws from.
extern BYTE m_AltarState[5];
extern bool View_Bal;
extern char Suc_Or_Fail;
extern char View_Suc_Or_Fail;
extern int Dark_elf_Num;
extern int Val_Hp;
extern int Delay;
extern int Add_Num;
extern BYTE Rank;
extern int Exp;
extern CLASS_TYPE HeroClass[5];
extern int HeroScore[5];
extern wchar_t HeroName[5][MAX_USERNAME_SIZE + 1];
extern BYTE m_CrywolfState;
// ZzzInterface.cpp: when the last chat macro went out.
extern uint64_t LastMacroTime;

namespace UI::EventPreview
{
namespace
{
using mu::ui::window::CManager;

Event s_Showing = Event::None;

// An entry with no window of its own: its texts show wherever the game draws them.
constexpr DWORD kNoWindow = 0xFFFFFFFF;

struct Entry
{
    std::wstring_view name;
    Event event;
    DWORD window;
    std::wstring_view what;
};

constexpr std::array kEntries = {
    Entry{L"bloodcastle", Event::BloodCastle, mu::ui::window::INTERFACE_BLOODCASTLE_TIME,
          L"Blood Castle timer, 8:15 left, 23 of 40 monsters"},
    Entry{L"chaoscastle", Event::ChaosCastle, mu::ui::window::INTERFACE_CHAOSCASTLE_TIME,
          L"Chaos Castle timer, 3:42 left (imminent), 34 of 70 left"},
    Entry{L"temple", Event::Temple, mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM,
          L"Illusion Temple HUD, allied 3 : illusion 5, a relic carrier and two allies"},
    Entry{L"templeresult", Event::TempleResult, mu::ui::window::INTERFACE_CURSEDTEMPLE_RESULT,
          L"Illusion Temple result, allied win 7 : 5, three players a side"},
    Entry{L"duelusers", Event::DuelSpectators, mu::ui::window::INTERFACE_DUELWATCH_USERLIST,
          L"duel spectator list, five spectators"},
    Entry{L"duelwatch", Event::DuelWatch, mu::ui::window::INTERFACE_DUELWATCH_MAINFRAME,
          L"watched duel frame, 3 : 5, both fighters hurt, five spectators"},
    Entry{L"crywolf", Event::CryWolf, mu::ui::window::INTERFACE_CRYWOLF,
          L"CryWolf battle HUD, altars in each state, Balgass at 62%, 12 minutes left"},
    Entry{L"crywolfresult", Event::CryWolfResult, mu::ui::window::INTERFACE_CRYWOLF,
          L"CryWolf result, success banner, rank and the hero list"},
    Entry{L"siege", Event::Siege, mu::ui::window::INTERFACE_SIEGEWARFARE,
          L"castle siege commander HUD, members, NPCs and two commands, 45 minutes left"},
    Entry{L"igs", Event::CashShop, mu::ui::window::INTERFACE_INGAMESHOP,
          L"cash shop from the local script and banner, storage and gift box filled"},
    Entry{L"status", Event::HudStatus, kNoWindow,
          L"HUD status texts, both crown switches held (shown at the switches, Valley of Loren 150-200, "
          L"180-230), a macro cooldown and the Blood Castle entry countdown"},
    Entry{L"bcresult", Event::BloodCastleResult, kNoWindow, L"Blood Castle result box, quest completed"},
    Entry{L"ccresult", Event::ChaosCastleResult, kNoWindow, L"Chaos Castle result box, quest failed"},
    Entry{L"dsrank", Event::DevilSquareRank, kNoWindow, L"Devil Square ranking box, five players, the hero third"},
    Entry{L"switchbox", Event::CrownSwitchBox, kNoWindow, L"crown switch progress box, another guild pushing it"},
    Entry{L"guildwar", Event::GuildWar, kNoWindow, L"guild war time, 0:42 left, and its result, 3 : 5 lost"},
    Entry{L"kanturu", Event::KanturuEntry, mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC,
          L"Kanturu entry window, the tower open: its subject and two state texts"},
};

void Log(const std::wstring& text, mu::ui::window::MESSAGE_TYPE type = mu::ui::window::TYPE_SYSTEM_MESSAGE)
{
    g_pSystemLogBox->AddText(text.c_str(), type);
}

// Shown and hidden without the window's opening or closing process, which may talk to the server.
void SetWindowShown(DWORD window, bool shown)
{
    if (window == kNoWindow)
        return;
    if (CManager* manager = g_pNewUISystem->GetNewUIManager())
        manager->ShowInterface(window, shown);
}

void SeedBloodCastle()
{
    g_pBloodCastle->SetTime(8 * 60 + 15);
    g_pBloodCastle->SetKillMonsterStatue(23, 40);
}

void SeedChaosCastle()
{
    g_pChaosCastleTime->SetTime(3 * 60 + 42);
    g_pChaosCastleTime->SetKillMonsterStatue(34, 70);
}

void SeedTemple()
{
    g_pCursedTempleWindow->ResetCursedTempleSystemInfo();
    const auto x = static_cast<std::uint8_t>(Hero->PositionX);
    const auto y = static_cast<std::uint8_t>(Hero->PositionY);
    const std::array party = {
        UI::CursedTemple::PartyPosition{static_cast<std::uint16_t>(Hero->Key), 0, x, y},
        UI::CursedTemple::PartyPosition{0x7001, 0, static_cast<std::uint8_t>(x + 8), static_cast<std::uint8_t>(y - 6)},
        UI::CursedTemple::PartyPosition{0x7002, 0, static_cast<std::uint8_t>(x - 10), static_cast<std::uint8_t>(y + 4)},
    };
    UI::CursedTemple::MatchStatus status{};
    status.remainingSeconds = 7 * 60 + 12;
    status.relicHolderIndex = 0x7003;
    status.relicX = static_cast<std::uint8_t>(x + 14);
    status.relicY = static_cast<std::uint8_t>(y + 12);
    status.alliedPoints = 3;
    status.illusionPoints = 5;
    status.localTeam = SEASON3A::eTeam_Allied;
    status.party = party;
    UI::CursedTemple::UpdateMatchStatus(status);
    UI::CursedTemple::SetSkillPoints(25);
}

void SeedTempleResult()
{
    g_pCursedTempleResultWindow->ResetGameResultInfo();
    g_pCursedTempleResultWindow->SetMyTeam(SEASON3A::eTeam_Allied);
    const std::array players = {
        UI::CursedTemple::PlayerResult{Hero->ID, 0, SEASON3A::eTeam_Allied, Hero->Class, 152340},
        UI::CursedTemple::PlayerResult{L"Valkyrie", 0, SEASON3A::eTeam_Allied, CLASS_ELF, 98120},
        UI::CursedTemple::PlayerResult{L"Ironclad", 0, SEASON3A::eTeam_Allied, CLASS_KNIGHT, 87455},
        UI::CursedTemple::PlayerResult{L"Hexweaver", 0, SEASON3A::eTeam_Illusion, CLASS_SUMMONER, 64210},
        UI::CursedTemple::PlayerResult{L"Duskblade", 0, SEASON3A::eTeam_Illusion, CLASS_DARK, 51980},
        UI::CursedTemple::PlayerResult{L"Lordship", 0, SEASON3A::eTeam_Illusion, CLASS_DARK_LORD, 47330},
    };
    g_pCursedTempleResultWindow->SetResult({7, 5, players});
}

void SeedDuelSpectators()
{
    g_DuelMgr.RemoveAllDuelWatchUser();
    for (const wchar_t* name : {L"Spectator", L"Valkyrie", L"Ironclad", L"Hexweaver", L"Duskblade"})
        g_DuelMgr.AddDuelWatchUser(name);
}

// The watched duel's frame takes the main HUD's place, as the duel-watch buff shows it.
void SeedDuelWatch()
{
    g_DuelMgr.SetCurrentChannel(0);
    g_DuelMgr.SetDuelPlayer(DUEL_HERO, 0, L"Valkyrie");
    g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, 0, L"Ironclad");
    g_DuelMgr.SetScore(DUEL_HERO, 3);
    g_DuelMgr.SetScore(DUEL_ENEMY, 5);
    g_DuelMgr.SetHP(DUEL_HERO, 80);
    g_DuelMgr.SetHP(DUEL_ENEMY, 45);
    g_DuelMgr.SetSD(DUEL_HERO, 60);
    g_DuelMgr.SetSD(DUEL_ENEMY, 20);
    g_DuelMgr.SetFighterRegenerated(TRUE);
    SeedDuelSpectators();
    SetWindowShown(mu::ui::window::INTERFACE_MAINFRAME, false);
    SetWindowShown(mu::ui::window::INTERFACE_BUFF_WINDOW, false);
    SetWindowShown(mu::ui::window::INTERFACE_DUELWATCH_USERLIST, true);
}

void ResetDuelWatch()
{
    SetWindowShown(mu::ui::window::INTERFACE_DUELWATCH_USERLIST, false);
    SetWindowShown(mu::ui::window::INTERFACE_MAINFRAME, true);
    SetWindowShown(mu::ui::window::INTERFACE_BUFF_WINDOW, true);
    g_DuelMgr.RemoveAllDuelWatchUser();
    for (int player = DUEL_HERO; player < MAX_DUEL_PLAYERS; ++player)
    {
        g_DuelMgr.SetDuelPlayer(player, 0, L"");
        g_DuelMgr.SetScore(player, 0);
        g_DuelMgr.SetHP(player, 0);
        g_DuelMgr.SetSD(player, 0);
    }
    g_DuelMgr.SetCurrentChannel(-1);
}

void SeedCryWolf()
{
    M34CryWolf1st::CryWolfMVPInit();
    m_CrywolfState = CRYWOLF_STATE_START;
    // Each altar look: free and applying, contracted, contracted and applying, occupied.
    const BYTE altars[5] = {0x02, 0x11, 0x12, 0x21, 0x52};
    for (int i = 0; i < 5; ++i)
        m_AltarState[i] = altars[i];
    Dark_elf_Num = 4;
    View_Bal = true;
    Val_Hp = 62;
    g_pCryWolfInterface->InitTime();
    UI::CryWolf::SetCountdown(0, 12);
}

void SeedCryWolfResult()
{
    M34CryWolf1st::CryWolfMVPInit();
    Suc_Or_Fail = 1;
    View_Suc_Or_Fail = 1;
    Add_Num = 11;
    Delay = 0;
    Rank = 2;
    Exp = 1234567;
    const wchar_t* names[5] = {L"Valkyrie", L"Ironclad", L"Hexweaver", L"Duskblade", L"Lordship"};
    const CLASS_TYPE classes[5] = {CLASS_ELF, CLASS_KNIGHT, CLASS_SUMMONER, CLASS_DARK, CLASS_DARK_LORD};
    for (int i = 0; i < 5; ++i)
    {
        HeroClass[i] = classes[i];
        HeroScore[i] = 3200 - i * 450;
        wcsncpy_s(HeroName[i], names[i], _TRUNCATE);
    }
}

void SeedSiege()
{
    g_pSiegeWarfare->CreatePreviewMiniMapUI(mu::ui::window::CSiegeWarfare::SIEGEWAR_TYPE_COMMANDER);
    battleCastle::SetBattleCastleStart(true);
    UI::Siege::SetMatchTime(0, 45);
    const std::array members = {
        UI::Siege::MapLocation{0, 80, 120},  UI::Siege::MapLocation{0, 86, 126}, UI::Siege::MapLocation{0, 92, 118},
        UI::Siege::MapLocation{0, 110, 150}, UI::Siege::MapLocation{0, 116, 158},
    };
    UI::Siege::ReplaceMemberLocations(members);
    const std::array npcs = {UI::Siege::MapLocation{0, 90, 200}, UI::Siege::MapLocation{1, 120, 210}};
    UI::Siege::AddNpcLocations(npcs);
    UI::Siege::SetCommanderMapInfo(0, 100, 140, 0);
    UI::Siege::SetCommanderMapInfo(1, 70, 180, 1);
}

// The entry countdown and the result boxes run on the map's match; off an event map the preview
// lends one.
bool s_LentMatch = false;

template <typename Match>
void LendMatch()
{
    if (matchEvent::g_csMatchInfo != nullptr && !s_LentMatch)
        return;
    matchEvent::DeleteEventMatch();
    matchEvent::g_csMatchInfo = new Match;
    s_LentMatch = true;
}

void ReturnMatch()
{
    if (s_LentMatch)
        matchEvent::DeleteEventMatch();
    s_LentMatch = false;
}

MatchResult SampleResult(const char* name, DWORD score, DWORD exp, DWORD zen)
{
    MatchResult result{};
    strncpy_s(reinterpret_cast<char*>(result.m_lpID), sizeof(result.m_lpID), name, _TRUNCATE);
    result.m_iScore = score;
    result.m_dwExp = exp;
    result.m_iZen = zen;
    return result;
}

void SeedBloodCastleResult()
{
    LendMatch<SEASON3B::CNewBloodCastleSystem>();
    const MatchResult result = SampleResult("testgmDk", 1200, 350000, 250000);
    matchEvent::SetMatchResult(255, 0, const_cast<MatchResult*>(&result), 1);
}

void SeedChaosCastleResult()
{
    LendMatch<SEASON3B::CNewChaosCastleSystem>();
    const MatchResult result = SampleResult("testgmDk", 23, 180000, 4);
    matchEvent::SetMatchResult(254, 0, const_cast<MatchResult*>(&result), 0);
}

void SeedGuildWar()
{
    g_wtMatchTimeLeft.m_Type = 1;
    g_wtMatchTimeLeft.m_Time = 42;
    g_wtMatchResult.Clear();
    wcsncpy_s(g_wtMatchResult.m_MatchTeamName1, L"Lionheart", _TRUNCATE);
    wcsncpy_s(g_wtMatchResult.m_MatchTeamName2, L"Ravens", _TRUNCATE);
    g_wtMatchResult.m_Score1 = 3;
    g_wtMatchResult.m_Score2 = 5;
}

void SeedDevilSquareRank()
{
    LendMatch<CSDevilSquareMatch>();
    MatchResult results[5] = {
        SampleResult("Valkyrie", 9800, 120000, 50000), SampleResult("Ironclad", 8700, 110000, 40000),
        SampleResult("testgmDk", 7600, 100000, 30000), SampleResult("Hexweaver", 5400, 80000, 20000),
        SampleResult("Duskblade", 3200, 60000, 10000),
    };
    matchEvent::SetMatchResult(5, 3, results);
}

void SeedHudStatus()
{
    Delete_Switch();
    Switch_Info = new CROWN_SWITCH_INFO[2];
    const wchar_t* holders[2][2] = {{L"Lionheart", L"Valkyrie"}, {L"Ravens", L"Ironclad"}};
    for (int i = 0; i < 2; ++i)
    {
        Switch_Info[i].m_bySwitchState = 1;
        wcsncpy_s(Switch_Info[i].m_szGuildName, holders[i][0], _TRUNCATE);
        wcsncpy_s(Switch_Info[i].m_szUserName, holders[i][1], _TRUNCATE);
    }
    LastMacroTime = GetTickCount64();
    LendMatch<CSDevilSquareMatch>();
    matchEvent::StartMatchCountDown(TYPE_MATCH_CASTLE_ENTER_CLOSE);
}

// The script and banner versions shipped under Data/InGameShopScript and Data/InGameShopBanner; the
// loader deletes a version whose files fail to load, so these must be ones that are there.
void SeedCashShop()
{
    g_InGameShopSystem->SetScriptVersion(512, 2012, 84);
    g_InGameShopSystem->ScriptDownload();
    g_InGameShopSystem->SetBannerVersion(583, 2011, 1);
    if (g_InGameShopSystem->BannerDownload())
        g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
    g_pInGameShop->OpeningProcess();

    // Two pages of storage: W Coin, items and a gift, the rows the server's list would send.
    g_pInGameShop->InitStorage(12, 9, 2, 1);
    for (int i = 0; i < 9; ++i)
    {
        if (i % 3 == 0)
            g_pInGameShop->AddStorageItem(100 + i, i, 1, 0, 0, 500 * (i + 1), L'C');
        else
            g_pInGameShop->AddStorageItem(100 + i, i, 1, 3, 0, 0, L'P');
    }
}

void Seed(Event event)
{
    switch (event)
    {
    case Event::BloodCastle: SeedBloodCastle(); break;
    case Event::ChaosCastle: SeedChaosCastle(); break;
    case Event::Temple: SeedTemple(); break;
    case Event::TempleResult: SeedTempleResult(); break;
    case Event::DuelSpectators: SeedDuelSpectators(); break;
    case Event::DuelWatch: SeedDuelWatch(); break;
    case Event::CryWolf: SeedCryWolf(); break;
    case Event::CryWolfResult: SeedCryWolfResult(); break;
    case Event::Siege: SeedSiege(); break;
    case Event::CashShop: SeedCashShop(); break;
    case Event::HudStatus: SeedHudStatus(); break;
    case Event::BloodCastleResult: SeedBloodCastleResult(); break;
    case Event::ChaosCastleResult: SeedChaosCastleResult(); break;
    case Event::DevilSquareRank: SeedDevilSquareRank(); break;
    case Event::GuildWar: SeedGuildWar(); break;
    case Event::KanturuEntry:
        g_pKanturu2ndEnterNpc->ReceiveKanturu3rdInfo(UI::Kanturu::Stage::Tower,
                                                     UI::Kanturu::Detail::TowerRevitalization, true, 0, 7200);
        break;
    case Event::CrownSwitchBox: UI::Siege::ShowCrownNotice(UI::Siege::CrownNotice::SwitchActivatedByOther, L"Ironclad", L"Ravens", 30000); break;
    default: break;
    }
}

// Back to what the window holds off its event. A timer keeps its values: the event overwrites them.
void Reset(Event event)
{
    switch (event)
    {
    case Event::Temple: g_pCursedTempleWindow->ResetCursedTempleSystemInfo(); break;
    case Event::TempleResult: g_pCursedTempleResultWindow->ResetGameResultInfo(); break;
    case Event::DuelSpectators: g_DuelMgr.RemoveAllDuelWatchUser(); break;
    case Event::DuelWatch: ResetDuelWatch(); break;
    case Event::CryWolf:
    case Event::CryWolfResult:
        M34CryWolf1st::CryWolfMVPInit();
        Suc_Or_Fail = -1;
        break;
    case Event::Siege:
        battleCastle::SetBattleCastleStart(false);
        UI::Siege::ResetMiniMap();
        break;
    case Event::CashShop: g_pInGameShop->ClearAllStorageItem(); break;
    case Event::HudStatus:
        Delete_Switch();
        LastMacroTime = 0;
        matchEvent::StartMatchCountDown(TYPE_MATCH_NONE);
        ReturnMatch();
        break;
    case Event::GuildWar:
        g_wtMatchResult.Clear();
        g_wtMatchTimeLeft.m_Time = 0;
        break;
    case Event::BloodCastleResult:
    case Event::ChaosCastleResult:
    case Event::DevilSquareRank:
    case Event::CrownSwitchBox:
        g_MessageBox->PopAllMessageBoxes();
        ReturnMatch();
        break;
    default: break;
    }
}

const Entry* Find(Event event)
{
    for (const Entry& entry : kEntries)
    {
        if (entry.event == event)
            return &entry;
    }
    return nullptr;
}

void List()
{
    Log(L"$preview <event>, or $preview off. Sample data, drawn off the event's map:");
    for (const Entry& entry : kEntries)
        Log(std::wstring(entry.name) + L" - " + std::wstring(entry.what));
}
} // namespace

bool IsShowing(Event event)
{
    return event != Event::None && s_Showing == event;
}

void Stop()
{
    const Entry* entry = Find(s_Showing);
    if (entry == nullptr)
        return;
    // Hidden while still previewed, so its close sends nothing.
    SetWindowShown(entry->window, false);
    Reset(entry->event);
    s_Showing = Event::None;
}

void HandleCommand(const std::wstring& argument)
{
    if (argument.empty() || argument == L"list")
    {
        List();
        return;
    }
    if (argument == L"off")
    {
        Stop();
        return;
    }
    for (const Entry& entry : kEntries)
    {
        if (argument != entry.name)
            continue;
        Stop();
        s_Showing = entry.event;
        Seed(entry.event);
        SetWindowShown(entry.window, true);
        Log(L"preview: " + std::wstring(entry.what) + L". $preview off ends it.");
        return;
    }
    Log(L"no such preview: " + argument, mu::ui::window::TYPE_ERROR_MESSAGE);
}
} // namespace UI::EventPreview
