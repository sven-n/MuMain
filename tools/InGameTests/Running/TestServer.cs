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

    private static string? FindContainerTool()
    {
        var names = OperatingSystem.IsWindows() ? new[] { "podman.exe", "docker.exe" } : ["podman", "docker"];
        var folders = (Environment.GetEnvironmentVariable("PATH") ?? string.Empty).Split(Path.PathSeparator, StringSplitOptions.RemoveEmptyEntries);
        return names.SelectMany(name => folders.Select(folder => Path.Combine(folder, name))).FirstOrDefault(File.Exists);
    }
}
