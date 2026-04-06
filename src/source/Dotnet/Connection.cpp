// Flow Code: VS1-NET-ERROR-MESSAGING
// Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
//
// AC-4 reserved message template (not triggered in this story):
//   "Server handshake failed. Check OpenMU version compatibility."
// AC-5 reserved message template (not triggered in this story):
//   "Login failed. Check credentials."

#include "stdafx.h"
#include <map>

#include "Connection.h"
#include "DotNetMessageFormat.h"

// Full definitions required: Connection.cpp allocates these types (new PacketFunctions_*()).
// Connection.h uses only forward declarations to avoid include-order failures in TUs
// that pull it in transitively through WSclient.h.
#include "PacketFunctions_ChatServer.h"
#include "PacketFunctions_ConnectServer.h"
#include "PacketFunctions_ClientToServer.h"

#include "PacketBindings_ChatServer.h"
#include "PacketBindings_ConnectServer.h"
#include "PacketBindings_ClientToServer.h"

// MEDIUM-4 fix: g_dotnetLibPath defined here (not in anonymous namespace in header) to prevent
// per-TU copies if Connection.h is ever included by a second translation unit.
// Declaration (extern) remains in Connection.h; only Connection.cpp includes Connection.h today.
#ifdef MU_DOTNET_LIB_DIR
const std::string g_dotnetLibPath =
    (std::filesystem::path(MU_DOTNET_LIB_DIR) / ("MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT))).string();
#else
const std::string g_dotnetLibPath =
    (std::filesystem::path("MUnique.Client.Library") += MU_DOTNET_LIB_EXT).string();
#endif

// Defined after g_dotnetLibPath so C++ TU initialization order guarantees the path is
// fully constructed before Load() is called. (SIOF fix — was inline in Connection.h)
const mu::platform::LibraryHandle munique_client_library_handle =
    mu::platform::Load(g_dotnetLibPath.c_str());

std::map<int32_t, Connection*> connections;

namespace DotNetBridge
{
bool g_dotnetErrorDisplayed = false;

// AC-1 + AC-2 + AC-7: Structured error reporting for DotNet bridge failures.
// Distinguishes library-not-found vs symbol-not-found via DotNetErrorKind.
// Writes to BOTH g_ErrorReport (persistent MuError.log) AND MessageBoxW dialog
// (mapped to SDL_ShowSimpleMessageBox via PlatformCompat.h shim).
// AC-STD-NFR-1: Dialog shown at most ONCE per session via g_dotnetErrorDisplayed guard.
// AC-STD-NFR-2: Called from main game thread only (single-threaded game loop).
void ReportDotNetError(const char* detail, DotNetErrorKind kind)
{
    if (g_dotnetErrorDisplayed)
    {
        return;
    }
    g_dotnetErrorDisplayed = true;

    // Determine platform name for AC-1 message (compile-time, impl file only — acceptable
    // per Dev Notes §Key Design Decisions: diagnostic string in implementation file).
    const char* platformName =
#if defined(__linux__)
        "Linux";
#elif defined(__APPLE__)
        "macOS";
#else
        "Windows";
#endif

    std::string msg;
    if (kind == DotNetErrorKind::LibraryNotFound)
    {
        // AC-1: Library not found — include resolved path and platform
        msg = FormatLibraryNotFoundMessage(g_dotnetLibPath, platformName);
    }
    else
    {
        // AC-2: Symbol not found — include function name
        msg = FormatSymbolNotFoundMessage(detail);
    }

    // Write to MuError.log (persistent diagnostic — g_ErrorReport)
    g_ErrorReport.Write(L"NET: %hs\r\n", msg.c_str());

    // Show user-visible dialog via MessageBoxW shim → SDL_ShowSimpleMessageBox (PlatformCompat.h)
    // ASCII-safe conversion: diagnostic messages contain only ASCII characters
    std::wstring wideMsg(msg.begin(), msg.end());
    MessageBoxW(nullptr, wideMsg.c_str(), L"Network Error", MB_ICONERROR | MB_OK);
}

bool IsManagedLibraryAvailable()
{
    if (munique_client_library_handle)
    {
        return true;
    }

    // AC-1: Pass library path; ReportDotNetError will use g_dotnetLibPath and platform name
    ReportDotNetError(nullptr, DotNetErrorKind::LibraryNotFound);
    return false;
}
} // namespace DotNetBridge

using DotNetBridge::DotNetErrorKind;
using DotNetBridge::IsManagedLibraryAvailable;
using DotNetBridge::ReportDotNetError;

using onPacketReceived = void(int32_t, int32_t, BYTE*);
using onDisconnected = void(int32_t);

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* Connect)(const char16_t*, int32_t, BYTE, onPacketReceived, onDisconnected);
typedef void(CORECLR_DELEGATE_CALLTYPE* Disconnect)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* BeginReceive)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* Send)(int32_t, const BYTE*, int32_t);

