#pragma once

#include "Data/GameData/MonsterData/UnknownMonsterLook.h"

#include <cstdint>

class CHARACTER;

// The monsters the game cannot show as themselves (monsters design, D64,
// Q-MN42e): a number the monster list has no entry for, an entry that names
// no model, a model whose file is missing. They get the unknown monsters' look
// of the monster list (Data/Monsters/Monsters.json, unknownMonsters) instead
// of the Bull Fighter's body, so that the missing piece shows in the game.
namespace GameLogic::Monsters::Unknown
{
// Makes the character of a monster the game cannot show as itself, for the
// reason, at the tile with the key: the look's model and scale when the look
// is for the reason, else the Bull Fighter's body (D44). CreateMonster calls
// it. Writes one log line for each number it makes, with the look or not.
CHARACTER* Create(int number, Data::Monsters::UnknownReason reason, int positionX, int positionY, int key);

// The slot gets a new character (CreateCharacter): it shows the look only
// when Create gives it again.
void Forget(const CHARACTER& character);

// Shows the look on a character that has it, in each frame it is on screen:
// a glow of the look's colour that pulses, the red X marks rising from it
// (Data/Effect/UnknownMonsterMark.OZT), and "I am a bug: {number}" when it
// first is on screen and again every messageSeconds. MoveCharacterVisual
// calls it.
void ShowOnScreen(CHARACTER& character);

// The clock of the glow, the marks and the messages in milliseconds:
// timeGetTime. Tests set their own; nullptr goes back to timeGetTime.
void SetClock(std::uint32_t (*clock)());

// Where the character's message goes: its chat bubble. Tests set their own;
// nullptr goes back to the chat bubble.
void SetSpeaker(void (*say)(CHARACTER& character, const wchar_t* text));
} // namespace GameLogic::Monsters::Unknown
