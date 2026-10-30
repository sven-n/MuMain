#pragma once

#include <cstdint>

// The lines the server shows by number with its server command packet (ReceiveServerCommand): the
// Devil Square and Blood Castle NPCs' lines, the Golden Archer's and the Rena hints. A number past
// a family's lines shows no text.
namespace Network::ServerMessages
{
// Command 1: a Devil Square NPC line for the numbers below 20, a Blood Castle one from 20 on.
const wchar_t* GetEventNpcLine(std::uint8_t number);

// Command 3: a line of the Golden Archer's Rena story and event.
const wchar_t* GetGoldenArcherLine(std::uint8_t number);

// Command 4: a hint that Rena can be exchanged for Zen.
const wchar_t* GetRenaLine(std::uint8_t number);
} // namespace Network::ServerMessages
