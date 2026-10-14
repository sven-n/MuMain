#pragma once

#include <string_view>

// Login and character-select screen changes the server reports. Strings are borrowed for the call.
namespace UI::LoginScene
{
// Shows the server picker once the list has arrived, unless the credits are playing.
void ServerListReceived();
void ShowLoginWindow();
void HideLoginWindow();

// `messageCode` is one of the scene's popup message ids (MESSAGE_* or RECEIVE_*_FAIL*).
void ShowMessage(int messageCode);
void CloseMessage();
void AddServerMessage(std::wstring_view text);

// Refreshes the character list after a character was created.
void CharacterCreated();
}
