// <copyright file="ConnectionManager.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging.Abstractions;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;
using Pipelines.Sockets.Unofficial;

/// <summary>
/// Class which manages the connections which are created through the game client.
/// To identify each connection, we use handles (a simple number).
/// </summary>
public unsafe partial class ConnectionManager
{
    /// <summary>
    /// The currently active connections, with their handle as key.
    /// </summary>
    private static readonly Dictionary<int, ConnectionWrapper> Connections = new();

    /// <summary>
    /// The currently used maximum handle number.
    /// </summary>
    private static int _maxHandle;

    /// <summary>
    /// Connects the specified host and port.
    /// </summary>
    /// <param name="hostPtr">The pointer to a string which contains the host (ip or hostname).</param>
    /// <param name="port">The port.</param>
    /// <param name="isEncrypted">
    /// A flag, if the connection is supposed to be encrypted.
    /// This is usually <c>1</c> for connections to the game server, but <c>0</c> for connections to the connect server.
    /// </param>
    /// <param name="onPacketReceived">The pointer to an unmanaged method which is called when a new packet got received.
    /// Parameters: handle, size, pointer to the data.</param>
    /// <param name="onDisconnected">The pointer to an unmanaged method which is called when the connection got disconnected.
    /// Parameter: handle.</param>
    /// <returns>
    /// The handle of the created connection. If negative, the connection couldn't be established.
    /// </returns>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_Connect")]
    public static int Connect(IntPtr hostPtr, int port, byte isEncrypted, delegate* unmanaged<int, int, byte*, void> onPacketReceived, delegate* unmanaged<int, void> onDisconnected)
    {
        try
        {
            var host = Marshal.PtrToStringAuto(hostPtr) ?? throw new ArgumentNullException(nameof(hostPtr));
            return ConnectInner(host, port, isEncrypted == 1, onPacketReceived, onDisconnected);
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Error establishing connection: {ex}");
            return -1;
        }
    }

    /// <summary>
    /// Sends a packet over the connection of the specified handle.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="data">The pointer to the packet data.</param>
    /// <param name="count">The count of bytes which should be sent.</param>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_Send")]
    public static void Send(int handle, byte* data, int count)
    {
        if (Connections.TryGetValue(handle, out var connection))
        {
            try
            {
                var bytes = new Span<byte>(data, count);
                bytes.SetPacketSize();
                connection.Send(bytes);
                Debug.WriteLine("Sent {0} bytes with handle {1}", count, handle);
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Error sending {0} bytes with handle {1}: {2}", count, handle, ex);
            }
        }
        else
        {
            Debug.WriteLine("Connection with handle {0} not found.", handle);
        }
    }

    /// <summary>
    /// Begins receiving data for the connection of the specified handle.
    /// </summary>
    /// <param name="connectionHandle">The handle of the connection.</param>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_BeginReceive")]
    public static void BeginReceive(int connectionHandle)
    {
        if (Connections.TryGetValue(connectionHandle, out var connection))
        {
            connection.BeginReceive();
        }
    }

    /// <summary>
    /// Disconnects the connection of the specified handle.
    /// </summary>
    /// <param name="connectionHandle">The handle of the connection.</param>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_Disconnect")]
    public static void Disconnect(int connectionHandle)
    {
        if (Connections.TryGetValue(connectionHandle, out var connection))
        {
            connection.DisconnectAndDispose();
        }
    }

    private static int ConnectInner(string host, int port, bool isEncrypted, delegate* unmanaged<int, int, byte*, void> onPacketReceived, delegate* unmanaged<int, void> onDisconnected)
    {
        var tcpClient = new TcpClient(host, port);

        var socketConnection = SocketConnection.Create(tcpClient.Client);

        var encryptor = isEncrypted ? new PipelinedXor32Encryptor(new PipelinedSimpleModulusEncryptor(socketConnection.Output, PipelinedSimpleModulusEncryptor.DefaultClientKey).Writer) : null;
        var decryptor = isEncrypted ? new PipelinedSimpleModulusDecryptor(socketConnection.Input, PipelinedSimpleModulusDecryptor.DefaultClientKey) : null;
        var connection = new Connection(socketConnection, decryptor, encryptor, new NullLogger<Connection>());

        var handle = Interlocked.Increment(ref _maxHandle);
        var wrapper = new ConnectionWrapper(handle, connection, onPacketReceived, onDisconnected);
        Connections.Add(handle, wrapper);

        connection.Disconnected += () =>
        {
            Connections.Remove(handle);
            return ValueTask.CompletedTask;
        };

        return handle;
    }
}