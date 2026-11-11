namespace MuMain.Tools.InGameTests.Scenarios;

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
