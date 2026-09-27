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
        new IcarusFlyingItemTakeOffScenario(),
    ];

    // The window's file dialogs need a single-threaded apartment on Windows.
    [STAThread]
    public static int Main(string[] args)
    {
        AttachToParentConsole();

        // The log and the report are English: "123,456 zen", "12.5 s", whatever the
        // machine's language.
        System.Globalization.CultureInfo.DefaultThreadCurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
        System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;

        var options = RunnerOptions.Parse(args, out var usageError);
        if (options is null)
        {
            Console.Error.WriteLine(usageError);
            Console.Error.WriteLine(RunnerOptions.Usage);
            return ExitUsage;
        }

        if (options.ListScenarios)
        {
            foreach (var group in AllScenarios.GroupBy(scenario => scenario.Category).OrderBy(group => group.Key))
            {
                Console.WriteLine(group.Key.DisplayName());
                foreach (var scenario in group)
                {
                    Console.WriteLine($"  {scenario.Name,-30} {scenario.Description}");
                }
            }

            return ExitPassed;
        }

        return options.Gui ? GuiApp.Run(options, args) : RunAsync(options).GetAwaiter().GetResult();
    }

    // On Windows the tester is a window program (WinExe), so starting it, e.g. by a
    // double-click, opens no empty console window. Started from a console, it
    // writes its log there; with its output redirected (the build target, a
    // script) it has that already. Must run before anything touches Console.
    private static void AttachToParentConsole()
    {
        const int AttachParentProcess = -1;
        const int StdOutputHandle = -11;
        if (OperatingSystem.IsWindows() && GetStdHandle(StdOutputHandle) == IntPtr.Zero)
        {
            AttachConsole(AttachParentProcess);
        }
    }

    [System.Runtime.InteropServices.DllImport("kernel32.dll")]
    private static extern bool AttachConsole(int processId);

    [System.Runtime.InteropServices.DllImport("kernel32.dll")]
    private static extern IntPtr GetStdHandle(int standardHandle);

    private static async Task<int> RunAsync(RunnerOptions options)
    {
        var unknown = options.ScenarioNames.Except(AllScenarios.Select(scenario => scenario.Name)).ToList();
        if (unknown.Count > 0)
        {
            Console.Error.WriteLine($"unknown scenario: {string.Join(", ", unknown)}; see --list");
            return ExitUsage;
        }

        var selected = AllScenarios
            .Where(scenario => options.ScenarioNames.Count == 0 || options.ScenarioNames.Contains(scenario.Name))
            .OrderBy(scenario => scenario.Category)
            .ToArray();
        var runOptions = new TestRunOptions(
            Path.GetFullPath(options.ClientPath!),
            options.ServerHost,
            options.ServerPort,
            options.FreshServer,
            options.OutputFolder,
            [.. selected.Select(scenario => new ScenarioSelection(scenario, options.StepDelay, options.ScreenshotQuality))],
            AllScenarios);
        try
        {
            var run = await TestRun.RunAsync(runOptions, Console.Out, null, CancellationToken.None);
            return run.AllPassed ? ExitPassed : ExitFailed;
        }
        catch (Exception exception) when (exception is InvalidOperationException or OperationCanceledException or IOException
                                              or UnauthorizedAccessException)
        {
            Console.Error.WriteLine(exception.Message);
            return ExitFailed;
        }
    }
}
