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

/// <summary>How a scenario went, step by step.</summary>
internal sealed record ScenarioResult(
    string Name,
    string Description,
    bool Passed,
    TimeSpan Duration,
    TimeSpan StepDelay,
    string? Failure,
    IReadOnlyList<StepResult> Steps);

/// <summary>A whole run: its settings, every scenario's result and where the report is.</summary>
internal sealed record TestRunResult(
    DateTime Started,
    TestRunOptions Options,
    IReadOnlyList<ScenarioResult> Scenarios,
    string Folder,
    string ReportPath)
{
    public bool AllPassed => this.Scenarios.All(scenario => scenario.Passed);
}
