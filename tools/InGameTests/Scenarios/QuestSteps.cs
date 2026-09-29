using System.Globalization;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>A class a quest changes a character to: the client's class number and its name.</summary>
internal sealed record QuestClass(int Number, string Name);

/// <summary>
/// The steps of the legacy quests that several scenarios take: Sebina's two
/// quests to the second class, Marlon's two quests, Devin's first quest.
/// </summary>
internal static class QuestSteps
{
    public static readonly QuestClass BladeKnight = new(8, "Blade Knight");
    public static readonly QuestClass SoulMaster = new(7, "Soul Master");
    public static readonly QuestClass MuseElf = new(9, "Muse Elf");

    private const string ScrollOfEmperor = "Scroll of the Emperor";
    private const string RingOfHonor = "Ring of Honor";
    private const string DarkStone = "Dark Stone";

    // The client's number of Infinity Arrow, which the Muse Elf gets from Marlon.
    private const int InfinityArrowSkill = 77;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    /// <summary>Logs in; the quest characters stand in Devias, next to Sebina and Devin.</summary>
    public static Task LogInAsync(ScenarioContext context, GameClient client, TestCharacter character, string where)
        => context.StepAsync(
            $"Log in as {character.Name}",
            $"{character.Name} enters the world {where}.",
            () => client.EnterWorldAsync(character.Account, character.Password, character.Name));

    /// <summary>
    /// Sebina's quests: "Find the Scroll of Emperor", then "Three Treasures of MU", which hands in
    /// <paramref name="classItem"/> and changes the class to <paramref name="newClass"/>.
    /// </summary>
    public static async Task SecondClassAsync(ScenarioContext context, GameClient client, string classItem, QuestClass newClass)
    {
        await context.StepAsync(
            "Talk to Sebina the Priest with a click on her",
            "Her quest dialog opens on \"Find the Scroll of the Emperor\", which costs 1,000,000 zen to take.",
            async () =>
            {
                var dialog = await Quests.TalkAsync(context, client, Quests.Sebina);
                Expect.That(dialog.Quest == Quests.ScrollOfEmperor, $"Sebina offers quest {dialog.Quest}, not the Scroll of Emperor");
            });
        await AcceptAsync(context, client, Quests.ScrollOfEmperor, 1_000_000);
        await HandInAsync(
            context,
            client,
            Quests.ScrollOfEmperor,
            [ScrollOfEmperor],
            "10 level-up points",
            10,
            since => ExpectPointsRewardAsync(client, since, 10));

        await context.StepAsync(
            "Close the dialog with Esc and talk to Sebina again",
            "Her next quest shows: \"Three Treasures of MU\", for 2,000,000 zen.",
            async () =>
            {
                var dialog = await Quests.TalkAsync(context, client, Quests.Sebina);
                Expect.That(dialog.Quest == Quests.TreasuresOfMu, $"Sebina offers quest {dialog.Quest}, not the Three Treasures of MU");
            });
        await AcceptAsync(context, client, Quests.TreasuresOfMu, 2_000_000);
        await HandInAsync(
            context,
            client,
            Quests.TreasuresOfMu,
            [classItem],
            $"10 level-up points and the class change: the character is a {newClass.Name} now",
            10,
            async since =>
            {
                await ExpectPointsRewardAsync(client, since, 10);
                await Quests.RewardAsync(client, "second_class", since);
                await ExpectClassAsync(client, newClass);
            });
    }

