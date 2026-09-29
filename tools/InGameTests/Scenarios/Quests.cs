using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// The legacy quests (the class changes, Marlon's and Devin's quests): talking
/// to their NPCs with a click and going through the quest dialog with clicks on
/// its answers, the way a player does.
/// </summary>
internal static class Quests
{
    /// <summary>Sebina the Priest in Devias: the second class change.</summary>
    public static readonly Npc Sebina = new("Sebina the Priest", 183, 32);

    /// <summary>Apostle Devin in Devias: the quests towards the third class.</summary>
    public static readonly Npc Devin = new("Apostle Devin", 181, 35);

    // Marlon wanders: OpenMU puts him on one of these spots, a random one, and
    // moves him to another every one to three hours.
    private static readonly (string Gate, int Map, Npc Spot)[] MarlonSpots =
    [
        ("Devias", 2, new Npc("Marlon", 197, 48)),
        ("Lorencia", 0, new Npc("Marlon", 136, 88)),
        ("Noria", 3, new Npc("Marlon", 169, 88)),
        ("Atlans", 7, new Npc("Marlon", 17, 35)),
    ];

    /// <summary>The quest numbers of the legacy quests.</summary>
    public const int ScrollOfEmperor = 0;
    public const int TreasuresOfMu = 1;
    public const int HeroStatus = 2;
    public const int DarkStone = 3;
    public const int EvidenceOfStrength = 4;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // Pages of one conversation before it gets to the answer that matters.
    private const int MaxPages = 15;

    /// <summary>The quest dialog on screen: its quest, page, text and answers.</summary>
    public sealed record Dialog(int Quest, int Page, string Text, IReadOnlyList<Answer> Answers, long NeedZen);

    /// <summary>An answer of the dialog and what a click on it does: next, accept, complete or close.</summary>
    public sealed record Answer(int Index, string Text, string Action);

    /// <summary>The quest dialog on screen, or null while it is closed.</summary>
    public static async Task<Dialog?> DialogAsync(GameClient client)
    {
        var dialog = (await client.StateAsync()).GetProperty("npc_quest");
        if (dialog.ValueKind != JsonValueKind.Object)
        {
            return null;
        }

        var answers = dialog.GetProperty("answers").EnumerateArray()
            .Select((answer, index) => new Answer(
                index,
                answer.GetProperty("text").GetString() ?? string.Empty,
                answer.GetProperty("action").GetString() ?? string.Empty))
            .ToList();
        return new Dialog(
            dialog.GetProperty("quest").GetInt32(),
            dialog.GetProperty("page").GetInt32(),
            dialog.GetProperty("text").GetString() ?? string.Empty,
            answers,
            dialog.GetProperty("need_zen").GetInt64());
    }

    /// <summary>
    /// Talks to <paramref name="npc"/> with a click on it and waits for the quest dialog; a dialog
    /// that is still open is closed first, as the server takes one talk at a time.
    /// </summary>
    public static async Task<Dialog> TalkAsync(ScenarioContext context, GameClient client, Npc npc)
    {
        await CloseAsync(context, client);
        await Npcs.TalkAsync(client, npc, "npc_quest");
        Dialog? dialog = null;
        await Expect.EventuallyAsync(
            async () => (dialog = await DialogAsync(client)) is not null,
            ServerAnswer,
            $"{npc.Name}'s quest dialog shows no page");
        return dialog!;
    }

    /// <summary>Closes the quest dialog with Esc, when it is open.</summary>
    public static async Task CloseAsync(ScenarioContext context, GameClient client)
    {
        if ((await client.OpenWindowsAsync()).Contains("npc_quest"))
        {
            await Keys.PressUntilAsync(
                context,
                client,
                "esc",
                async () => !(await client.OpenWindowsAsync()).Contains("npc_quest"),
                "Esc does not close the quest dialog");
        }
    }

    /// <summary>
    /// Goes through the conversation with clicks on its answers, page by page, until an answer
    /// does <paramref name="action"/> (accept or complete), and clicks that one. Returns the
    /// pages it went through, for the log.
    /// </summary>
    public static async Task<IReadOnlyList<int>> FollowToAsync(GameClient client, string action)
    {
        var pages = new List<int>();
        for (var turn = 0; turn < MaxPages; turn++)
        {
            var dialog = await DialogAsync(client)
                         ?? throw new ScenarioFailedException($"the quest dialog closed before an answer to {action}");
            pages.Add(dialog.Page);
            var wanted = dialog.Answers.FirstOrDefault(answer => answer.Action == action);
            if (wanted is not null)
            {
                await client.ClickElementAsync($"npc_quest.answer.{wanted.Index}");
                return pages;
            }

            var next = dialog.Answers.FirstOrDefault(answer => answer.Action == "next")
                       ?? throw new ScenarioFailedException(
                           $"page {dialog.Page} of the quest dialog (\"{dialog.Text}\") offers no answer to {action}: "
                           + string.Join(", ", dialog.Answers.Select(answer => $"\"{answer.Text}\" ({answer.Action})")));
            await client.ClickElementAsync($"npc_quest.answer.{next.Index}");
            await Expect.EventuallyAsync(
                async () => (await DialogAsync(client))?.Page != dialog.Page,
                ServerAnswer,
                $"a click on \"{next.Text}\" does not turn page {dialog.Page}");
        }

        throw new ScenarioFailedException($"no answer to {action} after {MaxPages} pages of the quest dialog");
    }

