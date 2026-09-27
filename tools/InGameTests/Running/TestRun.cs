using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>A scenario to run, and the pause after each of its client actions.</summary>
internal sealed record ScenarioSelection(Scenario Scenario, TimeSpan StepDelay);

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
    // A test server that was just recreated takes a few seconds to listen.
    private static readonly TimeSpan ServerStartTimeout = TimeSpan.FromSeconds(60);

    /// <summary>
    /// Runs the scenarios of <paramref name="options"/>. Cancelling stops after the
    /// scenario that is running; the report covers the scenarios that ran.
    /// </summary>
    /// <exception cref="InvalidOperationException">The server could not be prepared or does not answer.</exception>
    public static async Task<TestRunResult> RunAsync(
        TestRunOptions options,
        TextWriter log,
        TestRunListener? listener,
        CancellationToken cancellationToken)
    {
        var started = DateTime.Now;
        var folder = Path.GetFullPath(Path.Combine(options.OutputFolder, started.ToString("yyyyMMdd-HHmmss")));
        Directory.CreateDirectory(folder);

        if (options.FreshServer)
        {
            await TestServer.RecreateAsync(log, cancellationToken);
        }

        if (!await ServerReadiness.WaitAsync(options.ServerHost, options.ServerPort, ServerStartTimeout, cancellationToken))
        {
            throw new InvalidOperationException($"no game server answers on {options.ServerHost}:{options.ServerPort}; is the test server running?");
        }

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
            };
            var runner = new ScenarioRunner(
                clientOptions,
                Path.Combine(folder, scenario.Name),
                log,
                (number, title) => listener?.StepStarted?.Invoke(scenario.Name, number, title),
                step => listener?.StepFinished?.Invoke(scenario.Name, step));
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

        var reportPath = Path.Combine(folder, "report.html");
        var run = new TestRunResult(started, options, reported, folder, reportPath);
        await File.WriteAllTextAsync(reportPath, HtmlReport.Render(run), CancellationToken.None);
        await File.WriteAllTextAsync(Path.Combine(folder, "results.json"), JsonSerializer.Serialize(Summary(run), JsonOptions), CancellationToken.None);
        log.WriteLine($"{run.Count(ScenarioStatus.Passed)} passed, {run.Count(ScenarioStatus.Failed)} failed, {run.Count(ScenarioStatus.Skipped)} skipped");
        log.WriteLine($"report: {reportPath}");
        return run;
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
        passed = run.AllPassed,
        scenarios = run.Scenarios,
    };
}
