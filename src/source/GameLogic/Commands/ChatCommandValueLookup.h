#pragma once

#include "GameLogic/Commands/ChatCommandCatalog.h"

#include <string>
#include <vector>

// Turns what the values of chat command parameters refer to into something the
// player can use: the name behind a number, or the value of the own character.
//
// The server only sends what a value refers to - the names come from the data
// of the client, which may not match the server. That's why they are never
// more than a help: what the player entered is sent as it is.
namespace GameLogic::Commands::ValueLookup
{
// True when the own character has a value which fits the parameter, e.g. its
// position for a coordinate. Cheap enough to be asked every frame.
bool HasOwnValue(const ChatCommandParameter& parameter);

// The value of the own character which fits the parameter. Empty when there
// is none, e.g. because the character isn't in a guild.
std::wstring GetOwnValue(const ChatCommandParameter& parameter);

// The name of what the value of a parameter refers to, e.g. the map of a map
// number. Empty when it refers to nothing with a name, or to nothing known.
std::wstring DescribeValue(const ChatCommand& command, size_t parameterIndex, const std::vector<std::wstring>& values);
} // namespace GameLogic::Commands::ValueLookup
