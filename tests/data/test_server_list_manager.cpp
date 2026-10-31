#include "stdafx.h"

#include "doctest.h"

#include "I18N/All.h"
#include "Network/Server/ServerListManager.h"

#include <string>
#include <vector>

using Data::ServerList::ChannelType;
using Data::ServerList::ServerGroup;

namespace
{
// The shared instance's constructor is protected; each test gets a manager of its own.
class TestServerListManager : public CServerListManager
{
public:
    TestServerListManager() = default;
};

// The load the connect server sends; the login screen shows I18N::Game::Connect for it (InsertServer).
constexpr int LowLoad = 10;

ServerGroup Group(int id, const std::string& name)
{
    ServerGroup group;
    group.id = id;
    group.name = name;
    return group;
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

std::wstring ChannelName(const wchar_t* groupAndChannel)
{
    return std::wstring(groupAndChannel) + L" " + I18N::Game::Connect;
}
} // namespace

TEST_CASE("ServerListManager: the list order is the order of the groups")
{
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{Group(2, "Midgard"), Group(0, "Valhalla"), Group(1, "Helheim")});
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
    manager.SetServerGroups(std::vector<ServerGroup>{Group(0, "Valhalla")});

    manager.InsertServerGroup(ServerId(3, 1), LowLoad);
    CHECK(manager.GetServerGroupSize() == 0);

    manager.InsertServerGroup(ServerId(0, 2), LowLoad);
    CHECK(manager.GetServerGroupSize() == 1);
}

TEST_CASE("ServerListManager: the servers of a group are its channels")
{
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{Group(1, "Helheim")});
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

TEST_CASE("ServerListManager: the channel types reach the channels and their names")
{
    ServerGroup valhalla = Group(0, "Valhalla");
    valhalla.channels[0] = ChannelType::Gold;
    valhalla.channels[1] = ChannelType::GoldPvp;
    valhalla.channels[2] = ChannelType::NonPvp;
    valhalla.channels[19] = ChannelType::Gold;
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{valhalla});
    for (const int channel : {1, 2, 3, 4, 20})
    {
        manager.InsertServerGroup(ServerId(0, channel), LowLoad);
    }

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CServerGroup* group = groups[0];
    for (int channel = 0; channel < MAX_SERVER_PER_GROUP; ++channel)
    {
        CAPTURE(channel);
        CHECK(group->m_abyNonPvpServer[channel] == static_cast<BYTE>(valhalla.channels[channel]));
    }
    // A group with a non-PvP channel shows no PvP notice.
    CHECK_FALSE(group->m_bPvPServer);

    REQUIRE(group->GetServerSize() == 5);
    CHECK(group->GetServerInfo(0)->m_byNonPvP == static_cast<BYTE>(ChannelType::Gold));
    CHECK(std::wstring(group->GetServerInfo(0)->m_bName) == ChannelName(L"Valhalla-1(Gold)"));
    CHECK(std::wstring(group->GetServerInfo(1)->m_bName) == ChannelName(L"Valhalla-2(Gold PVP)"));
    CHECK(std::wstring(group->GetServerInfo(2)->m_bName) == ChannelName(L"Valhalla-3(Non-PVP)"));
    CHECK(std::wstring(group->GetServerInfo(3)->m_bName) == ChannelName(L"Valhalla-4"));
    CHECK(std::wstring(group->GetServerInfo(4)->m_bName) == ChannelName(L"Valhalla-20(Gold)"));
}

TEST_CASE("ServerListManager: a group without a non-PvP channel shows the PvP notice")
{
    ServerGroup helheim = Group(1, "Helheim");
    helheim.channels[0] = ChannelType::GoldPvp;
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{helheim});
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CHECK(groups[0]->m_bPvPServer);
}

TEST_CASE("ServerListManager: description and test server reach the group")
{
    ServerGroup test = Group(9, "Test");
    test.description = "Only for the team";
    test.testServer = true;
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{Group(0, "Valhalla"), test});
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
    manager.SetServerGroups(std::vector<ServerGroup>{Group(0, "Valhalla"), Group(1, "Helheim")});
    manager.SetServerGroups(std::vector<ServerGroup>{Group(1, "Asgard")});

    manager.InsertServerGroup(ServerId(0, 1), LowLoad);
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);

    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 1);
    CHECK(std::wstring(groups[0]->m_szName) == L"Asgard");
}

#ifdef _EDITOR
TEST_CASE("ServerListManager: a rebuild names the servers last sent with the new groups")
{
    TestServerListManager manager;
    manager.SetServerGroups(std::vector<ServerGroup>{Group(0, "Valhalla")});
    manager.SetTotalServer(3);
    manager.InsertServerGroup(ServerId(0, 1), LowLoad);
    manager.InsertServerGroup(ServerId(0, 2), LowLoad);
    manager.InsertServerGroup(ServerId(1, 1), LowLoad);
    REQUIRE(manager.GetReceivedServers().size() == 3);

    manager.SetServerGroups(std::vector<ServerGroup>{Group(1, "Helheim"), Group(0, "Asgard")});
    manager.RebuildServerGroups();

    CHECK(manager.GetReceivedServers().size() == 3);
    CHECK(manager.GetTotalServer() == 3);
    const std::vector<CServerGroup*> groups = GroupsInOrder(manager);
    REQUIRE(groups.size() == 2);
    CHECK(std::wstring(groups[0]->m_szName) == L"Helheim");
    CHECK(groups[0]->GetServerSize() == 1);
    CHECK(std::wstring(groups[1]->m_szName) == L"Asgard");
    REQUIRE(groups[1]->GetServerSize() == 2);
    CHECK(std::wstring(groups[1]->GetServerInfo(1)->m_bName) == ChannelName(L"Asgard-2"));
}
#endif
