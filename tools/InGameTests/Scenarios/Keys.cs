using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>Keys a scenario presses and checks.</summary>
internal static class Keys
{
    // A handled key changes what the client shows in the frame it is pressed.
    private static readonly TimeSpan Effect = TimeSpan.FromSeconds(2);
    private const int Presses = 3;

    /// <summary>
    /// Presses <paramref name="key"/> until <paramref name="done"/> holds, at most three times.
    /// </summary>
    /// <remarks>
    /// Now and then an injected key does not reach the window it is meant for
    /// (seen with Enter on the second client of a run, about one run in six;
    /// the cause is not known yet). A handled key shows at once, e.g. the dialog
    /// it answers closes, so a key that shows nothing after a moment was lost and
    /// is pressed again: the press never acts twice. Each extra press is noted in
    /// the log.
    /// </remarks>
    public static async Task PressUntilAsync(
        ScenarioContext context,
        GameClient client,
        string key,
        Func<Task<bool>> done,
        string failure)
    {
        for (var press = 1; press <= Presses; press++)
        {
            await client.SendAsync("hotkey", new { key });
            var deadline = DateTime.UtcNow + Effect;
            while (DateTime.UtcNow < deadline)
            {
                if (await done())
                {
                    return;
                }

                await Task.Delay(TimeSpan.FromMilliseconds(100));
            }

            if (press < Presses)
            {
                context.Note($"{key} on the {client.Role}'s client showed nothing; pressing it again");
            }
        }

        throw new ScenarioFailedException(failure);
    }
}
