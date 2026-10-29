#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The model files that replace a model's BMD (Data/Models/ModelFiles.json):
//
//   { "formatVersion": 1,
//     "models": [ { "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb",
//                   "actions": { "2": { "clip": "walk", "crossfade": 3 } } } ] }
//
//   model    the model's code name, one of GetModelFileSymbols() (the monster models);
//   file     a glTF binary, relative to Data/; it must exist;
//   actions  optional: per MU action index, the clip that plays it (default: the
//            clip "actionNN" plays action NN) and the crossfade into it in ticks
//            (default: the playback rule's prior-frame fade);
//   lod      optional: {"screenHeights": [h1, h2], "hysteresis": x}: below the
//            projected screen height h1 (a fraction of the window's height) the
//            file's LOD 1 is drawn, below h2 its LOD 2 (h1 > h2 > 0); a level
//            changes back to a finer one only above h x (1 + hysteresis)
//            (0 <= x < 1). Defaults: LodSettings.
// A model named twice, an unknown model or a missing file is an error; an
// unknown field is a warning. The model keeps its BMD for its action metadata
// and as the fallback.
//
// Developer overlay: the environment variable MU_DATA_OVERLAY names a
// directory whose Data/ is searched before the game's Data/, for the table
// itself and for every file it names. The game's files stay untouched.
namespace Data::Models
{
constexpr int ModelFilesFormatVersion = 1;

// The table, relative to a Data/ directory.
constexpr const char* ModelFilesPath = "Models/ModelFiles.json";

constexpr const char* DataOverlayVariable = "MU_DATA_OVERLAY";

// The highest action index "actions" may name (Action_t indices are shorts).
constexpr int MaxModelFileActionIndex = 32767;

struct ActionClip
{
    // Empty: the default clip "actionNN".
    std::string clip;
    // Ticks; nothing: the default fade.
    std::optional<float> crossfade;

    bool operator==(const ActionClip&) const = default;
};

// When a model file's LOD meshes are drawn (the entry's "lod").
struct LodSettings
{
    // Descending: below screenHeights[i] LOD i + 1 is drawn. At most MaxLodThresholds.
    std::vector<float> screenHeights{0.2f, 0.1f};
    float hysteresis = 0.1f;

    bool operator==(const LodSettings&) const = default;
};

// LOD 0 to 2: two thresholds.
constexpr int MaxLodThresholds = 2;

struct ModelFileEntry
{
    std::string model;
    int type = 0;
    // As written in the table, relative to Data/.
    std::string file;
    std::map<int, ActionClip> actions;
    LodSettings lod;
    // The file found by LoadModelFiles (overlay first); empty after ReadModelFilesJson.
    std::filesystem::path path;

    bool operator==(const ModelFileEntry&) const = default;
};

// The Data/ directories searched for a file, in order: the overlay's, then the game's.
struct DataDirectories
{
    std::optional<std::filesystem::path> overlay;
    std::filesystem::path game = "Data";

    // The first existing regular file `relative` (Data/-relative, "/"-separated), or nothing.
    [[nodiscard]] std::optional<std::filesystem::path> Find(std::string_view relative) const;
};

// The directories for MU_DATA_OVERLAY's value (nullptr or empty: no overlay):
// `<overlayValue>/Data` and `game`. An overlay that is no directory is left out
// with a warning in `issues`.
[[nodiscard]] DataDirectories MakeDataDirectories(const char* overlayValue, const std::filesystem::path& game,
                                                  std::vector<Items::ItemDataIssue>& issues);

// Reads the entries of the table text into `models` (file paths unchecked).
// Entries with fatal problems are left out; `issues` lists errors and warnings.
void ReadModelFilesJson(std::string_view text, const std::string& source, std::vector<ModelFileEntry>& models,
                        std::vector<Items::ItemDataIssue>& issues);

struct ModelFiles
{
    std::vector<ModelFileEntry> models;
    std::vector<Items::ItemDataIssue> issues;
    // The table that was read; empty when there is none.
    std::filesystem::path table;
    // The overlay's Data/ directory, searched before the game's; empty without one.
    std::filesystem::path overlay;

    // The entry of a model index (Models[type]), or nullptr.
    [[nodiscard]] const ModelFileEntry* Find(int type) const;
};

// Reads the first table found in `directories` (none at all: no models, no
// issue) and resolves every file it names in `directories`; an entry whose
// file is found nowhere is left out with an error.
[[nodiscard]] ModelFiles LoadModelFiles(const DataDirectories& directories);
} // namespace Data::Models
