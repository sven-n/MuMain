#pragma once

#include "stdafx.h"
// Flow Code: VS1-NET-CONNECTION-XPLAT
// Story: 3.1.2 - Connection.h Cross-Platform Updates
// Flow Code: VS1-NET-CHAR16T-ENCODING
// Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary

#include <coreclr_delegates.h>
#include <filesystem>

#include "PlatformLibrary.h"

// Forward declarations: Connection only holds/returns pointers to these types.
// Full headers are included by Connection.cpp. Using #include here caused
// include-order failures when Connection.h was pulled transitively through
// WSclient.h into TUs that had already partially processed a PacketFunctions_*
// header via a different path.
class PacketFunctions_ChatServer;
class PacketFunctions_ConnectServer;
class PacketFunctions_ClientToServer;

// MU_DOTNET_LIB_EXT is defined by CMake (FindDotnetAOT.cmake): ".dll" | ".dylib" | ".so"
// MU_DOTNET_LIB_DIR is defined by CMake for UNIX platforms (Linux + macOS) as the absolute
// binary output directory (CMAKE_RUNTIME_OUTPUT_DIRECTORY / TARGET_FILE_DIR:Main).
// On UNIX, dlopen() with a bare filename does NOT search the executable directory, so an
// absolute path is required. On Windows, LoadLibrary() searches the executable directory
// by default, so a bare filename suffices. (Story 3.3.2 Risk R6 mitigation)
// Defined in Connection.cpp (not anonymous namespace) to prevent per-TU copies if ever
// included by a second translation unit. (Story 3.4.1 MEDIUM-4 fix)
//
// SIOF mitigation (Story 3.3.1 MEDIUM-3): both g_dotnetLibPath and
// munique_client_library_handle are declared `extern` here and defined in Connection.cpp,
// after the #include of this header. C++ guarantees initialization order within a single
// translation unit follows definition order, so g_dotnetLibPath is fully initialized
// before munique_client_library_handle calls Load(). An `inline` handle variable would
// initialize at the point of #include "Connection.h" in Connection.cpp — before
// g_dotnetLibPath is defined — causing dlopen("") and a NULL handle.
extern const std::string g_dotnetLibPath;
extern const mu::platform::LibraryHandle munique_client_library_handle;

namespace DotNetBridge
{
// AC-2: Distinguishes library-not-found from symbol-not-found errors.
// Used as second parameter to ReportDotNetError() so callers can be explicit
// about which failure mode occurred (Option A from Dev Notes §Key Design Decisions).
enum class DotNetErrorKind
{
    LibraryNotFound,
    SymbolNotFound
};

void ReportDotNetError(const char* detail, DotNetErrorKind kind = DotNetErrorKind::LibraryNotFound);
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
        ReportDotNetError(name, DotNetErrorKind::SymbolNotFound);
    }

    return symbol;
}
} // namespace DotNetBridge

using DotNetBridge::LoadManagedSymbol;

// Compatibility bridge for XSLT-generated PacketBindings_*.h files and PacketFunctions_Custom.cpp.
inline void* symLoad(mu::platform::LibraryHandle handle, const char* name)
{
    return mu::platform::GetSymbol(handle, name);
}

// Re-resolve .NET packet function pointers that may be NULL due to SIOF (Static Initialization
// Order Fiasco). The inline variables in PacketBindings_*.h call GetSymbol() at static init,
// but the library handle may not be initialized yet depending on linker TU order.
// Must be called early in main() before any packet send operations.
void ResolvePacketBindings();

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
