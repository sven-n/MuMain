using System.Diagnostics;
using System.Text.Json;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Clients;

/// <summary>
/// A running client, driven through its control socket. The methods are the
/// steps scenarios are written in; see docs/control-socket.md for the commands.
/// </summary>
internal sealed class GameClient : IAsyncDisposable
{
    private static readonly TimeSpan CommandTimeout = TimeSpan.FromSeconds(30);
    private static readonly TimeSpan WorldTimeout = TimeSpan.FromSeconds(90);
    private static readonly TimeSpan ConnectRetryInterval = TimeSpan.FromMilliseconds(500);

    private readonly Process process;
    private readonly ControlConnection control;

    private GameClient(string role, Process process, ControlConnection control)
    {
        this.Role = role;
        this.process = process;
        this.control = control;
    }

    /// <summary>The scenario's name for this client, e.g. "seller".</summary>
    public string Role { get; }

    /// <summary>Starts a client and waits until its control socket answers.</summary>
    public static async Task<GameClient> StartAsync(ClientOptions options, string role, CancellationToken cancellationToken)
    {
        var socketPath = SocketPath(role);
        File.Delete(socketPath);

        // The client finds its data next to itself, so it runs in its own folder.
        var executable = Path.GetFullPath(options.ExecutablePath);
        var startInfo = new ProcessStartInfo(executable, $"/u{options.ServerHost} /p{options.ServerPort}")
        {
            WorkingDirectory = Path.GetDirectoryName(executable)!,
            UseShellExecute = false,
        };
        startInfo.Environment["MU_CONTROL_SOCKET"] = socketPath;
        var process = Process.Start(startInfo) ?? throw new InvalidOperationException($"could not start {options.ExecutablePath}");

        try
        {
            var control = await ConnectAsync(socketPath, process, options.StartTimeout, cancellationToken);
            var client = new GameClient(role, process, control);
            await client.SendAsync("ping");
            return client;
        }
        catch
        {
            process.Kill(entireProcessTree: true);
            process.Dispose();
            throw;
        }
    }

    /// <summary>Sends a command and returns its result.</summary>
    public Task<JsonElement> SendAsync(string command, object? fields = null, TimeSpan? timeout = null)
        => this.control.SendAsync(command, fields, timeout ?? CommandTimeout);

    /// <summary>Logs in and enters the world with <paramref name="character"/>.</summary>
    public async Task EnterWorldAsync(string account, string password, string character)
    {
        await this.SendAsync("login", new { account, password }, WorldTimeout);
        await this.SendAsync("select-char", new { name = character }, WorldTimeout);
    }

    /// <summary>The character and everything around it.</summary>
    public Task<JsonElement> StateAsync() => this.SendAsync("state");

    /// <summary>The names of the open windows.</summary>
    public async Task<IReadOnlyList<string>> OpenWindowsAsync()
    {
        var ui = await this.SendAsync("ui");
        return ui.GetProperty("windows").EnumerateArray().Select(window => window.GetString()!).ToList();
    }

    /// <summary>Opens the inventory with its key, unless it is open already.</summary>
    public async Task OpenInventoryAsync()
    {
        if (!(await this.OpenWindowsAsync()).Contains("inventory"))
        {
            await this.SendAsync("hotkey", new { key = "i" });
        }
    }

    /// <summary>Clicks the square of <paramref name="slot"/> in <paramref name="grid"/> like a player.</summary>
    public async Task ClickSlotAsync(string grid, int slot, string button = "left")
    {
        var pixel = await this.SendAsync("slot-pixel", new { grid, slot });
        await this.ClickAsync(pixel.GetProperty("x").GetDouble(), pixel.GetProperty("y").GetDouble(), button);
    }

