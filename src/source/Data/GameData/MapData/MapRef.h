#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <functional>
#include <span>
#include <string>
#include <vector>

// A map in data (monsters design, D67): the number the server sends, and for
// a map whose number the server gives to several maps it tells apart by a
// discriminator (the stages of Devil Square: 1 to 4 on 9, 5 to 7 on 32), one
// stage. In JSON a number, or {"map": 9, "stage": 2}.
namespace Data::Maps
{
// The map numbers the server sends (a byte) and the stages.
constexpr int MaxServerMap = 255;
constexpr int MaxStage = 255;

struct MapRef
{
    int map = 0;
    // 0: every stage of the map.
    int stage = 0;

    auto operator<=>(const MapRef&) const = default;
};

// Whether the list names the map the server sent with this stage; an entry
// without a stage names every stage of its map.
bool Names(std::span<const MapRef> maps, int serverMap, int stage);

// Whether two entries of map lists both name a map and stage: an entry without
// a stage names all of its map's stages (a map in two map variants).
bool Overlaps(const MapRef& first, const MapRef& second);

// Reads a list of maps, sorted and without repeats; false, with the problems
// sent to `fail(field, message)`, when an entry is not a map.
bool ReadMapRefs(const Items::Json::OrderedJson& json, const std::string& field, std::vector<MapRef>& maps,
                 const std::function<void(const std::string&, const std::string&)>& fail);

// A number for a whole map, an object for one stage.
Items::Json::OrderedJson WriteMapRefs(std::span<const MapRef> maps);
} // namespace Data::Maps
