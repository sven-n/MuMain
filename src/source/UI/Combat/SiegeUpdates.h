#pragma once

#include "UI/Combat/CastleSiegePhase.h"

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace UI::Siege
{
struct MapLocation
{
    std::uint8_t type;
    int x;
    int y;
};

struct SiegeDate
{
    std::uint16_t year;
    std::uint8_t month;
    std::uint8_t day;
    std::uint8_t hour;
    std::uint8_t minute;
};

struct GuardStatus
{
    CASTLESIEGE_STATE phase;
    std::wstring_view ownerGuild;
    std::wstring_view ownerGuildMaster;
    SiegeDate registrationStart;
    SiegeDate registrationEnd;
    SiegeDate battleStart;
    std::uint32_t secondsRemaining;
};

struct DeclarationGuild
{
    std::wstring name;
    int markCount;
    bool gaveUp;
    std::uint8_t sequence;
};

struct AttackingGuild
{
    std::wstring name;
    std::uint8_t side;
    std::uint8_t involvement;
    int score;
};

enum class CrownNotice
{
    SwitchReleased,
    SwitchActivated,
    SwitchActivatedByOther,
    RegistrationStarted,
    RegistrationSucceeded,
    RegistrationFailed,
    RegistrationByOther,
    RegistrationByOtherCamp,
    DefenseRemoved,
    DefenseActivated,
};

void ResetMiniMap();
void ShowMiniMap();
void EnsureLocalPlayerMiniMap();
void SetBattleSkillsActive(bool active);
void SetCommanderMapInfo(std::uint8_t team, std::uint8_t x, std::uint8_t y, std::uint8_t command);
void ReplaceMemberLocations(std::span<const MapLocation> locations);
void AddNpcLocations(std::span<const MapLocation> locations);
void SetMatchTime(std::uint8_t hour, std::uint8_t minute);
void ShowGuardStatus(const GuardStatus& status);
void ReplaceDeclarations(std::span<const DeclarationGuild> guilds);
void ReplaceAttackingGuilds(std::span<const AttackingGuild> guilds);
void ClearCrownNotices();
void ShowCrownNotice(CrownNotice notice, std::wstring_view actor = {},
                     std::wstring_view switchOwner = {}, std::uint32_t accessTimeMs = 0);

void OpenHuntZone(std::uint8_t type, bool enabled, int currentPrice, int unitPrice, int maxPrice);
void SetHuntZoneEntranceFee(int fee);
void SetHuntZonePublic(std::uint8_t enabled);
void OpenCatapult(int key, std::uint8_t weaponType);
void CatapultFired(int key, std::uint8_t result, std::uint8_t weaponType, int targetX, int targetY);
void CatapultFiredAtPlayer(std::uint8_t weaponType, int targetX, int targetY);
}
