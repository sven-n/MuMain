using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>A screenshot one client took after a step.</summary>
internal sealed record StepScreenshot(string Role, string Path);

/// <summary>One step of a scenario: what it did, what should have happened, and what did.</summary>
internal sealed record StepResult(
    int Number,
    string Title,
    string Expectation,
    bool Passed,
    TimeSpan Duration,
    string? Failure,
    IReadOnlyList<StepScreenshot> Screenshots);

/// <summary>What a client showed when its scenario failed: its `state` and its recent events, as JSON.</summary>
internal sealed record ClientDetails(string Role, string State, string Events);

/// <summary>What served the run: e.g. OpenMU, its version and commit, where its source is; parts may be unknown.</summary>
internal sealed record ServerVersion(string Name, string? Version, string? Commit, string? Source);

/// <summary>The client a run tested: the git commit it was built from, and whether its files differed from it.</summary>
internal sealed record ClientVersion(string Commit, bool Changed, string? Build);

/// <summary>How a scenario went in a run.</summary>
internal enum ScenarioStatus
{
    Passed,
    Failed,

    /// <summary>Not run: not selected, or the run was stopped before it.</summary>
    Skipped,
}

/// <summary>How a scenario went, step by step.</summary>
internal sealed record ScenarioResult(
    string Name,
    string Description,
    ScenarioCategory Category,
    ScenarioStatus Status,
    TimeSpan Duration,
    TimeSpan StepDelay,
    string? Failure,
    IReadOnlyList<StepResult> Steps,
    string? SkipReason = null,
    IReadOnlyList<ClientDetails>? FailureDetails = null,
    int ScreenshotQuality = 0)
{
    public bool Passed => this.Status == ScenarioStatus.Passed;

    public bool Failed => this.Status == ScenarioStatus.Failed;

    /// <summary>A scenario the run did not get to, and why.</summary>
    public static ScenarioResult Skip(Scenario scenario, string reason)
        => new(scenario.Name, scenario.Description, scenario.Category, ScenarioStatus.Skipped, TimeSpan.Zero, TimeSpan.Zero, null, [], reason);
}

/// <summary>A whole run: its settings, every scenario's result and the one file that reports it.</summary>
internal sealed record TestRunResult(
    DateTime Started,
    TestRunOptions Options,
    IReadOnlyList<ScenarioResult> Scenarios,
    string ReportPath,
    ClientVersion? Client = null,
    ServerVersion? Server = null)
{
    /// <summary>No scenario failed; skipped ones do not count.</summary>
    public bool AllPassed => !this.Scenarios.Any(scenario => scenario.Failed);

    public int Count(ScenarioStatus status) => this.Scenarios.Count(scenario => scenario.Status == status);
}
