#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/LightMood.h"
#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Look/LookFileSystem.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The look profile (Data/Render/Look.json, the light mood's values included) as one tuning
// surface: the profile as read, edited by name through its edit operations
// (Data/GameData/MapData/LookProfileEdits.h), and written back. Every edit is checked before it
// applies, so the live values are always values the file could hold. Names: the profile's
// (Data::Maps::LookValueInfos).
namespace Render::Look
{

struct LookFiles
{
    // False while the file is invalid or unreadable: its values are then built-in and cannot be
    // edited or saved.
    bool lookUsable = true;
    // Why the profile cannot be saved although it is usable: a format 1 file whose light mood
    // (Data/Maps/LightMood.json) is missing or invalid. Saving would write the Classic mood in
    // its place.
    std::optional<std::string> saveBlocked;
    Data::Maps::LookProfileData lookData;
};

struct LookFilesLoad
{
    LookFiles files;
    std::vector<Data::Items::ItemDataIssue> issues;
};

// Reads the profile the way the client starts: a missing file gives the built-in values (and an
// editable empty profile); an invalid one gives the built-in values with errors in `issues` and
// cannot be edited. A format 1 file takes its light mood from `legacyMoodFile` (an error that
// blocks saving when that is missing or invalid); beside a format 2 file, `legacyMoodFile` is
// only reported as no longer read.
[[nodiscard]] LookFilesLoad LoadLookFiles(LookFileSystem& files, const std::filesystem::path& lookFile,
                                          const std::filesystem::path& legacyMoodFile);

// A candidate: `lookFile` must exist and be valid. A format 1 candidate takes its light mood from
// `candidateMoodFile`, which must then be valid, or without one the moods of the shipped profile
// (`shippedLookFile`, read again), to look at only: it cannot be saved (`saveBlocked`). Nothing
// when the candidate is unusable; `issues` says why.
[[nodiscard]] std::optional<LookFiles> LoadCandidateFiles(LookFileSystem& files, const std::filesystem::path& lookFile,
                                                          const std::filesystem::path& candidateMoodFile,
                                                          const std::filesystem::path& shippedLookFile,
                                                          const std::filesystem::path& shippedMoodFile,
                                                          std::vector<Data::Items::ItemDataIssue>& issues);

// Where an effective value comes from.
enum class LookValueSource : std::uint8_t
{
    BuiltIn,
    Default,
    Environment,
    Map,
    // The player's own value ([EnhancedTuning] of config.ini), never part of the files.
    Personal,
};
[[nodiscard]] const char* ToString(LookValueSource source);

struct LookValueState
{
    std::string name;
    double value = 0.0;
    LookValueSource source = LookValueSource::BuiltIn;
    double min = 0.0;
    double max = 0.0;
};

// Every name (or only `name`) with its effective value on `world` and where it comes from. Empty
// for an unknown name.
[[nodiscard]] std::vector<LookValueState> DescribeLookValues(const LookFiles& files, int world,
                                                             std::optional<std::string_view> name = std::nullopt);
// Every name the profile has, in display order.
[[nodiscard]] std::vector<std::string> LookValueNames();

struct LookEdit
{
    std::string name;
    double value = 0.0;
};

struct LookEditResult
{
    bool applied = false;
    // Why the edit was refused, naming the value (and the other value of a broken rule).
    std::string error;
    // The effective values before, per edit, on the edited scope's world.
    std::vector<double> oldValues;
    bool moodChanged = false;
    // The maps of the entry a map edit or reset changed: they all get it.
    std::vector<int> entryMaps;
};

// Sets the edits in the default (`world` empty) or in the entry that has `world` (created when
// missing, with `mapName` as its name), all or none. Refused for an unknown name, a value out of
// range, a broken rule between values (mood figures below base) or a file that cannot be edited.
[[nodiscard]] LookEditResult SetLookValues(LookFiles& files, std::span<const LookEdit> edits, std::optional<int> world,
                                           std::string_view mapName = {});

// Back to `loaded`: every value, or one name in the default and in the entry that has `world`.
[[nodiscard]] LookEditResult ResetLookValues(LookFiles& files, const LookFiles& loaded,
                                             std::optional<std::string_view> name, int world);

struct LookSaveResult
{
    bool saved = false;
    std::string message;
};

// Writes the profile over `lookFile` (Data::Maps::WriteLookProfileJson): to `<file>.tmp`, read
// back, the original copied to `<file>.bak`, then the temporary renamed into place; the backup goes
// once it is. A failure leaves the file as it was. Refused while the profile is unusable or
// `saveBlocked`.
[[nodiscard]] LookSaveResult SaveLookFiles(LookFileSystem& files, const LookFiles& state,
                                           const std::filesystem::path& lookFile);

} // namespace Render::Look