    /// <summary>
    /// Marlon's "Gain Hero Status": one more level-up point per level from now on. The server
    /// pays the levels above 220 at once, so a level 400 character gets 180 points.
    /// </summary>
    public static async Task HeroStatusAsync(ScenarioContext context, GameClient client, bool museElf)
    {
        var level = (await client.StateAsync()).GetProperty("level").GetInt32();
        var paid = Math.Max(0, level - 220);
        await context.StepAsync(
            "Look for Marlon",
            "Marlon wanders between Devias, Lorencia, Noria and Atlans. The test server's log says where he is now; the "
            + "character warps to that map and walks up to him (on another server it looks on his spots until it finds him).",
            async () => context.Note($"Marlon is at {await FindMarlonNoteAsync(client)}"));
        await context.StepAsync(
            "Talk to Marlon with a click on him",
            "His quest dialog opens on \"Gain Hero Status\", which costs 3,000,000 zen to take.",
            async () =>
            {
                var dialog = await Quests.TalkAsync(context, client, await MarlonAsync(client));
                Expect.That(dialog.Quest == Quests.HeroStatus, $"Marlon offers quest {dialog.Quest}, not Gain Hero Status");
            });
        await AcceptAsync(context, client, Quests.HeroStatus, 3_000_000);
        await HandInAsync(
            context,
            client,
            Quests.HeroStatus,
            [RingOfHonor],
            $"one more level-up point per level; at level {level} the server pays the {paid} levels above 220 at once "
            + $"({paid} points)" + (museElf ? ", and the Muse Elf learns Infinity Arrow" : string.Empty),
            paid,
            async since =>
            {
                var amount = await Quests.RewardAsync(client, "points_per_level", since);
                Expect.That(amount == paid, $"the reward pays {amount} points, not {paid}");
                if (museElf)
                {
                    await Expect.EventuallyAsync(
                        async () => (await client.StateAsync()).GetProperty("skills").EnumerateArray().Any(skill => skill.GetInt32() == InfinityArrowSkill),
                        ServerAnswer,
                        "the Muse Elf does not have Infinity Arrow");
                }
            });
    }

    /// <summary>Marlon's "Secret of Dark Stone", for the Blade Knight only: the combo.</summary>
    public static async Task DarkStoneAsync(ScenarioContext context, GameClient client)
    {
        await context.StepAsync(
            "Close the dialog with Esc and talk to Marlon again",
            "His next quest shows: \"Secret of Dark Stone\", for 2,000,000 zen, which only a Blade Knight is offered.",
            async () =>
            {
                var dialog = await Quests.TalkAsync(context, client, await MarlonAsync(client));
                Expect.That(dialog.Quest == Quests.DarkStone, $"Marlon offers quest {dialog.Quest}, not the Secret of Dark Stone");
            });
        await AcceptAsync(context, client, Quests.DarkStone, 2_000_000);
        await HandInAsync(
            context,
            client,
            Quests.DarkStone,
            [DarkStone],
            "the combo: three skills in a row finish with an extra strike",
            0,
            async since =>
            {
                await Quests.RewardAsync(client, "combo", since);
                await Expect.EventuallyAsync(
                    async () => (await client.StateAsync()).GetProperty("combo").GetBoolean(),
                    ServerAnswer,
                    "the client does not show the combo");
            });
    }

    /// <summary>Devin's "Evidence of Strength": three items for 20 level-up points.</summary>
    public static async Task EvidenceOfStrengthAsync(ScenarioContext context, GameClient client, IReadOnlyList<string> items)
    {
        await context.StepAsync(
            "Talk to Apostle Devin with a click on him",
            "His quest dialog opens on \"Evidence of Strength\", which costs 5,000,000 zen to take.",
            async () =>
            {
                var dialog = await Quests.TalkAsync(context, client, Quests.Devin);
                Expect.That(dialog.Quest == Quests.EvidenceOfStrength, $"Devin offers quest {dialog.Quest}, not Evidence of Strength");
            });
        await AcceptAsync(context, client, Quests.EvidenceOfStrength, 5_000_000);
        await HandInAsync(
            context,
            client,
            Quests.EvidenceOfStrength,
            items,
            "20 level-up points",
            20,
            since => ExpectPointsRewardAsync(client, since, 20));
    }

