#pragma once

#include "stdafx.h"

#include <coreclr_delegates.h>

#include "PacketFunctions_ChatServer.h"
#include "PacketFunctions_ConnectServer.h"
#include "PacketFunctions_ClientToServer.h"

#include <cwchar>

#ifdef _WIN32
#include "windows.h"
#define symLoad GetProcAddress
#else
#include "dlfcn.h"
#define symLoad dlsym
#endif

#ifdef _WIN32
inline const HINSTANCE munique_client_library_handle = LoadLibrary(L"MUnique.Client.Library.dll");
#else
inline const void* munique_client_library_handle = dlopen("MUnique.Client.Library.dll", RTLD_LAZY);
#endif

namespace DotNetBridge
{
void ReportDotNetError(const char* detail);
bool IsManagedLibraryAvailable();

template<typename T>
T LoadManagedSymbol(const char* name)
{
    if (!IsManagedLibraryAvailable())
    {
        return nullptr;
    }

    const auto symbol = reinterpret_cast<T>(symLoad(munique_client_library_handle, name));
    if (!symbol)
    {
        ReportDotNetError(name);
    }

    return symbol;
}
}

using DotNetBridge::LoadManagedSymbol;

class Connection
{
private:
    static void OnPacketReceivedS(int32_t handle, int32_t size, BYTE* data);
    static void OnDisconnectedS(int32_t handle);

    PacketFunctions_ChatServer* _chatServer = { };
    PacketFunctions_ConnectServer* _connectServer = { };
    PacketFunctions_ClientToServer* _gameServer = { };

    int32_t _handle;
    void(*_packetHandler)(int32_t, const BYTE*, int32_t);

    void OnDisconnected();
    void OnPacketReceived(const BYTE* data, const int32_t length);

public:
    Connection(const wchar_t* host, int32_t port, bool isEncrypted, void(*packetHandler)(int32_t, const BYTE*, int32_t));
    ~Connection();

    bool IsConnected();
    void Send(const BYTE* data, const int32_t length);
    void Close();

    int32_t GetHandle() const { return _handle; }

    PacketFunctions_ChatServer* ToChatServer() const { return _chatServer; }
    PacketFunctions_ConnectServer* ToConnectServer() const { return _connectServer; }
    PacketFunctions_ClientToServer* ToGameServer() const { return _gameServer; }
};
