#include "stdafx.h"

#include "doctest.h"

#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"

#include <string>
#include <optional>
#include <utility>
#include <vector>

using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ChannelSettings;
using Data::ServerList::ServerGroup;
using Data::ServerList::ServerListData;
using Data::ServerList::ThemeColor;

namespace
{
// The shared instance's constructor is protected; each test gets a manager of its own.
class TestServerListManager : public CServerListManager
{
public:
    TestServerListManager() = default;
};

// The load the connect server sends; the button says nothing about it.
constexpr int LowLoad = 10;

ServerGroup Group(int id, const std::string& name)
{
    ServerGroup group;
    group.id = id;
    group.name = name;
    return group;
}

// The groups with the VIP types Gold (orange) and Silver (silver).
ServerListData List(std::vector<ServerGroup> groups)
{
    ServerListData list;
    for (const auto& [id, name] : {std::pair{"gold", "Gold"}, std::pair{"silver", "Silver"}})
    {
        Data::ServerList::VipType vipType;
        vipType.id = id;
        vipType.name.Set("en", name);
        list.vipTypes.push_back(vipType);
    }
    // Gold orange, Silver silver, no VIP type with PvP on gray; Gold with PvP off in a red of its own.
    Data::ServerList::CustomColor red;
    red.id = "goldRed";
    red.shades = {{"legacy", "#FF5A5A"}};
    list.colors.push_back(red);
    const auto rule = [&list](Data::ServerList::RuleVip vipMatch, const std::string& vip, std::optional<bool> pvp,
                              const std::string& color)
    {
        Data::ServerList::ButtonColorRule added;
        added.vipMatch = vipMatch;
        added.vip = vip;
        added.pvp = pvp;
        added.color = color;
        list.buttonColors.push_back(added);
    };
    rule(Data::ServerList::RuleVip::VipType, "gold", std::nullopt, "orange");
    rule(Data::ServerList::RuleVip::VipType, "gold", false, "goldRed");
    rule(Data::ServerList::RuleVip::VipType, "silver", std::nullopt, "silver");
    rule(Data::ServerList::RuleVip::NoVipType, "", true, "gray");
    list.groups = std::move(groups);
    return list;
}

ChannelSettings Channel(bool pvp, const std::string& vip)
{
    ChannelSettings channel;
    channel.pvp = pvp;
    channel.vip = vip;
    return channel;
}

int ServerId(int groupId, int channel)
{
    return groupId * MAX_SERVER_PER_GROUP + channel - 1;
}

std::vector<CServerGroup*> GroupsInOrder(CServerListManager& manager)
{
    std::vector<CServerGroup*> groups;
    CServerGroup* group = nullptr;
    manager.SetFirst();
    while (manager.GetNext(group))
    {
        groups.push_back(group);
    }
    return groups;
}
} // namespace

TEST_CASE("ServerListManager: the list order is the order of the groups")
{
    TestServerListManager manager;
    manager.SetServerList(List({Group(2, "Midgard"), Group(0, "Valhalla"), Group(1, "Helheim")}));
    for (const int groupId : {0, 1, 2})
    {
        manager.InsertServerGroup(ServerId(groupId, 1), LowLoad);
    }

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 3);
    CHECK(std::wstring(groups[0]->m_szName) == L"Midgard");
    CHECK(groups[0]->m_iSequence == 1);
    CHECK(groups[0]->m_iServerIndex == 2);
    CHECK(std::wstring(groups[1]->m_szName) == L"Valhalla");
    CHECK(groups[1]->m_iSequence == 2);
    CHECK(std::wstring(groups[2]->m_szName) == L"Helheim");
    CHECK(groups[2]->m_iSequence == 3);
}

TEST_CASE("ServerListManager: a server of a group the list does not have is not shown")
{
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla")}));

    manager.InsertServerGroup(ServerId(3, 1), LowLoad);
    CHECK(manager.GetServerGroupSize() == 0);

    manager.InsertServerGroup(ServerId(0, 2), LowLoad);
    CHECK(manager.GetServerGroupSize() == 1);
}

