#pragma once

#ifdef _EDITOR

#include <string>
#include <string_view>
#include <vector>

// The files the Sound Editor's picker offers (sound editor design, Q-SE1):
// the sound files of the client folder, which the editor never changes.
namespace MuEditor::SoundEditor
{
// "Data/Sound/a.wav", "Data/Sound/w39/b.wav": the .wav, .ogg and .mp3 files
// below Data/Sound of the working folder, sorted without case; read the
// first time and after RefreshClientSoundFiles.
const std::vector<std::string>& ClientSoundFiles();
void RefreshClientSoundFiles();

// The files below `folder` (as the game names them, "Data/Sound/..."),
// sorted without case.
std::vector<std::string> ListSoundFiles(const std::string& folder);

// The files that contain `text`, whatever the case; all for "".
std::vector<std::string> FilterFiles(const std::vector<std::string>& files, std::string_view text);

// Plays the file once at full level before it is picked, under its file's
// number (Audio/SoundNumbers.h), loaded first with the channels and 3D.
void PreviewFile(const std::string& file, int channels, bool spatial);
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
