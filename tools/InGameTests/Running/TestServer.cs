using System.Diagnostics;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The test server of docker-compose.yml: OpenMU in demo mode, which keeps its
/// data in memory. Recreating the container brings back OpenMU's test data,
/// whatever the last run changed.
/// </summary>
internal static class TestServer
{
    private static readonly TimeSpan RecreateTimeout = TimeSpan.FromMinutes(3);

    /// <summary>The compose file, copied next to the runner by the build.</summary>
    public static string ComposeFile => Path.Combine(AppContext.BaseDirectory, "docker-compose.yml");

    /// <summary>Recreates the test server with podman or docker, whichever is installed.</summary>
    /// <exception cref="InvalidOperationException">Neither is installed, or compose failed.</exception>
    public static async Task RecreateAsync(TextWriter log, CancellationToken cancellationToken)
    {
        var tool = FindContainerTool()
                   ?? throw new InvalidOperationException("neither podman nor docker is installed; start the test server by hand, or run without a fresh server");
        log.WriteLine($"recreating the test server with {Path.GetFileName(tool)}");

        var startInfo = new ProcessStartInfo(tool)
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
        };
        foreach (var argument in new[] { "compose", "-f", ComposeFile, "up", "-d", "--force-recreate" })
        {
            startInfo.ArgumentList.Add(argument);
        }

        RepairFolderVariables(startInfo);

        using var process = Process.Start(startInfo) ?? throw new InvalidOperationException($"could not start {tool}");
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(RecreateTimeout);
        var output = process.StandardOutput.ReadToEndAsync(timeout.Token);
        var errors = process.StandardError.ReadToEndAsync(timeout.Token);
        await process.WaitForExitAsync(timeout.Token);
        if (process.ExitCode != 0)
        {
            throw new InvalidOperationException($"`{Path.GetFileName(tool)} compose up` failed ({process.ExitCode}): {(await errors).Trim()} {(await output).Trim()}");
        }
    }

    // podman on Windows finds its connection to the VM through %APPDATA%. Some
    // terminals start their shells without it (e.g. the one inside the Claude
    // desktop app), and podman then falls back to a socket that does not exist
    // and reports "Cannot connect to Podman". The folders come from Windows.
    private static void RepairFolderVariables(ProcessStartInfo startInfo)
    {
        if (!OperatingSystem.IsWindows())
        {
            return;
        }

        foreach (var (name, folder) in new[]
                 {
                     ("APPDATA", Environment.SpecialFolder.ApplicationData),
                     ("LOCALAPPDATA", Environment.SpecialFolder.LocalApplicationData),
                 })
        {
            var current = startInfo.Environment.TryGetValue(name, out var value) ? value : null;
            if (string.IsNullOrEmpty(current) || !Directory.Exists(current))
            {
                startInfo.Environment[name] = Environment.GetFolderPath(folder);
            }
        }
    }

    private static string? FindContainerTool()
    {
        var names = OperatingSystem.IsWindows() ? new[] { "podman.exe", "docker.exe" } : ["podman", "docker"];
        var folders = (Environment.GetEnvironmentVariable("PATH") ?? string.Empty).Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries);
        return names.SelectMany(name => folders.Select(folder => Path.Combine(folder, name))).FirstOrDefault(File.Exists);
    }
}
