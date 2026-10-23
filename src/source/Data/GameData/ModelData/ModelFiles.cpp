#include "stdafx.h"

#include "ModelFiles.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ModelData/ModelFileSymbols.h"

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

#include <algorithm>
#include <cmath>
#include <set>
#include <system_error>

namespace Data::Models
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Models = "models";
constexpr const char* Model = "model";
constexpr const char* File = "file";
constexpr const char* Actions = "actions";
constexpr const char* Clip = "clip";
constexpr const char* Crossfade = "crossfade";
constexpr const char* Lod = "lod";
constexpr const char* ScreenHeights = "screenHeights";
constexpr const char* Hysteresis = "hysteresis";
} // namespace Keys

constexpr const char* UnknownField = "unknown field, ignored";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

void AddError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    AddIssue(issues, ItemDataIssueSeverity::Error, source, field, message);
}

void AddWarning(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
                const std::string& message)
{
    AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message);
}

// A Data/-relative path: not empty, not absolute, no "..", "/" as separator.
bool IsDataRelativePath(std::string_view file)
{
    if (file.empty() || file.front() == '/' || file.find('\\') != std::string_view::npos ||
        file.find(':') != std::string_view::npos)
    {
        return false;
    }
    size_t start = 0;
    while (start <= file.size())
    {
        const size_t end = std::min(file.find('/', start), file.size());
        const std::string_view part = file.substr(start, end - start);
        if (part.empty() || part == "..")
        {
            return false;
        }
        start = end + 1;
    }
    return true;
}

// "<digits>" naming an action index, without sign or leading zeros.
bool ReadActionIndex(const std::string& key, int& index)
{
    if (key.empty() || key.size() > 5 || (key.size() > 1 && key.front() == '0') ||
        !std::all_of(key.begin(), key.end(), [](char c) { return c >= '0' && c <= '9'; }))
    {
        return false;
    }
    index = std::stoi(key);
    return index <= MaxModelFileActionIndex;
}

// Reads "actions" of an entry; false on an error.
bool ReadActions(const OrderedJson& json, const std::string& field, const std::string& source, ModelFileEntry& entry,
                 std::vector<ItemDataIssue>& issues)
{
    const auto actions = json.find(Keys::Actions);
    if (actions == json.end())
    {
        return true;
    }
    const std::string actionsField = field + "." + Keys::Actions;
    if (!actions->is_object())
    {
        AddError(issues, source, actionsField, "must be an object of action indices");
        return false;
    }
    bool valid = true;
    for (const auto& [key, value] : actions->items())
    {
        const std::string actionField = actionsField + "." + key;
        int index = 0;
        if (!ReadActionIndex(key, index))
        {
            AddError(issues, source, actionField,
                     "is no action index (0 to " + std::to_string(MaxModelFileActionIndex) + ")");
            valid = false;
            continue;
        }
        if (!value.is_object())
        {
            AddError(issues, source, actionField, "must be an object with a clip and/or a crossfade");
            valid = false;
            continue;
        }
        ActionClip clip;
        bool hasValue = false;
        if (const auto name = value.find(Keys::Clip); name != value.end())
        {
            if (!name->is_string() || name->get<std::string>().empty())
            {
                AddError(issues, source, actionField + "." + Keys::Clip, "must be a clip name");
                valid = false;
            }
            else
            {
                clip.clip = name->get<std::string>();
                hasValue = true;
            }
        }
        if (const auto crossfade = value.find(Keys::Crossfade); crossfade != value.end())
        {
            if (!crossfade->is_number() || !std::isfinite(crossfade->get<double>()) || crossfade->get<double>() < 0.0)
            {
                AddError(issues, source, actionField + "." + Keys::Crossfade, "must be a number of ticks, 0 or more");
                valid = false;
            }
            else
            {
                clip.crossfade = crossfade->get<float>();
                hasValue = true;
            }
        }
        for (const auto& [actionKey, actionValue] : value.items())
        {
            if (actionKey != Keys::Clip && actionKey != Keys::Crossfade)
            {
                AddWarning(issues, source, actionField + "." + actionKey, UnknownField);
            }
        }
        if (!hasValue)
        {
            AddWarning(issues, source, actionField, "names neither a clip nor a crossfade, ignored");
            continue;
        }
        entry.actions[index] = std::move(clip);
    }
    return valid;
}