TEST_CASE("ServerListManager: the servers of a group are its channels")
{
    TestServerListManager manager;
    manager.SetServerList(List({Group(1, "Helheim")}));
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);
    manager.InsertServerGroup(ServerId(1, 20), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    REQUIRE(groups[0]->GetServerSize() == 2);
    CHECK(groups[0]->GetServerInfo(0)->m_iIndex == 1);
    CHECK(groups[0]->GetServerInfo(0)->m_iConnectIndex == ServerId(1, 1));
    CHECK(groups[0]->GetServerInfo(1)->m_iIndex == 20);
    CHECK(groups[0]->GetServerInfo(1)->m_iConnectIndex == ServerId(1, 20));
}

TEST_CASE("ServerListManager: the channel settings reach the channels, their buttons and colors")
{
    I18N::SetLocale("en");
    ServerGroup valhalla = Group(0, "Valhalla");
    valhalla.channels[0] = Channel(false, "gold");
    valhalla.channels[1] = Channel(true, "gold");
    valhalla.channels[2] = Channel(false, "");
    valhalla.channels[4] = Channel(true, "silver");
    valhalla.channels[19] = Channel(false, "gold");
    TestServerListManager manager;
    manager.SetServerList(List({valhalla}));
    for (const int channel : {1, 2, 3, 4, 5, 20})
    {
        manager.InsertServerGroup(ServerId(0, channel), LowLoad);
    }

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    const CServerGroup& group = *groups[0];
    CHECK(group.m_channels == valhalla.channels);
    // A group with a channel with PvP off shows no PvP notice.
    CHECK_FALSE(group.m_bPvPServer);

    REQUIRE(groups[0]->GetServerSize() == 6);
    const auto server = [&](int index) -> const CServerInfo& { return *groups[0]->GetServerInfo(index); };
    CHECK(server(0).m_channel == valhalla.channels[0]);
    const auto name = [&](int index) { return manager.GetServerButtonName(group, server(index).m_iIndex); };
    CHECK(name(0) == "Valhalla-1 (Gold Non-PvP)");
    CHECK(name(1) == "Valhalla-2 (Gold PvP)");
    CHECK(name(2) == "Valhalla-3 (Non-PvP)");
    CHECK(name(3) == "Valhalla-4 (PvP)");
    CHECK(name(4) == "Valhalla-5 (Silver PvP)");
    CHECK(name(5) == "Valhalla-20 (Gold Non-PvP)");

    const auto color = [&](int index, std::string_view theme)
    { return manager.GetServerButtonColor(server(index), theme); };
    // Gold with PvP off has its own red in the legacy theme; the modern theme has no shade of it.
    CHECK(color(0, "legacy").shade == "#FF5A5A");
    CHECK(color(0, "modern") == Data::ServerList::ButtonColor{});
    CHECK(color(1, "legacy").theme == ThemeColor::Orange);
    CHECK(color(2, "legacy") == Data::ServerList::ButtonColor{});
    CHECK(color(3, "legacy").theme == ThemeColor::Gray);
    CHECK(color(4, "modern").theme == ThemeColor::Silver);
}

TEST_CASE("ServerListManager: the button names the server's load only when it cannot be joined")
{
    I18N::SetLocale("en");
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla")}));
    manager.InsertServerGroup(ServerId(0, 1), 99);
    manager.InsertServerGroup(ServerId(0, 2), 100);
    manager.InsertServerGroup(ServerId(0, 3), 128);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    const auto load = [&](int index) { return manager.GetServerLoadText(*groups[0]->GetServerInfo(index)); };
    // Below 100 there is no load text.
    CHECK(load(0).empty());
    // The list's texts; the built-in ones without a load section.
    CHECK(load(1) == "(Full)");
    CHECK(load(2) == "Overloaded");

    // In the game's language.
    I18N::SetLocale("de");
    Data::ServerList::ServerListData list = List({Group(0, "Valhalla")});
    list.load.full.text.Set("de", "(Voll)");
    TestServerListManager germanManager;
    germanManager.SetServerList(list);
    germanManager.InsertServerGroup(ServerId(0, 2), 100);
    const std::vector<CServerGroup*> german = GroupsInOrder(germanManager);
    REQUIRE(german.size() == 1);
    CHECK(germanManager.GetServerLoadText(*german[0]->GetServerInfo(0)) == "(Voll)");
    I18N::SetLocale("en");
}

