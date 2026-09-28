using System.Text.Json;

namespace MuMain.Tools.InGameTests.Clients;

/// <summary>An item as <c>state</c> reports it: where it is, what it is, and its size in inventory squares.</summary>
internal sealed record ItemSlot(int Slot, string Name, int Width = 1, int Height = 1);

/// <summary>Reads the item lists of a <c>state</c> answer.</summary>
internal static class ItemSlots
{
    /// <summary>The items of <c>equipment</c> or <c>inventory</c>.</summary>
    public static IReadOnlyList<ItemSlot> Of(JsonElement state, string list)
        => state.GetProperty(list).EnumerateArray().Select(Read).ToList();

    /// <summary>The items of the open trade's <c>my_items</c> or <c>partner_items</c>; none without a trade.</summary>
    public static IReadOnlyList<ItemSlot> OfTrade(JsonElement state, string list)
        => state.GetProperty("trade").ValueKind == JsonValueKind.Object
            ? state.GetProperty("trade").GetProperty(list).EnumerateArray().Select(Read).ToList()
            : [];

    /// <summary>The first slot of the main inventory grid; the equipment comes before it.</summary>
    public const int FirstInventorySlot = 12;

    /// <summary>The columns and rows of the main inventory grid.</summary>
    public const int InventoryColumns = 8;
    public const int InventoryRows = 8;

    /// <summary>The item in <paramref name="slot"/>, or null.</summary>
    public static ItemSlot? At(IReadOnlyList<ItemSlot> items, int slot) => items.FirstOrDefault(item => item.Slot == slot);

    /// <summary>
    /// The top-left slot of the first free <paramref name="width"/> x <paramref name="height"/>
    /// area of the main inventory grid, row by row; null when there is none.
    /// </summary>
    public static int? FreeArea(IReadOnlyList<ItemSlot> inventory, int width, int height)
    {
        var used = inventory.Select(entry => entry.Slot).ToHashSet();
        for (var row = 0; row + height <= InventoryRows; row++)
        {
            for (var column = 0; column + width <= InventoryColumns; column++)
            {
                var fits = Enumerable.Range(0, height)
                    .SelectMany(dy => Enumerable.Range(0, width).Select(dx => FirstInventorySlot + ((row + dy) * InventoryColumns) + column + dx))
                    .All(slot => !used.Contains(slot));
                if (fits)
                {
                    return FirstInventorySlot + (row * InventoryColumns) + column;
                }
            }
        }

        return null;
    }

    private static ItemSlot Read(JsonElement item)
        => new(
            item.GetProperty("slot").GetInt32(),
            item.GetProperty("name").GetString() ?? string.Empty,
            item.TryGetProperty("width", out var width) ? width.GetInt32() : 1,
            item.TryGetProperty("height", out var height) ? height.GetInt32() : 1);
}