// Reads "screenHeights" of an entry's "lod" into `lod`; false on an error.
bool ReadLodScreenHeights(const OrderedJson& heights, const std::string& field, const std::string& source,
                          LodSettings& lod, std::vector<ItemDataIssue>& issues)
{
    const std::string heightsField = field + "." + Keys::ScreenHeights;
    if (!heights.is_array() || heights.size() > static_cast<size_t>(MaxLodThresholds))
    {
        AddError(issues, source, heightsField,
                 "must be a list of at most " + std::to_string(MaxLodThresholds) + " screen heights");
        return false;
    }
    std::vector<float> values;
    for (const auto& height : heights)
    {
        if (!height.is_number() || !std::isfinite(height.get<double>()) || height.get<double>() <= 0.0 ||
            (!values.empty() && height.get<float>() >= values.back()))
        {
            AddError(issues, source, heightsField, "must be positive numbers, each below the one before");
            return false;
        }
        values.push_back(height.get<float>());
    }
    lod.screenHeights = std::move(values);
    return true;
}

// Reads "lod" of an entry; false on an error.
bool ReadLod(const OrderedJson& json, const std::string& field, const std::string& source, ModelFileEntry& entry,
             std::vector<ItemDataIssue>& issues)
{
    const auto lod = json.find(Keys::Lod);
    if (lod == json.end())
    {
        return true;
    }
    const std::string lodField = field + "." + Keys::Lod;
    if (!lod->is_object())
    {
        AddError(issues, source, lodField, "must be an object with screenHeights and/or a hysteresis");
        return false;
    }
    LodSettings settings;
    bool valid = true;
    if (const auto heights = lod->find(Keys::ScreenHeights); heights != lod->end())
    {
        valid = ReadLodScreenHeights(*heights, lodField, source, settings, issues) && valid;
    }
    if (const auto hysteresis = lod->find(Keys::Hysteresis); hysteresis != lod->end())
    {
        if (!hysteresis->is_number() || !std::isfinite(hysteresis->get<double>()) || hysteresis->get<double>() < 0.0 ||
            hysteresis->get<double>() >= 1.0)
        {
            AddError(issues, source, lodField + "." + Keys::Hysteresis, "must be a number from 0 to below 1");
            valid = false;
        }
        else
        {
            settings.hysteresis = hysteresis->get<float>();
        }
    }
    for (const auto& [key, value] : lod->items())
    {
        if (key != Keys::ScreenHeights && key != Keys::Hysteresis)
        {
            AddWarning(issues, source, lodField + "." + key, UnknownField);
        }
    }
    if (valid)
    {
        entry.lod = std::move(settings);
    }
    return valid;
}

// Reads entry `index` of "models"; false when it cannot be used.
bool ReadEntry(const OrderedJson& json, size_t index, const std::string& source, ModelFileEntry& entry,
               std::vector<ItemDataIssue>& issues)
{
    const std::string field = std::string(Keys::Models) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        AddError(issues, source, field, "must be an object with a model and a file");
        return false;
    }
    bool valid = true;
    const auto model = json.find(Keys::Model);
    if (model == json.end() || !model->is_string())
    {
        AddError(issues, source, field + "." + Keys::Model, "missing or not text");
        valid = false;
    }
    else
    {
        entry.model = model->get<std::string>();
        const ModelFileSymbol* symbol = FindModelFileSymbol(entry.model);
        if (!symbol)
        {
            AddError(issues, source, field + "." + Keys::Model, entry.model + " is no model that may have a file");
            valid = false;
        }
        else
        {
            entry.type = symbol->type;
        }
    }
    const auto file = json.find(Keys::File);
    if (file == json.end() || !file->is_string())
    {
        AddError(issues, source, field + "." + Keys::File, "missing or not text");
        valid = false;
    }
    else if (!IsDataRelativePath(file->get<std::string>()))
    {
        AddError(issues, source, field + "." + Keys::File,
                 "must be a path relative to Data/ with \"/\" separators and no \"..\"");
        valid = false;
    }
    else
    {
        entry.file = file->get<std::string>();
    }
    if (!ReadActions(json, field, source, entry, issues))
    {
        valid = false;
    }
    if (!ReadLod(json, field, source, entry, issues))
    {
        valid = false;
    }
    for (const auto& [key, value] : json.items())
    {
        if (key != Keys::Model && key != Keys::File && key != Keys::Actions && key != Keys::Lod)
        {
            AddWarning(issues, source, field + "." + key, UnknownField);
        }
    }
    return valid;
}
} // namespace

