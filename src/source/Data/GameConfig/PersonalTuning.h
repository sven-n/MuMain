#pragma once

#include "Data/GameData/MapData/LookProfile.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The player's own Enhanced tuning: the [EnhancedTuning] section of config.ini.
//
//   [EnhancedTuning]
//   dynamic.effectGain=1.2
//   bloom.strength=0.5
//   mood.strengthPercent=70
//
// A key is a look value's name (Data::Maps::LookValueInfos) or mood.strengthPercent, the light
// mood's strength from 0 to 100. Only the values the player changed are written; a missing key
// follows the look profile (Data/Render/Look.json) and the current map. A key that is unknown, not
// a number or out of its range is ignored on its own: the other keys still apply. The section is
// written as a whole into a temporary file that replaces config.ini only once it is complete, so a
// failed save leaves the previous file as it was.
namespace Data::Config
{

inline constexpr std::string_view PersonalTuningSection = "EnhancedTuning";
inline constexpr std::string_view MoodStrengthKey = "mood.strengthPercent";
inline constexpr int MinMoodStrengthPercent = 0;
inline constexpr int MaxMoodStrengthPercent = 100;
// At most this many problems of the section are reported; the rest are counted in the last one.
inline constexpr std::size_t MaxPersonalTuningIssues = 8;

struct PersonalTuning
{
    Data::Maps::LookOverrides look;
    std::optional<int> moodStrengthPercent;

    [[nodiscard]] bool Empty() const;
    bool operator==(const PersonalTuning&) const = default;
};

// The value as config.ini holds it, and the number read back from that text: what a session
// applies is exactly what a restart reads.
[[nodiscard]] std::string FormatTuningNumber(double value);
[[nodiscard]] double NormalizeTuningNumber(double value);

// The [EnhancedTuning] entries of a config.ini text (any case of the section name). `issues` gets
// one line per ignored key, at most MaxPersonalTuningIssues.
[[nodiscard]] PersonalTuning ReadPersonalTuning(std::string_view configText, std::vector<std::string>& issues);

// `configText` with its [EnhancedTuning] section replaced by `tuning` (removed when it is empty,
// appended when there was none). Every other line stays as it was.
[[nodiscard]] std::string ReplacePersonalTuningSection(std::string_view configText, const PersonalTuning& tuning);

// Reads the section of `configFile`; a missing file has none.
[[nodiscard]] PersonalTuning LoadPersonalTuningFile(const std::filesystem::path& configFile,
                                                    std::vector<std::string>& issues);

struct PersonalTuningSaveResult
{
    bool saved = false;
    // Why nothing was saved; the file is then as it was before.
    std::string error;
};

// Writes the section into `configFile` through `<configFile>.tmp`, read back before it replaces
// the file.
[[nodiscard]] PersonalTuningSaveResult SavePersonalTuningFile(const std::filesystem::path& configFile,
                                                              const PersonalTuning& tuning);

} // namespace Data::Config