    /// <summary>Clicks a named window element that <c>ui</c> reports, e.g. "trade.confirm".</summary>
    public async Task ClickElementAsync(string element)
    {
        var ui = await this.SendAsync("ui");
        if (!ui.GetProperty("elements").TryGetProperty(element, out var rect))
        {
            throw new InvalidOperationException($"`{element}` is not on screen");
        }

        var x = rect.GetProperty("x").GetDouble() + (rect.GetProperty("width").GetDouble() / 2);
        var y = rect.GetProperty("y").GetDouble() + (rect.GetProperty("height").GetDouble() / 2);
        await this.ClickAsync(x, y, "left");
    }

    /// <summary>Picks an item up from one square and puts it down on another, with two clicks.</summary>
    public async Task MoveItemAsync(string fromGrid, int fromSlot, string toGrid, int toSlot)
    {
        await this.ClickSlotAsync(fromGrid, fromSlot);
        await this.ClickSlotAsync(toGrid, toSlot);
    }

    /// <summary>The newest event's sequence number; pass it to <see cref="WaitForEventAsync"/>.</summary>
    public async Task<long> LastEventSequenceAsync()
    {
        var events = await this.SendAsync("events", new { since = int.MaxValue });
        return events.GetProperty("last_seq").GetInt64();
    }

    /// <summary>
    /// Waits for an event recorded after <paramref name="since"/> whose fields
    /// match <paramref name="match"/>, and returns it.
    /// </summary>
    public Task<JsonElement> WaitForEventAsync(string name, IReadOnlyDictionary<string, string>? match, long since, TimeSpan timeout)
        => this.SendAsync(
            "wait-for",
            new { @event = name, match = match ?? new Dictionary<string, string>(), since, timeout = timeout.TotalSeconds },
            timeout + CommandTimeout);

    /// <summary>The events recorded after <paramref name="since"/>.</summary>
    public async Task<JsonElement> EventsSinceAsync(long since) => await this.SendAsync("events", new { since });

    /// <summary>Saves a screenshot of the next frame to <paramref name="path"/>.</summary>
    public Task ScreenshotAsync(string path) => this.SendAsync("screenshot", new { @out = path });

    public async ValueTask DisposeAsync()
    {
        // Back to the character list first: the server answers it after it saved the
        // character, and it handles the requests in order, so every move a scenario
        // made (e.g. putting equipment back) is kept. Closing at once can lose them.
        await this.TrySendAsync("logout", WorldTimeout);
        await this.TrySendAsync("quit", TimeSpan.FromSeconds(5));

        await this.control.DisposeAsync();
        if (!this.process.WaitForExit(TimeSpan.FromSeconds(10)))
        {
            this.process.Kill(entireProcessTree: true);
        }

        this.process.Dispose();
    }

    private Task ClickAsync(double x, double y, string button) => this.SendAsync("click-ui", new { x, y, button });

    // For closing: a client that is not in the world or already gone answers with an error.
    private async Task TrySendAsync(string command, TimeSpan timeout)
    {
        try
        {
            await this.SendAsync(command, timeout: timeout);
        }
        catch (Exception exception) when (exception is ControlException or TimeoutException or IOException)
        {
            // Closing anyway.
        }
    }

    // Windows allows 108 characters for an AF_UNIX path; the temp folder keeps it short.
    private static string SocketPath(string role)
    {
        var folder = Path.Combine(Path.GetTempPath(), "mu-in-game-tests");
        Directory.CreateDirectory(folder);
        return Path.Combine(folder, $"{Environment.ProcessId}-{role}.sock");
    }

    private static async Task<ControlConnection> ConnectAsync(string socketPath, Process process, TimeSpan timeout, CancellationToken cancellationToken)
    {
        var deadline = DateTime.UtcNow + timeout;
        while (true)
        {
            if (process.HasExited)
            {
                throw new InvalidOperationException($"the client exited with code {process.ExitCode} before its control socket answered");
            }

            try
            {
                return await ControlConnection.ConnectAsync(socketPath, cancellationToken);
            }
            catch (System.Net.Sockets.SocketException) when (DateTime.UtcNow < deadline)
            {
                await Task.Delay(ConnectRetryInterval, cancellationToken);
            }
        }
    }
}
