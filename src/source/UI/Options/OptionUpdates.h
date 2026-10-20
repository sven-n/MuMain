#pragma once

// Game options kept by the options window.
namespace UI::Options
{
// Options the server saved for this character.
void ApplySaved(bool autoAttack, bool whisperSound, bool slideHelp);
// Defaults on entering the game, before the saved options arrive.
void ResetForNewGame();
bool IsWhisperSoundOn();
}
