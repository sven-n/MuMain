#include "doctest.h"

#include "Dotnet/Connection.h"
#include "Dotnet/ManagedSymbols.h"
#include "Dotnet/PacketFunctions_ChatServer.h"

#include <cstring>

// This binary is built without Dotnet/SymbolSources/ManagedLibrarySource.cpp: it is a target with an adapter of its
// own, as a port would have.
namespace
{
int32_t g_keepAliveHandle = -1;
int g_otherExport = 0;

void CORECLR_DELEGATE_CALLTYPE SendKeepAliveStub(int32_t handle)
{
    g_keepAliveHandle = handle;
}

void CORECLR_DELEGATE_CALLTYPE SetLogCallbackStub(void(CORECLR_DELEGATE_CALLTYPE*)(BYTE, const char*)) {}

void* FindInFake(const char* name)
{
    if (std::strcmp(name, "SendKeepAlive") == 0)
    {
        return reinterpret_cast<void*>(&SendKeepAliveStub);
    }
    if (std::strcmp(name, "ConnectionManager_SetLogCallback") == 0)
    {
        return reinterpret_cast<void*>(&SetLogCallbackStub);
    }
    // The bridge calls the version export at startup when there is one; this fake has none.
    if (std::strcmp(name, "ConnectionManager_GetLibraryVersion") == 0)
    {
        return nullptr;
    }
    return &g_otherExport;
}
} // namespace

DotNetBridge::ManagedSymbolSource DotNetBridge::TargetManagedSymbolSource()
{
    return {"the test's fake exports", &FindInFake};
}

TEST_CASE("A target without the library adapter starts the bridge from a source of its own [network][dotnet]")
{
    InitializeDotNetBridge(DotNetBridge::TargetManagedSymbolSource());
    REQUIRE(DotNetBridge::ProcessManagedSymbols().State() == DotNetBridge::ManagedSymbolsState::Resolved);

    PacketFunctions_ChatServer chat;
    chat.SetHandle(7);
    chat.SendKeepAlive();

    CHECK(g_keepAliveHandle == 7);
}
