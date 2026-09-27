using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// sven-n/MuMain#631: in Icarus the last flight equipment cannot be taken off,
/// neither by right-click nor by dragging it out of its slot, while one of two
/// flying items can. Checked both ways round: the mount as the last flying
/// item, then the wings.
/// </summary>
internal sealed class IcarusFlyingItemTakeOffScenario : Scenario
{
    private const string Flyer = "flyer";
    private const int WingSlot = 7;
    private const int HelperSlot = 8;
    private const string IcarusGate = "Icarus";
    private const int IcarusMap = 10;
    private const int InventoryColumns = 8;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // A refused take-off sends nothing, so "still equipped after a while" is the answer.
    private static readonly TimeSpan RefusalWait = TimeSpan.FromSeconds(3);

    public override string Name => "icarus-flying-item-take-off";

    public override string Description => "in Icarus the last flying item stays on, by right-click and by dragging, wings or mount (#631)";

    public override ScenarioCategory Category => ScenarioCategory.GameBehaviour;

    public override IReadOnlyList<string> Roles => [Flyer];

    public override int StepCount => 11;

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
            // The mount as the last flying item.
            await context.StepAsync(
                $"Right-click '{wings.Name}'",
                $"'{wings.Name}' leaves the wing slot and goes into the inventory: '{mount.Name}' still flies, so the Elf may take "
                + "the wings off in Icarus.",
                () => TakesOffAsync(flyer, wings, WingSlot, $"although '{mount.Name}' flies"));
            await context.StepAsync(
                $"Right-click '{mount.Name}'",
                $"Nothing happens: '{mount.Name}' is the last flying item, and without it the Elf would fall in Icarus "
                + $"(sven-n/MuMain#631). The inventory has room for its {mount.Width}x{mount.Height} squares, so only this rule "
                + "keeps it on.",
                () => StaysOnAfterRightClickAsync(flyer, mount, HelperSlot));
            await context.StepAsync(
                $"Drag '{mount.Name}' into the inventory with two clicks",
                $"'{mount.Name}' stays in its slot although the free squares it is dropped on fit it: taking the last flying item "
                + "off by dragging is refused in Icarus as well (sven-n/MuMain#631).",
                () => StaysOnAfterDragAsync(flyer, mount, HelperSlot));

            // The other way round: the wings as the last flying item.
            await context.StepAsync(
                $"Put '{wings.Name}' back on",
                $"'{wings.Name}' is back in slot {WingSlot}, so the Elf wears two flying items again.",
                () => PutBackAsync(flyer, wings, WingSlot));
            await context.StepAsync(
                $"Right-click '{mount.Name}'",
                $"'{mount.Name}' leaves its slot and goes into the inventory: '{wings.Name}' still fly, so the Elf may take the "
                + "mount off in Icarus.",
                async () =>
                {
                    await FreeAreaAsync(flyer, mount);
                    await TakesOffAsync(flyer, mount, HelperSlot, $"although '{wings.Name}' fly");
                });
            await context.StepAsync(
                $"Right-click '{wings.Name}'",
                $"Nothing happens: '{wings.Name}' are the last flying item now (sven-n/MuMain#631). The inventory has room for "
                + $"their {wings.Width}x{wings.Height} squares, so only this rule keeps them on.",
                () => StaysOnAfterRightClickAsync(flyer, wings, WingSlot));
            await context.StepAsync(
                $"Drag '{wings.Name}' into the inventory with two clicks",
                $"'{wings.Name}' stay in their slot although the free squares they are dropped on fit them: dragging the last "
                + "flying item off is refused as well (sven-n/MuMain#631).",
                () => StaysOnAfterDragAsync(flyer, wings, WingSlot));
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
                    "Put the flying items back on",
                    $"'{wings.Name}' in slot {WingSlot} and '{mount.Name}' in slot {HelperSlot}, so the account is as the test data "
                    + "made it.",
                    async () =>
                    {
                        await PutBackAsync(flyer, wings, WingSlot);
                        await PutBackAsync(flyer, mount, HelperSlot);
                    });
            }
            catch (Exception cleanup) when (failed)
            {
                // The earlier failure is what the scenario reports; this one only follows from it.
                context.Note($"putting the flying items back failed too: {cleanup.Message}");
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

    // A right-click takes the item off: its slot empties at once, and the item
    // reaches the inventory with the server's answer.
    private static async Task TakesOffAsync(GameClient client, ItemSlot item, int slot, string because)
    {
        await client.ClickSlotAsync("equipment", slot, "right");
        await Expect.EventuallyAsync(
            async () =>
            {
                var state = await client.StateAsync();
                return ItemSlots.At(ItemSlots.Of(state, "equipment"), slot) is null
                       && ItemSlots.Of(state, "inventory").Any(entry => entry.Name == item.Name);
            },
            ServerAnswer,
            $"'{item.Name}' did not move into the inventory after a right-click, {because}");
    }

    // Right-click unequip does nothing without room for the item, and a drag
    // onto squares it does not fit is refused too: without room the item
    // stays on for that reason, not for the Icarus rule. So both first make
    // sure the inventory has room for it.
    private static async Task StaysOnAfterRightClickAsync(GameClient client, ItemSlot item, int slot)
    {
        await FreeAreaAsync(client, item);
        await client.ClickSlotAsync("equipment", slot, "right");
        await Expect.StillAfterAsync(
            async () => await EquippedAsync(client, slot) is not null,
            RefusalWait,
            $"a right-click took off '{item.Name}', the last flying item in Icarus");
    }

    private static async Task StaysOnAfterDragAsync(GameClient client, ItemSlot item, int slot)
    {
        var freeArea = await FreeAreaAsync(client, item);
        // The first click picks the item up, the second puts it down so that it
        // covers the free area; an equipment item hangs from the cursor by its middle.
        await client.ClickSlotAsync("equipment", slot);
        await client.DropOnAreaAsync("inventory", freeArea, item.Width, item.Height, InventoryColumns);
        await Expect.StillAfterAsync(
            async () => await EquippedAsync(client, slot) is not null,
            RefusalWait,
            $"dragging took off '{item.Name}', the last flying item in Icarus");
    }

    // The top-left slot of the first free area of the main inventory grid that
    // fits <paramref name="item"/>. `state` lists an item under every square it
    // covers, so a square is free when no item is listed on it.
    private static async Task<int> FreeAreaAsync(GameClient client, ItemSlot item)
    {
        const int FirstInventorySlot = 12;
        const int Columns = InventoryColumns;
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

    // Puts <paramref name="item"/> back into <paramref name="slot"/> unless it is
    // there: part of the scenario, and what leaves the test account as the
    // test data made it.
    private static async Task PutBackAsync(GameClient client, ItemSlot item, int slot)
    {
        if (await EquippedAsync(client, slot) is not null)
        {
            return;
        }

        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        var takenOff = inventory.FirstOrDefault(entry => entry.Name == item.Name)
                       ?? throw new ScenarioFailedException($"'{item.Name}' is neither equipped nor in the inventory; the test account is left without it");

        await client.SendAsync("equip", new { slot = takenOff.Slot, target_slot = slot });
        // `equip` answers once the request is sent; the client must not close before the server moved the item.
        await Expect.EventuallyAsync(
            async () => await EquippedAsync(client, slot) is not null,
            ServerAnswer,
            $"'{item.Name}' could not be put back on; the test account is left without it");
    }
}
