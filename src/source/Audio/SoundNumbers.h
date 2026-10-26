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
// The number past the ones files get (MAX_BUFFER on), which loads and plays
// nothing.
constexpr ESound NoNumber = static_cast<ESound>(static_cast<int>(MAX_BUFFER) + MAX_FILE_SOUNDS);

// The number of the file. When all MAX_FILE_SOUNDS numbers are taken:
// NoNumber.
ESound ForFile(std::wstring_view file);

// Releases the loaded sounds of all numbers given so far (a map change); the
// numbers stay with their files.
void ReleaseAll();
} // namespace Audio::SoundNumbers
