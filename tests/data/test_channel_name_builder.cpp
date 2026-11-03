#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/ServerListIds.h"
#include "UI/RmlBridge/ThemeInheritance.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

using Data::ServerList::BuildChannelName;
using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ChannelNameValues;
using Data::ServerList::ChannelSettings;
using Data::ServerList::FillChannelName;
using Data::ServerList::ServerListData;

namespace
{
ChannelNameValues Values(std::string_view vip, std::string_view pvp)
{
    ChannelNameValues values;
    values.name = "Valhalla";
    values.channel = 1;
    values.vip = vip;
    values.pvp = pvp;
    return values;
}

std::string Login(std::string_view text, std::string_view vip, std::string_view pvp)
{
    return FillChannelName(text, ChannelNamePlace::Login, Values(vip, pvp));
}

ServerListData GoldList()
{
    ServerListData list;
    Data::ServerList::VipType gold;
    gold.id = "gold";
    gold.name.Set("en", "Gold");
    gold.name.Set("es", "Oro");
    list.vipTypes.push_back(gold);
    list.names.nonPvp.Set("es", "no PvP");
    list.names.Template(ChannelNamePlace::Login).Set("es", "[{name}-{channel} ({pvp} {vip})]");
    return list;
}

ChannelSettings Channel(bool pvp, const std::string& vip)
{
    ChannelSettings channel;
    channel.pvp = pvp;
    channel.vip = vip;
    return channel;
}
} // namespace

TEST_CASE("ChannelNameBuilder: the placeholders get their values")
{
    CHECK(Login("[{name}-{channel} ({vip} {pvp})]", "Gold", "Non-PvP") == "[Valhalla-1 (Gold Non-PvP)]");
    CHECK(Login("[{name}-{channel} ({vip} {pvp})]", "Gold", "PvP") == "[Valhalla-1 (Gold PvP)]");
}

TEST_CASE("ChannelNameBuilder: an empty value goes with one space next to it")
{
    // The space after it, when there is none before.
    CHECK(Login("[{name}-{channel} ({vip} {pvp})]", "", "PvP") == "[Valhalla-1 (PvP)]");
    CHECK(Login("[{name}-{channel} ({vip} {pvp})]", "", "Non-PvP") == "[Valhalla-1 (Non-PvP)]");
    // The space before it.
    CHECK(Login("[{name}-{channel} {vip}]", "", "PvP") == "[Valhalla-1]");
    CHECK(Login("{name} {vip} {pvp}", "", "PvP") == "Valhalla PvP");
    // Only one space.
    CHECK(Login("{name}  {vip}", "", "PvP") == "Valhalla ");
}

TEST_CASE("ChannelNameBuilder: brackets left empty go with one space next to them")
{
    CHECK(Login("[{name}-{channel} ({vip} {pvp})]", "", "") == "[Valhalla-1]");
    CHECK(Login("{name} [{vip}] end", "", "") == "Valhalla end");
    // Inner brackets first, so both go.
    CHECK(Login("{name} [({vip})]", "", "") == "Valhalla");
    // Brackets the template has empty on their own stay.
    CHECK(Login("{name} () {pvp}", "", "PvP") == "Valhalla () PvP");
}

TEST_CASE("ChannelNameBuilder: other text stays as written")
{
    CHECK(Login("{vip}, {pvp}", "", "PvP") == ", PvP");
    CHECK(Login("{vip}, {pvp}", "Gold", "PvP") == "Gold, PvP");
    // A placeholder the place does not have.
    CHECK(Login("{name} {load}", "", "PvP") == "Valhalla {load}");
    CHECK(Login("{name} {vipName}", "", "PvP") == "Valhalla {vipName}");
    // From a brace problem on.
    CHECK(Login("{name} {broken", "", "PvP") == "Valhalla {broken");
}

TEST_CASE("ChannelNameBuilder: each place has its own placeholders")
{
    ChannelNameValues values = Values("Gold", "PvP");
    values.load = "(Full)";
    values.character = "Hero";
    CHECK(FillChannelName("{name}-{channel} {load}", ChannelNamePlace::Button, values) == "Valhalla-1 (Full)");
    CHECK(FillChannelName("[{name} / {character}]", ChannelNamePlace::Screenshot, values) == "[Valhalla / Hero]");
    CHECK(FillChannelName("{character}", ChannelNamePlace::Button, values) == "{character}");
    CHECK(FillChannelName("{load}", ChannelNamePlace::CharacterInfo, values) == "{load}");
}

