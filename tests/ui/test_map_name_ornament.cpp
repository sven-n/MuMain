#include "doctest.h"
#include "UI/HUD/MapNameOrnament.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace
{
constexpr std::string_view Layout = R"({"icons":{"valid":{"x":1,"y":2,"width":3,"height":4}}})";
constexpr std::string_view ValidMap = R"({"maps":[{"map":1,"icon":"valid"}]})";

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
} // namespace

TEST_CASE("map-name ornament rejects invalid root schema and clears old entries")
{
    UI::MapName::MapNameOrnament ornament;
    REQUIRE(ornament.Read(Layout, ValidMap));
    REQUIRE(ornament.FindIcon(1) != nullptr);

    for (const auto& [layout, maps] : {std::pair<std::string_view, std::string_view>{R"({})", R"({"maps":[]})"},
                                       {R"({"icons":[]})", R"({"maps":[]})"},
                                       {R"({"icons":{}})", R"({})"},
                                       {R"({"icons":{}})", R"({"maps":{}})"},
                                       {R"([])", R"({"maps":[]})"},
                                       {R"({"icons":{}})", R"([])"}})
    {
        CHECK_FALSE(ornament.Read(layout, maps));
        CHECK(ornament.FindIcon(1) == nullptr);
    }
}

TEST_CASE("map-name ornament skips malformed maps and unsafe geometry")
{
    UI::MapName::MapNameOrnament ornament;
    REQUIRE(ornament.Read(
        R"({"icons":{
            "valid":{"x":1,"y":2,"width":3,"height":4},
            "bad-position":{"x":3.5e38,"y":0,"width":1,"height":1},
            "missing-coordinate":{"y":0,"width":1,"height":1},
            "wrong-coordinate-type":{"x":"0","y":0,"width":1,"height":1},
            "zero-width":{"x":0,"y":0,"width":0,"height":1},
            "negative-height":{"x":0,"y":0,"width":1,"height":-1},
            "underflow-width":{"x":0,"y":0,"width":1e-100,"height":1}}})",
        R"({"maps":[
            {"map":1,"icon":"valid"},
            {"map":2147483648,"icon":"valid"},
            {"map":-2147483649,"icon":"valid"},
            {"map":2.5,"icon":"valid"},
            {"icon":"valid"},
            {"map":2,"icon":"missing"},
            {"map":2,"icon":false},
            {"map":3,"icon":"bad-position"},
            {"map":4,"icon":"missing-coordinate"},
            {"map":5,"icon":"wrong-coordinate-type"},
            {"map":6,"icon":"zero-width"},
            {"map":7,"icon":"negative-height"},
            {"map":8,"icon":"underflow-width"}]})"));

    CHECK(ornament.FindIcon(1) != nullptr);
    for (int map : {2, 3, 4, 5, 6, 7, 8})
        CHECK(ornament.FindIcon(map) == nullptr);
}

TEST_CASE("packaged map-name ornament aliases point at their configured icons")
{
    const std::filesystem::path directory = std::filesystem::path(MU_BIN_DIR) / "Data/Interface/MapNameOrnament";
    const auto layout = ReadFile(directory / "Layout.json");
    const auto maps = ReadFile(directory / "MapIcons.json");
    REQUIRE_FALSE(layout.empty());
    REQUIRE_FALSE(maps.empty());

    UI::MapName::MapNameOrnament ornament;
    REQUIRE(ornament.Read(layout, maps));
    REQUIRE(ornament.FindIcon(32) != nullptr);
    CHECK(ornament.FindIcon(32)->image == "devilsquare");
    REQUIRE(ornament.FindIcon(35) != nullptr);
    CHECK(ornament.FindIcon(35)->image == "crywolffortress");
}
