#include "doctest.h"

#include "Core/Platform/ServerPort.h"

TEST_CASE("Server port parser validates full range [core][network]")
{
    WORD port = 44405;
    CHECK(Core::Platform::ParseServerPort(L"1", port));
    CHECK(port == 1);
    CHECK(Core::Platform::ParseServerPort(L"65535", port));
    CHECK(port == 65535);
    CHECK_FALSE(Core::Platform::ParseServerPort(L"0", port));
    CHECK_FALSE(Core::Platform::ParseServerPort(L"65536", port));
    CHECK_FALSE(Core::Platform::ParseServerPort(L"44405x", port));
    CHECK_FALSE(Core::Platform::ParseServerPort(L"", port));
}
