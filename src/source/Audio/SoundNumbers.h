#pragma once

#include "Audio/DSPlaySound.h"

#include <string>
#include <string_view>

// Numbers for sounds that have no SOUND_* id (the monster models' sounds,
// monsters design Q-MN16): a sound file gets the next number after the
// SOUND_* list when it is first asked for, and keeps it for the session.
// Files are the same when the game opens the same file: without case, with
// \ and / alike.
namespace Audio::SoundNumbers
{
// The number of the file. When all MAX_FILE_SOUNDS numbers are taken:
// MAX_BUFFER, which loads and plays nothing.
ESound ForFile(std::wstring_view file);

// The file a number was given for; empty for a SOUND_* id and for a number
// not given.
std::wstring FileOf(int number);

// Releases the loaded sounds of all numbers given so far (a map change); the
// numbers stay with their files.
void ReleaseAll();
} // namespace Audio::SoundNumbers