std::optional<std::filesystem::path> DataDirectories::Find(std::string_view relative) const
{
    const auto findIn = [relative](const std::filesystem::path& directory) -> std::optional<std::filesystem::path>
    {
        std::filesystem::path path = directory / std::filesystem::path(std::string(relative));
#ifndef _WIN32
        // Game data is spelled as on Windows; match the case on disk.
        path = std::filesystem::path(MuResolvePath(path.generic_string().c_str()));
#endif
        std::error_code error;
        if (std::filesystem::is_regular_file(path, error))
        {
            return path;
        }
        return std::nullopt;
    };
    if (overlay)
    {
        if (auto path = findIn(*overlay))
        {
            return path;
        }
    }
    return findIn(game);
}

DataDirectories MakeDataDirectories(const char* overlayValue, const std::filesystem::path& game,
                                    std::vector<Items::ItemDataIssue>& issues)
{
    DataDirectories directories;
    directories.game = game;
    if (overlayValue == nullptr || *overlayValue == '\0')
    {
        return directories;
    }
    const std::filesystem::path overlayData = std::filesystem::path(overlayValue) / "Data";
    std::error_code error;
    if (!std::filesystem::is_directory(overlayData, error))
    {
        AddWarning(issues, DataOverlayVariable, "",
                   overlayData.generic_string() + " is no directory; the overlay is not used");
        return directories;
    }
    directories.overlay = overlayData;
    return directories;
}

void ReadModelFilesJson(std::string_view text, const std::string& source, std::vector<ModelFileEntry>& models,
                        std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, ModelFilesFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(Keys::Models);
    if (list == root.end() || !list->is_array())
    {
        AddError(issues, source, Keys::Models, "missing or not a list of models");
        return;
    }
    std::set<int> types;
    for (size_t index = 0; index < list->size(); ++index)
    {
        ModelFileEntry entry;
        if (!ReadEntry((*list)[index], index, source, entry, issues))
        {
            continue;
        }
        if (!types.insert(entry.type).second)
        {
            AddError(issues, source, std::string(Keys::Models) + "[" + std::to_string(index) + "]",
                     entry.model + " has more than one file");
            continue;
        }
        models.push_back(std::move(entry));
    }
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != Keys::Models)
        {
            AddWarning(issues, source, key, UnknownField);
        }
    }
}

const ModelFileEntry* ModelFiles::Find(int type) const
{
    const auto entry =
        std::find_if(models.begin(), models.end(), [type](const ModelFileEntry& model) { return model.type == type; });
    return entry != models.end() ? &*entry : nullptr;
}

ModelFiles LoadModelFiles(const DataDirectories& directories)
{
    ModelFiles result;
    const std::optional<std::filesystem::path> table = directories.Find(ModelFilesPath);
    if (!table)
    {
        return result;
    }
    result.table = *table;
    const std::string source = table->generic_string();
    const std::optional<std::string> text = Items::Json::ReadTextFile(*table);
    if (!text)
    {
        AddError(result.issues, source, "", "could not be read");
        return result;
    }
    std::vector<ModelFileEntry> models;
    ReadModelFilesJson(*text, source, models, result.issues);
    for (ModelFileEntry& entry : models)
    {
        const std::optional<std::filesystem::path> file = directories.Find(entry.file);
        if (!file)
        {
            AddError(result.issues, source, entry.model, "the file " + entry.file + " is not found");
            continue;
        }
        entry.path = *file;
        result.models.push_back(std::move(entry));
    }
    return result;
}
} // namespace Data::Models
