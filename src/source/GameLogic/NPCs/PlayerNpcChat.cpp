#include "stdafx.h"

#include "GameLogic/NPCs/PlayerNpcChat.h"

#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"

#include <cstdlib>
#include <span>

namespace GameLogic::Npcs::PlayerNpcChat
{
namespace
{
const wchar_t* const* const kDeviasLines[] = {
    &I18N::Game::RegisterTheLuckyNumberOnThe100WinningCard,
    &I18N::Game::PleaseCheckOutTheAnnouncementOn,
};
const wchar_t* const* const kLorenciaLines[] = {
    &I18N::Game::CollectingStonesPleaseGiveMeStonesThatYouVeAcquired,
};

// A world's lines and the one said last there; it starts as the first line.
struct WorldLines
{
    std::span<const wchar_t* const* const> lines;
    const wchar_t* const* lastLine;
};

WorldLines g_deviasLines{kDeviasLines, kDeviasLines[0]};
WorldLines g_lorenciaLines{kLorenciaLines, kLorenciaLines[0]};

WorldLines* FindWorldLines(int world)
{
    if (world == WD_2DEVIAS)
        return &g_deviasLines;
    if (world == WD_0LORENCIA)
        return &g_lorenciaLines;
    return nullptr;
}
} // namespace

const wchar_t* LineAtStop(int world, bool actionChanged)
{
    WorldLines* worldLines = FindWorldLines(world);
    if (worldLines == nullptr)
        return nullptr;

    if (actionChanged)
        worldLines->lastLine = worldLines->lines[rand() % worldLines->lines.size()];
    return *worldLines->lastLine;
}
} // namespace GameLogic::Npcs::PlayerNpcChat
