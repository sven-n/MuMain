namespace MuMain.Tools.InGameTests.Scenarios;

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

    /// <summary>The clients the scenario needs, by role.</summary>
    public abstract IReadOnlyList<string> Roles { get; }

    public abstract Task RunAsync(ScenarioContext context);
}
