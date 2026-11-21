#pragma once

#include <cstdint>

// Game options kept by the options window.
namespace UI::Options
{
// Options the server saved for this character.
void ApplySaved(bool autoAttack, bool whisperSound, bool slideHelp);
// The chat box size and transparency the server saved for this character (the key configuration's chat box byte,
// see GameLogic/KeyConfiguration/ChatBoxConfiguration.h); a value the byte doesn't store stays as it is. True when
// both were restored.
bool ApplySavedChatBox(std::uint8_t chatBox);
// The options of the key configuration at the values a character starts with: auto attack on, whisper sound off,
// slide help on, and the chat box at its default size and transparency. The options window and the chat box live
// across character switches, so without this a character with no stored configuration (a new one) would keep and
// save the one before's.
void ResetForNewCharacter();
bool IsWhisperSoundOn();
}
