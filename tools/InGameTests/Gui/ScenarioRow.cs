using System.ComponentModel;
using System.Runtime.CompilerServices;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Gui;

/// <summary>A scenario in the window's list: whether it runs, its own wait, and how it went.</summary>
internal sealed class ScenarioRow(Scenario scenario) : INotifyPropertyChanged
{
    private bool isChecked = true;
    private string delayText = string.Empty;
    private string status = string.Empty;

    public event PropertyChangedEventHandler? PropertyChanged;

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

    public string Status
    {
        get => this.status;
        set => this.Set(ref this.status, value);
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
