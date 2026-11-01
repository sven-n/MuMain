#include "stdafx.h"

#include "doctest.h"

#include "Core/Color/OklchMix.h"
#include "Core/Color/Rgb.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/LoadBar.h"
#include "UI/RmlBridge/ThemeInheritance.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

using Core::Color::MixOklch;
using Core::Color::ParseRgb;
using Core::Color::Rgb;
using Data::ServerList::LoadBarPart;
using Data::ServerList::LoadState;
using Data::ServerList::ServerListData;

namespace
{
constexpr Rgb Green{0x00, 0xBD, 0x8B};
constexpr Rgb Red{0xE0, 0x48, 0x3E};

// A theme with the load bar's tokens and the button colors' gray.
Data::ServerList::ThemeTokenReader Theme(std::map<std::string, std::string> tokens)
{
    return [tokens = std::move(tokens)](const char* token) -> std::optional<std::string>
    {
        const auto found = tokens.find(token);
        return found != tokens.end() ? std::optional<std::string>(found->second) : std::nullopt;
    };
}

const Data::ServerList::ThemeTokenReader LegacyTokens = Theme({{"server-load-base", "#00bd8b"},
                                                               {"server-load-full", "#e0483e"},
                                                               {"server-load-empty", "rgba(0, 0, 0, 160)"},
                                                               {"server-button-gray", "#E2E2E2"}});

// A list with a color of its own that has a shade in the legacy theme only.
ServerListData ListWithBlue()
{
    ServerListData list;
    Data::ServerList::CustomColor blue;
    blue.id = "loadBlue";
    blue.shades = {{"legacy", "#2060FF"}};
    list.colors.push_back(blue);
    return list;
}
} // namespace

TEST_CASE("Rgb: #RRGGBB in either case is read, written in capitals")
{
    CHECK(ParseRgb("#00bd8b") == Green);
    CHECK(ParseRgb("#00BD8B") == Green);
    CHECK(Core::Color::FormatRgb(Green) == "#00BD8B");
    for (const char* text : {"", "00BD8B", "#00BD8", "#00BD8B0", "#GG0000", "#+10000", "#-10000", " #00BD8B"})
    {
        CAPTURE(text);
        CHECK_FALSE(ParseRgb(text).has_value());
    }
}

TEST_CASE("MixOklch: the ends are the colors, a share outside 0 to 1 is held to them")
{
    CHECK(MixOklch(Green, Red, 0.0f) == Green);
    CHECK(MixOklch(Green, Red, 1.0f) == Red);
    CHECK(MixOklch(Green, Red, -1.0f) == Green);
    CHECK(MixOklch(Green, Red, 2.0f) == Red);
    CHECK(MixOklch(Red, Red, 0.5f) == Red);
}

TEST_CASE("MixOklch: green goes to red around the color wheel, through yellow and orange")
{
    // Halfway, an evenly mixed red, green and blue would be a gray brown (#70826D); around the
    // wheel it is a warm color: more red and green than blue.
    const Rgb half = MixOklch(Green, Red, 0.5f);
    CHECK(half.red > half.blue + 40);
    CHECK(half.green > half.blue + 40);
    // A tenth of the way it is still green, a little warmer than the base.
    const Rgb tenth = MixOklch(Green, Red, 0.1f);
    CHECK(tenth.green > tenth.red);
    CHECK(tenth.red > Green.red);
}

TEST_CASE("MixOklch: a gray end takes the hue of the other end")
{
    constexpr Rgb Gray{0x80, 0x80, 0x80};
    const Rgb toRed = MixOklch(Gray, Red, 0.5f);
    CHECK(toRed.red > toRed.green);
    CHECK(toRed.red > toRed.blue);
}

TEST_CASE("LoadState: a server is full from 100 and overloaded from 128")
{
    CHECK(Data::ServerList::GetLoadState(-5) == LoadState::Open);
    CHECK(Data::ServerList::GetLoadState(99) == LoadState::Open);
    CHECK(Data::ServerList::GetLoadState(100) == LoadState::Full);
    CHECK(Data::ServerList::GetLoadState(127) == LoadState::Full);
    CHECK(Data::ServerList::GetLoadState(128) == LoadState::Overloaded);
    CHECK(Data::ServerList::GetLoadState(255) == LoadState::Overloaded);
}