TEST_CASE("ServerListManager: a button's name leaves its load text out, for every channel the group can have")
{
    I18N::SetLocale("en");
    ServerGroup valhalla = Group(0, "Valhalla");
    valhalla.channels[6] = Channel(false, "gold");
    TestServerListManager manager;
    manager.SetServerList(List({valhalla}));
    manager.InsertServerGroup(ServerId(0, 2), 128);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    // The overloaded server's name without "Overloaded", which the button shows apart from it.
    CHECK(manager.GetServerButtonName(*groups[0], 2) == "Valhalla-2 (PvP)");
    CHECK(manager.GetServerLoadText(*groups[0]->GetServerInfo(0)) == "Overloaded");
    // A channel the connect server did not send, with its own settings.
    CHECK(manager.GetServerButtonName(*groups[0], 7) == "Valhalla-7 (Gold Non-PvP)");
    CHECK(manager.GetServerButtonName(*groups[0], 0).empty());
    CHECK(manager.GetServerButtonName(*groups[0], MAX_SERVER_PER_GROUP + 1).empty());
}

TEST_CASE("ServerListManager: the load texts a button can show, none without {load} in its template")
{
    I18N::SetLocale("en");
    ServerListData list = List({Group(0, "Valhalla")});
    list.load.overloaded.bold = true;
    list.load.overloaded.italic = true;
    TestServerListManager manager;
    manager.SetServerList(list);
    const std::vector<CServerListManager::ButtonLoadText> texts = manager.GetButtonLoadTexts();
    REQUIRE(texts.size() == 2);
    CHECK(texts[0].text == "(Full)");
    CHECK_FALSE(texts[0].bold);
    CHECK(texts[1].text == "Overloaded");
    CHECK(texts[1].bold);
    CHECK(texts[1].italic);

    list.names.Template(Data::ServerList::ChannelNamePlace::Button).Set("en", "{name}-{channel}");
    manager.SetServerList(list);
    CHECK(manager.GetButtonLoadTexts().empty());
}

TEST_CASE("ServerListManager: a group without a channel with PvP off shows the PvP notice")
{
    ServerGroup helheim = Group(1, "Helheim");
    helheim.channels[0] = Channel(true, "gold");
    TestServerListManager manager;
    manager.SetServerList(List({helheim}));
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CHECK(groups[0]->m_bPvPServer);
}

TEST_CASE("ServerListManager: the selected channel's titles name its PvP and VIP type")
{
    I18N::SetLocale("en");
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla")}));
    const auto titleFor = [&](const ChannelSettings& channel, ChannelNamePlace place)
    {
        manager.SetSelectServerInfo(0, L"Valhalla", 3, channel);
        return manager.GetSelectServerTitle(place, L"MaxiDK");
    };

    CHECK(titleFor(Channel(true, ""), ChannelNamePlace::Login) == L"[Valhalla-3 (PvP)]");
    CHECK(titleFor(Channel(false, ""), ChannelNamePlace::Login) == L"[Valhalla-3 (Non-PvP)]");
    CHECK(titleFor(Channel(true, "gold"), ChannelNamePlace::Login) == L"[Valhalla-3 (Gold PvP)]");
    CHECK(titleFor(Channel(false, "gold"), ChannelNamePlace::Login) == L"[Valhalla-3 (Gold Non-PvP)]");
    CHECK(titleFor(Channel(false, "gold"), ChannelNamePlace::CharacterInfo) == L"[Valhalla-3 (Gold Non-PvP)]");
    CHECK(titleFor(Channel(false, "gold"), ChannelNamePlace::Screenshot) == L"[Valhalla / MaxiDK]");
    // A VIP type the list doesn't have is none.
    CHECK(titleFor(Channel(true, "bronze"), ChannelNamePlace::Login) == L"[Valhalla-3 (PvP)]");

    manager.SetSelectServerInfo(0, L"Valhalla", 3, Channel(false, "gold"));
    CHECK(manager.IsNonPvP());
    manager.SetSelectServerInfo(0, L"Valhalla", 3, Channel(true, "gold"));
    CHECK_FALSE(manager.IsNonPvP());
}