// What the file check reports and the editor shows under a template.
TEST_CASE("ChannelNameBuilder: a template's problems are its braces or the placeholders its place lacks")
{
    using Data::ServerList::FindTemplateProblems;
    const auto fine = FindTemplateProblems("[{name}-{channel} ({vip} {pvp})]", ChannelNamePlace::Login);
    CHECK(fine.braceError.empty());
    CHECK(fine.unknownPlaceholders.empty());

    const auto unknown = FindTemplateProblems("{name} {load} {character} {nope}", ChannelNamePlace::Login);
    CHECK(unknown.braceError.empty());
    CHECK(unknown.unknownPlaceholders == std::vector<std::string>{"load", "character", "nope"});
    CHECK(FindTemplateProblems("{name} {load}", ChannelNamePlace::Button).unknownPlaceholders.empty());

    // A brace problem is the only one told.
    const auto braces = FindTemplateProblems("{name {nope}", ChannelNamePlace::Login);
    CHECK_FALSE(braces.braceError.empty());
    CHECK(braces.unknownPlaceholders.empty());
}

TEST_CASE("ChannelNameBuilder: a channel's name in a language")
{
    const ServerListData list = GoldList();
    const ChannelNameValues values = Values("", "");

    CHECK(BuildChannelName(list, ChannelNamePlace::Login, Channel(false, "gold"), values, "en") ==
          "[Valhalla-1 (Gold Non-PvP)]");
    CHECK(BuildChannelName(list, ChannelNamePlace::Login, Channel(false, "gold"), values, "es") ==
          "[Valhalla-1 (no PvP Oro)]");
    CHECK(BuildChannelName(list, ChannelNamePlace::Login, Channel(true, ""), values, "es") == "[Valhalla-1 (PvP)]");
    // A language without texts of its own uses the English ones.
    CHECK(BuildChannelName(list, ChannelNamePlace::Login, Channel(true, "gold"), values, "de") ==
          "[Valhalla-1 (Gold PvP)]");
    // A VIP type the list does not have is none.
    CHECK(BuildChannelName(list, ChannelNamePlace::Login, Channel(true, "silver"), values, "en") ==
          "[Valhalla-1 (PvP)]");
}

TEST_CASE("ChannelNameBuilder: the built-in texts name the four kinds of channel as the docs show")
{
    const ServerListData list = GoldList();
    // A server that can be joined: {load} is empty.
    ChannelNameValues values = Values("", "");
    values.character = "MaxiDK";

    const auto button = [&](bool pvp, const std::string& vip)
    { return BuildChannelName(list, ChannelNamePlace::Button, Channel(pvp, vip), values, "en"); };
    CHECK(button(true, "") == "Valhalla-1 (PvP)");
    CHECK(button(false, "") == "Valhalla-1 (Non-PvP)");
    CHECK(button(true, "gold") == "Valhalla-1 (Gold PvP)");
    CHECK(button(false, "gold") == "Valhalla-1 (Gold Non-PvP)");

    CHECK(BuildChannelName(list, ChannelNamePlace::CharacterInfo, Channel(false, "gold"), values, "en") ==
          "[Valhalla-1 (Gold Non-PvP)]");
    CHECK(BuildChannelName(list, ChannelNamePlace::Screenshot, Channel(false, "gold"), values, "en") ==
          "[Valhalla / MaxiDK]");
}

TEST_CASE("ServerListIds: the ids of VIP types and colors")
{
    CHECK(Data::ServerList::IsListId("gold"));
    CHECK(Data::ServerList::IsListId("goldPlus2"));
    CHECK_FALSE(Data::ServerList::IsListId(""));
    CHECK_FALSE(Data::ServerList::IsListId("Gold"));
    CHECK_FALSE(Data::ServerList::IsListId("2gold"));
    CHECK_FALSE(Data::ServerList::IsListId("gold plus"));
    CHECK_FALSE(Data::ServerList::IsListId("g\xC3\xB6ld"));

    CHECK(Data::ServerList::MakeListId("Gold", "vip") == "gold");
    CHECK(Data::ServerList::MakeListId("Gold Plus", "vip") == "goldPlus");
    CHECK(Data::ServerList::MakeListId("New VIP type", "vip") == "newVipType");
    CHECK(Data::ServerList::MakeListId("2 Silver", "vip") == "silver");
    CHECK(Data::ServerList::MakeListId("\xC3\x96", "color") == "color");

    // Theme folder names as the game has them; anything but A to Z stays.
    CHECK(Data::ServerList::ToLowerAscii("Legacy-Theme_2") == "legacy-theme_2");
    CHECK(Data::ServerList::ToLowerAscii("\xC3\x96") == "\xC3\x96");
}

TEST_CASE("ChannelNameBuilder: theme color names")
{
    for (int value = 0; value < Data::ServerList::ThemeColorCount; ++value)
    {
        const auto color = static_cast<Data::ServerList::ThemeColor>(value);
        CHECK(Data::ServerList::FindThemeColor(Data::ServerList::GetThemeColorName(color)) == color);
    }
    CHECK_FALSE(Data::ServerList::FindThemeColor("Orange").has_value());
}

