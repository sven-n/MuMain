#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ServerListData/ServerListJson.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// One-time check that Data/Local/ServerList.json holds what ServerList.bmd
// held: the bmd is read here the way CServerListManager::LoadServerListScript
// read it, and each group is compared with what the JSON gives the login screen.
namespace
{
constexpr int BmdNameLength = 32;
constexpr int BmdChannelCount = 15;
constexpr int BmdDescriptionCapacity = 1024;

#pragma pack(push, 1)
struct BmdServerGroup
{
    WORD index;
    char name[BmdNameLength];
    BYTE position;
    BYTE sequence;
    BYTE nonPvp[BmdChannelCount];
    short descriptionLength;
};
#pragma pack(pop)

struct OldGroup
{
    std::string name;
    BYTE position = 0;
    BYTE sequence = 0;
    BYTE nonPvp[BmdChannelCount] = {};
};

std::map<int, OldGroup> ReadBmd(const std::filesystem::path& file)
{
    std::map<int, OldGroup> groups;
    FILE* fp = std::fopen(file.string().c_str(), "rb");
    REQUIRE(fp != nullptr);
    BmdServerGroup entry{};
    char description[BmdDescriptionCapacity];
    while (std::fread(&entry, sizeof(entry), 1, fp) != 0)
    {
        BuxConvert(reinterpret_cast<BYTE*>(&entry), sizeof(entry));
        std::fread(description, entry.descriptionLength, 1, fp);
        OldGroup group;
        group.name.assign(entry.name, strnlen(entry.name, BmdNameLength));
        group.position = entry.position;
        group.sequence = entry.sequence;
        std::memcpy(group.nonPvp, entry.nonPvp, sizeof(group.nonPvp));
        groups.insert({entry.index, group});
    }
    std::fclose(fp);
    return groups;
}
} // namespace

TEST_CASE("ServerList.json holds what ServerList.bmd held")
{
    const std::filesystem::path local = std::filesystem::path(MU_TEST_DATA_DIR) / "Local";
    const std::map<int, OldGroup> old = ReadBmd(local / "ServerList.bmd");
    const Data::ServerList::ServerListLoadResult loaded =
        Data::ServerList::LoadServerListFile(local / "ServerList.json");

    REQUIRE(loaded.issues.empty());
    REQUIRE(loaded.groups.size() == old.size());
    for (size_t index = 0; index < loaded.groups.size(); ++index)
    {
        const Data::ServerList::ServerGroup& group = loaded.groups[index];
        CAPTURE(group.id);
        const auto found = old.find(group.id);
        REQUIRE(found != old.end());
        const OldGroup& before = found->second;
        CHECK(group.name == before.name);
        // The login screen orders the groups by their sequence; the JSON by the list.
        CHECK(static_cast<int>(index + 1) == before.sequence);
        // Only the center position (test server) is drawn apart.
        CHECK(group.testServer == (before.position == 2));
        for (int channel = 0; channel < Data::ServerList::MaxChannelsPerGroup; ++channel)
        {
            CAPTURE(channel);
            // The old loader set the channels after the 15th to 0.
            const BYTE expected = channel < BmdChannelCount ? before.nonPvp[channel] : 0;
            CHECK(static_cast<BYTE>(group.channels[channel]) == expected);
        }
        // The old loader read the description but never kept it: the login
        // screen showed none.
        CHECK(group.description.empty());
    }
}