TEST_CASE("ServerListManager: the UTF-8 title is built again only when something it shows changed")
{
    I18N::SetLocale("en");
    ServerListData list = List({Group(0, "Valhalla")});
    TestServerListManager manager;
    manager.SetServerList(list);
    manager.SetSelectServerInfo(0, L"Valhalla", 1, Channel(false, "gold"));

    const std::string& title = manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login);
    CHECK(title == "[Valhalla-1 (Gold Non-PvP)]");
    // The same string while nothing changed.
    CHECK(&manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == &title);

    manager.SetSelectServerInfo(0, L"Valhalla", 2, Channel(true, ""));
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Valhalla-2 (PvP)]");

    list.names.pvp.Set("en", "Open PvP");
    manager.SetServerList(list);
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Valhalla-2 (Open PvP)]");

    list.names.pvp.Set("es", "PvP abierto");
    manager.SetServerList(list);
    I18N::SetLocale("es");
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Valhalla-2 (PvP abierto)]");
    I18N::SetLocale("en");
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Valhalla-2 (Open PvP)]");
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::CharacterInfo) == "[Valhalla-2 (Open PvP)]");
}

// MuEditor changes the list while the game runs: the titles of the channel the player joined name it as the list
// does now.
TEST_CASE("ServerListManager: the selected channel follows the list's changes to its group")
{
    I18N::SetLocale("en");
    ServerListData list = List({Group(0, "Valhalla")});
    list.groups[0].channels[2] = Channel(false, "gold");
    TestServerListManager manager;
    manager.SetServerList(list);
    manager.SetSelectServerInfo(0, L"Valhalla", 3, list.groups[0].channels[2]);
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Valhalla-3 (Gold Non-PvP)]");
    CHECK(manager.IsNonPvP());

    // The VIP type gets another ID, which its channels get too; PvP goes on and the group is renamed.
    list.vipTypes[0].id = "golden";
    list.groups[0].channels[2] = Channel(true, "golden");
    list.groups[0].name = "Asgard";
    manager.SetServerList(list);
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Asgard-3 (Gold PvP)]");
    CHECK(manager.GetSelectServerTitle(ChannelNamePlace::Screenshot, L"MaxiDK") == L"[Asgard / MaxiDK]");
    CHECK_FALSE(manager.IsNonPvP());

    // A list without the group keeps the channel as it was; its VIP type is none there.
    manager.SetServerList(List({Group(1, "Helheim")}));
    CHECK(manager.GetSelectServerTitleUtf8(ChannelNamePlace::Login) == "[Asgard-3 (PvP)]");
}

TEST_CASE("ServerListManager: the titles follow the list's texts in the game's language")
{
    ServerListData list = List({Group(0, "Valhalla")});
    list.vipTypes[0].name.Set("es", "Oro");
    list.names.nonPvp.Set("es", "no PvP");
    list.names.Template(ChannelNamePlace::Login).Set("es", "[{name}-{channel} ({pvp} {vip})]");
    TestServerListManager manager;
    manager.SetServerList(list);
    manager.SetSelectServerInfo(0, L"Valhalla", 1, Channel(false, "gold"));

    I18N::SetLocale("es");
    CHECK(manager.GetSelectServerTitle(ChannelNamePlace::Login) == L"[Valhalla-1 (no PvP Oro)]");
    I18N::SetLocale("en");
    CHECK(manager.GetSelectServerTitle(ChannelNamePlace::Login) == L"[Valhalla-1 (Gold Non-PvP)]");
}

