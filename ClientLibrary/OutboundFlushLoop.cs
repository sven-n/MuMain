// <copyright file="OutboundFlushLoop.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Threading.Channels;
using System.Threading.Tasks;

/// <summary>
/// Coalesces output-pipe flush requests onto one background loop.
/// </summary>
internal sealed class OutboundFlushLoop : IAsyncDisposable
{
    private readonly Channel<bool> _requests = Channel.CreateBounded<bool>(new BoundedChannelOptions(1)
    {
        FullMode = BoundedChannelFullMode.DropWrite,
        SingleReader = true,
        SingleWriter = false,
    });
    private readonly Func<ValueTask> _flushAsync;
    private readonly Action<Exception>? _onFailure;
    private readonly Task _flushLoop;

    /// <summary>
    /// Initializes a new instance of the <see cref="OutboundFlushLoop"/> class.
    /// </summary>
    /// <param name="flushAsync">The serialized flush operation.</param>
    /// <param name="onFailure">The optional flush failure callback.</param>
    internal OutboundFlushLoop(Func<ValueTask> flushAsync, Action<Exception>? onFailure = null)
    {
        this._flushAsync = flushAsync;
        this._onFailure = onFailure;
        this._flushLoop = this.RunAsync();
    }

    /// <inheritdoc />
    async ValueTask IAsyncDisposable.DisposeAsync() => await this.CompleteAsync().ConfigureAwait(false);

    /// <summary>
    /// Requests a flush without queueing duplicate pending requests.
    /// </summary>
    internal void RequestFlush() => this._requests.Writer.TryWrite(true);

    /// <summary>
    /// Stops accepting new flush requests after queued requests are drained.
    /// </summary>
    internal void Stop() => this._requests.Writer.TryComplete();

    /// <summary>
    /// Stops the loop after queued requests are drained.
    /// </summary>
    /// <returns>A task which completes after the loop exits.</returns>
    internal async ValueTask CompleteAsync()
    {
        this.Stop();
#pragma warning disable VSTHRD003 // The background loop is intentionally owned by this instance.
        await this._flushLoop.ConfigureAwait(false);
#pragma warning restore VSTHRD003
    }

    private async Task RunAsync()
    {
        try
        {
            await foreach (var request in this._requests.Reader.ReadAllAsync().ConfigureAwait(false))
            {
                await this._flushAsync().ConfigureAwait(false);
            }
        }
        catch (Exception exception)
        {
            this._requests.Writer.TryComplete(exception);
            this._onFailure?.Invoke(exception);
        }
    }
}
