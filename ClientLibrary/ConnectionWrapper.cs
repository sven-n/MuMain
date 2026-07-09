// <copyright file="ConnectionWrapper.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Buffers;
using System.Diagnostics;
using System.IO.Pipelines;
using System.Threading.Tasks;
using MUnique.OpenMU.Network;
using Nito.AsyncEx.Synchronous;

/// <summary>
/// A wrapper for a <see cref="Connection"/>.
/// </summary>
public sealed class ConnectionWrapper : IDisposable
{
    private readonly int _handle;
    private readonly Connection _connection;

    /// <summary>
    /// The unmanaged callback to a packet handler. Parameters:
    ///   - handle
    ///   - packet size
    ///   - pointer to packet.
    /// </summary>
    private readonly unsafe delegate* unmanaged<int, int, byte*, void> _onPacketReceived;

    /// <summary>
    /// The unmanaged callback to a disconnect handler. Parameter: handle.
    /// </summary>
    private readonly unsafe delegate* unmanaged<int, void> _onDisconnected;

    private volatile bool _isDisposed;

    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectionWrapper"/> class.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="connection">The connection.</param>
    /// <param name="onPacketReceived">
    /// The pointer to an unmanaged method which is called when a new packet got received.
    /// Parameters: handle, size, pointer to the data.
    /// </param>
    /// <param name="onDisconnected">
    /// The pointer to an unmanaged method which is called when the connection got disconnected.
    /// Parameter: handle.
    /// </param>
    public unsafe ConnectionWrapper(int handle, Connection connection, delegate* unmanaged<int, int, byte*, void> onPacketReceived, delegate* unmanaged<int, void> onDisconnected)
    {
        this._handle = handle;
        this._connection = connection;
        this._onPacketReceived = onPacketReceived;
        this._onDisconnected = onDisconnected;

        connection.PacketReceived += this.OnPacketReceivedAsync;
        connection.Disconnected += this.OnDisconnectedAsync;
    }

    /// <summary>
    /// Gets the output pipe writer.
    /// </summary>
    internal PipeWriter Output => this._connection.Output;

    /// <summary>
    /// Begins receiving packets from the client.
    /// </summary>
    public void BeginReceive()
    {
        // we never want it on the main thread, so we do a Task.Run.
        _ = Task.Run(this._connection.BeginReceiveAsync);
    }

    /// <inheritdoc />
    public void Dispose()
    {
        if (this._isDisposed)
        {
            return;
        }

        this._isDisposed = true;
        this._connection.Dispose();
    }

    /// <summary>
    /// Disconnects the connection.
    /// </summary>
    public void DisconnectAndDispose()
    {
        _ = Task.Run(async () =>
        {
            try
            {
                await this._connection.DisconnectAsync();
                this._connection.Dispose();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
        });
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="bytes">The bytes.</param>
    public void Send(Span<byte> bytes)
    {
        if (this._isDisposed)
        {
            Console.Error.WriteLine($"[NET] Send called on disposed connection handle={this._handle}");
            return;
        }

        using var l = this._connection.OutputLock.Lock();
        var targetSpan = this._connection.Output.GetSpan(bytes.Length);

        bytes.CopyTo(targetSpan);
        this._connection.Output.Advance(bytes.Length);
        this._connection.Output.FlushAsync().AsTask().WaitAndUnwrapException();
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="packetFactory">The factory which creates the packet and returns the length of it.</param>
    public void CreateAndSend(Func<PipeWriter, int> packetFactory)
    {
        if (this._isDisposed)
        {
            Console.Error.WriteLine($"[NET] CreateAndSend called on disposed connection handle={this._handle}");
            return;
        }

        // Diagnostic: null checks for properties that could cause SIGSEGV in Native AOT.
        if (this._connection.OutputLock is null)
        {
            Console.Error.WriteLine($"[NET] CreateAndSend: OutputLock is null, handle={this._handle}");
            return;
        }

        if (this._connection.Output is null)
        {
            Console.Error.WriteLine($"[NET] CreateAndSend: Output (PipeWriter) is null, handle={this._handle}");
            return;
        }

        Console.Error.WriteLine($"[NET] CreateAndSend: about to lock+send, handle={this._handle}");
        using var l = this._connection.OutputLock.Lock();
        var length = packetFactory(this._connection.Output);
        this._connection.Output.Advance(length);
        this._connection.Output.FlushAsync().AsTask().WaitAndUnwrapException();
        Console.Error.WriteLine($"[NET] CreateAndSend: sent {length} bytes, handle={this._handle}");
    }

    private unsafe ValueTask OnPacketReceivedAsync(ReadOnlySequence<byte> args)
    {
        if (this._isDisposed)
        {
            Trace.WriteLine($"[NET] OnPacketReceivedAsync called on disposed connection handle={this._handle}");
            return ValueTask.CompletedTask;
        }

        using var memoryOwner = MemoryPool<byte>.Shared.Rent((int)args.Length);
        var packet = memoryOwner.Memory.Slice(0, (int)args.Length);
        args.CopyTo(packet.Span);

        fixed (byte* packetPtr = &packet.Span.GetPinnableReference())
        {
            try
            {
                this._onPacketReceived(this._handle, packet.Length, packetPtr);
            }
            catch (Exception ex)
            {
                Trace.WriteLine($"[NET] Error in OnPacketReceivedAsync handle={this._handle}: {ex}");
            }
        }

        return ValueTask.CompletedTask;
    }

    private unsafe ValueTask OnDisconnectedAsync()
    {
        try
        {
            Trace.WriteLine($"[NET] Connection disconnected, handle={this._handle}");
            this._onDisconnected(this._handle);
            this.Dispose();
        }
        catch (Exception ex)
        {
            Trace.WriteLine($"[NET] Error in OnDisconnectedAsync handle={this._handle}: {ex}");
        }

        return ValueTask.CompletedTask;
    }
}