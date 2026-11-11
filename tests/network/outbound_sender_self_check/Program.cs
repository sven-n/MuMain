using MUnique.Client.Library;

var timeout = TimeSpan.FromSeconds(5);
await VerifyCoalescingAndDrainAsync(timeout).ConfigureAwait(false);
await VerifyFailureReportingAsync(timeout).ConfigureAwait(false);

static async Task VerifyCoalescingAndDrainAsync(TimeSpan timeout)
{
    var firstFlushStarted = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
    var releaseFirstFlush = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
    var flushCount = 0;
    Exception? failure = null;
    var flushLoop = new OutboundFlushLoop(async () =>
    {
        if (Interlocked.Increment(ref flushCount) == 1)
        {
            firstFlushStarted.TrySetResult();
            await releaseFirstFlush.Task.ConfigureAwait(false);
        }
    }, exception => failure = exception);

    try
    {
        flushLoop.RequestFlush();
        await firstFlushStarted.Task.WaitAsync(timeout).ConfigureAwait(false);

        flushLoop.RequestFlush();
        flushLoop.RequestFlush();
        var completion = flushLoop.CompleteAsync().AsTask();

        if (completion.IsCompleted)
        {
            throw new InvalidOperationException("Flush loop completed before the active flush finished.");
        }

        releaseFirstFlush.TrySetResult();
        await completion.WaitAsync(timeout).ConfigureAwait(false);

        if (flushCount != 2)
        {
            throw new InvalidOperationException($"Flush requests were not coalesced: {flushCount} flushes.");
        }

        if (failure is not null)
        {
            throw new InvalidOperationException("Successful flushes reported a failure.", failure);
        }
    }
    finally
    {
        releaseFirstFlush.TrySetResult();
        await flushLoop.CompleteAsync().AsTask().WaitAsync(timeout).ConfigureAwait(false);
    }
}

static async Task VerifyFailureReportingAsync(TimeSpan timeout)
{
    var expectedFailure = new IOException("flush failed");
    var reportedFailure = new TaskCompletionSource<Exception>(TaskCreationOptions.RunContinuationsAsynchronously);
    var flushLoop = new OutboundFlushLoop(
        () => ValueTask.FromException(expectedFailure),
        exception => reportedFailure.TrySetResult(exception));

    try
    {
        flushLoop.RequestFlush();
        var actualFailure = await reportedFailure.Task.WaitAsync(timeout).ConfigureAwait(false);
        if (!ReferenceEquals(actualFailure, expectedFailure))
        {
            throw new InvalidOperationException("Flush failure callback received the wrong exception.");
        }
    }
    finally
    {
        await flushLoop.CompleteAsync().AsTask().WaitAsync(timeout).ConfigureAwait(false);
    }
}
