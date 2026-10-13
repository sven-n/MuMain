#pragma once

#include <cstdint>

namespace UI::Kanturu
{
enum class Stage : std::uint8_t
{
    None = 0,
    Standby = 1,
    MayaBattle = 2,
    NightmareBattle = 3,
    Tower = 4,
    End = 5,
};

enum class Detail : std::uint8_t
{
    None = 0,
    StandbyStart = 1,
    TowerRevitalization = 1,
    TowerNotify = 2,
    MayaStandby1 = 1,
    MayaNotify = 2,
    MayaMonster1 = 3,
    Maya1 = 4,
    MayaEnd1 = 5,
    MayaEndCycle1 = 6,
    MayaStandby2 = 7,
    MayaMonster2 = 8,
    Maya2 = 9,
    MayaEnd2 = 10,
    MayaEndCycle2 = 11,
    MayaStandby3 = 12,
    MayaMonster3 = 13,
    Maya3 = 14,
    MayaEnd3 = 15,
    MayaEndCycle3 = 16,
    MayaEnd = 17,
    MayaEndCycle = 18,
    NightmareBattle = 3,
};

enum class EntryResult : std::uint8_t
{
    None = 0,
    UserLimit = 1,
    MissingMoonstone = 2,
    Failed = 3,
    FailedAgain = 4,
    RidingUnicorn = 5,
    TransformationRing = 6,
    MissingHelper = 7,
};

void ShowEntryInfo(Stage stage, Detail detail, bool canEnter, std::uint8_t userCount, int remainingSeconds);
void CompleteEntry(EntryResult result);
void SetBattleInfoVisible(bool visible);
void SetBattleTime(std::uint8_t timeLimit);
}
