using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>A scenario to run, the pause after each of its client actions, and its screenshots' JPEG quality.</summary>
internal sealed record ScenarioSelection(Scenario Scenario, TimeSpan StepDelay, int ScreenshotQuality = ClientOptions.DefaultScreenshotQuality);

/// <summary>
/// What a run does: which client, which server, which scenarios, where the results go.
/// <paramref name="AllScenarios"/> are all there are, so the report lists the ones not run as skipped.
/// </summary>
internal sealed record TestRunOptions(
    string ClientPath,
    string ServerHost,
    int ServerPort,
    bool FreshServer,
    string OutputFolder,
    IReadOnlyList<ScenarioSelection> Scenarios,
    IReadOnlyList<Scenario> AllScenarios);

/// <summary>What a run tells whoever watches it, e.g. the window, while it goes.</summary>
internal sealed class TestRunListener
{
    public Action<string>? ScenarioStarted { get; init; }

    public Action<string, int, string>? StepStarted { get; init; }

    public Action<string, StepResult>? StepFinished { get; init; }

    public Action<ScenarioResult>? ScenarioFinished { get; init; }
}

/// <summary>
/// One run of scenarios, the same for the command line and the window: prepares
/// the server, runs the scenarios one after the other and writes the report.
/// </summary>
internal static class TestRun
{
    // The largest file GitHub takes as an attachment of a pull request comment.
    private const int GitHubAttachmentLimitMegabytes = 25;

    // A test server that was just recreated takes a few seconds to listen.
    private static readonly TimeSpan ServerStartTimeout = TimeSpan.FromSeconds(60);

    /// <summary>
    /// Runs the scenarios of <paramref name="options"/>. Cancelling stops after the
    /// scenario that is running; the report covers the scenarios that ran.
    /// </summary>
    /// <exception cref="InvalidOperationException">
    /// The server could not be prepared or does not answer, or the report cannot be written;
    /// when the report fails at the end, the screenshots stay in the work folder the message names.
    /// </exception>
    public static async Task<TestRunResult> RunAsync(
        TestRunOptions options,
        TextWriter log,
        TestRunListener? listener,
        CancellationToken cancellationToken)
    {
        var started = DateTime.Now;
        // Before any scenario: a folder the report cannot go to would lose the whole run at its end.
        var output = EnsureWritable(options.OutputFolder);

        // The screenshots are taken into a work folder; the report takes them in,
        // and the folder goes away: a run leaves one file. When the report cannot
        // be written, the folder stays, so the run is not lost.
        var folder = Path.Combine(Path.GetTempPath(), "mu-in-game-tests", $"run-{started:yyyyMMdd-HHmmss}-{Environment.ProcessId}");
        Directory.CreateDirectory(folder);
        try
        {
            var run = await RunInAsync(folder, output, started, options, log, listener, cancellationToken);
            TryDelete(folder, log);
            return run;
        }
        catch (Exception exception) when (exception is not ReportNotWrittenException)
        {
            // A run that ended before its report has no screenshots worth keeping.
            TryDelete(folder, log);
            throw;
        }
    }

    // The output folder, created and tried with a file, as a full path.
    private static string EnsureWritable(string outputFolder)
    {
        var output = Path.GetFullPath(outputFolder);
        try
        {
            Directory.CreateDirectory(output);
            var probe = Path.Combine(output, $".write-test-{Environment.ProcessId}");
            File.WriteAllText(probe, string.Empty);
            File.Delete(probe);
            return output;
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            throw new InvalidOperationException($"the report cannot be written to {output}: {exception.Message}", exception);
        }
    }

