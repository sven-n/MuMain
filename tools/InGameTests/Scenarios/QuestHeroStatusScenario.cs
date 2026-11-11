namespace MuMain.Tools.InGameTests.Scenarios;

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
