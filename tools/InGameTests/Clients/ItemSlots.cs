using System.Text.Json;

namespace MuMain.Tools.InGameTests.Clients;

/// <summary>An item as <c>state</c> reports it: where it is and what it is.</summary>
internal sealed record ItemSlot(int Slot, string Name);

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

    /// <summary>The item in <paramref name="slot"/>, or null.</summary>
    public static ItemSlot? At(IReadOnlyList<ItemSlot> items, int slot) => items.FirstOrDefault(item => item.Slot == slot);

    private static ItemSlot Read(JsonElement item)
        => new(item.GetProperty("slot").GetInt32(), item.GetProperty("name").GetString() ?? string.Empty);
}
