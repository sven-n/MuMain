using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// A line typed into the chat box reaches the player standing next to one, and
/// the answer comes back: Enter opens the box, the text is typed, Enter sends it.
/// </summary>
internal sealed class ChatScenario : Scenario
{
    private const string First = "first";
    private const string Second = "second";

    // A free spot in Lorencia's town; normal chat reaches the players in view.
    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;
    private const int MeetingX = 135;
    private const int MeetingY = 128;
    private const int ChatDistance = 2;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "chat";

    public override string Description => "a line typed into the chat box reaches the player next to one, and the answer comes back";

    public override ScenarioCategory Category => ScenarioCategory.PlayerInteractions;

    public override IReadOnlyList<string> Roles => [First, Second];

    public override int StepCount => 7;

    public override async Task RunAsync(ScenarioContext context)
    {
        var first = context.Client(First);
        var second = context.Client(Second);
        var firstCharacter = TestAccounts.ChatFirst;
        var secondCharacter = TestAccounts.ChatSecond;
        var line = $"Hello {secondCharacter.Name}, this is {firstCharacter.Name}";
        var answer = $"Hi {firstCharacter.Name}, {secondCharacter.Name} here";

        await context.StepAsync(
            $"The first player logs in as {firstCharacter.Name}",
            $"{firstCharacter.Name} enters the world in Lorencia, its home town.",
            () => first.EnterWorldAsync(firstCharacter.Account, firstCharacter.Password, firstCharacter.Name),
            [First]);
        await context.StepAsync(
            $"The second player logs in as {secondCharacter.Name}",
            $"{secondCharacter.Name} enters the world in Lorencia, somewhere else in the town.",
            () => second.EnterWorldAsync(secondCharacter.Account, secondCharacter.Password, secondCharacter.Name),
            [Second]);
        await context.StepAsync(
            "The first player walks to the meeting spot",
            $"{firstCharacter.Name} stands at ({MeetingX},{MeetingY}), a free spot in Lorencia's town, or one tile from it.",
            async () =>
            {
                await first.WarpAsync(LorenciaGate, LorenciaMap);
                await Meeting.WalkToAsync(first, MeetingX, MeetingY);
            },
            [First]);
        await context.StepAsync(
            "The second player walks up to the first",
            $"{secondCharacter.Name} stands at most {ChatDistance} tiles from {firstCharacter.Name}: normal chat reaches the players in view.",
            async () =>
            {
                await second.WarpAsync(LorenciaGate, LorenciaMap);
                await Meeting.WalkUpToAsync(second, first, secondCharacter.Name, ChatDistance);
            });
        await context.StepAsync(
            "The first player opens the chat box with Enter",
            "The chat box opens at the bottom of the screen with the cursor in its text field.",
            () => OpenChatAsync(context, first),
            [First]);
        await context.StepAsync(
            $"The first player types \"{line}\" and sends it with Enter",
            $"The chat box closes, and the line shows in both chat logs as said by {firstCharacter.Name}.",
            () => SendLineAsync(context, first, firstCharacter.Name, line, [first, second]));
        await context.StepAsync(
            $"The second player answers \"{answer}\" the same way",
            $"Enter opens {secondCharacter.Name}'s chat box, the answer is typed and Enter sends it; it shows in both chat logs "
            + $"as said by {secondCharacter.Name}.",
            async () =>
            {
                await OpenChatAsync(context, second);
                await SendLineAsync(context, second, secondCharacter.Name, answer, [first, second]);
            });
    }

    private static Task OpenChatAsync(ScenarioContext context, GameClient client)
        => Keys.PressUntilAsync(
            context,
            client,
            "enter",
            async () => (await client.OpenWindowsAsync()).Contains("chat_input"),
            $"the {client.Role}'s chat box does not open");

    // Types the line into the open chat box and sends it; every one of
    // <paramref name="hearers"/> has to get it from <paramref name="sender"/>.
    private static async Task SendLineAsync(
        ScenarioContext context,
        GameClient client,
        string sender,
        string line,
        IReadOnlyList<GameClient> hearers)
    {
        var sequences = new List<long>();
        foreach (var hearer in hearers)
        {
            sequences.Add(await hearer.LastEventSequenceAsync());
        }

        await client.SendAsync("type", new { text = line });
        // Enter sends the line and closes the box.
        await Keys.PressUntilAsync(
            context,
            client,
            "enter",
            async () => !(await client.OpenWindowsAsync()).Contains("chat_input"),
            $"the {client.Role}'s chat box does not send the line");
        for (var index = 0; index < hearers.Count; index++)
        {
            await hearers[index].WaitForEventAsync(
                "chat",
                new Dictionary<string, string> { ["sender"] = sender, ["text"] = line, ["kind"] = "public" },
                sequences[index],
                ServerAnswer);
        }
    }
}
