using System.Collections.Concurrent;
using MUnique.Client.Library;

var releaseFirstSend = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
var sent = new ConcurrentQueue<byte>();

await using var sender = new OutboundPacketSender(async packet =>
{
    if (sent.IsEmpty)
    {
        await releaseFirstSend.Task.ConfigureAwait(false);
    }

    sent.Enqueue(packet.Span[0]);
});

if (!sender.TryEnqueue([1]) || !sender.TryEnqueue([2]))
{
    throw new InvalidOperationException("Sender rejected packets below capacity.");
}

await Task.Delay(50).ConfigureAwait(false);
if (!sent.IsEmpty)
{
    throw new InvalidOperationException("Blocked sink completed before release.");
}

releaseFirstSend.SetResult();
await sender.CompleteAsync().ConfigureAwait(false);

if (!sent.SequenceEqual(new byte[] { 1, 2 }))
{
    throw new InvalidOperationException($"FIFO order lost: {string.Join(',', sent)}");
}
