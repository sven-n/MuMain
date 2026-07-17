namespace MUnique.Client.Library;

using System;
using System.Buffers;
using System.IO.Pipelines;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;

internal sealed class OutboundPacketSender : IAsyncDisposable
{
    private const int Capacity = 1024;
    private readonly Channel<byte[]> _packets = Channel.CreateBounded<byte[]>(new BoundedChannelOptions(Capacity)
    {
        FullMode = BoundedChannelFullMode.Wait,
        SingleReader = true,
        SingleWriter = false,
    });
    private readonly Func<ReadOnlyMemory<byte>, ValueTask> _sendAsync;
    private readonly Action<Exception>? _onFailure;
    private readonly Task _sendLoop;
    private int _highWaterMark;
    private int _pendingCount;

    internal OutboundPacketSender(Func<ReadOnlyMemory<byte>, ValueTask> sendAsync, Action<Exception>? onFailure = null)
    {
        this._sendAsync = sendAsync;
        this._onFailure = onFailure;
        this._sendLoop = Task.Run(this.RunAsync);
    }

    internal int HighWaterMark => Volatile.Read(ref this._highWaterMark);

    internal int PendingCount => Volatile.Read(ref this._pendingCount);

    internal bool TryEnqueue(byte[] packet)
    {
        var pendingCount = Interlocked.Increment(ref this._pendingCount);
        this.UpdateHighWaterMark(pendingCount);
        if (this._packets.Writer.TryWrite(packet))
        {
            return true;
        }

        Interlocked.Decrement(ref this._pendingCount);
        return false;
    }

    internal void Complete() => this._packets.Writer.TryComplete();

    internal async ValueTask CompleteAsync()
    {
        this.Complete();
        await this._sendLoop.ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this.CompleteAsync().ConfigureAwait(false);

    private async Task RunAsync()
    {
        try
        {
            await foreach (var packet in this._packets.Reader.ReadAllAsync().ConfigureAwait(false))
            {
                Interlocked.Decrement(ref this._pendingCount);
                await this._sendAsync(packet).ConfigureAwait(false);
            }
        }
        catch (Exception exception)
        {
            this._packets.Writer.TryComplete(exception);
            this._onFailure?.Invoke(exception);
        }
    }

    private void UpdateHighWaterMark(int pendingCount)
    {
        var highWaterMark = Volatile.Read(ref this._highWaterMark);
        while (pendingCount > highWaterMark)
        {
            var previous = Interlocked.CompareExchange(ref this._highWaterMark, pendingCount, highWaterMark);
            if (previous == highWaterMark)
            {
                return;
            }

            highWaterMark = previous;
        }
    }
}

internal sealed class PacketPipeWriter : PipeWriter
{
    private readonly ArrayBufferWriter<byte> _buffer = new();

    internal byte[] ToArray(int length) => this._buffer.WrittenSpan[..length].ToArray();

    public override void Advance(int bytes) => this._buffer.Advance(bytes);

    public override void CancelPendingFlush()
    {
    }

    public override void Complete(Exception? exception = null)
    {
    }

    public override ValueTask<FlushResult> FlushAsync(CancellationToken cancellationToken = default) => new(new FlushResult(false, false));

    public override Memory<byte> GetMemory(int sizeHint = 0) => this._buffer.GetMemory(sizeHint);

    public override Span<byte> GetSpan(int sizeHint = 0) => this._buffer.GetSpan(sizeHint);
}
