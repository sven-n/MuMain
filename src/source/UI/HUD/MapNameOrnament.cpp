#include "stdafx.h"
#include "UI/HUD/MapNameOrnament.h"

#include "Data/DataHandler/TextFile.h"

#include "json.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>

namespace UI::MapName
{
namespace
{
using Json = nlohmann::json;

constexpr const char* LayoutFile = "Layout.json";
constexpr const char* MapIconsFile = "MapIcons.json";

std::optional<Json> ParseObject(std::string_view text)
{
    Json json = Json::parse(text, nullptr, false);
    if (json.is_discarded() || !json.is_object())
        return std::nullopt;
    return json;
}

std::optional<float> ReadFiniteFloat(const Json& object, const char* key)
{
    const auto value = object.find(key);
    if (value == object.end() || !value->is_number())
        return std::nullopt;

    const double number = value->get<double>();
    if (!std::isfinite(number) || number < -std::numeric_limits<float>::max() ||
        number > std::numeric_limits<float>::max())
        return std::nullopt;

    const float result = static_cast<float>(number);
    return std::isfinite(result) ? std::optional<float>(result) : std::nullopt;
}

std::optional<int> ReadMapNumber(const Json& entry)
{
    const auto map = entry.find("map");
    if (map == entry.end() || !map->is_number_integer())
        return std::nullopt;

    if (map->is_number_unsigned())
    {
        const auto number = map->get<std::uint64_t>();
        if (number > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
            return std::nullopt;
        return static_cast<int>(number);
    }

    const auto number = map->get<std::int64_t>();
    if (number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
        return std::nullopt;
    return static_cast<int>(number);
}

// Layout.json's "icons": { "<image>": { "x": 59, "y": 46, "width": 43, "height": 36 }, ... }
std::optional<MapIcon> ReadPlacement(const std::string& image, const Json& placement)
{
    if (!placement.is_object())
        return std::nullopt;
    const auto x = ReadFiniteFloat(placement, "x");
    const auto y = ReadFiniteFloat(placement, "y");
    const auto width = ReadFiniteFloat(placement, "width");
    const auto height = ReadFiniteFloat(placement, "height");
    if (!x || !y || !width || !height || *width <= 0.f || *height <= 0.f)
        return std::nullopt;
    return MapIcon{image, *x, *y, *width, *height};
}

std::string ReadFileText(const char* name)
{
    const auto text = Data::ReadTextFile(std::filesystem::path(OrnamentDirectory) / name);
    if (!text)
        g_ErrorReport.Write(L"> [MapName] %hs/%hs could not be read.\r\n", OrnamentDirectory, name);
    return text.value_or(std::string());
}
} // namespace

bool MapNameOrnament::Read(std::string_view layoutJson, std::string_view mapIconsJson)
{
    m_Icons.clear();
    const auto layout = ParseObject(layoutJson);
    const auto mapIcons = ParseObject(mapIconsJson);
    if (!layout || !mapIcons)
        return false;

    const auto iconsValue = layout->find("icons");
    const auto mapsValue = mapIcons->find("maps");
    if (iconsValue == layout->end() || !iconsValue->is_object() || mapsValue == mapIcons->end() ||
        !mapsValue->is_array())
        return false;

    const Json& icons = *iconsValue;
    for (const Json& entry : *mapsValue)
    {
        if (!entry.is_object())
            continue;
        const auto map = ReadMapNumber(entry);
        const auto imageValue = entry.find("icon");
        if (!map || imageValue == entry.end() || !imageValue->is_string())
            continue;
        const auto image = imageValue->get<std::string>();
        const auto placement = icons.find(image);
        if (placement == icons.end())
            continue;
        if (auto icon = ReadPlacement(image, *placement))
            m_Icons[*map] = std::move(*icon);
    }
    return true;
}

const MapIcon* MapNameOrnament::FindIcon(int map) const
{
    const auto it = m_Icons.find(map);
    return it != m_Icons.end() ? &it->second : nullptr;
}

const MapNameOrnament& GetMapNameOrnament()
{
    static MapNameOrnament ornament;
    static bool valid = false;
    if (!valid)
    {
        valid = ornament.Read(ReadFileText(LayoutFile), ReadFileText(MapIconsFile));
        if (!valid)
            g_ErrorReport.Write(L"> [MapName] the ornament files in %hs are not valid.\r\n", OrnamentDirectory);
    }
    return ornament;
}
} // namespace UI::MapName
