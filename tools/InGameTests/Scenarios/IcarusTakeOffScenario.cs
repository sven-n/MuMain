using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// sven-n/MuMain#631: in Icarus the last flight equipment cannot be taken off,
/// neither by right-click nor by dragging it out of its slot.
/// </summary>
internal sealed class IcarusTakeOffScenario : Scenario
{
    private const string Flyer = "flyer";
    private const int WingSlot = 7;
    private const int HelperSlot = 8;
    private const string IcarusGate = "Icarus";
    private const int IcarusMap = 10;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // A refused take-off sends nothing, so "still equipped after a while" is the answer.
    private static readonly TimeSpan RefusalWait = TimeSpan.FromSeconds(3);

    public override string Name => "icarus-take-off";

    public override string Description => "in Icarus the last flying item stays on, by right-click and by dragging (#631)";

    public override IReadOnlyList<string> Roles => [Flyer];

    public override async Task RunAsync(ScenarioContext context)
    {
        var flyer = context.Client(Flyer);
        var character = TestAccounts.IcarusFlyer;

        context.Log("the character enters Icarus with wings and a flying mount");
        await flyer.EnterWorldAsync(character.Account, character.Password, character.Name);
        var wings = await EquippedEventuallyAsync(flyer, WingSlot, $"{character.Name} wears no wings");
        var mount = await EquippedEventuallyAsync(flyer, HelperSlot, $"{character.Name} has no flying mount");
        // The client refuses a warp to the map the character is on.
        if ((await flyer.StateAsync()).GetProperty("map").GetInt32() != IcarusMap)
        {
            await flyer.SendAsync("warp", new { gate = IcarusGate }, TimeSpan.FromSeconds(60));
        }
        await flyer.OpenInventoryAsync();

        try
        {
            context.Log($"right-clicking '{wings.Name}' takes it off: '{mount.Name}' still flies");
            await flyer.ClickSlotAsync("equipment", WingSlot, "right");
            // The slot empties at once; the item reaches the inventory with the server's answer.
            await Expect.EventuallyAsync(
                async () =>
                {
                    var state = await flyer.StateAsync();
                    return ItemSlots.At(ItemSlots.Of(state, "equipment"), WingSlot) is null
                           && ItemSlots.Of(state, "inventory").Any(item => item.Name == wings.Name);
                },
                ServerAnswer,
                $"'{wings.Name}' did not move into the inventory after a right-click, although '{mount.Name}' flies");

            // Right-click unequip does nothing without room in the inventory; with
            // room, only the Icarus rule can keep the mount on.
            var freeSlot = await FirstFreeInventorySlotAsync(flyer);
            context.Log($"right-clicking '{mount.Name}', the last flying item, leaves it on");
            await flyer.ClickSlotAsync("equipment", HelperSlot, "right");
            await Expect.StillAfterAsync(
                async () => await EquippedAsync(flyer, HelperSlot) is not null,
                RefusalWait,
                $"a right-click took off '{mount.Name}', the last flying item in Icarus");

            context.Log($"dragging '{mount.Name}' out of its slot leaves it on");
            await flyer.MoveItemAsync("equipment", HelperSlot, "inventory", freeSlot);
            await Expect.StillAfterAsync(
                async () => await EquippedAsync(flyer, HelperSlot) is not null,
                RefusalWait,
                $"dragging took off '{mount.Name}', the last flying item in Icarus");
        }
        finally
        {
            await PutWingsBackAsync(context, flyer, wings);
        }
    }

    private static async Task<ItemSlot?> EquippedAsync(GameClient client, int slot)
        => ItemSlots.At(ItemSlots.Of(await client.StateAsync(), "equipment"), slot);

    // Right after entering the world the equipment may still be on its way from the server.
    private static async Task<ItemSlot> EquippedEventuallyAsync(GameClient client, int slot, string failure)
    {
        ItemSlot? item = null;
        await Expect.EventuallyAsync(async () => (item = await EquippedAsync(client, slot)) is not null, ServerAnswer, failure);
        return item!;
    }

    // The inventory grid starts after the 12 equipment slots.
    private static async Task<int> FirstFreeInventorySlotAsync(GameClient client)
    {
        const int FirstInventorySlot = 12;
        const int InventorySquares = 64;
        var used = ItemSlots.Of(await client.StateAsync(), "inventory").Select(item => item.Slot).ToHashSet();
        return Enumerable.Range(FirstInventorySlot, InventorySquares).FirstOrDefault(slot => !used.Contains(slot), -1) is var free and >= 0
            ? free
            : throw new ScenarioFailedException("the inventory has no free square");
    }

    // Leaves the test account as the test data made it.
    private static async Task PutWingsBackAsync(ScenarioContext context, GameClient client, ItemSlot wings)
    {
        if (await EquippedAsync(client, WingSlot) is not null)
        {
            return;
        }

        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        var takenOff = inventory.FirstOrDefault(item => item.Name == wings.Name);
        if (takenOff is null)
        {
            throw new ScenarioFailedException($"'{wings.Name}' is neither equipped nor in the inventory; the test account is left without it");
        }

        context.Log($"putting '{wings.Name}' back on");
        await client.SendAsync("equip", new { slot = takenOff.Slot, target_slot = WingSlot });
        // `equip` answers once the request is sent; the client must not close before the server moved the item.
        await Expect.EventuallyAsync(
            async () => await EquippedAsync(client, WingSlot) is not null,
            ServerAnswer,
            $"'{wings.Name}' could not be put back on; the test account is left without it");
    }
}
