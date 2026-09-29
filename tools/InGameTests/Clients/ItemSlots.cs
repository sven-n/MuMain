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
        => FreeArea(inventory.Select(entry => entry.Slot).ToHashSet(), width, height, InventoryColumns, InventoryRows, FirstInventorySlot);

    /// <summary>
    /// The top-left slot of the first <paramref name="width"/> x <paramref name="height"/> area of
    /// a grid (<paramref name="columns"/> x <paramref name="rows"/>, slots numbered row by row from
    /// <paramref name="firstSlot"/>) that none of the <paramref name="used"/> squares covers; null
    /// when there is none.
    /// </summary>
    public static int? FreeArea(IReadOnlySet<int> used, int width, int height, int columns, int rows, int firstSlot)
    {
        for (var row = 0; row + height <= rows; row++)
        {
            for (var column = 0; column + width <= columns; column++)
            {
                var fits = Squares(firstSlot + (row * columns) + column, width, height, columns).All(slot => !used.Contains(slot));
                if (fits)
                {
                    return firstSlot + (row * columns) + column;
                }
            }
        }

        return null;
    }

    /// <summary>The squares an item of <paramref name="width"/> x <paramref name="height"/> covers from <paramref name="topLeft"/>.</summary>
    public static IEnumerable<int> Squares(int topLeft, int width, int height, int columns)
        => Enumerable.Range(0, height).SelectMany(dy => Enumerable.Range(0, width).Select(dx => topLeft + (dy * columns) + dx));

    private static ItemSlot Read(JsonElement item)
        => new(
            item.GetProperty("slot").GetInt32(),
            item.GetProperty("name").GetString() ?? string.Empty,
            item.TryGetProperty("width", out var width) ? width.GetInt32() : 1,
            item.TryGetProperty("height", out var height) ? height.GetInt32() : 1);
}
