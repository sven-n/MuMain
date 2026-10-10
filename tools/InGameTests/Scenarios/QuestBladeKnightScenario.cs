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
