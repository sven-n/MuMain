using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>An NPC a scenario talks to: its name as the client shows it and where it stands.</summary>
internal sealed record Npc(string Name, int X, int Y);

/// <summary>Walking up to NPCs and talking to them the way a player does: with a click on them.</summary>
internal static class Npcs
{
    /// <summary>Hanzo the Blacksmith in Lorencia: sells weapons and shields, and repairs.</summary>
    public static readonly Npc Hanzo = new("Hanzo the Blacksmith", 116, 141);

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    // Tiles around an NPC to stand on, nearest first: the NPC's own tile is taken.
    private static readonly (int X, int Y)[] Around =
        [(0, 2), (2, 0), (0, -2), (-2, 0), (2, 2), (-2, 2), (2, -2), (-2, -2), (0, 3), (3, 0), (0, -3), (-3, 0)];

    /// <summary>Walks to a free tile next to <paramref name="npc"/>.</summary>
    public static async Task WalkUpToAsync(GameClient client, Npc npc)
    {
        foreach (var (dx, dy) in Around)
        {
            try
            {
                await Meeting.WalkToAsync(client, npc.X + dx, npc.Y + dy);
                return;
            }
            catch (ControlException exception) when (exception.Error == "no_path")
            {
                // A wall or the NPC's counter; try the next tile.
            }
        }

        throw new ScenarioFailedException($"no tile next to {npc.Name} ({npc.X},{npc.Y}) can be walked to");
    }

    /// <summary>
    /// Talks to <paramref name="npc"/> with a left click on it, and waits for
    /// <paramref name="window"/> to open. The click goes to the pixel <c>nearby</c>
    /// reports, the middle of the box the mouse picks the NPC by.
    /// </summary>
    public static async Task TalkAsync(GameClient client, Npc npc, string window)
    {
        var pixel = await PixelAsync(client, npc);
        await client.SendAsync("click-ui", new { x = pixel.X, y = pixel.Y, button = "left" });
        await Expect.EventuallyAsync(
            async () => (await client.OpenWindowsAsync()).Contains(window),
            ServerAnswer,
            $"a click on {npc.Name} at ({pixel.X},{pixel.Y}) does not open the {window} window");
    }

    // Where the client draws the NPC; it has to be on screen.
    private static async Task<(double X, double Y)> PixelAsync(GameClient client, Npc npc)
    {
        (double X, double Y)? pixel = null;
        await Expect.EventuallyAsync(
            async () =>
            {
                foreach (var entry in (await client.StateAsync()).GetProperty("nearby").EnumerateArray())
                {
                    if (entry.GetProperty("name").GetString() == npc.Name
                        && entry.TryGetProperty("pixel", out var found) && found.ValueKind == JsonValueKind.Object)
                    {
                        pixel = (found.GetProperty("x").GetDouble(), found.GetProperty("y").GetDouble());
                        return true;
                    }
                }

                return false;
            },
            ServerAnswer,
            $"{npc.Name} is not on the {client.Role}'s screen");
        return pixel!.Value;
    }
}
