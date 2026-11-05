#pragma once

#ifdef _EDITOR

#include "Audio/SoundSource.h"
#include "Data/GameData/MapData/MapObjectSound.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The rows of the Sound Editor (sound editor design, Q-SE1): every sound the
// game loads, from the three files that name them.
namespace MuEditor::SoundEditor
{
// Where a sound lives.
enum class RowSource
{
    // Data/Audio/SoundFiles.json: a sound of the code, by its SOUND_ id.
    Code,
    // Data/Monsters/Sounds.json: a sound of a monster sound set.
    MonsterSet,
    // Data/Maps/ObjectSounds.json: a map object sound.
    MapObject,
    // A SOUND_ id the game played that nothing loads (a problem row).
    NotLoaded,
};

// What is wrong with a row ("Only problems").
enum class RowProblem
{
    None,
    // Its file is not in the client folder (ClientSoundFiles, read once and
    // after RefreshClientSoundFiles): it is silent.
    MissingFile,
    // Another sound of the code loads its file too.
    SharedFile,
    // The game played it, but nothing loaded it.
    PlayedNotLoaded,
};

struct SoundRow
{
    RowSource source = RowSource::Code;
    // Unique among the rows, and what a folder names the sound by (Q-SE4):
    // "SOUND_CLICK01", "set:Death Tree:Data/Sound/w34/dt_idle1.wav" (a set's
    // sound by its id, else its file),
    // "objects:Data/Sound/aKalimaWaterFall.wav"; a second row with the same
    // one (two map object sounds of one file) gets "#2" after it, and so on.
    std::string key;
    // The SOUND_ id, or "" (a set's sound without one, a map object sound).
    std::string id;
    // The set's name, or "".
    std::string set;
    // Its place in the set's sounds or in the object sounds.
    size_t index = 0;
    // The name the data gives it; "" for none.
    std::string name;
    std::string file;
    int channels = 1;
    bool spatial = false;
    // A code sound's stored type where no source or place names one (Q-SE2).
    std::optional<Audio::SoundKind> type;
    // The sound number the game plays it as; -1 for none.
    int number = -1;
    // When it loads, as the session saw it (code sounds), or what loads it.
    std::string loads;
    // The data file it lives in ("Data/Audio/SoundFiles.json").
    std::string livesIn;
    RowProblem problem = RowProblem::None;
};

// Every row: the code's sounds in their file's order, then the sets' (by set
// name), the map object sounds and the sounds played that nothing loads.
std::vector<SoundRow> BuildRows();

// The rows of these sets' sounds (in the sets' order) and map object sounds,
// with only their source, set, index and key, keyed as BuildRows keys them:
// what the folders name them by (SoundKeyFollow).
std::vector<SoundRow> KeyRows(const std::vector<Data::Monsters::MonsterSoundSet>& sets,
                              const std::vector<Data::Maps::MapObjectSound>& objects);

// The text the editor shows for the row (Q-SE1): its name, else its id,
// else "<set>: <file without folder>", else its file.
std::string ShownName(const SoundRow& row);

// The second, grey text after the name: the id, or the set; "" for a row
// that shows it already.
std::string IdText(const SoundRow& row);

// Whether the row's Play button plays something: it has a file, and the file
// is in the client folder.
bool CanPlay(const SoundRow& row);

// Whether the row's name, id, set or file contains `text`, whatever the case;
// always for "".
bool Matches(const SoundRow& row, std::string_view text);

// "Data/Sound/a.wav" as LoadWaveFile takes it: "Data\Sound\a.wav".

// The name the editor shows for a sound number: the name of the sound the
// data gives it, else "" (the mixer and the overlay then show the file).
std::string NameOfNumber(int number);
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
