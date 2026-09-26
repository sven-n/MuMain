namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>Checks a scenario makes; a failed one ends the scenario.</summary>
internal static class Expect
{
    /// <summary>Fails the scenario with <paramref name="message"/> unless <paramref name="condition"/> holds.</summary>
    public static void That(bool condition, string message)
    {
        if (!condition)
        {
            throw new ScenarioFailedException(message);
        }
    }

    /// <summary>
    /// Polls <paramref name="condition"/> until it holds; fails with <paramref name="message"/>
    /// after <paramref name="timeout"/>. The server answers a click some frames later.
    /// </summary>
    public static async Task EventuallyAsync(Func<Task<bool>> condition, TimeSpan timeout, string message)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (!await condition())
        {
            if (DateTime.UtcNow >= deadline)
            {
                throw new ScenarioFailedException(message);
            }

            await Task.Delay(TimeSpan.FromMilliseconds(200));
        }
    }

    /// <summary>
    /// Checks that <paramref name="condition"/> still holds after <paramref name="wait"/>:
    /// for a refusal, where "nothing happens" is the expected answer.
    /// </summary>
    public static async Task StillAfterAsync(Func<Task<bool>> condition, TimeSpan wait, string message)
    {
        await Task.Delay(wait);
        That(await condition(), message);
    }
}
