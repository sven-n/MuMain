using System.Diagnostics;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The test server of docker-compose.yml: OpenMU in demo mode, which keeps its
/// data in memory. Recreating the container brings back OpenMU's test data,
/// whatever the last run changed.
/// </summary>
internal static class TestServer
{
    private static readonly TimeSpan RecreateTimeout = TimeSpan.FromMinutes(3);

    /// <summary>Writes the compose file the tester carries to its temp folder and returns its path.</summary>
    private static async Task<string> WriteComposeFileAsync(CancellationToken cancellationToken)
    {
        var folder = Path.Combine(Path.GetTempPath(), "mu-in-game-tests");
        Directory.CreateDirectory(folder);
        var path = Path.Combine(folder, "docker-compose.yml");
        await using var resource = typeof(TestServer).Assembly.GetManifestResourceStream("docker-compose.yml")
                                   ?? throw new InvalidOperationException("the tester carries no docker-compose.yml");
        await using var file = File.Create(path);
        await resource.CopyToAsync(file, cancellationToken);
        return path;
    }

    /// <summary>Recreates the test server with podman or docker, whichever is installed.</summary>
    /// <exception cref="InvalidOperationException">Neither is installed, or compose failed.</exception>
    public static async Task RecreateAsync(TextWriter log, CancellationToken cancellationToken)
    {
        var tool = FindContainerTool()
                   ?? throw new InvalidOperationException("neither podman nor docker is installed; start the test server by hand, or run without a fresh server");
        var toolName = Path.GetFileName(tool);
        log.WriteLine($"recreating the test server with {toolName}");

        // The compose file names its project, so where it is written does not matter.
        string[] compose = ["compose", "-f", await WriteComposeFileAsync(cancellationToken), "up", "-d", "--force-recreate"];
        var (exitCode, output) = await RunAsync(tool, compose, null, cancellationToken);
        if (exitCode != 0 && IsPodman(tool) && output.Contains("Cannot connect to Podman", StringComparison.Ordinal))
        {
            // podman keeps the connection to its machine in a file that can be
            // missing (e.g. the machine was set up by a program whose AppData
            // Windows redirects); podman then knows no machine at all. The
            // machine's own configuration still describes it.
            if (await WriteMachineConnectionsAsync(tool, cancellationToken) is { } connections)
            {
                log.WriteLine("podman has no connection to its machine; connecting as `podman machine inspect` describes it");
                (exitCode, output) = await RunAsync(tool, compose, new() { ["PODMAN_CONNECTIONS_CONF"] = connections }, cancellationToken);
            }
        }

        if (exitCode != 0)
        {
            throw new InvalidOperationException($"`{toolName} compose up` failed ({exitCode}): {output}");
        }
    }

    // A connections file like the one `podman machine init` writes, for the
    // running machine: podman's own connection with IsMachine, so compose uses
    // the machine's forwarded socket. Null when no machine runs.
    private static async Task<string?> WriteMachineConnectionsAsync(string podman, CancellationToken cancellationToken)
    {
        var (exitCode, output) = await RunAsync(podman, ["machine", "inspect"], null, cancellationToken, errorsToo: false);
        if (exitCode != 0)
        {
            return null;
        }

        var machine = JsonNode.Parse(output)?.AsArray().FirstOrDefault(entry => (string?)entry?["State"] == "running");
        if (machine?["Name"]?.GetValue<string>() is not { } name
            || machine["SSHConfig"] is not { } ssh
            || ssh["Port"]?.GetValue<int>() is not { } port
            || ssh["IdentityPath"]?.GetValue<string>() is not { } identity)
        {
            return null;
        }

        var user = ssh["RemoteUsername"]?.GetValue<string>() ?? "user";
        var rootful = machine["Rootful"]?.GetValue<bool>() ?? false;
        var uid = MachineUserId(machine["ConfigDir"]?["Path"]?.GetValue<string>(), name);

        var connections = new JsonObject
        {
            [name] = Connection($"ssh://{user}@127.0.0.1:{port}/run/user/{uid}/podman/podman.sock", identity),
            [$"{name}-root"] = Connection($"ssh://root@127.0.0.1:{port}/run/podman/podman.sock", identity),
        };
        var file = new JsonObject
        {
            ["Connection"] = new JsonObject
            {
                ["Default"] = rootful ? $"{name}-root" : name,
                ["Connections"] = connections,
            },
            ["Farm"] = new JsonObject(),
        };

        var folder = Path.Combine(Path.GetTempPath(), "mu-in-game-tests");
        Directory.CreateDirectory(folder);
        var path = Path.Combine(folder, "podman-connections.json");
        await File.WriteAllTextAsync(path, file.ToJsonString(), cancellationToken);
        return path;

        static JsonObject Connection(string uri, string identity) => new()
        {
            ["URI"] = uri,
            ["Identity"] = identity,
            ["IsMachine"] = true,
        };
    }

    // The machine user's id, which names its podman socket; podman keeps it in
    // the machine's configuration. 1000 is what podman gives the user.
    private static int MachineUserId(string? configDir, string name)
    {
        const int DefaultUserId = 1000;
        try
        {
            var config = configDir is null ? null : Path.Combine(configDir, $"{name}.json");
            return config is not null && File.Exists(config)
                   && JsonNode.Parse(File.ReadAllText(config))?["HostUser"]?["UID"]?.GetValue<int>() is { } uid
                ? uid
                : DefaultUserId;
        }
        catch (Exception exception) when (exception is IOException or JsonException or InvalidOperationException)
        {
            return DefaultUserId;
        }
    }

    private static async Task<(int ExitCode, string Output)> RunAsync(
        string tool,
        IEnumerable<string> arguments,
        Dictionary<string, string>? environment,
        CancellationToken cancellationToken,
        bool errorsToo = true)
    {
        var startInfo = new ProcessStartInfo(tool)
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
        };
        foreach (var argument in arguments)
        {
            startInfo.ArgumentList.Add(argument);
        }

        foreach (var (name, value) in environment ?? [])
        {
            startInfo.Environment[name] = value;
        }

        using var process = Process.Start(startInfo) ?? throw new InvalidOperationException($"could not start {tool}");
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(RecreateTimeout);
        var output = process.StandardOutput.ReadToEndAsync(timeout.Token);
        var errors = process.StandardError.ReadToEndAsync(timeout.Token);
        await process.WaitForExitAsync(timeout.Token);
        var text = errorsToo ? $"{(await errors).Trim()} {(await output).Trim()}".Trim() : (await output).Trim();
        return (process.ExitCode, text);
    }

    private static bool IsPodman(string tool) => Path.GetFileNameWithoutExtension(tool).Equals("podman", StringComparison.OrdinalIgnoreCase);

    private static string? FindContainerTool()
    {
        var names = OperatingSystem.IsWindows() ? new[] { "podman.exe", "docker.exe" } : ["podman", "docker"];
        var folders = (Environment.GetEnvironmentVariable("PATH") ?? string.Empty).Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries);
        return names.SelectMany(name => folders.Select(folder => Path.Combine(folder, name))).FirstOrDefault(File.Exists);
    }
}
