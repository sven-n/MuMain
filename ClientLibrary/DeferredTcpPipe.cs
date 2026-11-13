// <copyright file="DeferredTcpPipe.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Collections.Generic;
using System.IO.Pipelines;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

/// <summary>
/// A TCP connection that can be written to before it is open: the pipes exist at once, the socket connects in the
/// background, and what was written meanwhile goes out once it opens. A connect that fails completes
/// <see cref="Input"/> with the error, as a connection the server closes would.
/// </summary>
internal sealed class DeferredTcpPipe : IDuplexPipe, IDisposable
{
    /// <summary>Idle time, in seconds, before the OS starts sending TCP keep-alive probes.</summary>
    private const int KeepAliveIdleSeconds = 4;

    /// <summary>Interval, in seconds, between TCP keep-alive probes.</summary>
    private const int KeepAliveIntervalSeconds = 2;

    /// <summary>Number of unanswered keep-alive probes before the connection is dropped.</summary>
    private const int KeepAliveRetryCount = 3;

    /// <summary>
    /// How long a close waits for the server to close its side after the last bytes went out. Closing at once can
    /// reset the connection, and the server then drops what it has not read yet.
    /// </summary>
    private static readonly TimeSpan LingerTime = TimeSpan.FromSeconds(2);

    /// <summary>
    /// How long a disposed connection may still take to send what is queued and linger. A server that stopped reading
    /// would otherwise hold the socket for as long as it stays up.
    /// </summary>
    private static readonly TimeSpan CloseTime = TimeSpan.FromSeconds(5);

    /// <summary>
    /// How long one address gets before the next one is tried alongside it (RFC 8305's recommended 250 ms).
    /// </summary>
    private static readonly TimeSpan AttemptDelay = TimeSpan.FromMilliseconds(250);

    private readonly Pipe _input = new(new PipeOptions(useSynchronizationContext: false));
    private readonly Pipe _output = new(new PipeOptions(useSynchronizationContext: false));
    private readonly CancellationTokenSource _connectCancel = new();
    private readonly CancellationTokenSource _closeDeadline = new();
    private int _disposed;

    /// <inheritdoc />
    public PipeReader Input => this._input.Reader;

    /// <inheritdoc />
    public PipeWriter Output => this._output.Writer;

    /// <summary>
    /// Starts connecting in the background and returns at once.
    /// </summary>
    /// <param name="host">The host name or address.</param>
    /// <param name="port">The port.</param>
    /// <param name="timeout">How long the whole connect may take.</param>
    /// <param name="onOpened">Called once the socket is connected.</param>
    /// <param name="onFailed">Called when it could not connect; <see cref="Input"/> then ends with the error.</param>
    public void Start(string host, int port, TimeSpan timeout, Action onOpened, Action<Exception> onFailed)
    {
        _ = Task.Run(() => this.RunAsync(host, port, timeout, onOpened, onFailed));
    }

    /// <summary>
    /// Cancels a connect still running, and gives an open connection <see cref="CloseTime"/> to finish closing before
    /// its socket is closed. It closes when its writer completes <see cref="Output"/>: completing it here could cut
    /// off a packet an encryptor is still writing.
    /// </summary>
    public void Dispose()
    {
        if (Interlocked.Exchange(ref this._disposed, 1) != 0)
        {
            return;
        }

        this._connectCancel.Cancel();
        this._closeDeadline.CancelAfter(CloseTime);
    }