    /// <summary>What the client knows of a legacy quest: active, complete, not_started or none.</summary>
    public static async Task<string> StateAsync(GameClient client, int quest)
        => (await client.StateAsync()).GetProperty("quests")[quest].GetProperty("state").GetString() ?? string.Empty;

    public static async Task ExpectStateAsync(GameClient client, int quest, string state)
    {
        var now = string.Empty;
        await Expect.EventuallyAsync(
            async () => (now = await StateAsync(client, quest)) == state,
            ServerAnswer,
            () => $"quest {quest} is {now}, not {state}");
    }

    /// <summary>A quest reward event since <paramref name="since"/>; its amount.</summary>
    public static async Task<int> RewardAsync(GameClient client, string reward, long since)
    {
        var found = await client.WaitForEventAsync(
            "quest",
            new Dictionary<string, string> { ["change"] = "reward", ["reward"] = reward },
            since,
            ServerAnswer);
        return found.GetProperty("event").GetProperty("amount").GetInt32();
    }

    /// <summary>
    /// Walks up to Marlon and returns where he stands, and how he was found. The test server's
    /// log says on which of his spots it put him last; without it (another server), or when he
    /// is not there, the character looks on his spots, the map it is on first.
    /// </summary>
    public static async Task<(Npc Marlon, string How)> FindMarlonAsync(GameClient client)
    {
        var logged = await MarlonFromLogAsync(client);
        if (logged is not null && MarlonSpots.FirstOrDefault(candidate => candidate.Map == logged.Map) is { Gate: not null } known)
        {
            await client.WarpAsync(known.Gate, known.Map);
            var spot = known.Spot with { X = logged.X, Y = logged.Y };
            await Npcs.WalkUpToAsync(client, spot);
            if (await SeenAsync(client, spot.Name) is { } position)
            {
                return (spot with { X = position.X, Y = position.Y }, "the test server's log");
            }
        }

        return (await SearchMarlonAsync(client), logged is null ? "a search of his spots" : "a search of his spots; he was not where the log said");
    }

    // A freshly started test server puts Marlon somewhere some 20 s after it
    // starts, and a quick scenario can ask before that: on the test server the
    // log is asked again for a while.
    private static async Task<Running.WanderingNpcSpot?> MarlonFromLogAsync(GameClient client)
    {
        if (!Running.TestServer.IsTestServer(client.ServerHost, client.ServerPort))
        {
            return null;
        }

        var deadline = DateTime.UtcNow + MarlonSpawnWait;
        while (true)
        {
            var spot = await Running.TestServer.WanderingNpcAsync(client.ServerHost, client.ServerPort, "Marlon", CancellationToken.None);
            if (spot is not null || DateTime.UtcNow >= deadline)
            {
                return spot;
            }

            await Task.Delay(TimeSpan.FromSeconds(2));
        }
    }

    private static readonly TimeSpan MarlonSpawnWait = TimeSpan.FromSeconds(30);

    private static async Task<Npc> SearchMarlonAsync(GameClient client)
    {
        var map = (await client.StateAsync()).GetProperty("map").GetInt32();
        foreach (var (gate, spotMap, spot) in MarlonSpots.OrderBy(candidate => candidate.Map == map ? 0 : 1))
        {
            await client.WarpAsync(gate, spotMap);
            await Npcs.WalkUpToAsync(client, spot);
            if (await SeenAsync(client, spot.Name) is { } position)
            {
                return spot with { X = position.X, Y = position.Y };
            }
        }

        throw new ScenarioFailedException("Marlon is on none of his spots in Devias, Lorencia, Noria and Atlans");
    }

    private static async Task<(int X, int Y)?> SeenAsync(GameClient client, string name)
    {
        // The NPCs around a spot come in with the server's view update.
        (int X, int Y)? seen = null;
        try
        {
            await Expect.EventuallyAsync(
                async () => (seen = await Meeting.SeenPositionAsync(client, name)) is not null,
                TimeSpan.FromSeconds(5),
                string.Empty);
        }
        catch (ScenarioFailedException)
        {
            return null;
        }

        return seen;
    }
}