    // Follows the conversation to its accept answer: the quest is active and its zen is paid.
    private static Task AcceptAsync(ScenarioContext context, GameClient client, int quest, long zen)
        => context.StepAsync(
            "Follow the conversation and take the quest",
            $"A click on each page's answer leads to the one that takes the quest: it is active now, and {zen.ToString("N0", CultureInfo.InvariantCulture)} "
            + "zen are paid.",
            async () =>
            {
                var had = await client.ZenAsync();
                var pages = await Quests.FollowToAsync(client, "accept");
                context.Note($"pages {string.Join(" > ", pages)}");
                await Quests.ExpectStateAsync(client, quest, "active");
                var now = 0L;
                await Expect.EventuallyAsync(
                    async () => (now = await client.ZenAsync()) == had - zen,
                    ServerAnswer,
                    () => $"{now:N0} zen after taking the quest, not {had - zen:N0}");
            });

    // The dialog asks for the items now; following it to the hand-in answer
    // completes the quest: the items leave the inventory, `check` sees the
    // reward events, and the level-up points grow by `points`.
    private static Task HandInAsync(
        ScenarioContext context,
        GameClient client,
        int quest,
        IReadOnlyList<string> items,
        string rewards,
        int points,
        Func<long, Task> check)
        => context.StepAsync(
            $"Hand in {string.Join(", ", items.Select(item => $"'{item}'"))}",
            "The dialog now asks for what the quest wants; clicking through to the answer that hands it in completes the "
            + $"quest: {(items.Count == 1 ? "the item leaves" : "the items leave")} the inventory, and the reward is {rewards}.",
            async () =>
            {
                var had = new Dictionary<string, int>();
                foreach (var item in items)
                {
                    had[item] = await client.CountAsync(item);
                    Expect.That(had[item] > 0, $"the inventory has no '{item}' to hand in");
                }

                var pointsBefore = (await client.StateAsync()).GetProperty("level_up_points").GetInt32();
                var since = await client.LastEventSequenceAsync();
                var pages = await Quests.FollowToAsync(client, "complete");
                context.Note($"pages {string.Join(" > ", pages)}");
                await Quests.ExpectStateAsync(client, quest, "complete");
                foreach (var item in items)
                {
                    var left = 0;
                    await Expect.EventuallyAsync(
                        async () => (left = await client.CountAsync(item)) == had[item] - 1,
                        ServerAnswer,
                        () => $"{left} '{item}' left after handing it in, not {had[item] - 1}");
                }

                await check(since);
                await ExpectPointsNowAsync(client, pointsBefore + points);
            });

    // The reward event for level-up points, with its amount.
    private static async Task ExpectPointsRewardAsync(GameClient client, long since, int amount)
    {
        var paid = await Quests.RewardAsync(client, "level_up_points", since);
        Expect.That(paid == amount, $"the reward pays {paid} level-up points, not {amount}");
    }

    private static async Task ExpectPointsNowAsync(GameClient client, int expected)
    {
        var points = 0;
        await Expect.EventuallyAsync(
            async () => (points = (await client.StateAsync()).GetProperty("level_up_points").GetInt32()) == expected,
            ServerAnswer,
            () => $"{points} level-up points, not {expected}");
    }

    private static async Task ExpectClassAsync(GameClient client, QuestClass expected)
    {
        var name = string.Empty;
        await Expect.EventuallyAsync(
            async () =>
            {
                var state = await client.StateAsync();
                name = state.GetProperty("class_name").GetString() ?? string.Empty;
                return state.GetProperty("class").GetInt32() == expected.Number;
            },
            ServerAnswer,
            () => $"the character is a {name}, not a {expected.Name}");
    }

    private static async Task<string> FindMarlonNoteAsync(GameClient client)
    {
        var (marlon, how) = await Quests.FindMarlonAsync(client);
        var state = await client.StateAsync();
        return $"({marlon.X},{marlon.Y}) in {state.GetProperty("map_name").GetString()}, found by {how}";
    }

    // Marlon where the character sees him now.
    private static async Task<Npc> MarlonAsync(GameClient client)
        => await Meeting.SeenPositionAsync(client, "Marlon") is { } seen
            ? new Npc("Marlon", seen.X, seen.Y)
            : throw new ScenarioFailedException("Marlon is not in view");
}
