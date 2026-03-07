#include "stdafx.h"
#include <map>

#include "Connection.h"

#include "PacketBindings_ChatServer.h"
#include "PacketBindings_ConnectServer.h"
#include "PacketBindings_ClientToServer.h"

std::map<int32_t, Connection*> connections;

namespace DotNetBridge
{
bool g_dotnetErrorDisplayed = false;

void ReportDotNetError(const char* detail)
{
    if (g_dotnetErrorDisplayed)
    {
        return;
    }
    g_dotnetErrorDisplayed = true;
    g_ErrorReport.Write(L"NET: Connection \u2014 library load failed: %hs\r\n", detail ? detail : "unknown error");
}

bool IsManagedLibraryAvailable()
{
    if (munique_client_library_handle)
    {
        return true;
    }

    const std::string libName = std::string("MUnique.Client.Library") + MU_DOTNET_LIB_EXT + " missing";
    ReportDotNetError(libName.c_str());
    return false;
}
} // namespace DotNetBridge

using DotNetBridge::IsManagedLibraryAvailable;
using DotNetBridge::ReportDotNetError;

using onPacketReceived = void(int32_t, int32_t, BYTE*);
using onDisconnected = void(int32_t);

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* Connect)(const wchar_t*, int32_t, BYTE, onPacketReceived, onDisconnected);
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

Connection::Connection(const wchar_t* host, int32_t port, bool isEncrypted,
                       void (*packetHandler)(int32_t, const BYTE*, int32_t))
{
    this->_packetHandler = packetHandler;
    if (!dotnet_connect)
    {
        ReportDotNetError("ConnectionManager_Connect");
        this->_handle = 0;
        return;
    }

    this->_handle = dotnet_connect(host, port, isEncrypted ? 1 : 0, &OnPacketReceivedS, &OnDisconnectedS);

    if (IsConnected())
    {
        connections[this->_handle] = this;
        if (dotnet_beginreceive)
        {
            dotnet_beginreceive(this->_handle);
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
        ReportDotNetError("ConnectionManager_Send");
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
        return;
    }

    connections.erase(this->_handle);
    this->_handle = 0;
}

void Connection::OnPacketReceived(const BYTE* data, const int32_t size)
{
    this->_packetHandler(this->_handle, data, size);
}