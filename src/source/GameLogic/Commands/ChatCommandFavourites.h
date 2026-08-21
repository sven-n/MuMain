#pragma once

#include <string>

// The chat commands the player marked as a favourite.
//
// They are shown at the top of the command list, so that the few commands
// somebody actually uses don't have to be looked for in everything the server
// offers. They belong to the installation, not to a character.
namespace GameLogic::Commands::Favourites
{
bool Contains(const std::wstring& command);

// Marks the command when it isn't marked yet and unmarks it otherwise.
// Returns whether it is a favourite afterwards.
bool Toggle(const std::wstring& command);
} // namespace GameLogic::Commands::Favourites
