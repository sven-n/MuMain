#pragma once

#include "stdafx.h"
// Flow Code: VS1-NET-CONNECTION-XPLAT
// Story: 3.1.2 - Connection.h Cross-Platform Updates
// Flow Code: VS1-NET-CHAR16T-ENCODING
// Story: 3.2.1 - char16_t Encoding at .NET Interop Boundary

#include <atomic>
#include <coreclr_delegates.h>
#include <memory>

#include "ManagedSymbols.h"
#include "Network/ConnectionLifecycle.h"

class PacketFunctions_ChatServer;
class PacketFunctions_ConnectServer;
class PacketFunctions_ClientToServer;

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

// An export of the network library, from the table InitializeDotNetBridge resolved.
template <typename T> T LoadManagedSymbol(const char* name)
{
    return reinterpret_cast<T>(LookUpManagedSymbol(name));
}
} // namespace DotNetBridge

using DotNetBridge::LoadManagedSymbol;

// Resolves the network library's exports from the target's source, once, before any connection, and registers the
// native services the library calls. A source that lacks a required export turns the network off.
void InitializeDotNetBridge(const DotNetBridge::ManagedSymbolSource& source);

class Connection
{
private:
    static void OnPacketReceivedS(int32_t handle, int32_t size, BYTE* data);
    static void OnDisconnectedS(int32_t handle);
    static void OnOpenedS(int32_t handle);

    std::unique_ptr<PacketFunctions_ChatServer> _chatServer;
    std::unique_ptr<PacketFunctions_ConnectServer> _connectServer;
    std::unique_ptr<PacketFunctions_ClientToServer> _gameServer;

    // Atomic: a packet callback reads it on a thread-pool thread while the game thread clears it.
    std::atomic<int32_t> _handle = 0;
    int32_t _id = 0;
    Network::ConnectionLifecycle _lifecycle;
    void (*_packetHandler)(int32_t, const BYTE*, int32_t);
    bool _receiving = false;
    bool _firstPacketLogged = false;

    void Unregister();
    void Apply(Network::ConnectionEvent event);
    void OnPacketReceived(const BYTE* data, const int32_t length);

public:
    // Starts connecting and returns at once; State() tells how it goes. Sends made while connecting go out once
    // the connection opens. Without startReceiving, what the server sends waits until StartReceiving().
    Connection(const char16_t* host, int32_t port, bool isEncrypted,
               void (*packetHandler)(int32_t, const BYTE*, int32_t), bool startReceiving = true);
    ~Connection();

    // The handle is registered for this object; a copy would unregister it when destroyed.
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    // Changes only on the game thread, as the packet queue delivers the network's events.
    Network::ConnectionState State() const
    {
        return _lifecycle.State();
    }

    bool FailedToOpen() const
    {
        return _lifecycle.FailedToOpen();
    }

    void StartReceiving();
    void Send(const BYTE* data, const int32_t length);
    void Close();

    // Applies an opened or closed event the packet queue delivered for a connection's handle; game thread only.
    // A connection destroyed since then is no longer registered and gets nothing; false then.
    static bool ApplyQueuedEvent(int32_t handle, Network::ConnectionEvent event);

    int32_t GetHandle() const
    {
        return _handle.load();
    }

    // The handle its packets are queued under. Unlike GetHandle() it outlives a close by the
    // server, so what the server sent right before closing (e.g. why) is still handled; Close()
    // ends it, since the client wants nothing more from a connection it closed itself.
    int32_t GetId() const
    {
        return _id;
    }

    PacketFunctions_ChatServer* ToChatServer() const
    {
        return _chatServer.get();
    }
    PacketFunctions_ConnectServer* ToConnectServer() const
    {
        return _connectServer.get();
    }
    PacketFunctions_ClientToServer* ToGameServer() const
    {
        return _gameServer.get();
    }
};
