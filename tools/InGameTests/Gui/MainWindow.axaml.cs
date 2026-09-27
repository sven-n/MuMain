using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
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
    private List<ScenarioRow> runRows = [];
    private string? reportPath;
    private TestRunResult? lastRun;

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
                row.QualityText = saved.Quality ?? string.Empty;
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

        this.DefaultQualityBox.Text = startOptions.ScreenshotQualityGiven
            ? startOptions.ScreenshotQuality.ToString(CultureInfo.InvariantCulture)
            : this.settings.ScreenshotQuality.ToString(CultureInfo.InvariantCulture);

        this.BrowseButton.Click += this.OnBrowse;
        this.CheckAllButton.Click += this.OnCheckAll;
        this.RunButton.Click += this.OnRun;
        this.StopButton.Click += this.OnStop;
        this.OpenReportButton.Click += this.OnOpenReport;
        this.OpenReportFolderButton.Click += this.OnOpenReportFolder;
        this.Closing += this.OnClosing;
        this.UpdateSelection();
        this.VersionText.Text = "Client commit: known after a run · server: asking…";
        _ = this.ShowServerVersionAsync();
    }

    // What the test server is, before any run; the client says its commit only when it runs.
    private async Task ShowServerVersionAsync()
    {
        ServerVersion? server = null;
        if (RunnerOptions.TryParseServer(this.ServerBox.Text?.Trim() ?? string.Empty, out var host, out var port))
        {
            server = await Task.Run(() => TestServer.DescribeAsync(host, port, CancellationToken.None));
        }

        if (!this.IsRunning && this.lastRun is null)
        {
            this.VersionText.Text = $"Client commit: known after a run · {DescribeServer(server)}";
        }
    }

    // Changes that were not committed show in the yellow note next to it.
    private static string DescribeClient(ClientVersion? client)
        => client is null ? "tested client: commit unknown" : $"tested client: commit {Short(client.Commit)}";

    private static string DescribeServer(ServerVersion? server)
        => server is null
            ? "server: unknown (not the running test server)"
            : $"server: {server.Name} {server.Version ?? "version unknown"}, commit {(server.Commit is null ? "unknown" : Short(server.Commit))}";

    private static string Short(string commit) => commit.Length > 10 ? commit[..10] : commit;

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

        this.runRows = [.. this.rows.Where(row => row.IsChecked)];
        this.OverallPanel.IsVisible = true;
        this.ShowOverallResult(null);
        this.UpdateOverallProgress();

        var log = new WindowLog(this.AppendLog);
        var listener = new TestRunListener
        {
            StepStarted = (name, number, title) => Dispatcher.UIThread.Post(() => this.Row(name).StepStarted(number, title)),
            StepFinished = (name, step) => Dispatcher.UIThread.Post(() =>
            {
                this.Row(name).StepFinished(step);
                this.UpdateOverallProgress();
            }),
            ScenarioFinished = result => Dispatcher.UIThread.Post(() =>
            {
                this.Row(result.Name).Finished(result);
                this.UpdateOverallProgress();
            }),
        };

        try
        {
            var token = this.stopRequest.Token;
            var run = await Task.Run(() => TestRun.RunAsync(options, log, listener, token));
            this.reportPath = run.ReportPath;
            this.lastRun = run;
            this.VersionText.Text = $"Last run: {DescribeClient(run.Client)} · {DescribeServer(run.Server)}";
            this.ChangedClientNote.IsVisible = run.Client is { Changed: true };
            this.ShowOverallResult(run);
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

        if (!TryParseQuality(this.DefaultQualityBox.Text, out var defaultQuality) || defaultQuality is null)
        {
            return this.Refuse("The screenshot quality is a JPEG quality from 1 to 100.");
        }

        var selections = new List<ScenarioSelection>();
        foreach (var row in this.rows.Where(row => row.IsChecked))
        {
            if (!TryParseDelay(row.DelayText, out var delay))
            {
                return this.Refuse($"The wait of '{row.Name}' is a number of milliseconds, or empty for the value above.");
            }

            if (!TryParseQuality(row.QualityText, out var quality))
            {
                return this.Refuse($"The quality of '{row.Name}' is a JPEG quality from 1 to 100, or empty for the value above.");
            }

            selections.Add(new ScenarioSelection(
                row.Scenario,
                TimeSpan.FromMilliseconds(delay ?? defaultDelay.Value),
                quality ?? defaultQuality.Value));
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

    // Empty is no value; otherwise a whole number from 1 to 100.
    private static bool TryParseQuality(string? text, out int? quality)
    {
        quality = null;
        if (string.IsNullOrWhiteSpace(text))
        {
            return true;
        }

        if (!int.TryParse(text.Trim(), out var value) || value is < 1 or > 100)
        {
            return false;
        }

        quality = value;
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
        this.settings.ScreenshotQuality = TryParseQuality(this.DefaultQualityBox.Text, out var quality) && quality is { } value
            ? value
            : Clients.ClientOptions.DefaultScreenshotQuality;
        this.settings.CollapsedCategories = [.. this.groups.Where(group => !group.IsExpanded).Select(group => group.Category.ToString())];
        this.settings.Scenarios = this.rows.ToDictionary(
            row => row.Name,
            row => new GuiSettings.ScenarioSettings { Checked = row.IsChecked, Delay = row.DelayText, Quality = row.QualityText });
        this.settings.Save();
    }

    private void SetRunning(bool running)
    {
        this.RunButton.IsEnabled = !running;
        this.StopButton.IsEnabled = running;
        this.OpenReportButton.IsEnabled = !running && this.reportPath is not null;
        this.OpenReportFolderButton.IsEnabled = !running;
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
            this.TryOpen(() => Process.Start(new ProcessStartInfo(this.reportPath) { UseShellExecute = true }));
        }
    }

    // A system without a program for the file (e.g. no xdg-open) must not end the window.
    private void TryOpen(Action open)
    {
        try
        {
            open();
        }
        catch (Exception exception) when (exception is System.ComponentModel.Win32Exception or IOException
                                              or UnauthorizedAccessException or InvalidOperationException)
        {
            this.RunStatusText.Text = $"could not open it: {exception.Message}";
        }
    }

    // The file browser at the report, with the file selected where the system
    // can; before a run, or when the report is gone, at the folder the reports go to.
    private void OnOpenReportFolder(object? sender, RoutedEventArgs e) => this.TryOpen(this.OpenReportFolder);

    private void OpenReportFolder()
    {
        if (this.reportPath is null || !File.Exists(this.reportPath))
        {
            var folder = Path.GetFullPath(this.startOptions.OutputFolder);
            Directory.CreateDirectory(folder);
            Process.Start(new ProcessStartInfo(folder) { UseShellExecute = true });
            return;
        }

        if (OperatingSystem.IsWindows())
        {
            Process.Start("explorer.exe", $"/select,\"{this.reportPath}\"");
        }
        else if (OperatingSystem.IsMacOS())
        {
            Process.Start("open", ["-R", this.reportPath]);
        }
        else
        {
            Process.Start(new ProcessStartInfo(Path.GetDirectoryName(this.reportPath)!) { UseShellExecute = true });
        }
    }

    // All steps of the run's tests together. A finished test counts all its
    // steps, passed or not, so the bar is full when the run is.
    private void UpdateOverallProgress()
    {
        var total = this.runRows.Sum(row => row.StepCount);
        var done = this.runRows.Sum(row => row.Passed || row.Failed ? row.StepCount : row.CompletedSteps);
        var finished = this.runRows.Count(row => row.Passed || row.Failed);
        this.OverallProgress.Maximum = Math.Max(total, 1);
        this.OverallProgress.Value = done;
        this.OverallProgress.ProgressTextFormat = $"{done} / {total} steps · {finished} of {this.runRows.Count} tests";
    }

    // The end of a run replaces the overall bar: PASSED in green when every test
    // of the run passed, FAILED in red when one failed; a stopped run keeps the
    // bar. Null shows the bar again, for a new run.
    private void ShowOverallResult(TestRunResult? run)
    {
        var ran = run?.Scenarios.Where(scenario => scenario.Status != ScenarioStatus.Skipped).ToList() ?? [];
        var complete = run is not null && ran.Count == this.runRows.Count;
        var failed = ran.Count(scenario => scenario.Failed);
        var passed = complete && failed == 0;
        var steps = ran.Sum(scenario => scenario.Steps.Count);

        this.OverallPassed.IsVisible = passed;
        this.OverallFailed.IsVisible = run is not null && failed > 0;
        this.OverallProgress.IsVisible = !this.OverallPassed.IsVisible && !this.OverallFailed.IsVisible;
        this.OverallPassedText.Text = $"PASSED · {steps} steps · {Tests(ran.Count)}";
        this.OverallFailedText.Text = $"FAILED · {failed} of {Tests(ran.Count)}";

        static string Tests(int count) => count == 1 ? "1 test" : $"{count} tests";
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
