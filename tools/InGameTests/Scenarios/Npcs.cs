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
        var pixel = await Meeting.PixelOfAsync(client, npc.Name);
        await client.SendAsync("click-ui", new { x = pixel.X, y = pixel.Y, button = "left" });
        await Expect.EventuallyAsync(
            async () => (await client.OpenWindowsAsync()).Contains(window),
            ServerAnswer,
            $"a click on {npc.Name} at ({pixel.X},{pixel.Y}) does not open the {window} window");
    }
}
