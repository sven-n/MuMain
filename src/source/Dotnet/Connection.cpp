// Flow Code: VS1-NET-ERROR-MESSAGING
// Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
//
// AC-4 reserved message template (not triggered in this story):
//   "Server handshake failed. Check OpenMU version compatibility."
// AC-5 reserved message template (not triggered in this story):
//   "Login failed. Check credentials."

#include "stdafx.h"

#include "Connection.h"
#include "DotNetMessageFormat.h"
#include "HandleRegistry.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Network/EndpointForLog.h"

// Full definitions required: Connection.cpp allocates and frees these types (unique_ptr members).
// Connection.h uses only forward declarations to avoid include-order failures in TUs
// that pull it in transitively through WSclient.h.
#include "PacketFunctions_ChatServer.h"
#include "PacketFunctions_ConnectServer.h"
#include "PacketFunctions_ClientToServer.h"

// MEDIUM-4 fix: g_dotnetLibPath defined here (not in anonymous namespace in header) to prevent
// per-TU copies if Connection.h is ever included by a second translation unit.
// Declaration (extern) remains in Connection.h; only Connection.cpp includes Connection.h today.
const std::string g_dotnetLibPath = ManagedLibraryPath();

// Defined after g_dotnetLibPath so C++ TU initialization order guarantees the path is
// fully constructed before Load() is called. (SIOF fix — was inline in Connection.h)
const mu::platform::LibraryHandle munique_client_library_handle =
    mu::platform::Load(g_dotnetLibPath.c_str());

namespace
{
enum class ManagedLogLevel : BYTE
{
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

void CORECLR_DELEGATE_CALLTYPE WriteManagedLog(BYTE level, const char* message)
{
    const auto logger = mu::log::Get("dotnet");
    const char* safeMessage = message != nullptr ? message : "<null managed log message>";
    switch (static_cast<ManagedLogLevel>(level))
    {
    case ManagedLogLevel::Debug:
        logger->debug("{}", safeMessage);
        break;
    case ManagedLogLevel::Info:
        logger->info("{}", safeMessage);
        break;
    case ManagedLogLevel::Warning:
        logger->warn("{}", safeMessage);
        break;
    case ManagedLogLevel::Error:
        logger->error("{}", safeMessage);
        break;
    case ManagedLogLevel::Critical:
        logger->critical("{}", safeMessage);
        break;
    }
}

using ManagedLogCallback = void(CORECLR_DELEGATE_CALLTYPE*)(BYTE, const char*);
using SetManagedLogCallback = void(CORECLR_DELEGATE_CALLTYPE*)(ManagedLogCallback);
using GetManagedLibraryVersion = const char*(CORECLR_DELEGATE_CALLTYPE*)();

// In a release download the library's commit is the client's, so a different one means a library from another
// download. (A local build rebuilds the library only when its sources change, so there it can be older.)
// Looked up with FindSymbol, not LoadManagedSymbol: a library from before this function is no error worth a dialog
// or an error line in the log.
void WriteLibraryVersion()
{
    const auto getLibraryVersion = reinterpret_cast<GetManagedLibraryVersion>(
        mu::platform::FindSymbol(munique_client_library_handle, "ConnectionManager_GetLibraryVersion"));
    const char* version = getLibraryVersion != nullptr ? getLibraryVersion() : nullptr;
    g_ErrorReport.Write(L"ClientLibrary \t\t: %hs\r\n",
                        version != nullptr ? version : "version unknown (older library)");
}

} // anonymous namespace

void InitializeDotNetBridge()
{
    if (!munique_client_library_handle)
    {
        mu::log::Get("dotnet")->error("NET: InitializeDotNetBridge -- library not loaded, skipping");
        g_ErrorReport.Write(L"ClientLibrary \t\t: not loaded\r\n");
        return;
    }

    WriteLibraryVersion();
    const auto setManagedLogCallback =
        LoadManagedSymbol<SetManagedLogCallback>("ConnectionManager_SetLogCallback");
    if (setManagedLogCallback != nullptr)
    {
        setManagedLogCallback(&WriteManagedLog);
    }
}

namespace
{
// The open connections by handle. The managed library calls back on thread-pool threads; see
// HandleRegistry for how that stays away from a Connection being destroyed.
//
// Never destroyed, like IncomingPacketQueue::Instance(): the game connection is still open when the
// process exits, and a callback that arrives while the statics are torn down must still find a
// working registry.
DotNetBridge::HandleRegistry<Connection>& Connections()
{
    static auto* const registry = new DotNetBridge::HandleRegistry<Connection>();
    return *registry;
}
} // anonymous namespace

namespace DotNetBridge
{
bool g_dotnetErrorDisplayed = false;

// AC-1 + AC-2 + AC-7: Structured error reporting for DotNet bridge failures.
// Distinguishes library-not-found vs symbol-not-found via DotNetErrorKind.
// Writes to BOTH spdlog (persistent MuError.log) AND MessageBoxW dialog
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

