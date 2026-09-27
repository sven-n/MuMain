using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Text;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Platform.Storage;
using Avalonia.Threading;
using MuMain.Tools.InGameTests.Running;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Gui;

/// <summary>
/// Chooses scenarios and how slowly they run, runs them, and opens the report of
/// the run: every step with what should happen and a screenshot of each client.
/// </summary>
internal sealed partial class MainWindow : Window
{
    private readonly RunnerOptions startOptions;
    private readonly GuiSettings settings = GuiSettings.Load();
    private readonly ObservableCollection<ScenarioRow> rows = [];
    private readonly List<ScenarioGroup> groups;
    private CancellationTokenSource? stopRequest;
    private string? reportPath;

    // For the XAML previewer; the application uses the other constructor.
    public MainWindow()
        : this(RunnerOptions.Parse(["--gui"], out _)!)
    {
    }

    public MainWindow(RunnerOptions startOptions)
    {
        this.startOptions = startOptions;
        this.InitializeComponent();

        foreach (var scenario in Program.AllScenarios.OrderBy(scenario => scenario.Category))
        {
            var row = new ScenarioRow(scenario);
            if (this.settings.Scenarios.TryGetValue(scenario.Name, out var saved))
            {
                row.IsChecked = saved.Checked;
                row.DelayText = saved.Delay ?? string.Empty;
            }

            row.PropertyChanged += this.OnRowChanged;
            this.rows.Add(row);
        }

        this.groups = this.rows
            .GroupBy(row => row.Scenario.Category)
            .Select(group => new ScenarioGroup(group.Key, [.. group], !this.settings.CollapsedCategories.Contains(group.Key.ToString())))
            .ToList();
        this.ScenarioList.ItemsSource = this.groups;

        // The command line wins over what the window remembers.
        this.ClientPathBox.Text = startOptions.ClientPath is not null
            ? Path.GetFullPath(startOptions.ClientPath)
            : this.settings.ClientPath ?? string.Empty;
        this.ServerBox.Text = startOptions.ServerGiven || this.settings.Server is null
            ? $"{startOptions.ServerHost}:{startOptions.ServerPort}"
            : this.settings.Server;
        this.FreshServerBox.IsChecked = startOptions.FreshServer || this.settings.FreshServer;
        this.DefaultDelayBox.Text = startOptions.StepDelay > TimeSpan.Zero
            ? ((int)startOptions.StepDelay.TotalMilliseconds).ToString()
            : this.settings.StepDelayMilliseconds.ToString();

        this.BrowseButton.Click += this.OnBrowse;
        this.CheckAllButton.Click += this.OnCheckAll;
        this.RunButton.Click += this.OnRun;
        this.StopButton.Click += this.OnStop;
        this.OpenReportButton.Click += this.OnOpenReport;
        this.Closing += this.OnClosing;
        this.UpdateSelection();
    }

    private bool IsRunning => this.stopRequest is not null;

