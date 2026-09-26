// In-game tests: start editor builds of the client with the control socket,
// drive them through scenarios against a running server, and report each
// scenario as passed or failed. See docs/in-game-tests.md.

using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Scenarios;

namespace MuMain.Tools.InGameTests;

internal static class Program
{
    private const int ExitPassed = 0;
    private const int ExitFailed = 1;
    private const int ExitUsage = 2;

    private static readonly Scenario[] AllScenarios =
    [
        new TradeScenario(),
        new IcarusTakeOffScenario(),
    ];

    public static async Task<int> Main(string[] args)
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

        var selected = options.ScenarioNames.Count == 0
            ? AllScenarios
            : AllScenarios.Where(scenario => options.ScenarioNames.Contains(scenario.Name)).ToArray();
        var unknown = options.ScenarioNames.Except(AllScenarios.Select(scenario => scenario.Name)).ToList();
        if (unknown.Count > 0)
        {
            Console.Error.WriteLine($"unknown scenario: {string.Join(", ", unknown)}; see --list");
            return ExitUsage;
        }

        var clientOptions = new ClientOptions(options.ClientPath!, options.ServerHost, options.ServerPort);
        var runner = new ScenarioRunner(clientOptions, options.OutputFolder, Console.Out);
        var failed = 0;
        foreach (var scenario in selected)
        {
            if (!await runner.RunAsync(scenario, CancellationToken.None))
            {
                failed++;
            }
        }

        Console.WriteLine($"{selected.Length - failed} passed, {failed} failed");
        return failed == 0 ? ExitPassed : ExitFailed;
    }
}
