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
    ScenarioStatus Status,
    TimeSpan Duration,
    TimeSpan StepDelay,
    string? Failure,
    IReadOnlyList<StepResult> Steps,
    string? SkipReason = null)
{
    public bool Passed => this.Status == ScenarioStatus.Passed;

    public bool Failed => this.Status == ScenarioStatus.Failed;

    /// <summary>A scenario the run did not get to, and why.</summary>
    public static ScenarioResult Skip(Scenario scenario, string reason)
        => new(scenario.Name, scenario.Description, ScenarioStatus.Skipped, TimeSpan.Zero, TimeSpan.Zero, null, [], reason);
}

/// <summary>A whole run: its settings, every scenario's result and where the report is.</summary>
internal sealed record TestRunResult(
    DateTime Started,
    TestRunOptions Options,
    IReadOnlyList<ScenarioResult> Scenarios,
    string Folder,
    string ReportPath)
{
    /// <summary>No scenario failed; skipped ones do not count.</summary>
    public bool AllPassed => !this.Scenarios.Any(scenario => scenario.Failed);

    public int Count(ScenarioStatus status) => this.Scenarios.Count(scenario => scenario.Status == status);
}
