#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MapData/MapRef.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

// What the readers and writers of the monster area's lists share
// (Models.json, Sounds.json).
namespace Data::Monsters::Json
{
using Items::Json::KeySet;
using Items::Json::OrderedJson;

// The problems of one entry of a list: each names the entry and its field
// ("model MONSTER_MODEL_HOUND file", "sound set Hound sounds[0]").
class EntryIssues
{
public:
    // `entry` names the entry: "model MONSTER_MODEL_HOUND".
    EntryIssues(const std::string& source, std::string entry, std::vector<Items::ItemDataIssue>& issues);

    void Fatal(const std::string& key, const std::string& message);
    void Warning(const std::string& key, const std::string& message);
    // Warns about the keys of `json` that `known` does not have.
    void WarnAboutUnknownKeys(const OrderedJson& json, const KeySet& known, const std::string& prefix);

    // No fatal problem so far.
    bool IsValid() const
    {
        return m_valid;
    }

private:
    void Add(Items::ItemDataIssueSeverity severity, const std::string& key, const std::string& message);

    const std::string& m_source;
    std::string m_entry;
    std::vector<Items::ItemDataIssue>& m_issues;
    bool m_valid = true;
};

std::optional<long long> ReadWhole(const OrderedJson& json);
std::optional<int> ReadWholeIn(const OrderedJson& json, long long low, long long high);
// "must be a whole number from 1 to 4".
std::string RangeText(long long low, long long high);

// A list of whole numbers from low to high, each once unless `repeats`;
// false on fatal problems, which are reported.
bool ReadWholeList(const OrderedJson& json, const std::string& key, long long low, long long high,
                   std::vector<int>& values, EntryIssues& issues, bool repeats = false);
// A text that is not empty.
bool ReadText(const OrderedJson& json, const std::string& key, std::string& text, EntryIssues& issues);
// A path of the client folder (Paths::FindPathProblem).
bool ReadPath(const OrderedJson& json, const std::string& key, std::string& path, EntryIssues& issues);
// Animation names (stop1, walk, attack1, ... or a number from 12), each once;
// sorted.
bool ReadAnimationList(const OrderedJson& json, const std::string& key, std::vector<int>& animations,
                       EntryIssues& issues);
// A list of maps: server map numbers and stages (D67).
bool ReadMaps(const OrderedJson& json, const std::string& key, std::vector<Data::Maps::MapRef>& maps,
              EntryIssues& issues);
// Two frames, the first below the second: [7, 8].
bool ReadFrames(const OrderedJson& json, const std::string& key, std::pair<float, float>& frames, EntryIssues& issues);

// 0.7 instead of 0.69999998807907104: the shortest text that reads back as
// the same float; 270 instead of 270.0.
OrderedJson WriteFloat(float value);
OrderedJson WriteAnimationList(const std::vector<int>& animations);
OrderedJson WriteFrames(const std::pair<float, float>& frames);
} // namespace Data::Monsters::Json
