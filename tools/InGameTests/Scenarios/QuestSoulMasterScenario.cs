namespace MuMain.Tools.InGameTests.Scenarios;

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
