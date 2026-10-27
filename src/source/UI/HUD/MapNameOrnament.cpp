#include "stdafx.h"
#include "UI/HUD/MapNameOrnament.h"

#include "Data/DataHandler/TextFile.h"

#include "json.hpp"

#include <filesystem>
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

bool IsNumber(const Json& object, const char* key)
{
    return object.contains(key) && object[key].is_number();
}

// Layout.json's "icons": { "<image>": { "x": 59, "y": 46, "width": 43, "height": 36 }, ... }
std::optional<MapIcon> ReadPlacement(const std::string& image, const Json& placement)
{
    if (!placement.is_object())
        return std::nullopt;
    for (const char* key : {"x", "y", "width", "height"})
    {
        if (!IsNumber(placement, key))
            return std::nullopt;
    }
    return MapIcon{image, placement["x"].get<float>(), placement["y"].get<float>(), placement["width"].get<float>(),
                   placement["height"].get<float>()};
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
    if (!layout || !mapIcons || !(*layout)["icons"].is_object() || !(*mapIcons)["maps"].is_array())
        return false;

    const Json& icons = (*layout)["icons"];
    for (const Json& entry : (*mapIcons)["maps"])
    {
        if (!entry.is_object() || !entry.contains("map") || !entry["map"].is_number_integer() ||
            !entry.contains("icon") || !entry["icon"].is_string())
            continue;
        const auto image = entry["icon"].get<std::string>();
        if (!icons.contains(image))
            continue;
        if (auto icon = ReadPlacement(image, icons[image]))
            m_Icons[entry["map"].get<int>()] = std::move(*icon);
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
    static const MapNameOrnament ornament = []
    {
        MapNameOrnament read;
        if (!read.Read(ReadFileText(LayoutFile), ReadFileText(MapIconsFile)))
            g_ErrorReport.Write(L"> [MapName] the ornament files in %hs are not valid.\r\n", OrnamentDirectory);
        return read;
    }();
    return ornament;
}
} // namespace UI::MapName