    private static async Task<TestRunResult> RunInAsync(
        string folder,
        string output,
        DateTime started,
        TestRunOptions options,
        TextWriter log,
        TestRunListener? listener,
        CancellationToken cancellationToken)
    {
        if (options.FreshServer)
        {
            await TestServer.RecreateAsync(log, cancellationToken);
        }

        if (!await ServerReadiness.WaitAsync(options.ServerHost, options.ServerPort, ServerStartTimeout, cancellationToken))
        {
            throw new InvalidOperationException($"no game server answers on {options.ServerHost}:{options.ServerPort}; is the test server running?");
        }

        var server = await TestServer.DescribeAsync(options.ServerHost, options.ServerPort, cancellationToken);
        ClientVersion? client = null;
        var results = new List<ScenarioResult>();
        foreach (var selection in options.Scenarios)
        {
            if (cancellationToken.IsCancellationRequested)
            {
                log.WriteLine("stopped");
                break;
            }

            var scenario = selection.Scenario;
            listener?.ScenarioStarted?.Invoke(scenario.Name);
            var clientOptions = new ClientOptions(options.ClientPath, options.ServerHost, options.ServerPort)
            {
                StepDelay = selection.StepDelay,
                ScreenshotQuality = selection.ScreenshotQuality,
            };
            var runner = new ScenarioRunner(
                clientOptions,
                Path.Combine(folder, scenario.Name),
                log,
                (number, title) => listener?.StepStarted?.Invoke(scenario.Name, number, title),
                step => listener?.StepFinished?.Invoke(scenario.Name, step),
                startedClient => client ??= startedClient.Version);
            var result = await runner.RunAsync(scenario, CancellationToken.None);
            results.Add(result);
            listener?.ScenarioFinished?.Invoke(result);
        }

        // Every scenario there is, in its order; the ones that did not run as skipped.
        var selected = options.Scenarios.Select(selection => selection.Scenario.Name).ToHashSet();
        var reported = options.AllScenarios
            .OrderBy(scenario => scenario.Category)
            .Select(scenario => results.FirstOrDefault(result => result.Name == scenario.Name)
                                ?? ScenarioResult.Skip(scenario, selected.Contains(scenario.Name) ? "the run was stopped before it" : "not selected for this run"))
            .Concat(results.Where(result => options.AllScenarios.All(scenario => scenario.Name != result.Name)))
            .ToList();

        var reportPath = Path.Combine(output, $"in-game-report-{started:yyyyMMdd-HHmmss}.html");
        var run = new TestRunResult(started, options, reported, reportPath, client, server);
        try
        {
            await File.WriteAllTextAsync(reportPath, HtmlReport.Render(run, JsonSerializer.Serialize(Summary(run), JsonOptions)), CancellationToken.None);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            throw new ReportNotWrittenException($"the report could not be written to {reportPath} ({exception.Message}); the screenshots are kept in {folder}", exception);
        }
        log.WriteLine($"{run.Count(ScenarioStatus.Passed)} passed, {run.Count(ScenarioStatus.Failed)} failed, {run.Count(ScenarioStatus.Skipped)} skipped");
        log.WriteLine($"report: {reportPath}");
        var megabytes = new FileInfo(reportPath).Length / (1024.0 * 1024.0);
        if (megabytes > GitHubAttachmentLimitMegabytes)
        {
            log.WriteLine($"the report has {megabytes:0.0} MB; GitHub takes files up to {GitHubAttachmentLimitMegabytes} MB in a pull request comment");
        }

        return run;
    }

    // Screenshots and the like; a folder that cannot go away costs only disk space.
    private static void TryDelete(string folder, TextWriter log)
    {
        try
        {
            Directory.Delete(folder, recursive: true);
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            log.WriteLine($"could not remove the work folder {folder}: {exception.Message}");
        }
    }

    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        Converters = { new System.Text.Json.Serialization.JsonStringEnumConverter() },
    };

    // The run without the scenario objects, for tools that read the results.
    private static object Summary(TestRunResult run) => new
    {
        started = run.Started,
        client = run.Options.ClientPath,
        server = $"{run.Options.ServerHost}:{run.Options.ServerPort}",
        freshServer = run.Options.FreshServer,
        clientCommit = run.Client?.Commit,
        clientChanged = run.Client?.Changed,
        serverName = run.Server?.Name,
        serverVersion = run.Server?.Version,
        serverCommit = run.Server?.Commit,
        passed = run.AllPassed,
        scenarios = run.Scenarios.Select(scenario => new
        {
            name = scenario.Name,
            description = scenario.Description,
            category = scenario.Category.ToString(),
            status = scenario.Status.ToString(),
            seconds = Math.Round(scenario.Duration.TotalSeconds, 1),
            stepDelayMilliseconds = scenario.StepDelay.TotalMilliseconds,
            screenshotQuality = scenario.ScreenshotQuality,
            failure = scenario.Failure,
            skipReason = scenario.SkipReason,
            // The screenshots are in the page, by step and client; the failure
            // details once, in the page's blocks.
            steps = scenario.Steps.Select(step => new
            {
                number = step.Number,
                title = step.Title,
                expectation = step.Expectation,
                passed = step.Passed,
                seconds = Math.Round(step.Duration.TotalSeconds, 1),
                failure = step.Failure,
                screenshots = step.Screenshots.Select(screenshot => screenshot.Role),
            }),
        }),
    };
}

/// <summary>The report of a finished run could not be written; its screenshots are kept.</summary>
internal sealed class ReportNotWrittenException(string message, Exception innerException)
    : InvalidOperationException(message, innerException);
