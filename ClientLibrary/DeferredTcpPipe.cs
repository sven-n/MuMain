// <copyright file="DeferredTcpPipe.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.IO.Pipelines;
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
    /// <summary>
    /// How long a close waits for the server to close its side after the last bytes went out. Closing at once can
    /// reset the connection, and the server then drops what it has not read yet.
    /// </summary>
    private static readonly TimeSpan LingerTime = TimeSpan.FromSeconds(2);

    /// <summary>Idle time, in seconds, before the OS starts sending TCP keep-alive probes.</summary>
    private const int KeepAliveIdleSeconds = 4;

    /// <summary>Interval, in seconds, between TCP keep-alive probes.</summary>
    private const int KeepAliveIntervalSeconds = 2;

    /// <summary>Number of unanswered keep-alive probes before the connection is dropped.</summary>
    private const int KeepAliveRetryCount = 3;

    private readonly Pipe _input = new(new PipeOptions(useSynchronizationContext: false));
    private readonly Pipe _output = new(new PipeOptions(useSynchronizationContext: false));
    private readonly CancellationTokenSource _connectCancel = new();
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
    /// Cancels a connect still running. An open connection closes when its writer completes <see cref="Output"/>:
    /// completing it here could cut off a packet an encryptor is still writing.
    /// </summary>
    public void Dispose()
    {
        if (Interlocked.Exchange(ref this._disposed, 1) != 0)
        {
            return;
        }

        this._connectCancel.Cancel();
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

    private static async Task<Socket> ConnectAsync(string host, int port, CancellationToken cancellationToken)
    {
        var socket = new Socket(SocketType.Stream, ProtocolType.Tcp);
        try
        {
            await socket.ConnectAsync(host, port, cancellationToken).ConfigureAwait(false);
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
