namespace MuMain.Tools.InGameTests.Scenarios;

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
