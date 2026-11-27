#include "stdafx.h"
#include "doctest.h"

#include "MUHelper/MuHelperData.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>

TEST_CASE("Helper data keeps the bytes the client does not use [data][helper]")
{
    PRECEIVE_MUHELPER_DATA received{};
    for (size_t i = 0; i < std::size(received._UnusedPadding); ++i)
    {
        received._UnusedPadding[i] = static_cast<BYTE>(0xA0 + i);
    }

    MUHelper::ConfigData config;
    MUHelper::ConfigDataSerDe::Deserialize(received, config);
    config.bUseCombo = true;

    PRECEIVE_MUHELPER_DATA saved{};
    MUHelper::ConfigDataSerDe::Serialize(config, saved);

    CHECK(std::equal(std::begin(saved._UnusedPadding), std::end(saved._UnusedPadding),
                     std::begin(received._UnusedPadding)));
}

TEST_CASE("Helper data of a new configuration leaves the unused bytes empty [data][helper]")
{
    PRECEIVE_MUHELPER_DATA saved;
    std::memset(&saved, 0xFF, sizeof(saved));

    MUHelper::ConfigDataSerDe::Serialize(MUHelper::ConfigData{}, saved);

    CHECK(std::all_of(std::begin(saved._UnusedPadding), std::end(saved._UnusedPadding),
                      [](BYTE value) { return value == 0; }));
}

// OpenMU stores the minimum socket count of its pickup option in byte 30 of the data, the first unused one here.
TEST_CASE("Helper data keeps the server's setting in byte 30 [data][helper]")
{
    constexpr size_t ServerSettingByte = 30;
    constexpr BYTE MinimumSockets = 3;
    static_assert(offsetof(PRECEIVE_MUHELPER_DATA, _UnusedPadding) == ServerSettingByte);
    PRECEIVE_MUHELPER_DATA received{};
    reinterpret_cast<BYTE*>(&received)[ServerSettingByte] = MinimumSockets;

    MUHelper::ConfigData config;
    MUHelper::ConfigDataSerDe::Deserialize(received, config);
    PRECEIVE_MUHELPER_DATA saved{};
    MUHelper::ConfigDataSerDe::Serialize(config, saved);

    CHECK(reinterpret_cast<const BYTE*>(&saved)[ServerSettingByte] == MinimumSockets);
}
