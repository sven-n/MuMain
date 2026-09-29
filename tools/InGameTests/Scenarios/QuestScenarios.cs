using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Sebina's two quests turn a Dark Knight into a Blade Knight; one of the points
/// they reward is then spent on strength with the character window's "+".
/// </summary>
internal sealed class QuestBladeKnightScenario : Scenario
{
    private const string Player = "player";
    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "quest-blade-knight";

    public override string Description => "Sebina's quests turn a Dark Knight into a Blade Knight; a rewarded point goes to strength";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 6 + 2;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        await QuestSteps.LogInAsync(context, player, TestAccounts.QuestKnight, "in Devias as a level 150 Dark Knight, next to Sebina");
        await QuestSteps.SecondClassAsync(context, player, "Broken Sword", QuestSteps.BladeKnight);

        await context.StepAsync(
            "Close the dialog with Esc and open the character window with C",
            "The character window shows the class \"Blade Knight\" and the level-up points to spend, with a \"+\" next to each stat.",
            async () =>
            {
                await Quests.CloseAsync(context, player);
                await player.SendAsync("hotkey", new { key = "c" });
                await Expect.EventuallyAsync(
                    () => player.HasElementAsync("character.stat.strength"),
                    ServerAnswer,
                    "the character window shows no \"+\" for strength");
            });
        await context.StepAsync(
            "Click the \"+\" next to strength",
            "Strength goes up by one and the level-up points down by one.",
            async () =>
            {
                var state = await player.StateAsync();
                var strength = state.GetProperty("stats").GetProperty("strength").GetInt32();
                var points = state.GetProperty("level_up_points").GetInt32();
                await player.ClickElementAsync("character.stat.strength");
                var now = (Strength: 0, Points: 0);
                await Expect.EventuallyAsync(
                    async () =>
                    {
                        var after = await player.StateAsync();
                        now = (after.GetProperty("stats").GetProperty("strength").GetInt32(), after.GetProperty("level_up_points").GetInt32());
                        return now == (strength + 1, points - 1);
                    },
                    ServerAnswer,
                    () => $"strength {now.Strength} and {now.Points} points after the click, not {strength + 1} and {points - 1}");
            });
    }
}

/// <summary>Sebina's two quests turn a Dark Wizard into a Soul Master.</summary>
internal sealed class QuestSoulMasterScenario : Scenario
{
    private const string Player = "player";

    public override string Name => "quest-soul-master";

    public override string Description => "Sebina's quests turn a Dark Wizard into a Soul Master";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 6;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        await QuestSteps.LogInAsync(context, player, TestAccounts.QuestWizard, "in Devias as a level 150 Dark Wizard, next to Sebina");
        await QuestSteps.SecondClassAsync(context, player, "Soul Shard of Wizard", QuestSteps.SoulMaster);
    }
}

/// <summary>Sebina's two quests turn an Elf into a Muse Elf.</summary>
internal sealed class QuestMuseElfScenario : Scenario
{
    private const string Player = "player";

    public override string Name => "quest-muse-elf";

    public override string Description => "Sebina's quests turn an Elf into a Muse Elf";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 6;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        await QuestSteps.LogInAsync(context, player, TestAccounts.QuestElf, "in Devias as a level 150 Elf, next to Sebina");
        await QuestSteps.SecondClassAsync(context, player, "Tear of Elf", QuestSteps.MuseElf);
    }
}

/// <summary>
/// Marlon's "Gain Hero Status" for a Soul Master or a Muse Elf: one more level-up
/// point per level. At level 400 the server pays the 180 levels above 220 at once.
/// </summary>
internal sealed class QuestHeroStatusScenario(bool museElf) : Scenario
{
    private const string Player = "player";

    public override string Name => museElf ? "quest-hero-status-elf" : "quest-hero-status-wizard";

    public override string Description => museElf
        ? "a Muse Elf gains Marlon's hero status: a point more per level, 180 at once at level 400, and Infinity Arrow"
        : "a Soul Master gains Marlon's hero status: a point more per level, 180 at once at level 400";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 6 + 4;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        var character = museElf ? TestAccounts.HeroElf : TestAccounts.HeroWizard;
        await QuestSteps.LogInAsync(context, player, character, $"in Devias as a level 400 {(museElf ? "Elf" : "Dark Wizard")}, next to Sebina");
        await QuestSteps.SecondClassAsync(
            context,
            player,
            museElf ? "Tear of Elf" : "Soul Shard of Wizard",
            museElf ? QuestSteps.MuseElf : QuestSteps.SoulMaster);
        await QuestSteps.HeroStatusAsync(context, player, museElf);
    }
}

/// <summary>
/// A Blade Knight's way to the combo: Sebina's quests, then Marlon's "Gain Hero
/// Status" and "Secret of Dark Stone". The combo stays after logging in again.
/// </summary>
internal sealed class QuestComboScenario : Scenario
{
    private const string Player = "player";
    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "quest-combo";

    public override string Description => "a Dark Knight becomes a Blade Knight, gains hero status and Marlon's combo, which stays after a new login";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 6 + 4 + 3 + 1;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        var character = TestAccounts.ComboKnight;
        await QuestSteps.LogInAsync(context, player, character, "in Devias as a level 220 Dark Knight, next to Sebina");
        await QuestSteps.SecondClassAsync(context, player, "Broken Sword", QuestSteps.BladeKnight);
        await QuestSteps.HeroStatusAsync(context, player, museElf: false);
        await QuestSteps.DarkStoneAsync(context, player);
        await context.StepAsync(
            "Log out and log in again",
            $"{character.Name} is back in the world as a Blade Knight, and the quest list the server sends shows the combo quest "
            + "done: the combo is still there.",
            async () =>
            {
                await Quests.CloseAsync(context, player);
                await player.SendAsync("logout", timeout: TimeSpan.FromSeconds(60));
                await player.EnterWorldAsync(character.Account, character.Password, character.Name);
                await Expect.EventuallyAsync(
                    async () => (await player.StateAsync()).GetProperty("combo").GetBoolean(),
                    ServerAnswer,
                    "the combo is gone after logging in again");
                await Quests.ExpectStateAsync(player, Quests.DarkStone, "complete");
            });
    }
}

/// <summary>
/// Apostle Devin's "Evidence of Strength", the first quest towards the third class:
/// three items for 20 level-up points.
/// </summary>
internal sealed class QuestEvidenceOfStrengthScenario : Scenario
{
    private const string Player = "player";

    public override string Name => "quest-evidence-of-strength";

    public override string Description => "Apostle Devin takes three items for 20 level-up points, the first quest towards the third class";

    public override ScenarioCategory Category => ScenarioCategory.Quests;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 1 + 3;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        await QuestSteps.LogInAsync(context, player, TestAccounts.EvidenceLord, "in Devias as a level 400 Dark Lord, next to Devin");
        await QuestSteps.EvidenceOfStrengthAsync(
            context,
            player,
            ["Death-beam Knight Flame", "Hell-Miner Horn", "Dark Phoenix Feather"]);
    }
}
