// <copyright file="ConnectionManager.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Logging.Abstractions;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;

/// <summary>
/// Class which manages the connections which are created through the game client.
/// To identify each connection, we use handles (a simple number).
/// </summary>
public unsafe partial class ConnectionManager
{
    /// <summary>
    /// How long a connect may take before it counts as failed: two handshake retries on Windows, three on Linux.
    /// </summary>
    private static readonly TimeSpan ConnectTimeout = TimeSpan.FromSeconds(10);

    /// <summary>
    /// The currently active connections, with their handle as key.
    /// </summary>
    private static readonly ConcurrentDictionary<int, ConnectionWrapper> Connections = new();

    /// <summary>
    /// The currently used maximum handle number.
    /// </summary>
    private static int _maxHandle;

    /// <summary>
    /// Registers the native logger used by managed diagnostics.
    /// </summary>
    /// <param name="callback">The native logging callback.</param>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SetLogCallback")]
    public static void SetLogCallback(delegate* unmanaged<byte, byte*, void> callback)
    {
        ManagedLog.SetCallback(callback);
    }

    /// <summary>
    /// Starts connecting to the specified host and port and returns the handle at once. Packets sent with the handle
    /// before the connection opens go out once it does.
    /// </summary>
    /// <param name="hostPtr">The pointer to a string which contains the host (ip or hostname).</param>
    /// <param name="port">The port.</param>
    /// <param name="isEncrypted">
    /// A flag, if the connection is supposed to be encrypted.
    /// This is usually <c>1</c> for connections to the game server, but <c>0</c> for connections to the connect server.
    /// </param>
    /// <param name="onPacketReceived">The pointer to an unmanaged method which is called when a new packet got received.
    /// Parameters: handle, size, pointer to the data.</param>
    /// <param name="onDisconnected">The pointer to an unmanaged method which is called when the connection got disconnected,
    /// or could not be opened. Parameter: handle.</param>
    /// <param name="onOpened">The pointer to an unmanaged method which is called when the connection opened.
    /// Parameter: handle.</param>
    /// <returns>
    /// The handle of the created connection. If negative, no connect could be started.
    /// </returns>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_BeginConnect")]
    public static int BeginConnect(IntPtr hostPtr, int port, byte isEncrypted, delegate* unmanaged<int, int, byte*, void> onPacketReceived, delegate* unmanaged<int, void> onDisconnected, delegate* unmanaged<int, void> onOpened)
    {
        try
        {
            var host = NativeInterop.PtrToWideString(hostPtr) ?? throw new ArgumentNullException(nameof(hostPtr));
            return BeginConnectInner(host, port, isEncrypted == 1, onPacketReceived, onDisconnected, onOpened);
        }
        catch (Exception ex)
        {
            Debug.WriteLine($"Error establishing connection: {ex}");
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Connection failed: {ConnectFailure.Describe(ex)}");
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
                // A send failure means the connection is broken. Tear it down so
                // the Disconnected event fires and the client can auto-reconnect,
                // instead of silently swallowing the error and looking online.
                Debug.WriteLine($"Error sending {count} bytes with handle {handle}: {ex}");
                ManagedLog.Write(ManagedLog.Level.Error, $"NET: Send failed, handle={handle}, bytes={count}: {ex}");
                connection.DisconnectAndDispose();
            }
        }
        else
        {
            Debug.WriteLine("Connection with handle {0} not found.", handle);
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Send skipped; connection handle={handle} not found");
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

    /// <summary>
    /// Forgets a connection that is shutting down, so sends for its handle stop reaching it.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    internal static void Forget(int handle)
    {
        Connections.TryRemove(handle, out _);
    }

    private static int BeginConnectInner(string host, int port, bool isEncrypted, delegate* unmanaged<int, int, byte*, void> onPacketReceived, delegate* unmanaged<int, void> onDisconnected, delegate* unmanaged<int, void> onOpened)
    {
        var pipe = new DeferredTcpPipe();
        var encryptor = isEncrypted ? new PipelinedXor32Encryptor(new PipelinedSimpleModulusEncryptor(pipe.Output, PipelinedSimpleModulusEncryptor.DefaultClientKey).Writer) : null;
        var decryptor = isEncrypted ? new PipelinedSimpleModulusDecryptor(pipe.Input, PipelinedSimpleModulusDecryptor.DefaultClientKey) : null;
        var connection = new Connection(pipe, decryptor, encryptor, new NullLogger<Connection>());
        var handle = Interlocked.Increment(ref _maxHandle);
        var wrapper = new ConnectionWrapper(handle, connection, pipe, onPacketReceived, onDisconnected, onOpened);
        if (!Connections.TryAdd(handle, wrapper))
        {
            wrapper.Dispose();
            throw new InvalidOperationException($"Connection handle {handle} already exists.");
        }

        wrapper.Connect(host, port, ConnectTimeout);
        return handle;
    }
}
