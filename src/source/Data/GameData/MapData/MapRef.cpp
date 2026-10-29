#include "stdafx.h"

#include "MapRef.h"

#include <algorithm>
#include <optional>

namespace Data::Maps
{
namespace
{
using Items::Json::OrderedJson;

constexpr const char* MapKey = "map";
constexpr const char* StageKey = "stage";

std::optional<int> ReadNumber(const OrderedJson& json, long long low, long long high)
{
    long long number = 0;
    if (!json.is_number_integer() || !Items::Json::ReadWholeNumber(json, number) || number < low || number > high)
    {
        return std::nullopt;
    }
    return static_cast<int>(number);
}

std::optional<MapRef> ReadMapRef(const OrderedJson& json)
{
    if (const std::optional<int> map = ReadNumber(json, 0, MaxServerMap))
    {
        return MapRef{*map, 0};
    }
    if (!json.is_object() || json.size() != 2 || !json.contains(MapKey) || !json.contains(StageKey))
    {
        return std::nullopt;
    }
    const std::optional<int> map = ReadNumber(json[MapKey], 0, MaxServerMap);
    const std::optional<int> stage = ReadNumber(json[StageKey], 1, MaxStage);
    if (!map || !stage)
    {
        return std::nullopt;
    }
    return MapRef{*map, *stage};
}
} // namespace

bool Names(std::span<const MapRef> maps, int serverMap, int stage)
{
    return std::any_of(maps.begin(), maps.end(), [&](const MapRef& ref)
                       { return ref.map == serverMap && (ref.stage == 0 || ref.stage == stage); });
}

bool Overlaps(const MapRef& first, const MapRef& second)
{
    return first.map == second.map && (first.stage == 0 || second.stage == 0 || first.stage == second.stage);
}

bool ReadMapRefs(const OrderedJson& json, const std::string& field, std::vector<MapRef>& maps,
                 const std::function<void(const std::string&, const std::string&)>& fail)
{
    if (!json.is_array())
    {
        fail(field, "must be a list of maps");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        const std::optional<MapRef> ref = ReadMapRef(json[index]);
        if (!ref)
        {
            fail(field + "[" + std::to_string(index) + "]",
                 "must be a map number from 0 to " + std::to_string(MaxServerMap) +
                     ", or {\"map\": number, \"stage\": 1 to " + std::to_string(MaxStage) + "}");
            valid = false;
            continue;
        }
        maps.push_back(*ref);
    }
    std::sort(maps.begin(), maps.end());
    maps.erase(std::unique(maps.begin(), maps.end()), maps.end());
    return valid;
}

OrderedJson WriteMapRefs(std::span<const MapRef> maps)
{
    OrderedJson json = OrderedJson::array();
    for (const MapRef& ref : maps)
    {
        if (ref.stage == 0)
        {
            json.push_back(ref.map);
            continue;
        }
        OrderedJson stage = OrderedJson::object();
        stage[MapKey] = ref.map;
        stage[StageKey] = ref.stage;
        json.push_back(std::move(stage));
    }
    return json;
}
} // namespace Data::Maps
