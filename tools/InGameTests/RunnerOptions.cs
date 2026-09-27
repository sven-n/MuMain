namespace MuMain.Tools.InGameTests;

/// <summary>The command line of the test runner.</summary>
internal sealed class RunnerOptions
{
    public const string Usage = """
        usage: InGameTests --client <path to Main> [--server <host:port>] [--fresh-server] [--scenario <name>]... [--out <folder>] [-t <ms>] [-q <1-100>]
               InGameTests --gui [--client <path to Main>] [--server <host:port>] [--fresh-server] [--out <folder>]
               InGameTests --list

          --client        an editor build of Main (ENABLE_CONTROL_SOCKET=ON); default the Main next to the tester
          --server        the game server the clients log in to, default 127.0.0.1:56901, the test server
          --fresh-server  recreate the test server (docker-compose.yml, with podman or docker) before the run
          --scenario      run only this scenario; repeat for several, default all
          --out           where the report goes (one HTML file per run), default in-game-test-results
                          (next to the tester when it sits next to Main)
          -t              pause this many milliseconds after every action, to watch a scenario, default 0
          -q              JPEG quality of the step screenshots, 1 to 100, default 70; lower makes the report smaller
          --gui           open the window to choose and run scenarios; also without any option
          --list          list the scenarios
        """;

    private const string DefaultHost = "127.0.0.1";
    private const int DefaultPort = 56901;
    private const string DefaultOutputFolder = "in-game-test-results";

    public string? ClientPath { get; private set; }

    public string ServerHost { get; private set; } = DefaultHost;

    public int ServerPort { get; private set; } = DefaultPort;

    /// <summary>Whether --server was given, so the window can prefer it over its saved value.</summary>
    public bool ServerGiven { get; private set; }

    public bool FreshServer { get; private set; }

    public List<string> ScenarioNames { get; } = [];

    public string OutputFolder { get; private set; } = DefaultOutputFolder;

    public bool ListScenarios { get; private set; }

    public bool Gui { get; private set; }

    public TimeSpan StepDelay { get; private set; } = TimeSpan.Zero;

    public int ScreenshotQuality { get; private set; } = Clients.ClientOptions.DefaultScreenshotQuality;

    /// <summary>Whether -q was given, so the window can prefer it over its saved value.</summary>
    public bool ScreenshotQualityGiven { get; private set; }

    /// <summary>Parses <paramref name="args"/>; null with <paramref name="error"/> when they are not usable.</summary>
    public static RunnerOptions? Parse(string[] args, out string error)
    {
        var options = new RunnerOptions { Gui = args.Length == 0 };
        var outputGiven = false;
        error = string.Empty;
        for (var i = 0; i < args.Length; i++)
        {
            var value = i + 1 < args.Length ? args[i + 1] : null;
            switch (args[i])
            {
                case "--list":
                    options.ListScenarios = true;
                    continue;
                case "--gui":
                    options.Gui = true;
                    continue;
                case "--fresh-server":
                    options.FreshServer = true;
                    continue;
                case "--client" when value is not null:
                    options.ClientPath = value;
                    break;
                case "--scenario" when value is not null:
                    options.ScenarioNames.Add(value);
                    break;
                case "--out" when value is not null:
                    options.OutputFolder = value;
                    outputGiven = true;
                    break;
                case "-t" when value is not null:
                    if (!int.TryParse(value, out var milliseconds) || milliseconds < 0)
                    {
                        error = $"-t takes milliseconds, not '{value}'";
                        return null;
                    }

                    options.StepDelay = TimeSpan.FromMilliseconds(milliseconds);
                    break;
                case "-q" when value is not null:
                    if (!int.TryParse(value, out var quality) || quality is < 1 or > 100)
                    {
                        error = $"-q takes a JPEG quality from 1 to 100, not '{value}'";
                        return null;
                    }

                    (options.ScreenshotQuality, options.ScreenshotQualityGiven) = (quality, true);
                    break;
                case "--server" when value is not null:
                    if (!TryParseServer(value, out var host, out var port))
                    {
                        error = $"--server takes host:port, not '{value}'";
                        return null;
                    }

                    (options.ServerHost, options.ServerPort, options.ServerGiven) = (host, port, true);
                    break;
                default:
                    error = $"unknown or incomplete option '{args[i]}'";
                    return null;
            }

            i++;
        }

        // Published next to Main (ENABLE_IN_GAME_TESTS), the tester finds its
        // client and keeps its reports there, wherever it is started from.
        if (ClientNextToTester() is { } client)
        {
            options.ClientPath ??= client;
            if (!outputGiven)
            {
                options.OutputFolder = Path.Combine(AppContext.BaseDirectory, DefaultOutputFolder);
            }
        }

        var needsClient = !options.ListScenarios && !options.Gui;
        if (needsClient && (options.ClientPath is null || !File.Exists(options.ClientPath)))
        {
            error = $"--client has to name an existing Main, not '{options.ClientPath}'";
            return null;
        }

        return options;
    }

    // Main next to the tester: Main.exe on Windows, Main on Linux, the bundle on macOS.
    private static string? ClientNextToTester()
        => new[] { "Main.exe", "Main", Path.Combine("Main.app", "Contents", "MacOS", "Main") }
            .Select(name => Path.Combine(AppContext.BaseDirectory, name))
            .FirstOrDefault(File.Exists);

    /// <summary>Splits <c>host:port</c>.</summary>
    public static bool TryParseServer(string value, out string host, out int port)
    {
        host = string.Empty;
        port = 0;
        var separator = value.LastIndexOf(':');
        if (separator <= 0 || !int.TryParse(value[(separator + 1)..], out port) || port is <= 0 or > ushort.MaxValue)
        {
            return false;
        }

        host = value[..separator];
        return true;
    }
}
