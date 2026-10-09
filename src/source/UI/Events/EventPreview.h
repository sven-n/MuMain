#pragma once

#include <string>

// Developer previews of the windows that only draw while a server drives them (the event windows,
// the cash shop): `$preview <event>` fills a window with sample values through the setters the
// server's packets use and lets it draw off its map; `$preview off` resets that state and hides it again. A previewed
// window sends nothing to the server.
namespace UI::EventPreview
{
enum class Event
{
    None,
    BloodCastle,
    ChaosCastle,
    Temple,
    TempleResult,
    DuelSpectators,
    DuelWatch,
    CryWolf,
    CryWolfResult,
    Siege,
    CashShop,
    HudStatus,
    BloodCastleResult,
    ChaosCastleResult,
    DevilSquareRank,
    CrownSwitchBox,
    GuildWar,
};

// True while `event` is previewed: its window may draw off its map, and its close sends nothing.
bool IsShowing(Event event);

// "$preview <event>", "$preview off", or "$preview" for the list; replies in the system log.
void HandleCommand(const std::wstring& argument);

// Ends the preview in progress, if any.
void Stop();
} // namespace UI::EventPreview