Connect dotnet_connect = LoadManagedSymbol<Connect>("ConnectionManager_Connect");

Disconnect dotnet_disconnect = LoadManagedSymbol<Disconnect>("ConnectionManager_Disconnect");

BeginReceive dotnet_beginreceive = LoadManagedSymbol<BeginReceive>("ConnectionManager_BeginReceive");

Send dotnet_send = LoadManagedSymbol<Send>("ConnectionManager_Send");

void Connection::OnPacketReceivedS(const int32_t handle, const int32_t size, BYTE* data)
{
    const auto it = connections.find(handle);
    if (it == connections.end())
    {
        return;
    }

    if (Connection* connection = it->second)
    {
        connection->OnPacketReceived(data, size);
    }
}

void Connection::OnDisconnectedS(const int32_t handle)
{
    const auto it = connections.find(handle);
    if (it == connections.end())
    {
        return;
    }

    if (Connection* connection = it->second)
    {
        connection->OnDisconnected();
    }
}

Connection::Connection(const char16_t* host, int32_t port, bool isEncrypted,
                       void (*packetHandler)(int32_t, const BYTE*, int32_t))
{
    this->_packetHandler = packetHandler;
    if (!dotnet_connect)
    {
        ReportDotNetError("ConnectionManager_Connect", DotNetErrorKind::SymbolNotFound);
        this->_handle = 0;
        return;
    }

    this->_handle = dotnet_connect(host, port, isEncrypted ? 1 : 0, &OnPacketReceivedS, &OnDisconnectedS);

    g_ErrorReport.Write(L"NET: dotnet_connect returned handle=%d (encrypted=%d)\r\n",
                        this->_handle, isEncrypted ? 1 : 0);

    if (IsConnected())
    {
        connections[this->_handle] = this;
        if (dotnet_beginreceive)
        {
            dotnet_beginreceive(this->_handle);
            g_ErrorReport.Write(L"NET: BeginReceive started for handle=%d\r\n", this->_handle);
        }

        // cppcheck-suppress [noCopyConstructor, noOperatorEq]
        _chatServer = new PacketFunctions_ChatServer();
        _connectServer = new PacketFunctions_ConnectServer();
        _gameServer = new PacketFunctions_ClientToServer();

        _chatServer->SetHandle(this->_handle);
        _connectServer->SetHandle(this->_handle);
        _gameServer->SetHandle(this->_handle);
    }
}

Connection::~Connection()
{
    if (!IsConnected())
    {
        return;
    }

    if (dotnet_disconnect)
    {
        dotnet_disconnect(_handle);
    }

    SAFE_DELETE(_chatServer);
    SAFE_DELETE(_connectServer);
    SAFE_DELETE(_gameServer);
}

bool Connection::IsConnected()
{
    return this->_handle > 0;
}

void Connection::Send(const BYTE* data, const int32_t size)
{
    if (!IsConnected())
    {
        return;
    }

    if (!dotnet_send)
    {
        ReportDotNetError("ConnectionManager_Send", DotNetErrorKind::SymbolNotFound);
        return;
    }

    dotnet_send(this->_handle, data, size);
}

void Connection::Close()
{
    if (!IsConnected())
    {
        return;
    }

    if (dotnet_disconnect)
    {
        dotnet_disconnect(this->_handle);
    }
}

void Connection::OnDisconnected()
{
    if (!IsConnected())
    {
        g_ErrorReport.Write(L"NET: OnDisconnected called but already disconnected\r\n");
        return;
    }

    g_ErrorReport.Write(L"NET: OnDisconnected — handle=%d, erasing from connection map\r\n", this->_handle);
    connections.erase(this->_handle);
    this->_handle = 0;
}

void Connection::OnPacketReceived(const BYTE* data, const int32_t size)
{
    // Diagnostic: log first packet to confirm .NET→C++ callback path is working.
    static bool s_firstPacketLogged = false;
    if (!s_firstPacketLogged)
    {
        g_ErrorReport.Write(L"NET: First packet received — handle=%d size=%d (callback path working)\r\n",
                            this->_handle, size);
        s_firstPacketLogged = true;
    }
    this->_packetHandler(this->_handle, data, size);
}