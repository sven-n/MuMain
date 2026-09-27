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

    public override int StepCount => 7;

    public override async Task RunAsync(ScenarioContext context)
    {
        var flyer = context.Client(Flyer);
        var character = TestAccounts.IcarusFlyer;

        var (wings, mount) = await context.StepAsync(
            $"Log in as {character.Name}",
            $"{character.Name} enters the world wearing wings (slot {WingSlot}) and a flying mount (slot {HelperSlot}); "
            + "the test data puts it in Noria, the Elves' home map.",
            async () =>
            {
                await flyer.EnterWorldAsync(character.Account, character.Password, character.Name);
                return (await EquippedEventuallyAsync(flyer, WingSlot, $"{character.Name} wears no wings"),
                        await EquippedEventuallyAsync(flyer, HelperSlot, $"{character.Name} has no flying mount"));
            });
        await context.StepAsync(
            "Warp to Icarus",
            "The Elf stands in Icarus. The warp is allowed because it can fly.",
            () => flyer.WarpAsync(IcarusGate, IcarusMap));
        await context.StepAsync(
            "Open the inventory with I",
            "The inventory window opens and shows the equipment.",
            () => flyer.OpenInventoryAsync());

        var failed = false;
        try
        {
            await context.StepAsync(
                $"Right-click '{wings.Name}'",
                $"'{wings.Name}' leaves the wing slot and goes into the inventory: '{mount.Name}' still flies, so the Elf may take "
                + "the wings off in Icarus.",
                async () =>
                {
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
                });

            // Right-click unequip does nothing without room for the item, and a
            // drag onto squares it does not fit is refused too: without room the
            // mount stays on for that reason, not for the Icarus rule. So both
            // steps first make sure the inventory has room for it.
            await context.StepAsync(
                $"Right-click '{mount.Name}'",
                $"Nothing happens: '{mount.Name}' is the last flying item, and without it the Elf would fall in Icarus "
                + $"(sven-n/MuMain#631). The inventory has room for its {mount.Width}x{mount.Height} squares, so only this rule "
                + "keeps it on.",
                async () =>
                {
                    await FreeAreaAsync(flyer, mount);
                    await flyer.ClickSlotAsync("equipment", HelperSlot, "right");
                    await Expect.StillAfterAsync(
                        async () => await EquippedAsync(flyer, HelperSlot) is not null,
                        RefusalWait,
                        $"a right-click took off '{mount.Name}', the last flying item in Icarus");
                });
            await context.StepAsync(
                $"Drag '{mount.Name}' into the inventory with two clicks",
                $"'{mount.Name}' stays in its slot although the free squares it is dropped on fit it: taking the last flying item "
                + "off by dragging is refused in Icarus as well (sven-n/MuMain#631).",
                async () =>
                {
                    var freeArea = await FreeAreaAsync(flyer, mount);
                    await flyer.MoveItemAsync("equipment", HelperSlot, "inventory", freeArea);
                    await Expect.StillAfterAsync(
                        async () => await EquippedAsync(flyer, HelperSlot) is not null,
                        RefusalWait,
                        $"dragging took off '{mount.Name}', the last flying item in Icarus");
                });
        }
        catch
        {
            failed = true;
            throw;
        }
        finally
        {
            try
            {
                await context.StepAsync(
                    $"Put '{wings.Name}' back on",
                    $"'{wings.Name}' is back in slot {WingSlot}, so the account is as the test data made it.",
                    () => PutWingsBackAsync(flyer, wings));
            }
            catch (Exception cleanup) when (failed)
            {
                // The earlier failure is what the scenario reports; this one only follows from it.
                context.Note($"putting the wings back failed too: {cleanup.Message}");
            }
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

    // The top-left slot of the first free area of the main inventory grid that
    // fits <paramref name="item"/>. `state` lists an item under every square it
    // covers, so a square is free when no item is listed on it.
    private static async Task<int> FreeAreaAsync(GameClient client, ItemSlot item)
    {
        const int FirstInventorySlot = 12;
        const int Columns = 8;
        const int Rows = 8;
        var used = ItemSlots.Of(await client.StateAsync(), "inventory").Select(entry => entry.Slot).ToHashSet();
        for (var row = 0; row + item.Height <= Rows; row++)
        {
            for (var column = 0; column + item.Width <= Columns; column++)
            {
                var fits = Enumerable.Range(0, item.Height)
                    .SelectMany(dy => Enumerable.Range(0, item.Width).Select(dx => FirstInventorySlot + ((row + dy) * Columns) + column + dx))
                    .All(slot => !used.Contains(slot));
                if (fits)
                {
                    return FirstInventorySlot + (row * Columns) + column;
                }
            }
        }

        throw new ScenarioFailedException(
            $"the inventory has no free {item.Width}x{item.Height} area for '{item.Name}'; a refused take-off would prove nothing");
    }

    // Leaves the test account as the test data made it.
    private static async Task PutWingsBackAsync(GameClient client, ItemSlot wings)
    {
        if (await EquippedAsync(client, WingSlot) is not null)
        {
            return;
        }

        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        var takenOff = inventory.FirstOrDefault(item => item.Name == wings.Name)
                       ?? throw new ScenarioFailedException($"'{wings.Name}' is neither equipped nor in the inventory; the test account is left without it");

        await client.SendAsync("equip", new { slot = takenOff.Slot, target_slot = WingSlot });
        // `equip` answers once the request is sent; the client must not close before the server moved the item.
        await Expect.EventuallyAsync(
            async () => await EquippedAsync(client, WingSlot) is not null,
            ServerAnswer,
            $"'{wings.Name}' could not be put back on; the test account is left without it");
    }
}
