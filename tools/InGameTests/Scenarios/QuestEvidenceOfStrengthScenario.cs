namespace MuMain.Tools.InGameTests.Scenarios;

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
