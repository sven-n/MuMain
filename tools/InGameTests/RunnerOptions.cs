namespace MuMain.Tools.InGameTests;

/// <summary>The command line of the test runner.</summary>
internal sealed class RunnerOptions
{
    public const string Usage = """
        usage: InGameTests --client <path to Main> [--server <host:port>] [--scenario <name>]... [--out <folder>]
               InGameTests --list

          --client    an editor build of Main (ENABLE_CONTROL_SOCKET=ON), e.g. out/build/windows-x64-mueditor/src/Release/Main.exe
          --server    the server the clients connect to, default 127.0.0.1:44405
          --scenario  run only this scenario; repeat for several, default all
          --out       where failure screenshots, events and states go, default in-game-test-results
          --list      list the scenarios
        """;

    private const string DefaultHost = "127.0.0.1";
    private const int DefaultPort = 44405;

    public string? ClientPath { get; private set; }

    public string ServerHost { get; private set; } = DefaultHost;

    public int ServerPort { get; private set; } = DefaultPort;

    public List<string> ScenarioNames { get; } = [];

    public string OutputFolder { get; private set; } = "in-game-test-results";

    public bool ListScenarios { get; private set; }

    /// <summary>Parses <paramref name="args"/>; null with <paramref name="error"/> when they are not usable.</summary>
    public static RunnerOptions? Parse(string[] args, out string error)
    {
        var options = new RunnerOptions();
        error = string.Empty;
        for (var i = 0; i < args.Length; i++)
        {
            var value = i + 1 < args.Length ? args[i + 1] : null;
            switch (args[i])
            {
                case "--list":
                    options.ListScenarios = true;
                    continue;
                case "--client" when value is not null:
                    options.ClientPath = value;
                    break;
                case "--scenario" when value is not null:
                    options.ScenarioNames.Add(value);
                    break;
                case "--out" when value is not null:
                    options.OutputFolder = value;
                    break;
                case "--server" when value is not null:
                    if (!options.TrySetServer(value))
                    {
                        error = $"--server takes host:port, not '{value}'";
                        return null;
                    }

                    break;
                default:
                    error = $"unknown or incomplete option '{args[i]}'";
                    return null;
            }

            i++;
        }

        if (!options.ListScenarios && (options.ClientPath is null || !File.Exists(options.ClientPath)))
        {
            error = $"--client has to name an existing Main, not '{options.ClientPath}'";
            return null;
        }

        return options;
    }

    private bool TrySetServer(string value)
    {
        var separator = value.LastIndexOf(':');
        if (separator <= 0 || !int.TryParse(value[(separator + 1)..], out var port) || port is <= 0 or > ushort.MaxValue)
        {
            return false;
        }

        this.ServerHost = value[..separator];
        this.ServerPort = port;
        return true;
    }
}