TEST_CASE("ServerListManager: description and test server reach the group")
{
    ServerGroup test = Group(9, "Test");
    test.description = "Only for the team";
    test.testServer = true;
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla"), test}));
    manager.InsertServerGroup(ServerId(0, 1), LowLoad);
    manager.InsertServerGroup(ServerId(9, 1), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 2);
    CHECK(groups[0]->m_iWidthPos == CServerGroup::SBP_LEFT);
    CHECK(std::wstring(groups[0]->m_szDescription).empty());
    CHECK(groups[1]->m_iWidthPos == CServerGroup::SBP_CENTER);
    CHECK(std::wstring(groups[1]->m_szDescription) == L"Only for the team");
}

TEST_CASE("ServerListManager: new groups replace the old ones")
{
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla"), Group(1, "Helheim")}));
    manager.SetServerList(List({Group(1, "Asgard")}));

    manager.InsertServerGroup(ServerId(0, 1), LowLoad);
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CHECK(std::wstring(groups[0]->m_szName) == L"Asgard");
}

#ifdef _EDITOR
TEST_CASE("ServerListManager: the editor's preview shows the channels the connect server did not send")
{
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla"), Group(1, "Helheim")}));
    manager.InsertServerGroup(ServerId(0, 3), LowLoad);
    manager.SetOfflinePreview(true, 75);
    manager.RebuildServerGroups();

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 2);
    REQUIRE(groups[0]->GetServerSize() == MAX_SERVER_PER_GROUP);
    REQUIRE(groups[1]->GetServerSize() == MAX_SERVER_PER_GROUP);
    // In channel order: the sent one keeps its load, the others have the preview's and can't be joined.
    for (int channel = 1; channel <= MAX_SERVER_PER_GROUP; ++channel)
    {
        const CServerInfo* server = groups[0]->GetServerInfo(channel - 1);
        REQUIRE(server != nullptr);
        CHECK(server->m_iIndex == channel);
        CHECK(server->m_offline == (channel != 3));
        CHECK(server->m_iPercent == (channel == 3 ? LowLoad : 75));
    }
    CHECK(manager.GetReceivedServers().size() == 1);

    manager.SetOfflinePreview(false, 75);
    manager.RebuildServerGroups();
    const std::vector<CServerGroup*> sent = GroupsInOrder(manager);
    REQUIRE(sent.size() == 1);
    REQUIRE(sent[0]->GetServerSize() == 1);
    CHECK_FALSE(sent[0]->GetServerInfo(0)->m_offline);
}

TEST_CASE("ServerListManager: a rebuild names the servers last sent with the new groups")
{
    I18N::SetLocale("en");
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla")}));
    manager.SetTotalServer(3);
    manager.InsertServerGroup(ServerId(0, 1), LowLoad);
    manager.InsertServerGroup(ServerId(0, 2), LowLoad);
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);
    REQUIRE(manager.GetReceivedServers().size() == 3);

    manager.SetServerList(List({Group(1, "Helheim"), Group(0, "Asgard")}));
    manager.RebuildServerGroups();

    CHECK(manager.GetReceivedServers().size() == 3);
    CHECK(manager.GetTotalServer() == 3);
    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 2);
    CHECK(std::wstring(groups[0]->m_szName) == L"Helheim");
    CHECK(groups[0]->GetServerSize() == 1);
    CHECK(std::wstring(groups[1]->m_szName) == L"Asgard");
    REQUIRE(groups[1]->GetServerSize() == 2);
    CHECK(manager.GetServerButtonName(*groups[1], groups[1]->GetServerInfo(1)->m_iIndex) == "Asgard-2 (PvP)");
}
#endif
