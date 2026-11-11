#pragma once

#include "Core/Globals/_enum.h"

#include <cstdint>
#include <span>
#include <string>

// Cursed Temple state the server reports, applied to the entry, match and result windows. Spans are
// borrowed for the call.
namespace UI::CursedTemple
{
struct PartyPosition
{
    std::uint16_t userIndex;
    std::uint8_t mapNumber;
    std::uint8_t x;
    std::uint8_t y;
};

struct MatchStatus
{
    std::uint16_t remainingSeconds;
    // 0xffff while nobody carries the relic.
    std::uint16_t relicHolderIndex;
    std::uint8_t relicX;
    std::uint8_t relicY;
    std::uint8_t alliedPoints;
    std::uint8_t illusionPoints;
    SEASON3A::eCursedTempleTeam localTeam;
    std::span<const PartyPosition> party;
};

struct SkillResult
{
    std::uint16_t skill;
    std::uint16_t sourceKey;
    std::uint16_t targetKey;
    bool succeeded;
};

struct PlayerResult
{
    std::wstring name;
    std::uint8_t mapNumber;
    SEASON3A::eCursedTempleTeam team;
    CLASS_TYPE playerClass;
    int addedExperience;
};

struct MatchResult
{
    std::uint8_t alliedPoints;
    std::uint8_t illusionPoints;
    std::span<const PlayerResult> players;
};

// The temple NPC offers entry.
void OpenEntryOffer(std::uint8_t remainingTime, std::uint8_t entryCount);
// Players waiting at each of the six temple levels.
void UpdateEntryCounts(std::span<const std::uint8_t, 6> counts);

void UpdateMatchStatus(const MatchStatus& status);
void ResolveSkill(const SkillResult& result);
void EndSkill(std::uint16_t skill, std::uint16_t targetKey);
void SetSkillPoints(std::uint8_t points);
// Closes the match window and opens the result window.
void ShowMatchResult(const MatchResult& result);
// Closes other windows and opens the match window with its tutorial.
void BeginReadyPhase();
}
