using System.Diagnostics;
using System.Text.Json;
using MuMain.Tools.InGameTests.Control;
using MuMain.Tools.InGameTests.Running;

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
    // Commands that only read; everything else acts, and is followed by the step delay.
    private static readonly HashSet<string> ReadingCommands =
        ["ping", "scene", "state", "nearby", "events", "wait-for", "screenshot", "ui", "slot-pixel"];

    private readonly Process process;
    private readonly ControlConnection control;
    private readonly TimeSpan stepDelay;

    private GameClient(string role, Process process, ControlConnection control, TimeSpan stepDelay)
    {
        this.Role = role;
        this.process = process;
        this.control = control;
        this.stepDelay = stepDelay;
    }

    /// <summary>What the client says it was built from; null for a client that does not say.</summary>
    public ClientVersion? Version { get; private set; }

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

        ControlConnection? control = null;
        try
        {
            control = await ConnectAsync(socketPath, process, options.StartTimeout, cancellationToken);
            var client = new GameClient(role, process, control, options.StepDelay);
            var ping = await client.SendAsync("ping");
            if (ping.TryGetProperty("commit", out var commit) && commit.GetString() is { } hash)
            {
                client.Version = new ClientVersion(
                    hash,
                    ping.TryGetProperty("commit_changed", out var changed) && changed.GetBoolean(),
                    ping.TryGetProperty("build", out var build) ? build.GetString() : null);
            }

            return client;
        }
        catch
        {
            if (control is not null)
            {
                await control.DisposeAsync();
            }

            process.Kill(entireProcessTree: true);
            process.Dispose();
            throw;
        }
    }

    /// <summary>Sends a command and returns its result; after an action, waits the step delay.</summary>
    public async Task<JsonElement> SendAsync(string command, object? fields = null, TimeSpan? timeout = null)
    {
        var result = await this.control.SendAsync(command, fields, timeout ?? CommandTimeout);
        if (this.stepDelay > TimeSpan.Zero && !ReadingCommands.Contains(command))
        {
            await Task.Delay(this.stepDelay);
        }

        return result;
    }

    /// <summary>Logs in and enters the world with <paramref name="character"/>.</summary>
    public async Task EnterWorldAsync(string account, string password, string character)
    {
        await this.SendAsync("login", new { account, password }, WorldTimeout);
        await this.SendAsync("select-char", new { name = character }, WorldTimeout);
    }

    /// <summary>
    /// Warps through the gate <paramref name="gate"/>, unless the character is on
    /// <paramref name="map"/> already: the client refuses a warp to its own map.
    /// </summary>
    public async Task WarpAsync(string gate, int map)
    {
        if ((await this.StateAsync()).GetProperty("map").GetInt32() != map)
        {
            await this.SendAsync("warp", new { gate }, WorldTimeout);
        }
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

    /// <summary>
    /// Puts down the item on the cursor so that it covers the <paramref name="width"/> x
    /// <paramref name="height"/> area whose top-left square is <paramref name="topLeft"/>. An item
    /// picked from an equipment slot hangs from the cursor by its middle, so the click goes to the
    /// middle of the area, between its first and its last square.
    /// </summary>
    public async Task DropOnAreaAsync(string grid, int topLeft, int width, int height, int columns)
    {
        var first = await this.SendAsync("slot-pixel", new { grid, slot = topLeft });
        var last = await this.SendAsync("slot-pixel", new { grid, slot = topLeft + ((height - 1) * columns) + width - 1 });
        await this.ClickAsync(
            (first.GetProperty("x").GetDouble() + last.GetProperty("x").GetDouble()) / 2,
            (first.GetProperty("y").GetDouble() + last.GetProperty("y").GetDouble()) / 2,
            "left");
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

    /// <summary>Saves the next frame to <paramref name="path"/> as a JPEG of <paramref name="quality"/> (1 to 100).</summary>
    public Task ScreenshotAsync(string path, int quality = 100) => this.SendAsync("screenshot", new { @out = path, quality });

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
