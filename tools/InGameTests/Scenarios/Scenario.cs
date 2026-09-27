namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// What kind of behaviour a scenario checks. The window and the report group the
/// scenarios by it, in this order.
/// </summary>
internal enum ScenarioCategory
{
    /// <summary>Players doing something with each other, e.g. trading.</summary>
    PlayerInteractions,

    /// <summary>How the game treats a player on its own, e.g. the rules of a map.</summary>
    GameBehaviour,
}

/// <summary>The names the window and the report show for the categories.</summary>
internal static class ScenarioCategories
{
    public static string DisplayName(this ScenarioCategory category) => category switch
    {
        ScenarioCategory.PlayerInteractions => "Player Interactions",
        ScenarioCategory.GameBehaviour => "Game Behaviour",
        _ => category.ToString(),
    };
}

/// <summary>
/// One in-game test. It gets a running, logged-out client per role and drives
/// them like players; the thing it tests has to go through the real input path
/// (clicks and keys), setup may use direct commands.
/// </summary>
internal abstract class Scenario
{
    /// <summary>The name used on the command line, e.g. "trade".</summary>
    public abstract string Name { get; }

    /// <summary>What the scenario checks, for the scenario list.</summary>
    public abstract string Description { get; }

    /// <summary>What kind of behaviour it checks, for grouping.</summary>
    public abstract ScenarioCategory Category { get; }

    /// <summary>The clients the scenario needs, by role.</summary>
    public abstract IReadOnlyList<string> Roles { get; }

    /// <summary>
    /// How many steps a passing run takes, for the window's progress bar. The runner
    /// notes a passing run that took a different number.
    /// </summary>
    public abstract int StepCount { get; }

    public abstract Task RunAsync(ScenarioContext context);
}
