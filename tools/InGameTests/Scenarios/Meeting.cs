using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Walking characters to each other: trading, a party invitation or a personal
/// shop needs the other player next to one, and chat reaches only the players in view.
/// </summary>
internal static class Meeting
{
    /// <summary>Walking across a town, see <see cref="WalkUpToAsync"/>.</summary>
    public static readonly TimeSpan WalkTimeout = TimeSpan.FromSeconds(120);

    /// <summary>
    /// Walks <paramref name="walker"/> to at most <paramref name="distance"/> tiles from where
    /// <paramref name="host"/> stands, as the host's client sees it: that client checks the
    /// distance of a trade request or a party invitation.
    /// </summary>
    /// <remarks>
    /// A warp to a town lands anywhere in it, and a character that is in the town
    /// already starts wherever it stood, so how far each one walks changes from run
    /// to run: across the whole town at worst. The walk gets time for that. And
    /// <c>move</c> answers within a tile of its target while the walk may still take
    /// its last step, so the walker heads for where the host stands now, until the
    /// two are next to each other.
    ///
    /// The walker's own client can also show it a step further than the others see
    /// it, when its last step did not reach them. Then the walker walks a few tiles
    /// away and comes back: a <c>move</c> onto a tile in reach sends no walk at all.
    /// </remarks>
    public static async Task WalkUpToAsync(GameClient walker, GameClient host, string walkerName, int distance = 1)
    {
        var hostPosition = (X: 0, Y: 0);
        var walkerPosition = (X: 0, Y: 0);
        (int X, int Y)? seenPosition = null;
        await Expect.EventuallyAsync(
            async () =>
            {
                hostPosition = await PositionAsync(host);
                walkerPosition = await PositionAsync(walker);
                seenPosition = await SeenPositionAsync(host, walkerName);
                var nextToHost = IsWithin(walkerPosition, hostPosition, distance);
                if (nextToHost && seenPosition is { } seen && IsWithin(seen, hostPosition, distance))
                {
                    return true;
                }

                if (nextToHost)
                {
                    await StepAwayAsync(walker, hostPosition);
                    return false;
                }

                await WalkToAsync(walker, hostPosition.X, hostPosition.Y);
                return false;
            },
            WalkTimeout,
            () => $"the {walker.Role} ({walkerPosition.X},{walkerPosition.Y}; the {host.Role}'s client sees it at "
                  + $"{(seenPosition is { } seen ? $"({seen.X},{seen.Y})" : "no place")}) did not get next to the {host.Role} "
                  + $"({hostPosition.X},{hostPosition.Y})");
    }

    /// <summary>
    /// Walks to a tile. The client ends one <c>move</c> after 30 s; a longer walk (across the
    /// town, or with a slowly drawing client) goes on with the next one from where the last
    /// stopped, until the walk's own time is up.
    /// </summary>
    public static async Task WalkToAsync(GameClient client, int x, int y)
    {
        var deadline = DateTime.UtcNow + WalkTimeout;
        while (true)
        {
            try
            {
                await client.SendAsync("move", new { x, y }, WalkTimeout);
                return;
            }
            catch (ControlException exception) when (exception.Error == "timeout" && DateTime.UtcNow < deadline)
            {
                // Walk on.
            }
        }
    }

    /// <summary>Where the client's own character stands.</summary>
    public static async Task<(int X, int Y)> PositionAsync(GameClient client)
    {
        var position = (await client.StateAsync()).GetProperty("position");
        return (position[0].GetInt32(), position[1].GetInt32());
    }

    /// <summary>Where the client of <paramref name="observer"/> sees the player <paramref name="name"/>, or null.</summary>
    public static Task<(int X, int Y)?> SeenPositionAsync(GameClient observer, string name)
        => SeenPositionAsync(observer, entry => IsNamed(entry, name));

    /// <summary>Where the client of <paramref name="observer"/> sees <paramref name="npc"/>, or null.</summary>
    public static Task<(int X, int Y)?> SeenPositionAsync(GameClient observer, Npc npc)
        => SeenPositionAsync(observer, entry => IsNpc(entry, npc));

    /// <summary>
    /// Where the client of <paramref name="observer"/> draws the player <paramref name="name"/>,
    /// in window pixels: the middle of the box the mouse picks it by. Waits until it is on screen.
    /// </summary>
    public static Task<(double X, double Y)> PixelOfAsync(GameClient observer, string name)
        => PixelOfAsync(observer, entry => IsNamed(entry, name), name);

    /// <summary>The same for <paramref name="npc"/>, found by its monster number.</summary>
    public static Task<(double X, double Y)> PixelOfAsync(GameClient observer, Npc npc)
        => PixelOfAsync(observer, entry => IsNpc(entry, npc), $"{npc.Name} ({npc.Number})");

    private static bool IsNamed(JsonElement entry, string name)
        => entry.TryGetProperty("name", out var entryName) && entryName.GetString() == name;

    // Summons and transformed players carry a monster number too, as players.
    private static bool IsNpc(JsonElement entry, Npc npc)
        => entry.TryGetProperty("monster", out var monster) && monster.ValueKind == JsonValueKind.Number
            && monster.GetInt32() == npc.Number && entry.GetProperty("kind").GetString() != "player";

    private static async Task<(int X, int Y)?> SeenPositionAsync(GameClient observer, Func<JsonElement, bool> matches)
    {
        foreach (var entry in (await observer.StateAsync()).GetProperty("nearby").EnumerateArray())
        {
            if (matches(entry) && entry.TryGetProperty("position", out var position))
            {
                return (position[0].GetInt32(), position[1].GetInt32());
            }
        }

        return null;
    }

    private static async Task<(double X, double Y)> PixelOfAsync(GameClient observer, Func<JsonElement, bool> matches, string what)
    {
        (double X, double Y)? pixel = null;
        await Expect.EventuallyAsync(
            async () =>
            {
                foreach (var entry in (await observer.StateAsync()).GetProperty("nearby").EnumerateArray())
                {
                    if (matches(entry)
                        && entry.TryGetProperty("pixel", out var found) && found.ValueKind == JsonValueKind.Object)
                    {
                        pixel = (found.GetProperty("x").GetDouble(), found.GetProperty("y").GetDouble());
                        return true;
                    }
                }

                return false;
            },
            TimeSpan.FromSeconds(10),
            $"{what} is not on the {observer.Role}'s screen");
        return pixel!.Value;
    }

    private static bool IsWithin((int X, int Y) position, (int X, int Y) other, int distance)
        => Math.Abs(position.X - other.X) <= distance && Math.Abs(position.Y - other.Y) <= distance;

    // Three tiles away from the host, in the first direction that can be walked.
    private static async Task StepAwayAsync(GameClient walker, (int X, int Y) hostPosition)
    {
        const int Away = 3;
        foreach (var (dx, dy) in new[] { (Away, 0), (-Away, 0), (0, Away), (0, -Away) })
        {
            try
            {
                await WalkToAsync(walker, hostPosition.X + dx, hostPosition.Y + dy);
                return;
            }
            catch (ControlException exception) when (exception.Error == "no_path")
            {
                // A wall or a building; try the next direction.
            }
        }
    }
}
