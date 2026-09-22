#pragma once

// CharacterScene.h - Character selection scene

#include "Core/Platform/WinCompat.h"

// Character scene lifecycle
void CreateCharacterScene();
void NewMoveCharacterScene();
bool NewRenderCharacterScene(HDC hDC);

// Character management
void StartGame();

namespace Scenes
{
// The server places each character at its own index, so a deleted character
// leaves a hole: enumerate with `CharacterNameInSlot` over every slot rather
// than over a count.
// Slot of the character with that name, or -1. Case-sensitive, as the
// server spells names.
[[nodiscard]] int FindCharacterSlot(const wchar_t* name);
// Name in that slot, or an empty string when the slot is empty.
[[nodiscard]] const wchar_t* CharacterNameInSlot(int slot);
// Level of the character in that slot, or 0.
[[nodiscard]] int CharacterLevelInSlot(int slot);

// Enters the world with the character in that slot, the way a double click
// on it does. False when the slot is empty or out of range.
bool StartGameWithSlot(int slot);
} // namespace Scenes
