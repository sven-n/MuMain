#pragma once

#include "stdafx.h"
// Flow Code: VS1-NET-CONNECTION-XPLAT
// Story: 3.1.2 - Connection.h Cross-Platform Updates
// Flow Code: VS1-NET-CHAR16T-ENCODING
// Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary

#include <coreclr_delegates.h>
#include <filesystem>

#include "PlatformLibrary.h"
#include "PacketFunctions_ChatServer.h"
#include "PacketFunctions_ConnectServer.h"
#include "PacketFunctions_ClientToServer.h"

namespace
{
// MU_DOTNET_LIB_EXT is defined by CMake (FindDotnetAOT.cmake): ".dll" | ".dylib" | ".so"
// MU_DOTNET_LIB_DIR is defined by CMake for UNIX platforms (Linux + macOS) as the absolute
// binary output directory (CMAKE_RUNTIME_OUTPUT_DIRECTORY / TARGET_FILE_DIR:Main).
// On UNIX, dlopen() with a bare filename does NOT search the executable directory, so an
// absolute path is required. On Windows, LoadLibrary() searches the executable directory
// by default, so a bare filename suffices. (Story 3.3.2 Risk R6 mitigation)
#ifdef MU_DOTNET_LIB_DIR
inline const std::string g_dotnetLibPath =
    (std::filesystem::path(MU_DOTNET_LIB_DIR) / ("MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT))).string();
#else
inline const std::string g_dotnetLibPath =
    (std::filesystem::path("MUnique.Client.Library") += MU_DOTNET_LIB_EXT).string();
#endif
} // namespace

inline const mu::platform::LibraryHandle munique_client_library_handle =
    mu::platform::Load(g_dotnetLibPath.c_str());

namespace DotNetBridge
{
void ReportDotNetError(const char* detail);
bool IsManagedLibraryAvailable();

template <typename T> T LoadManagedSymbol(const char* name)
{
    if (!IsManagedLibraryAvailable())
    {
        return nullptr;
    }

    const auto symbol = reinterpret_cast<T>(mu::platform::GetSymbol(munique_client_library_handle, name));
    if (!symbol)
    {
        ReportDotNetError(name);
    }

    return symbol;
}
} // namespace DotNetBridge

using DotNetBridge::LoadManagedSymbol;

// Compatibility bridge for XSLT-generated PacketBindings_*.h files.
// The generated files call symLoad(handle, "SymbolName"); update GenerateBindingsHeader.xslt
// and regenerate PacketBindings_*.h to emit mu::platform::GetSymbol() directly,
// then remove this function.
inline void* symLoad(mu::platform::LibraryHandle handle, const char* name)
{
    return mu::platform::GetSymbol(handle, name);
}

class Connection
{
private:
    static void OnPacketReceivedS(int32_t handle, int32_t size, BYTE* data);
    static void OnDisconnectedS(int32_t handle);

    PacketFunctions_ChatServer* _chatServer = {};
    PacketFunctions_ConnectServer* _connectServer = {};
    PacketFunctions_ClientToServer* _gameServer = {};

    int32_t _handle;
    void (*_packetHandler)(int32_t, const BYTE*, int32_t);

    void OnDisconnected();
    void OnPacketReceived(const BYTE* data, const int32_t length);

public:
    Connection(const char16_t* host, int32_t port, bool isEncrypted,
               void (*packetHandler)(int32_t, const BYTE*, int32_t));
    ~Connection();

    bool IsConnected();
    void Send(const BYTE* data, const int32_t length);
    void Close();

    int32_t GetHandle() const
    {
        return _handle;
    }

    PacketFunctions_ChatServer* ToChatServer() const
    {
        return _chatServer;
    }
    PacketFunctions_ConnectServer* ToConnectServer() const
    {
        return _connectServer;
    }
    PacketFunctions_ClientToServer* ToGameServer() const
    {
        return _gameServer;
    }
};
