// In-game tests: start editor builds of the client with the control socket,
// drive them through scenarios against a running server, and report each
// scenario, step by step with screenshots. See docs/in-game-tests.md.

using MuMain.Tools.InGameTests.Gui;
using MuMain.Tools.InGameTests.Running;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests;

internal static class Program
{
    private const int ExitPassed = 0;
    private const int ExitFailed = 1;
    private const int ExitUsage = 2;

    /// <summary>Every scenario, in the order they run.</summary>
    public static readonly Scenario[] AllScenarios =
    [
        new TradeScenario(),
        new IcarusTakeOffScenario(),
    ];

    // The window's file dialogs need a single-threaded apartment on Windows.
    [STAThread]
    public static int Main(string[] args)
    {
        var options = RunnerOptions.Parse(args, out var usageError);
        if (options is null)
        {
            Console.Error.WriteLine(usageError);
            Console.Error.WriteLine(RunnerOptions.Usage);
            return ExitUsage;
        }

        if (options.ListScenarios)
        {
            foreach (var scenario in AllScenarios)
            {
                Console.WriteLine($"{scenario.Name,-20} {scenario.Description}");
            }

            return ExitPassed;
        }

        return options.Gui ? GuiApp.Run(options, args) : RunAsync(options).GetAwaiter().GetResult();
    }

    private static async Task<int> RunAsync(RunnerOptions options)
    {
        var unknown = options.ScenarioNames.Except(AllScenarios.Select(scenario => scenario.Name)).ToList();
        if (unknown.Count > 0)
        {
            Console.Error.WriteLine($"unknown scenario: {string.Join(", ", unknown)}; see --list");
            return ExitUsage;
        }

        var selected = options.ScenarioNames.Count == 0
            ? AllScenarios
            : AllScenarios.Where(scenario => options.ScenarioNames.Contains(scenario.Name)).ToArray();
        var runOptions = new TestRunOptions(
            Path.GetFullPath(options.ClientPath!),
            options.ServerHost,
            options.ServerPort,
            options.FreshServer,
            options.OutputFolder,
            [.. selected.Select(scenario => new ScenarioSelection(scenario, options.StepDelay))]);
        try
        {
            var run = await TestRun.RunAsync(runOptions, Console.Out, null, CancellationToken.None);
            return run.AllPassed ? ExitPassed : ExitFailed;
        }
        catch (InvalidOperationException exception)
        {
            Console.Error.WriteLine(exception.Message);
            return ExitFailed;
        }
    }
}