namespace
{
Data::ServerList::ButtonColorRule Rule(Data::ServerList::RuleVip vipMatch, const std::string& vip,
                                       std::optional<bool> pvp, const std::string& color)
{
    Data::ServerList::ButtonColorRule rule;
    rule.vipMatch = vipMatch;
    rule.vip = vip;
    rule.pvp = pvp;
    rule.color = color;
    return rule;
}
} // namespace

TEST_CASE("ButtonColors: of the rules that fit, the most specific one colors the button")
{
    using Data::ServerList::RuleVip;
    ServerListData list = GoldList();
    Data::ServerList::VipType silver;
    silver.id = "silver";
    silver.name.Set("en", "Silver");
    list.vipTypes.push_back(silver);
    // In an order that would give other colors if the first fitting rule won.
    list.buttonColors = {Rule(RuleVip::Any, "", std::nullopt, "gray"), Rule(RuleVip::Any, "", false, "default"),
                         Rule(RuleVip::AnyVipType, "", std::nullopt, "silver"),
                         Rule(RuleVip::VipType, "gold", std::nullopt, "orange"),
                         Rule(RuleVip::VipType, "gold", false, "red")};
    const auto color = [&](bool pvp, const std::string& vip)
    {
        const Data::ServerList::ButtonColorRule* rule = Data::ServerList::FindButtonColorRule(list, Channel(pvp, vip));
        return rule != nullptr ? rule->color : std::string("none");
    };

    CHECK(color(false, "gold") == "red");
    CHECK(color(true, "gold") == "orange");
    // Any VIP type beats a rule for PvP only.
    CHECK(color(false, "silver") == "silver");
    CHECK(color(false, "") == "default");
    CHECK(color(true, "") == "gray");

    list.buttonColors.clear();
    CHECK(color(true, "gold") == "none");
    CHECK(Data::ServerList::GetButtonColor(list, Channel(true, "gold"), "legacy") == Data::ServerList::ButtonColor{});
}

TEST_CASE("ButtonColors: every bundled theme has a shade for each theme color")
{
    // MU_TEST_DATA_DIR is the client folder's Data.
    for (const char* theme : {"legacy", "modern", "classic"})
    {
        const UI::RmlBridge::ThemeInheritance inheritance(std::filesystem::path(MU_TEST_DATA_DIR) / "Interface" /
                                                          "RmlUi" / "themes" / theme);
        for (int value = 0; value < Data::ServerList::ThemeColorCount; ++value)
        {
            const char* token = Data::ServerList::GetThemeColorToken(static_cast<Data::ServerList::ThemeColor>(value));
            CAPTURE(theme);
            CAPTURE(token);
            const auto setting = inheritance.GetSetting("Tokens", token);
            REQUIRE(setting.has_value());
            CHECK(Data::ServerList::IsShade(setting->value));
        }
    }
}

TEST_CASE("ButtonColors: a color of the list's own has a shade per theme")
{
    using Data::ServerList::RuleVip;
    ServerListData list = GoldList();
    Data::ServerList::CustomColor red;
    red.id = "goldRed";
    red.shades = {{"legacy", "#FF5A5A"}, {"modern", "#C0564E"}};
    list.colors.push_back(red);
    list.buttonColors = {Rule(RuleVip::VipType, "gold", std::nullopt, "goldRed"),
                         Rule(RuleVip::NoVipType, "", std::nullopt, "orange")};

    CHECK(Data::ServerList::GetButtonColor(list, Channel(true, "gold"), "legacy").shade == "#FF5A5A");
    CHECK(Data::ServerList::GetButtonColor(list, Channel(true, "gold"), "modern").shade == "#C0564E");
    // A theme without a shade of it draws its default color.
    CHECK(Data::ServerList::GetButtonColor(list, Channel(true, "gold"), "dark") == Data::ServerList::ButtonColor{});
    CHECK(Data::ServerList::GetButtonColor(list, Channel(true, ""), "dark").theme ==
          Data::ServerList::ThemeColor::Orange);

    CHECK(Data::ServerList::IsShade("#00ff7F"));
    CHECK_FALSE(Data::ServerList::IsShade("00FF7F"));
    CHECK_FALSE(Data::ServerList::IsShade("#00FF7"));
    CHECK_FALSE(Data::ServerList::IsShade("#00FF7G"));
    CHECK(Data::ServerList::IsKnownColor(list, "silver"));
    CHECK(Data::ServerList::IsKnownColor(list, "goldRed"));
    CHECK_FALSE(Data::ServerList::IsKnownColor(list, "purple"));
}