TEST_CASE("LoadBar: each segment is 10 % of the load, the reached part of it lit")
{
    const Data::ServerList::LoadBarSegments segments = Data::ServerList::GetLoadBarSegments(Green, Red, 35);
    CHECK(segments[0].lit == doctest::Approx(1.0f));
    CHECK(segments[2].lit == doctest::Approx(1.0f));
    CHECK(segments[3].lit == doctest::Approx(0.5f));
    CHECK(segments[4].lit == doctest::Approx(0.0f));
    CHECK(segments[9].lit == doctest::Approx(0.0f));

    CHECK(Data::ServerList::GetLoadBarSegments(Green, Red, 0)[0].lit == doctest::Approx(0.0f));
    CHECK(Data::ServerList::GetLoadBarSegments(Green, Red, -10)[0].lit == doctest::Approx(0.0f));
    CHECK(Data::ServerList::GetLoadBarSegments(Green, Red, 1)[0].lit == doctest::Approx(0.1f));
    CHECK(Data::ServerList::GetLoadBarSegments(Green, Red, 99)[9].lit == doctest::Approx(0.9f));
}

TEST_CASE("LoadBar: segment k is the base color moved k tenths toward the full color")
{
    const Data::ServerList::LoadBarSegments segments = Data::ServerList::GetLoadBarSegments(Green, Red, 50);
    CHECK(segments[0].color == Green);
    for (int index = 1; index < Data::ServerList::LoadBarSegmentCount; ++index)
    {
        CAPTURE(index);
        CHECK(segments[static_cast<size_t>(index)].color == MixOklch(Green, Red, index / 10.0f));
    }
    // The tenth segment is 90 % of the way: the full color is left for a full server.
    CHECK(segments[9].color != Red);
}

TEST_CASE("LoadBar: a full or overloaded server has every segment lit in the full color")
{
    for (const int percent : {100, 127, 128, 200})
    {
        CAPTURE(percent);
        for (const Data::ServerList::LoadBarSegment& segment :
             Data::ServerList::GetLoadBarSegments(Green, Red, percent))
        {
            CHECK(segment.lit == doctest::Approx(1.0f));
            CHECK(segment.color == Red);
        }
    }
}

TEST_CASE("LoadBar: default is the theme's token, a theme color its button token")
{
    ServerListData list;
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Base, "legacy", LegacyTokens) == Green);
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, "legacy", LegacyTokens) == Red);
    CHECK(Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Empty, "legacy"));
    // The legacy track is rgba(): drawn by the stylesheet, nothing for the game to set.
    CHECK_FALSE(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Empty, "legacy", LegacyTokens).has_value());

    list.load.bar.empty = "gray";
    CHECK_FALSE(Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Empty, "legacy"));
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Empty, "legacy", LegacyTokens) ==
          Rgb{0xE2, 0xE2, 0xE2});
}

TEST_CASE("LoadBar: a color of the list's own is its shade, or the theme's own without one")
{
    ServerListData list = ListWithBlue();
    list.load.bar.full = "loadBlue";
    CHECK_FALSE(Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Full, "legacy"));
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, "legacy", LegacyTokens) ==
          Rgb{0x20, 0x60, 0xFF});
    // No shade for modern: the theme's own full color.
    CHECK(Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Full, "modern"));
    const auto modern = Theme({{"server-load-full", "#A22C1E"}});
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, "modern", modern) == Rgb{0xA2, 0x2C, 0x1E});
    // The color named directly, as the editor's color list asks.
    CHECK(Data::ServerList::ResolveLoadBarColor(list, "loadBlue", LoadBarPart::Base, "legacy", LegacyTokens) ==
          Rgb{0x20, 0x60, 0xFF});
}

