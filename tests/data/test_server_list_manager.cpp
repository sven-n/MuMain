#include "stdafx.h"

#include "doctest.h"

#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"

#include <string>
#include <tuple>
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

// The load the connect server sends; the login screen shows I18N::Game::Connect for it.
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
    for (const auto& [id, name, color] :
         {std::tuple{"gold", "Gold", ThemeColor::Orange}, std::tuple{"silver", "Silver", ThemeColor::Silver}})
    {
        Data::ServerList::VipType vipType;
        vipType.id = id;
        vipType.name.Set("en", name);
        vipType.themeColor = color;
        list.vipTypes.push_back(vipType);
    }
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

std::wstring ButtonText(const wchar_t* channelName)
{
    return std::wstring(channelName) + L" " + I18N::Game::Connect;
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
    CHECK(manager.GetServerButtonText(group, server(0)) == ButtonText(L"Valhalla-1 (Gold Non-PvP)"));
    CHECK(manager.GetServerButtonText(group, server(1)) == ButtonText(L"Valhalla-2 (Gold PvP)"));
    CHECK(manager.GetServerButtonText(group, server(2)) == ButtonText(L"Valhalla-3 (Non-PvP)"));
    CHECK(manager.GetServerButtonText(group, server(3)) == ButtonText(L"Valhalla-4 (PvP)"));
    CHECK(manager.GetServerButtonText(group, server(4)) == ButtonText(L"Valhalla-5 (Silver PvP)"));
    CHECK(manager.GetServerButtonText(group, server(5)) == ButtonText(L"Valhalla-20 (Gold Non-PvP)"));

    CHECK(manager.GetServerButtonColor(server(0)) == ThemeColor::Orange);
    CHECK(manager.GetServerButtonColor(server(2)) == ThemeColor::Default);
    CHECK(manager.GetServerButtonColor(server(3)) == ThemeColor::Gray);
    CHECK(manager.GetServerButtonColor(server(4)) == ThemeColor::Silver);
}

TEST_CASE("ServerListManager: the button names the server's load")
{
    I18N::SetLocale("en");
    TestServerListManager manager;
    manager.SetServerList(List({Group(0, "Valhalla")}));
    manager.InsertServerGroup(ServerId(0, 1), 100);
    manager.InsertServerGroup(ServerId(0, 2), 128);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CHECK(manager.GetServerButtonText(*groups[0], *groups[0]->GetServerInfo(0)) ==
          std::wstring(L"Valhalla-1 (PvP) ") + I18N::Game::Full);
    CHECK(manager.GetServerButtonText(*groups[0], *groups[0]->GetServerInfo(1)) ==
          std::wstring(L"Valhalla-2 (PvP) ") + I18N::Game::NowPreparing);
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
        manager.SetSelectServerInfo(L"Valhalla", 3, channel);
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

    manager.SetSelectServerInfo(L"Valhalla", 3, Channel(false, "gold"));
    CHECK(manager.IsNonPvP());
    manager.SetSelectServerInfo(L"Valhalla", 3, Channel(true, "gold"));
    CHECK_FALSE(manager.IsNonPvP());
}

TEST_CASE("ServerListManager: the titles follow the list's texts in the game's language")
{
    ServerListData list = List({Group(0, "Valhalla")});
    list.vipTypes[0].name.Set("es", "Oro");
    list.names.nonPvp.Set("es", "no PvP");
    list.names.Template(ChannelNamePlace::Login).Set("es", "[{name}-{channel} ({pvp} {vip})]");
    TestServerListManager manager;
    manager.SetServerList(list);
    manager.SetSelectServerInfo(L"Valhalla", 1, Channel(false, "gold"));

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
    CHECK(manager.GetServerButtonText(*groups[1], *groups[1]->GetServerInfo(1)) == ButtonText(L"Asgard-2 (PvP)"));
}
#endif
