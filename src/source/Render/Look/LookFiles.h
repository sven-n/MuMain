#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
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

// The look profile (Data/Render/Look.json) and the light mood (Data/Maps/LightMood.json) as one
// tuning surface: their documents as read, edited in place by name, and written back. Every edit
// is validated by the files' own readers before it applies, so the live values are always values
// a file could hold. Names: the look profile's names (Data::Maps::LookValueInfos) and "mood.<key>"
// for the light mood's keys (Data::Maps::LightMoodValueInfos).
namespace Render::Look
{

using LookDocument = Data::Items::Json::OrderedJson;

inline constexpr std::string_view MoodNamePrefix = "mood.";

struct LookFiles
{
    // The documents; `moodUsable` is false while the light mood file is missing or invalid (its
    // values are then Classic and cannot be edited or saved).
    LookDocument look;
    LookDocument mood;
    bool lookUsable = true;
    bool moodUsable = false;
    Data::Maps::LookProfileData lookData;
    Data::Maps::LightMoodData moodData;
};

struct LookFilesLoad
{
    LookFiles files;
    std::vector<Data::Items::ItemDataIssue> issues;
};

// Reads both files the way the client starts: a missing look profile gives the built-in values
// (and an editable empty profile); an invalid file gives its built-in or Classic values with
// errors in `issues` and cannot be edited.
[[nodiscard]] LookFilesLoad LoadLookFiles(LookFileSystem& files, const std::filesystem::path& lookFile,
                                          const std::filesystem::path& moodFile);

// A candidate: `lookFile` must exist and both files must be valid; without `moodFile` the shipped
// light mood applies. Nothing when the candidate is unusable; `issues` says why.
[[nodiscard]] std::optional<LookFiles> LoadCandidateFiles(LookFileSystem& files, const std::filesystem::path& lookFile,
                                                          const std::filesystem::path& candidateMoodFile,
                                                          const std::filesystem::path& shippedMoodFile,
                                                          std::vector<Data::Items::ItemDataIssue>& issues);

// Where an effective value comes from.
enum class LookValueSource : std::uint8_t
{
    BuiltIn,
    Default,
    Map,
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
// Every name the profile and the light mood have, in display order.
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
    // The effective values before and after, per edit, on the edited scope's world.
    std::vector<double> oldValues;
    bool moodChanged = false;
};

// Sets the edits in the default (`world` empty) or in `world`'s entry (created when missing, with
// `mapName` as its name), all or none. Refused for an unknown name, a value out of range, a broken
// rule between values (mood figures below base) or a file that cannot be edited.
[[nodiscard]] LookEditResult SetLookValues(LookFiles& files, std::span<const LookEdit> edits, std::optional<int> world,
                                           std::string_view mapName = {});

// Back to `loaded`: every value, or one name in the default and in `world`'s entry.
[[nodiscard]] LookEditResult ResetLookValues(LookFiles& files, const LookFiles& loaded,
                                             std::optional<std::string_view> name, int world);

// A profile document as the shipped files lay it out: the default one value per line, each map
// entry on one line.
[[nodiscard]] std::string FormatLookDocument(const LookDocument& document);

struct LookSaveResult
{
    bool saved = false;
    // The files are mixed: a restore failed, the backups are kept.
    bool backupsKept = false;
    std::string message;
};

// Writes the usable documents over `lookFile` and `moodFile`: each is written to `<file>.tmp` and
// read back, each original copied to `<file>.bak`, then the temporaries are renamed into place
// one after the other. A failure before the first rename removes the temporaries and backups and
// leaves both files as they were; a failed second rename restores the first file from its backup.
// Only a failed restore leaves the files mixed; the backups are then kept and named.
[[nodiscard]] LookSaveResult SaveLookFiles(LookFileSystem& files, const LookFiles& state,
                                           const std::filesystem::path& lookFile,
                                           const std::filesystem::path& moodFile);

} // namespace Render::Look