TEST_CASE("LoadBar: a theme without the tokens has the built-in base and full colors")
{
    const ServerListData list;
    const auto none = Theme({});
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Base, "other", none) == Green);
    CHECK(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, "other", none) == Red);
    CHECK_FALSE(Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Empty, "other", none).has_value());
}

TEST_CASE("LoadBar: each shipped theme has its bar's colors")
{
    // MU_TEST_DATA_DIR is the client folder's Data.
    const std::filesystem::path themes = std::filesystem::path(MU_TEST_DATA_DIR) / "Interface/RmlUi/themes";
    for (const char* theme : {"legacy", "modern", "classic"})
    {
        CAPTURE(theme);
        const UI::RmlBridge::ThemeInheritance inheritance(themes / theme);
        const auto reader = [&inheritance](const char* token) -> std::optional<std::string>
        {
            const auto setting = inheritance.GetSetting("Tokens", token);
            return setting ? std::optional<std::string>(setting->value) : std::nullopt;
        };
        for (const LoadBarPart part : {LoadBarPart::Base, LoadBarPart::Full})
        {
            const std::optional<std::string> token = reader(Data::ServerList::GetLoadBarToken(part));
            REQUIRE(token.has_value());
            CHECK(ParseRgb(*token).has_value());
        }
        CHECK(reader(Data::ServerList::GetLoadBarToken(LoadBarPart::Empty)).has_value());
    }
}

TEST_CASE("ButtonTextParts: the button's text is split at its load text")
{
    ServerListData list;
    Data::ServerList::ChannelSettings channel;
    Data::ServerList::ChannelNameValues values;
    values.name = "Valhalla";
    values.channel = 1;
    values.load = "(Full)";
    const auto parts = [&](const std::string& text)
    {
        list.names.Template(Data::ServerList::ChannelNamePlace::Button).Set(Data::LocalizedString::NeutralLocale, text);
        return Data::ServerList::BuildButtonTextParts(list, channel, values, "en");
    };
    using Parts = Data::ServerList::ButtonTextParts;
    CHECK(parts("{name}-{channel} ({vip} {pvp}) {load}") == Parts{"Valhalla-1 (PvP) ", "(Full)", ""});
    CHECK(parts("{load} {name}") == Parts{"", "(Full)", " Valhalla"});
    CHECK(parts("{name}") == Parts{"Valhalla", "", ""});
    CHECK(parts("{name} {load} / {load}") == Parts{"Valhalla ", "(Full)", " / (Full)"});

    // A server that can be joined: no load text, and its space goes with it.
    values.load = "";
    CHECK(parts("{name}-{channel} ({vip} {pvp}) {load}") == Parts{"Valhalla-1 (PvP)", "", ""});
}

TEST_CASE("Load bar: a text over the bar covers the segments reaching into it")
{
    using Data::ServerList::CoveredSegments;
    using Data::ServerList::GetCoveredSegments;
    using Data::ServerList::SegmentSpan;
    // Ten segments 14 wide, 2 apart, as the legacy theme lays out a 158 wide bar.
    std::vector<SegmentSpan> segments;
    for (int index = 0; index < Data::ServerList::LoadBarSegmentCount; ++index)
    {
        segments.push_back({static_cast<float>(index) * 16.0f, 14.0f});
    }
    // A text 80 wide, centered: from 39 to 119, into segments 2 (32..46) to 7 (112..126).
    CHECK(GetCoveredSegments(segments, 39.0f, 119.0f) == CoveredSegments{2, 8});
    // One in the gap at the middle (78..80) covers none; edges that only touch don't count.
    CHECK(GetCoveredSegments(segments, 78.0f, 80.0f) == CoveredSegments{});
    CHECK(GetCoveredSegments(segments, 46.0f, 112.0f) == CoveredSegments{3, 7});
    // Wider than the bar: all of them.
    CHECK(GetCoveredSegments(segments, -10.0f, 200.0f) == CoveredSegments{0, 10});
    CHECK(GetCoveredSegments({}, 0.0f, 10.0f) == CoveredSegments{});
}
