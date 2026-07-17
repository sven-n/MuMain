// <copyright file="ConnectionWrapper.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Buffers;
using System.Diagnostics;
using System.IO;
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
    private readonly OutboundPacketSender _outboundSender;
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
        this._outboundSender = new OutboundPacketSender(this.SendPacketAsync, this.OnSendFailed);

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
        if (this._isDisposed)
        {
            return;
        }

        this._isDisposed = true;
        this._outboundSender.Complete();
        this._connection.Dispose();
    }

    /// <summary>
    /// Disconnects the connection.
    /// </summary>
    public void DisconnectAndDispose()
    {
        if (Interlocked.Exchange(ref this._isDisconnecting, 1) != 0)
        {
            return;
        }

        _ = Task.Run(async () =>
        {
            try
            {
                await this._outboundSender.CompleteAsync().ConfigureAwait(false);
                await this._connection.DisconnectAsync().ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
            finally
            {
                this._isDisposed = true;
                this._connection.Dispose();
            }
        });
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="bytes">The bytes.</param>
    public void Send(Span<byte> bytes)
    {
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
        {
            Console.Error.WriteLine($"[NET] Send called on disposed connection handle={this._handle}");
            return;
        }

        this.Enqueue(bytes.ToArray());
    }

    /// <summary>
    /// Sends the specified bytes.
    /// </summary>
    /// <param name="packetFactory">The factory which creates the packet and returns the length of it.</param>
    public void CreateAndSend(Func<PipeWriter, int> packetFactory)
    {
        if (this._isDisposed || Volatile.Read(ref this._isDisconnecting) != 0)
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

        var packetWriter = new PacketPipeWriter();
        var length = packetFactory(packetWriter);
        packetWriter.Advance(length);
        this.Enqueue(packetWriter.ToArray(length));
    }

    private void Enqueue(byte[] packet)
    {
        if (!this._outboundSender.TryEnqueue(packet))
        {
            this.OnSendFailed(new InvalidOperationException($"Outbound packet queue is full, handle={this._handle}"));
        }
    }

    private async ValueTask SendPacketAsync(ReadOnlyMemory<byte> packet)
    {
        var lockStarted = Stopwatch.GetTimestamp();
        using var outputLock = await this._connection.OutputLock.LockAsync().ConfigureAwait(false);
        var lockElapsed = Stopwatch.GetElapsedTime(lockStarted);
        packet.Span.CopyTo(this._connection.Output.GetSpan(packet.Length));
        this._connection.Output.Advance(packet.Length);
        var flushStarted = Stopwatch.GetTimestamp();
        await this._connection.Output.FlushAsync().ConfigureAwait(false);
        var flushElapsed = Stopwatch.GetElapsedTime(flushStarted);
        this.WriteOutboundDiagnostic(packet.Length, lockElapsed, flushElapsed);
    }

    private void WriteOutboundDiagnostic(int packetLength, TimeSpan lockElapsed, TimeSpan flushElapsed)
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
        Console.Error.WriteLine($"[OutboundQueue] handle={this._handle} depth={this._outboundSender.PendingCount} high={this._outboundSender.HighWaterMark} bytes={packetLength} lock={lockElapsed.TotalMilliseconds:F1}ms flush={flushElapsed.TotalMilliseconds:F1}ms");
    }

    private void OnSendFailed(Exception exception)
    {
        Console.Error.WriteLine($"[NET] Send failed, handle={this._handle}: {exception}");
        this.DisconnectAndDispose();
    }

    private async Task RunReceiveLoopAsync()
    {
        try
        {
            await this.WriteNetworkDiagnosticAsync($"Receive loop started, handle={this._handle}").ConfigureAwait(false);
            await this._connection.BeginReceiveAsync().ConfigureAwait(false);
            await this.WriteNetworkDiagnosticAsync($"Receive loop ended, handle={this._handle}; remote peer closed the connection").ConfigureAwait(false);
        }
        catch (Exception ex)
        {
            await this.WriteNetworkDiagnosticAsync($"Receive loop failed, handle={this._handle}: {ex}").ConfigureAwait(false);
        }
    }

    private async Task WriteNetworkDiagnosticAsync(string message)
    {
        var logLine = $"{DateTimeOffset.Now:O} [NET] {message}{Environment.NewLine}";
        await File.AppendAllTextAsync(Path.Combine(Path.GetTempPath(), "MuNetwork.log"), logLine).ConfigureAwait(false);
        await Console.Error.WriteAsync(logLine).ConfigureAwait(false);
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

    private async ValueTask OnDisconnectedAsync()
    {
        try
        {
            Trace.WriteLine($"[NET] Connection disconnected, handle={this._handle}");
            await this.WriteNetworkDiagnosticAsync($"Disconnected event, handle={this._handle}").ConfigureAwait(false);
            this.NotifyDisconnected();
            this.Dispose();
        }
        catch (Exception ex)
        {
            Trace.WriteLine($"[NET] Error in OnDisconnectedAsync handle={this._handle}: {ex}");
        }
    }

    private unsafe void NotifyDisconnected()
    {
        this._onDisconnected(this._handle);
    }
}
