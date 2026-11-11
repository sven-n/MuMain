using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Every equipment slot, one after the other: the item is dragged off into the
/// inventory with two clicks, and put back on with a right-click on it.
/// </summary>
internal sealed class EquipAllSlotsScenario : Scenario
{
    private const string Player = "player";

    // The equipment slots in the order `state` numbers them.
    private static readonly string[] SlotNames =
    [
        "right hand", "left hand", "helm", "armor", "pants", "gloves", "boots", "wings", "pet", "pendant", "right ring", "left ring",
    ];

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "equip-all-slots";

    public override string Description => "the item of every equipment slot is dragged off into the inventory and put back on with a right-click";

    public override ScenarioCategory Category => ScenarioCategory.GameBehaviour;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 2 + (2 * SlotNames.Length);

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        var character = TestAccounts.Equipper;

        await context.StepAsync(
            $"Log in as {character.Name}",
            $"{character.Name} enters the world in Lorencia, wearing something in all {SlotNames.Length} equipment slots.",
            async () =>
            {
                await player.EnterWorldAsync(character.Account, character.Password, character.Name);
                // Right after entering the world the equipment may still be on its way.
                var worn = 0;
                await Expect.EventuallyAsync(
                    async () => (worn = ItemSlots.Of(await player.StateAsync(), "equipment").Count) == SlotNames.Length,
                    ServerAnswer,
                    () => $"{character.Name} wears {worn} items, not one in each of the {SlotNames.Length} slots");
            });
        await context.StepAsync(
            "Open the inventory with I",
            "The inventory opens and shows every equipment slot filled.",
            () => player.OpenInventoryAsync());

        for (var slot = 0; slot < SlotNames.Length; slot++)
        {
            var item = ItemSlots.At(ItemSlots.Of(await player.StateAsync(), "equipment"), slot)
                       ?? throw new ScenarioFailedException($"the {SlotNames[slot]} slot ({slot}) is empty before its turn");
            var inventorySlot = 0;
            await context.StepAsync(
                $"Drag '{item.Name}' off the {SlotNames[slot]} slot into the inventory",
                $"The first click picks '{item.Name}' up from the {SlotNames[slot]} slot, the second puts it down on a free "
                + $"{item.Width}x{item.Height} area of the inventory. The slot is empty and the item lies in the inventory.",
                async () => inventorySlot = await TakeOffAsync(player, item, slot));
            await context.StepAsync(
                $"Right-click '{item.Name}' in the inventory",
                $"'{item.Name}' goes back on into the {SlotNames[slot]} slot and leaves the inventory.",
                () => PutOnAsync(player, item, slot, inventorySlot));
        }
    }

    // Two clicks: the first picks the item up, the second puts it down so that
    // it covers a free area; an equipment item hangs from the pointer by its
    // middle. Returns the inventory slot of its top-left square.
    private static async Task<int> TakeOffAsync(GameClient client, ItemSlot item, int slot)
    {
        var area = ItemSlots.FreeArea(ItemSlots.Of(await client.StateAsync(), "inventory"), item.Width, item.Height)
                   ?? throw new ScenarioFailedException($"the inventory has no free {item.Width}x{item.Height} area for '{item.Name}'");
        await client.ClickSlotAsync("equipment", slot);
        await client.DropOnAreaAsync("inventory", area, item.Width, item.Height, ItemSlots.InventoryColumns);
        await Expect.EventuallyAsync(
            async () =>
            {
                var state = await client.StateAsync();
                return ItemSlots.At(ItemSlots.Of(state, "equipment"), slot) is null
                       && ItemSlots.At(ItemSlots.Of(state, "inventory"), area)?.Name == item.Name;
            },
            ServerAnswer,
            $"'{item.Name}' did not move from slot {slot} onto inventory slot {area}");
        return area;
    }

    private static async Task PutOnAsync(GameClient client, ItemSlot item, int slot, int inventorySlot)
    {
        await client.ClickSlotAsync("inventory", inventorySlot, "right");
        await Expect.EventuallyAsync(
            async () =>
            {
                var state = await client.StateAsync();
                return ItemSlots.At(ItemSlots.Of(state, "equipment"), slot)?.Name == item.Name
                       && ItemSlots.At(ItemSlots.Of(state, "inventory"), inventorySlot)?.Name != item.Name;
            },
            ServerAnswer,
            $"a right-click on '{item.Name}' did not put it back on into slot {slot}");
    }
}