    // Write to MuError.log (persistent diagnostic — spdlog)
    mu::log::Get("dotnet")->error("NET: {}", msg);

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

namespace
{
using onPacketReceived = void(int32_t, int32_t, BYTE*);
using onDisconnected = void(int32_t);

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* Connect)(const char16_t*, int32_t, BYTE, onPacketReceived, onDisconnected);
typedef void(CORECLR_DELEGATE_CALLTYPE* Disconnect)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* BeginReceive)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* Send)(int32_t, const BYTE*, int32_t);

Connect GetConnect()
{
    static const auto function = LoadManagedSymbol<Connect>("ConnectionManager_Connect");
    return function;
}

Disconnect GetDisconnect()
{
    static const auto function = LoadManagedSymbol<Disconnect>("ConnectionManager_Disconnect");
    return function;
}

BeginReceive GetBeginReceive()
{
    static const auto function = LoadManagedSymbol<BeginReceive>("ConnectionManager_BeginReceive");
    return function;
}

Send GetSend()
{
    static const auto function = LoadManagedSymbol<Send>("ConnectionManager_Send");
    return function;
}
} // anonymous namespace

void Connection::OnPacketReceivedS(const int32_t handle, const int32_t size, BYTE* data)
{
    Connections().Dispatch(handle, [data, size](Connection& connection) { connection.OnPacketReceived(data, size); });
}

void Connection::OnDisconnectedS(const int32_t handle)
{
    Connections().Dispatch(handle, [](Connection& connection) { connection.OnDisconnected(); });
}

Connection::Connection(const char16_t* host, int32_t port, bool isEncrypted,
                       void (*packetHandler)(int32_t, const BYTE*, int32_t))
{
    this->_packetHandler = packetHandler;
    const auto connect = GetConnect();
    if (!connect)
    {
        this->_handle = 0;
        return;
    }

    // Before connecting, so a failure the library logs follows the address it was for.
    mu::log::Get("dotnet")->info("NET: connecting to {} (encrypted={})",
                                 Network::DescribeEndpointForLog(std::u16string_view(host ? host : u""), port),
                                 isEncrypted ? 1 : 0);
    const int32_t handle = connect(host, port, isEncrypted ? 1 : 0, &OnPacketReceivedS, &OnDisconnectedS);
    this->_handle = handle;

    mu::log::Get("dotnet")->info("NET: dotnet_connect returned handle={} (encrypted={})", handle, isEncrypted ? 1 : 0);

    if (IsConnected())
    {
        this->_id = handle;

        _chatServer = std::make_unique<PacketFunctions_ChatServer>();
        _connectServer = std::make_unique<PacketFunctions_ConnectServer>();
        _gameServer = std::make_unique<PacketFunctions_ClientToServer>();

        _chatServer->SetHandle(handle);
        _connectServer->SetHandle(handle);
        _gameServer->SetHandle(handle);

        // Register only once the object is complete: callbacks can reach it from here on.
        Connections().Add(handle, this);
        if (const auto beginReceive = GetBeginReceive())
        {
            beginReceive(handle);
            mu::log::Get("dotnet")->info("NET: BeginReceive started for handle={}", handle);
        }
    }
}

Connection::~Connection()
{
    // Unregister before disconnecting: this waits for a callback running on this connection, and
    // the disconnect callback the managed side sends later finds no connection.
    Unregister();
    Close();
}

bool Connection::IsConnected()
{
    return this->_handle > 0;
}

void Connection::Send(const BYTE* data, const int32_t size)
{
    const int32_t handle = this->_handle;
    if (handle <= 0)
    {
        return;
    }

    const auto send = GetSend();
    if (!send)
    {
        return;
    }

    send(handle, data, size);
}

void Connection::Close()
{
    this->_id = 0;
    const int32_t handle = this->_handle;
    if (handle <= 0)
    {
        return;
    }

    if (const auto disconnect = GetDisconnect())
    {
        disconnect(handle);
    }
}

void Connection::Unregister()
{
    Connections().Remove(this->_handle);
}

void Connection::OnDisconnected()
{
    if (!IsConnected())
    {
        mu::log::Get("dotnet")->info("NET: OnDisconnected called but already disconnected");
        return;
    }

    mu::log::Get("dotnet")->info("NET: OnDisconnected -- handle={}, erasing from connection map", GetHandle());
    Unregister();
    this->_handle = 0;
}

void Connection::OnPacketReceived(const BYTE* data, const int32_t size)
{
    const int32_t handle = this->_handle;
    if (!this->_firstPacketLogged)
    {
        mu::log::Get("dotnet")->info("NET: First packet received -- handle={} size={} (callback path working)", handle,
                                     size);
        this->_firstPacketLogged = true;
    }
    this->_packetHandler(handle, data, size);
}
