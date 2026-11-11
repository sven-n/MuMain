// <copyright file="ConnectionWrapper.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Buffers;
using System.Diagnostics;
using System.IO.Pipelines;
using System.Threading;
using System.Threading.Tasks;
using MUnique.OpenMU.Network;

/// <summary>
/// A wrapper for a <see cref="Connection"/>.
/// </summary>
public sealed class ConnectionWrapper : IDisposable
{
    private readonly int _handle;
    private readonly Connection _connection;
    private readonly OutboundFlushLoop _outboundFlushLoop;
    private readonly bool _networkDiagnosticsEnabled = Environment.GetEnvironmentVariable("MU_NETWORK_DIAGNOSTICS") == "1";

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
    private int _isDisconnecting;
    private long _lastOutboundDiagnosticTimestamp;

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
        this._outboundFlushLoop = new OutboundFlushLoop(this.FlushOutputAsync, this.OnSendFailed);

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
        _ = Task.Run(this.RunReceiveLoopAsync);
    }

    /// <inheritdoc />
    public void Dispose()
    {
        this.BeginShutdown(false);
    }

    /// <summary>
    /// Disconnects the connection.
    /// </summary>
    public void DisconnectAndDispose()
    {
        this.BeginShutdown(true);
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="bytes">The bytes.</param>
    public void Send(Span<byte> bytes)
    {
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Send called on disposed connection handle={this._handle}");
            return;
        }

        using var outputLock = this._connection.OutputLock.Lock();
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
        {
            return;
        }

        bytes.CopyTo(this._connection.Output.GetSpan(bytes.Length));
        this._connection.Output.Advance(bytes.Length);
        this._outboundFlushLoop.RequestFlush();
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="packetFactory">The factory which creates the packet and returns the length of it.</param>
    public void CreateAndSend(Func<PipeWriter, int> packetFactory)
    {
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: CreateAndSend called on disposed connection handle={this._handle}");
            return;
        }

        using var outputLock = this._connection.OutputLock.Lock();
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
        {
            return;
        }

        var length = packetFactory(this._connection.Output);
        this._connection.Output.Advance(length);
        this._outboundFlushLoop.RequestFlush();
    }

    private async ValueTask FlushOutputAsync()
    {
        var lockStarted = Stopwatch.GetTimestamp();
        using var outputLock = await this._connection.OutputLock.LockAsync().ConfigureAwait(false);
        var lockElapsed = Stopwatch.GetElapsedTime(lockStarted);
        var flushStarted = Stopwatch.GetTimestamp();
        await this._connection.Output.FlushAsync().ConfigureAwait(false);
        var flushElapsed = Stopwatch.GetElapsedTime(flushStarted);
        this.WriteOutboundDiagnostic(lockElapsed, flushElapsed);
    }

    private void WriteOutboundDiagnostic(TimeSpan lockElapsed, TimeSpan flushElapsed)
    {
        if (!this._networkDiagnosticsEnabled)
        {
            return;
        }

        var now = Stopwatch.GetTimestamp();
        var previous = Volatile.Read(ref this._lastOutboundDiagnosticTimestamp);
        if (lockElapsed < TimeSpan.FromMilliseconds(100)
            && flushElapsed < TimeSpan.FromMilliseconds(100)
            && previous != 0
            && Stopwatch.GetElapsedTime(previous, now) < TimeSpan.FromSeconds(5))
        {
            return;
        }

        Volatile.Write(ref this._lastOutboundDiagnosticTimestamp, now);
        ManagedLog.Write(
            ManagedLog.Level.Info,
            $"NET: Outbound flush handle={this._handle} lock={lockElapsed.TotalMilliseconds:F1}ms flush={flushElapsed.TotalMilliseconds:F1}ms");
    }

    private void OnSendFailed(Exception exception)
    {
        ManagedLog.Write(ManagedLog.Level.Error, $"NET: Send failed, handle={this._handle}: {exception}");
        this.DisconnectAndDispose();
    }

    private void BeginShutdown(bool disconnect)
    {
        if (Interlocked.Exchange(ref this._isDisconnecting, 1) != 0)
        {
            return;
        }

        this._isDisposed = true;
        _ = Task.Run(() => this.ShutdownAsync(disconnect));
    }

    private async Task ShutdownAsync(bool disconnect)
    {
        try
        {
            using (await this._connection.OutputLock.LockAsync().ConfigureAwait(false))
            {
                this._outboundFlushLoop.Stop();
            }

            await this._outboundFlushLoop.CompleteAsync().ConfigureAwait(false);
            if (disconnect)
            {
                await this._connection.DisconnectAsync().ConfigureAwait(false);
            }
        }
        catch (Exception ex)
        {
            Debug.WriteLine(ex);
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Shutdown failed, handle={this._handle}: {ex}");
        }
        finally
        {
            this._connection.Dispose();
        }
    }

    private async Task RunReceiveLoopAsync()
    {
        try
        {
            this.WriteNetworkDiagnostic($"Receive loop started, handle={this._handle}");
            await this._connection.BeginReceiveAsync().ConfigureAwait(false);
            this.WriteNetworkDiagnostic($"Receive loop ended, handle={this._handle}; remote peer closed the connection");
        }
        catch (Exception ex)
        {
            this.WriteNetworkDiagnostic($"Receive loop failed, handle={this._handle}: {ex}");
        }
    }

    private void WriteNetworkDiagnostic(string message)
    {
        if (!this._networkDiagnosticsEnabled)
        {
            return;
        }

        ManagedLog.Write(ManagedLog.Level.Info, $"NET: {message}");
    }

    private unsafe ValueTask OnPacketReceivedAsync(ReadOnlySequence<byte> args)
    {
        if (this._isDisposed)
        {
            Trace.WriteLine($"[NET] OnPacketReceivedAsync called on disposed connection handle={this._handle}");
            ManagedLog.Write(ManagedLog.Level.Debug, $"NET: Packet received on disposed connection handle={this._handle}");
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
                ManagedLog.Write(ManagedLog.Level.Error, $"NET: Packet callback failed, handle={this._handle}: {ex}");
            }
        }

        return ValueTask.CompletedTask;
    }

    private async ValueTask OnDisconnectedAsync()
    {
        try
        {
            Trace.WriteLine($"[NET] Connection disconnected, handle={this._handle}");
            this.WriteNetworkDiagnostic($"Disconnected event, handle={this._handle}");
            this.NotifyDisconnected();
            this.Dispose();
        }
        catch (Exception ex)
        {
            Trace.WriteLine($"[NET] Error in OnDisconnectedAsync handle={this._handle}: {ex}");
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Disconnect callback failed, handle={this._handle}: {ex}");
        }
    }

    private unsafe void NotifyDisconnected()
    {
        this._onDisconnected(this._handle);
    }
}
