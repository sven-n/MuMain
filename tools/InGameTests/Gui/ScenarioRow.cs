using System.ComponentModel;
using System.Runtime.CompilerServices;
using MuMain.Tools.InGameTests.Running;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Gui;

/// <summary>
/// A scenario in the window's list: whether it runs, its own wait, and how far it
/// got: a progress bar over its steps while it runs, then PASS or the step it failed at.
/// </summary>
internal sealed class ScenarioRow(Scenario scenario) : INotifyPropertyChanged
{
    private bool isChecked = true;
    private string delayText = string.Empty;
    private string qualityText = string.Empty;
    private RowState state = RowState.Idle;
    private int completedSteps;
    private int failedStep;
    private int stepsRun;
    private string detail = string.Empty;

    public event PropertyChangedEventHandler? PropertyChanged;

    private enum RowState
    {
        Idle,
        Waiting,
        Running,
        Passed,
        Failed,
    }

    public Scenario Scenario { get; } = scenario;

    public string Name => this.Scenario.Name;

    public string Description => this.Scenario.Description;

    public bool IsChecked
    {
        get => this.isChecked;
        set => this.Set(ref this.isChecked, value);
    }

    /// <summary>Milliseconds to pause after each action; empty takes the run's value.</summary>
    public string DelayText
    {
        get => this.delayText;
        set => this.Set(ref this.delayText, value);
    }

    /// <summary>JPEG quality of the screenshots, 1 to 100; empty takes the run's value.</summary>
    public string QualityText
    {
        get => this.qualityText;
        set => this.Set(ref this.qualityText, value);
    }

    /// <summary>The steps a passing run takes: the progress bar's end.</summary>
    public int StepCount => this.Scenario.StepCount;

    public int CompletedSteps => this.completedSteps;

    /// <summary>"3 / 11" on the progress bar.</summary>
    public string ProgressText => this.state == RowState.Waiting ? "waiting" : $"{this.completedSteps} / {this.StepCount}";

    public bool ShowsProgress => this.state is RowState.Waiting or RowState.Running;

    public bool Passed => this.state == RowState.Passed;

    public bool Failed => this.state == RowState.Failed;

    public string FailedText => this.failedStep > 0 ? $"FAILED at step {this.failedStep}"
        : this.stepsRun > 0 ? $"FAILED after step {this.stepsRun}"
        : "FAILED before step 1";

    /// <summary>The running step, or why it failed: the tooltip of the progress and the result.</summary>
    public string Detail => this.detail;

    /// <summary>Queued for the run that starts; unchecked rows are cleared.</summary>
    public void Queue()
    {
        this.stepsRun = 0;
        this.Change(this.isChecked ? RowState.Waiting : RowState.Idle, 0, 0, string.Empty);
    }

    public void StepStarted(int number, string title)
        => this.Change(RowState.Running, number - 1, 0, $"step {number} of {this.StepCount}: {title}");

    public void StepFinished(StepResult step)
        => this.Change(RowState.Running, step.Passed ? step.Number : step.Number - 1, 0, this.detail);

    public void Finished(ScenarioResult result)
    {
        if (result.Passed)
        {
            this.Change(RowState.Passed, this.StepCount, 0, $"{result.Steps.Count} steps · {result.Duration.TotalSeconds:0} s");
            return;
        }

        // No step failed but some ran: the scenario failed between two steps.
        this.stepsRun = result.Steps.Count;
        var failed = result.Steps.FirstOrDefault(step => !step.Passed);
        this.Change(RowState.Failed, this.completedSteps, failed?.Number ?? 0, failed is null
            ? result.Failure ?? "failed"
            : $"{failed.Title}: {failed.Failure}");
    }

    /// <summary>A queued row the run did not get to, because it was stopped or could not start.</summary>
    public void NotRun()
    {
        if (this.state == RowState.Waiting)
        {
            this.Change(RowState.Idle, 0, 0, string.Empty);
        }
    }

    private void Change(RowState newState, int completed, int failedAt, string newDetail)
    {
        this.state = newState;
        this.completedSteps = Math.Min(completed, this.StepCount);
        this.failedStep = failedAt;
        this.detail = newDetail;
        foreach (var name in new[]
                 {
                     nameof(this.CompletedSteps), nameof(this.ProgressText), nameof(this.ShowsProgress),
                     nameof(this.Passed), nameof(this.Failed), nameof(this.FailedText), nameof(this.Detail),
                 })
        {
            this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }

    private void Set<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
        {
            return;
        }

        field = value;
        this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }
}

/// <summary>The scenarios of one category, in a box of the window's list that folds in and out.</summary>
internal sealed class ScenarioGroup(ScenarioCategory category, IReadOnlyList<ScenarioRow> rows, bool isExpanded)
    : INotifyPropertyChanged
{
    private bool isExpanded = isExpanded;

    public event PropertyChangedEventHandler? PropertyChanged;

    public ScenarioCategory Category { get; } = category;

    public string Title => this.Category.DisplayName();

    public string Summary => this.Rows.Count == 1 ? "1 test" : $"{this.Rows.Count} tests";

    public IReadOnlyList<ScenarioRow> Rows { get; } = rows;

    public bool IsExpanded
    {
        get => this.isExpanded;
        set
        {
            if (this.isExpanded != value)
            {
                this.isExpanded = value;
                this.PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(this.IsExpanded)));
            }
        }
    }
}
