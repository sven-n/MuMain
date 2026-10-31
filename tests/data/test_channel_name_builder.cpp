#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ServerListData/ChannelNameBuilder.h"

#include <string>

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
    values.load = "Connect";
    values.character = "Hero";
    CHECK(FillChannelName("{name}-{channel} {load}", ChannelNamePlace::Button, values) == "Valhalla-1 Connect");
    CHECK(FillChannelName("[{name} / {character}]", ChannelNamePlace::Screenshot, values) == "[Valhalla / Hero]");
    CHECK(FillChannelName("{character}", ChannelNamePlace::Button, values) == "{character}");
    CHECK(FillChannelName("{load}", ChannelNamePlace::CharacterInfo, values) == "{load}");
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
    ChannelNameValues values = Values("", "");
    values.load = "Connect";
    values.character = "MaxiDK";

    const auto button = [&](bool pvp, const std::string& vip)
    { return BuildChannelName(list, ChannelNamePlace::Button, Channel(pvp, vip), values, "en"); };
    CHECK(button(true, "") == "Valhalla-1 (PvP) Connect");
    CHECK(button(false, "") == "Valhalla-1 (Non-PvP) Connect");
    CHECK(button(true, "gold") == "Valhalla-1 (Gold PvP) Connect");
    CHECK(button(false, "gold") == "Valhalla-1 (Gold Non-PvP) Connect");

    CHECK(BuildChannelName(list, ChannelNamePlace::CharacterInfo, Channel(false, "gold"), values, "en") ==
          "[Valhalla-1 (Gold Non-PvP)]");
    CHECK(BuildChannelName(list, ChannelNamePlace::Screenshot, Channel(false, "gold"), values, "en") ==
          "[Valhalla / MaxiDK]");
}

TEST_CASE("ChannelNameBuilder: VIP type ids")
{
    CHECK(Data::ServerList::IsVipTypeId("gold"));
    CHECK(Data::ServerList::IsVipTypeId("goldPlus2"));
    CHECK_FALSE(Data::ServerList::IsVipTypeId(""));
    CHECK_FALSE(Data::ServerList::IsVipTypeId("Gold"));
    CHECK_FALSE(Data::ServerList::IsVipTypeId("2gold"));
    CHECK_FALSE(Data::ServerList::IsVipTypeId("gold plus"));
    CHECK_FALSE(Data::ServerList::IsVipTypeId("g\xC3\xB6ld"));

    CHECK(Data::ServerList::MakeVipTypeId("Gold") == "gold");
    CHECK(Data::ServerList::MakeVipTypeId("Gold Plus") == "goldPlus");
    CHECK(Data::ServerList::MakeVipTypeId("New VIP type") == "newVipType");
    CHECK(Data::ServerList::MakeVipTypeId("2 Silver") == "silver");
    CHECK(Data::ServerList::MakeVipTypeId("\xC3\x96") == "vip");
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
