#include "stdafx.h"
#include <map>

#include "Connection.h"
#include "DotNetRuntime.h"

std::map<int32_t, Connection*> connections;

using onPacketReceived = void(int32_t, int32_t, BYTE*);
using onDisconnected = void(int32_t);

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* Connect)(const char*, int32_t, onPacketReceived, onDisconnected);
typedef void(CORECLR_DELEGATE_CALLTYPE* Disconnect)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* BeginReceive)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* Send)(int32_t, const BYTE*, int32_t);

char_t* type_name = L"MUnique.Client.ManagedLibrary.ConnectionManager, MUnique.Client.ManagedLibrary";
Connect dotnet_connect = reinterpret_cast<Connect>(
    g_dotnet->get_method(
        type_name,
    L"Connect"));

Disconnect dotnet_disconnect = reinterpret_cast<Disconnect>(
    g_dotnet->get_method(
        type_name,
        L"Disconnect"));

BeginReceive dotnet_beginreceive = reinterpret_cast<BeginReceive>(
    g_dotnet->get_method(
        type_name,
        L"BeginReceive"));

Send dotnet_send = reinterpret_cast<Send>(
    g_dotnet->get_method(
        type_name,
        L"Send"));


void Connection::OnPacketReceivedS(const int32_t handle, const int32_t size, BYTE* data)
{
    Connection* connection = connections.at(handle);
    if (connection != nullptr)
    {
        connection->OnPacketReceived(data, size);
    }
}

void Connection::OnDisconnectedS(const int32_t handle)
{
    Connection* connection = connections.at(handle);
    if (connection != nullptr)
    {
        connection->OnDisconnected();
    }
}

Connection::Connection(const char* host, int32_t port, void(*packetHandler)(int32_t, const BYTE*, int32_t))
{
	this->_packetHandler = packetHandler;
	this->_handle = dotnet_connect(host, port, &OnPacketReceivedS, &OnDisconnectedS);
    
    if (IsConnected())
    {
        connections[this->_handle] = this;
        dotnet_beginreceive(this->_handle);
    }
}

Connection::~Connection()
{
    if (!IsConnected())
    {
	    return;
    }

    dotnet_disconnect(_handle);
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

    dotnet_send(this->_handle, data, size);
}

void Connection::Close()
{
    if (!IsConnected())
    {
        return;
    }

    dotnet_disconnect(this->_handle);
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
    printf("Received packet, size %d", size);
    this->_packetHandler(this->_handle, data, size);
}