    /// <summary>
    /// Sets the options a game connection wants: small packets go out at once, and a peer that vanished is noticed
    /// within about ten seconds instead of minutes.
    /// </summary>
    /// <param name="socket">The connected socket.</param>
    internal static void ConfigureSocket(Socket socket)
    {
        socket.NoDelay = true;
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Debug, $"NET: Failed to enable SO_KEEPALIVE: {ex}");
        }

        // Each set on its own, so one the platform lacks doesn't skip the others.
        TrySetSocketOption(socket, SocketOptionName.TcpKeepAliveTime, KeepAliveIdleSeconds);
        TrySetSocketOption(socket, SocketOptionName.TcpKeepAliveInterval, KeepAliveIntervalSeconds);
        TrySetSocketOption(socket, SocketOptionName.TcpKeepAliveRetryCount, KeepAliveRetryCount);

        // Windows sometimes ignores the options above; SIO_KEEPALIVE_VALS sets idle time and interval reliably.
        try
        {
            if (OperatingSystem.IsWindows())
            {
                var values = new byte[12];
                BitConverter.GetBytes(1u).CopyTo(values, 0);
                BitConverter.GetBytes((uint)(KeepAliveIdleSeconds * 1000)).CopyTo(values, 4);
                BitConverter.GetBytes((uint)(KeepAliveIntervalSeconds * 1000)).CopyTo(values, 8);
                socket.IOControl(IOControlCode.KeepAliveValues, values, null);
            }
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Debug, $"NET: Failed to configure keep-alive via IOControl: {ex}");
        }
    }

    /// <summary>
    /// Connects to the first of the host's addresses that answers, trying them side by side as RFC 8305 ("Happy
    /// Eyeballs") describes: the families alternate in the resolver's order, and each next attempt starts
    /// <see cref="AttemptDelay"/> after the previous one, or as soon as it fails. A host whose IPv6 address doesn't
    /// answer (<c>localhost</c> with an IPv4-only server: Windows retries a refused connect for about 2 s) then
    /// connects over IPv4 within a fraction of a second.
    /// </summary>
    /// <param name="host">The host name or address.</param>
    /// <param name="port">The port.</param>
    /// <param name="cancellationToken">Ends the whole attempt.</param>
    /// <returns>The connected socket.</returns>
    internal static async Task<Socket> ConnectAsync(string host, int port, CancellationToken cancellationToken)
    {
        IPAddress[] addresses = IPAddress.TryParse(host, out var literal)
            ? new[] { literal }
            : await Dns.GetHostAddressesAsync(host, cancellationToken).ConfigureAwait(false);
        var ordered = Interleave(addresses);
        if (ordered.Count == 0)
        {
            throw new SocketException((int)SocketError.HostNotFound);
        }

        using var race = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        var attempts = new List<Task<Socket>>();
        try
        {
            Exception? lastError = null;
            var next = 0;
            while (true)
            {
                cancellationToken.ThrowIfCancellationRequested();
                if (next < ordered.Count)
                {
                    attempts.Add(ConnectOneAsync(ordered[next++], port, race.Token));
                }

                if (attempts.Count == 0)
                {
                    throw lastError ?? new SocketException((int)SocketError.HostUnreachable);
                }

                var waitForNext = next < ordered.Count ? Task.Delay(AttemptDelay, race.Token) : Task.Delay(Timeout.Infinite, race.Token);
                var finished = await Task.WhenAny(attempts.Append<Task>(waitForNext)).ConfigureAwait(false);
                if (finished is not Task<Socket> attempt)
                {
                    continue;
                }

                attempts.Remove(attempt);
                if (attempt.IsCompletedSuccessfully)
                {
                    return await attempt.ConfigureAwait(false);
                }

                lastError = attempt.Exception?.InnerException ?? lastError;
            }
        }
        finally
        {
            // The attempts still running end with the race; one that connected anyway is closed.
            await race.CancelAsync().ConfigureAwait(false);
            foreach (var loser in attempts)
            {
                _ = loser.ContinueWith(
                    attempt =>
                    {
                        if (attempt.IsCompletedSuccessfully)
                        {
                            attempt.Result.Dispose();
                        }
                        else
                        {
                            _ = attempt.Exception;
                        }
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Default);
            }
        }
    }

    /// <summary>
    /// The addresses with their families alternating, starting with the family the resolver put first.
    /// </summary>
    /// <param name="addresses">The addresses in the resolver's order.</param>
    /// <returns>The order to try them in.</returns>
    internal static List<IPAddress> Interleave(IReadOnlyList<IPAddress> addresses)
    {
        if (addresses.Count == 0)
        {
            return new List<IPAddress>();
        }

        var first = addresses[0].AddressFamily;
        var preferred = new Queue<IPAddress>(addresses.Where(a => a.AddressFamily == first));
        var other = new Queue<IPAddress>(addresses.Where(a => a.AddressFamily != first));
        var ordered = new List<IPAddress>(addresses.Count);
        while (preferred.Count > 0 || other.Count > 0)
        {
            if (preferred.Count > 0)
            {
                ordered.Add(preferred.Dequeue());
            }

            if (other.Count > 0)
            {
                ordered.Add(other.Dequeue());
            }
        }

        return ordered;
    }

    private static void TrySetSocketOption(Socket socket, SocketOptionName option, int value)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Tcp, option, value);
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Debug, $"NET: Failed to set socket option {option}: {ex}");
        }
    }

    private static async Task<Socket> ConnectOneAsync(IPAddress address, int port, CancellationToken cancellationToken)
    {
        var socket = new Socket(address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
        try
        {
            await socket.ConnectAsync(new IPEndPoint(address, port), cancellationToken).ConfigureAwait(false);
            return socket;
        }
        catch
        {
            socket.Dispose();
            throw;
        }
    }

    private async Task RunAsync(string host, int port, TimeSpan timeout, Action onOpened, Action<Exception> onFailed)
    {
        Socket socket;
        using (var attempt = CancellationTokenSource.CreateLinkedTokenSource(this._connectCancel.Token))
        {
            try
            {
                attempt.CancelAfter(timeout);
                socket = await ConnectAsync(host, port, attempt.Token).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                var error = ex is OperationCanceledException && !this._connectCancel.IsCancellationRequested
                    ? new SocketException((int)SocketError.TimedOut)
                    : ex;
                await this.FailAsync(error, onFailed).ConfigureAwait(false);
                return;
            }
        }

        if (this._connectCancel.IsCancellationRequested)
        {
            socket.Dispose();
            await this.FailAsync(new OperationCanceledException(), onFailed).ConfigureAwait(false);
            return;
        }

        ConfigureSocket(socket);
        onOpened();
        using var stream = new NetworkStream(socket, ownsSocket: true);

        // Closing the socket ends a send or receive still pending past the deadline.
        using var deadline = this._closeDeadline.Token.Register(socket.Dispose);
        var receiving = this.ReceiveAsync(stream);
        await this.SendAsync(stream, socket).ConfigureAwait(false);

        // Our side is closed (or broken): give the server its chance to close too, then let go of the socket.
        await Task.WhenAny(receiving, Task.Delay(LingerTime)).ConfigureAwait(false);
        socket.Dispose();
        await receiving.ConfigureAwait(false);
    }

    private async Task FailAsync(Exception error, Action<Exception> onFailed)
    {
        long unsent = 0;
        if (this._output.Reader.TryRead(out var pending))
        {
            unsent = pending.Buffer.Length;
            this._output.Reader.AdvanceTo(pending.Buffer.End);
        }

        if (!this._connectCancel.IsCancellationRequested)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Connection failed: {ConnectFailure.Describe(error)}");
            if (unsent > 0)
            {
                ManagedLog.Write(ManagedLog.Level.Error, $"NET: {unsent} bytes written before the connection opened were not sent");
            }
        }

        await this._output.Reader.CompleteAsync().ConfigureAwait(false);
        await this._input.Writer.CompleteAsync(error).ConfigureAwait(false);
        onFailed(error);
    }

    // Sends until the output is completed (our close), then shuts the sending side so the server sees the end.
    private async Task SendAsync(NetworkStream stream, Socket socket)
    {
        Exception? error = null;
        try
        {
            await this._output.Reader.CopyToAsync(stream).ConfigureAwait(false);
            socket.Shutdown(SocketShutdown.Send);
        }
        catch (Exception ex)
        {
            error = ex;
        }

        await this._output.Reader.CompleteAsync(error).ConfigureAwait(false);
    }

    // Receives until the server closes or the socket fails. Once the reader is gone (our close), it keeps reading and
    // drops the bytes, so unread data doesn't turn the close into a reset.
    private async Task ReceiveAsync(NetworkStream stream)
    {
        Exception? error = null;
        var readerGone = false;
        var discard = new byte[4096];
        try
        {
            while (true)
            {
                if (readerGone)
                {
                    if (await stream.ReadAsync(discard).ConfigureAwait(false) == 0)
                    {
                        break;
                    }

                    continue;
                }

                var memory = this._input.Writer.GetMemory(8192);
                var read = await stream.ReadAsync(memory).ConfigureAwait(false);
                if (read == 0)
                {
                    break;
                }

                this._input.Writer.Advance(read);
                var flush = await this._input.Writer.FlushAsync().ConfigureAwait(false);
                readerGone = flush.IsCompleted;
            }
        }
        catch (Exception ex)
        {
            error = ex;
        }

        await this._input.Writer.CompleteAsync(error).ConfigureAwait(false);
    }
}
