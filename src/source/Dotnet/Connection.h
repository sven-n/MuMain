#pragma once

#include "stdafx.h"
// Flow Code: VS1-NET-CONNECTION-XPLAT
// Story: 3.1.2 - Connection.h Cross-Platform Updates
// Flow Code: VS1-NET-CHAR16T-ENCODING
// Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary

#include <coreclr_delegates.h>
#include <filesystem>

#include "Core/Platform/PlatformLibrary.h"

class PacketFunctions_ChatServer;
class PacketFunctions_ConnectServer;
class PacketFunctions_ClientToServer;

// MU_DOTNET_LIB_EXT is defined by CMake (FindDotnetAOT.cmake): ".dll" | ".dylib" | ".so".
// POSIX loaders do not search the executable directory for a bare filename, so resolve the
// library beside the running executable. Windows keeps its native executable-directory search.
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
inline std::string ManagedLibraryPath()
{
    const auto libraryName = "MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT);
#ifdef _WIN32
    return libraryName;
#else
    constexpr DWORD executablePathCapacity = 4096;
    wchar_t executablePath[executablePathCapacity] = {};
    if (GetModuleFileNameW(nullptr, executablePath, executablePathCapacity) == 0)
    {
        return libraryName;
    }

    return (std::filesystem::path(mu_narrow_path(executablePath)).parent_path() / libraryName).string();
#endif
}

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

// Register native services required by the managed bridge.
void InitializeDotNetBridge();

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
    bool _firstPacketLogged = false;

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