    private void OnRowChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(ScenarioRow.IsChecked))
        {
            this.UpdateSelection();
        }
    }

    private void UpdateSelection()
    {
        var checkedCount = this.rows.Count(row => row.IsChecked);
        this.CheckAllButton.Content = checkedCount == this.rows.Count ? "Uncheck all" : "Check all";
        this.SelectionText.Text = $"{checkedCount} of {this.rows.Count} tests checked";
    }

    private void OnCheckAll(object? sender, RoutedEventArgs e)
    {
        var check = this.rows.Any(row => !row.IsChecked);
        foreach (var row in this.rows)
        {
            row.IsChecked = check;
        }
    }

    private async void OnBrowse(object? sender, RoutedEventArgs e)
    {
        var files = await this.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions
        {
            Title = "The editor build of Main (with the control socket)",
            AllowMultiple = false,
        });
        if (files.Count > 0 && files[0].TryGetLocalPath() is { } path)
        {
            this.ClientPathBox.Text = path;
        }
    }

    private async void OnRun(object? sender, RoutedEventArgs e)
    {
        if (this.IsRunning || this.ReadRunOptions() is not { } options)
        {
            return;
        }

        this.SaveSettings();
        this.stopRequest = new CancellationTokenSource();
        this.SetRunning(true);
        this.LogBox.Text = string.Empty;
        this.RunStatusText.Text = "running…";
        foreach (var row in this.rows)
        {
            row.Queue();
        }

        var log = new WindowLog(this.AppendLog);
        var listener = new TestRunListener
        {
            StepStarted = (name, number, title) => Dispatcher.UIThread.Post(() => this.Row(name).StepStarted(number, title)),
            StepFinished = (name, step) => Dispatcher.UIThread.Post(() => this.Row(name).StepFinished(step)),
            ScenarioFinished = result => Dispatcher.UIThread.Post(() => this.Row(result.Name).Finished(result)),
        };

        try
        {
            var token = this.stopRequest.Token;
            var run = await Task.Run(() => TestRun.RunAsync(options, log, listener, token));
            this.reportPath = run.ReportPath;
            this.RunStatusText.Text = $"{run.Count(ScenarioStatus.Passed)} passed, {run.Count(ScenarioStatus.Failed)} failed, "
                                      + $"{run.Count(ScenarioStatus.Skipped)} skipped · report written";
        }
        catch (OperationCanceledException)
        {
            // Stop while the server was being prepared: no scenario ran.
            this.AppendLog("stopped before the first test" + Environment.NewLine);
            this.RunStatusText.Text = "stopped before the first test";
        }
        catch (Exception exception)
        {
            // An async void handler must not let anything escape: it would end the window.
            this.AppendLog(exception.Message + Environment.NewLine);
            this.RunStatusText.Text = exception.Message;
        }
        finally
        {
            // Posted updates run before this continuation; what is still queued did not run.
            foreach (var row in this.rows)
            {
                row.NotRun();
            }

            this.stopRequest.Dispose();
            this.stopRequest = null;
            this.SetRunning(false);
        }
    }

    // What the fields say, or null after telling what is wrong.
    private TestRunOptions? ReadRunOptions()
    {
        var clientPath = this.ClientPathBox.Text?.Trim() ?? string.Empty;
        if (!File.Exists(clientPath))
        {
            return this.Refuse("Choose the editor build of Main: the client file does not exist.");
        }

        if (!RunnerOptions.TryParseServer(this.ServerBox.Text?.Trim() ?? string.Empty, out var host, out var port))
        {
            return this.Refuse("The game server is host:port, e.g. 127.0.0.1:56901.");
        }

        if (!TryParseDelay(this.DefaultDelayBox.Text, out var defaultDelay) || defaultDelay is null)
        {
            return this.Refuse("The wait after each action is a number of milliseconds, 0 or more.");
        }

        var selections = new List<ScenarioSelection>();
        foreach (var row in this.rows.Where(row => row.IsChecked))
        {
            if (!TryParseDelay(row.DelayText, out var delay))
            {
                return this.Refuse($"The wait of '{row.Name}' is a number of milliseconds, or empty for the value above.");
            }

            selections.Add(new ScenarioSelection(row.Scenario, TimeSpan.FromMilliseconds(delay ?? defaultDelay.Value)));
        }

        if (selections.Count == 0)
        {
            return this.Refuse("Check at least one test.");
        }

        return new TestRunOptions(
            clientPath,
            host,
            port,
            this.FreshServerBox.IsChecked == true,
            this.startOptions.OutputFolder,
            selections,
            Program.AllScenarios);
    }

    // Empty is no value; otherwise a whole number of milliseconds, 0 or more.
    private static bool TryParseDelay(string? text, out int? milliseconds)
    {
        milliseconds = null;
        if (string.IsNullOrWhiteSpace(text))
        {
            return true;
        }

        if (!int.TryParse(text.Trim(), out var value) || value < 0)
        {
            return false;
        }

        milliseconds = value;
        return true;
    }

    private TestRunOptions? Refuse(string message)
    {
        this.RunStatusText.Text = message;
        return null;
    }

    private void SaveSettings()
    {
        this.settings.ClientPath = this.ClientPathBox.Text?.Trim();
        this.settings.Server = this.ServerBox.Text?.Trim();
        this.settings.FreshServer = this.FreshServerBox.IsChecked == true;
        this.settings.StepDelayMilliseconds = TryParseDelay(this.DefaultDelayBox.Text, out var delay) ? delay ?? 0 : 0;
        this.settings.CollapsedCategories = [.. this.groups.Where(group => !group.IsExpanded).Select(group => group.Category.ToString())];
        this.settings.Scenarios = this.rows.ToDictionary(
            row => row.Name,
            row => new GuiSettings.ScenarioSettings { Checked = row.IsChecked, Delay = row.DelayText });
        this.settings.Save();
    }

    private void SetRunning(bool running)
    {
        this.RunButton.IsEnabled = !running;
        this.StopButton.IsEnabled = running;
        this.OpenReportButton.IsEnabled = !running && this.reportPath is not null;
        this.CheckAllButton.IsEnabled = !running;
        this.ScenarioList.IsEnabled = !running;
        this.BrowseButton.IsEnabled = !running;
    }

    private void OnStop(object? sender, RoutedEventArgs e)
    {
        this.stopRequest?.Cancel();
        this.StopButton.IsEnabled = false;
        this.RunStatusText.Text = "stopping after the running test…";
    }

    private void OnOpenReport(object? sender, RoutedEventArgs e)
    {
        if (this.reportPath is not null && File.Exists(this.reportPath))
        {
            Process.Start(new ProcessStartInfo(this.reportPath) { UseShellExecute = true });
        }
    }

    // Closing while clients run would leave them behind.
    private void OnClosing(object? sender, WindowClosingEventArgs e)
    {
        if (this.IsRunning)
        {
            e.Cancel = true;
            this.RunStatusText.Text = "A run is going on: stop it first, and close the window when it has finished.";
            return;
        }

        // Which categories are folded in is kept even without a run.
        this.settings.CollapsedCategories = [.. this.groups.Where(group => !group.IsExpanded).Select(group => group.Category.ToString())];
        this.settings.Save();
    }

    private ScenarioRow Row(string name) => this.rows.First(row => row.Name == name);

    private void AppendLog(string text)
    {
        this.LogBox.Text += text;
        this.LogBox.CaretIndex = this.LogBox.Text?.Length ?? 0;
    }

    /// <summary>The run's log, shown in the window as it is written, from whatever thread writes it.</summary>
    private sealed class WindowLog(Action<string> append) : TextWriter
    {
        public override Encoding Encoding => Encoding.UTF8;

        public override void Write(char value) => this.Write(value.ToString());

        public override void Write(string? value)
        {
            if (!string.IsNullOrEmpty(value))
            {
                Dispatcher.UIThread.Post(() => append(value));
            }
        }
    }
}
