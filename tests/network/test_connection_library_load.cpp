#include "doctest.h"

#include "Core/Platform/PlatformLibrary.h"

TEST_CASE("Connection library loader handles missing libraries [network][dotnet]")
{
    const auto handle = mu::platform::Load("NonExistent.Client.Library.xyz");
    CHECK(handle == nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Connect") == nullptr);
    mu::platform::Unload(handle);
}
