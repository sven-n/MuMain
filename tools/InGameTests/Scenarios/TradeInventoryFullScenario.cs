using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// A trade whose items do not fit: into the second player's inventory, into
/// the first player's, and into neither. Each time the trade closes for both
/// with "inventory full", and both keep what they had.
/// </summary>
internal sealed class TradeInventoryFullScenario : Scenario
{
    private const string First = "first";
    private const string Second = "second";

    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;
    private const int MeetingX = 135;
    private const int MeetingY = 128;

    private const int TradeColumns = 8;
    private const int TradeRows = 4;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "trade-inventory-full";

    public override string Description => "a trade whose items do not fit into one side's inventory, the other's, or both, closes and changes nothing";

    public override ScenarioCategory Category => ScenarioCategory.PlayerInteractions;

    public override IReadOnlyList<string> Roles => [First, Second];

    public override int StepCount => 4 + 5 + 5 + 6;

    public override async Task RunAsync(ScenarioContext context)
    {
        var first = context.Client(First);
        var second = context.Client(Second);
        var firstCharacter = TestAccounts.FullTraderFirst;
        var secondCharacter = TestAccounts.FullTraderSecond;

        await context.StepAsync(
            $"The first player logs in as {firstCharacter.Name}",
            $"{firstCharacter.Name} enters the world in Lorencia with no free 2x2 area in its inventory.",
            () => first.EnterWorldAsync(firstCharacter.Account, firstCharacter.Password, firstCharacter.Name));
        await context.StepAsync(
            $"The second player logs in as {secondCharacter.Name}",
            $"{secondCharacter.Name} enters the world in Lorencia, with no free 2x2 area either.",
            () => second.EnterWorldAsync(secondCharacter.Account, secondCharacter.Password, secondCharacter.Name));
        await context.StepAsync(
            "The first player walks to the meeting spot",
            $"{firstCharacter.Name} stands at ({MeetingX},{MeetingY}), a free spot in Lorencia's town, or one tile from it.",
            async () =>
            {
                await first.WarpAsync(LorenciaGate, LorenciaMap);
                await Meeting.WalkToAsync(first, MeetingX, MeetingY);
            });
        await context.StepAsync(
            "The second player walks up to the first",
            $"{secondCharacter.Name} stands next to {firstCharacter.Name}: a trade needs the partner at most one tile away.",
            async () =>
            {
                await second.WarpAsync(LorenciaGate, LorenciaMap);
                await Meeting.WalkUpToAsync(second, first, secondCharacter.Name);
            });

        // Round 1: the second player has no room for what the first offers.
        var firstLarge = await LargeItemAsync(first);
        await ExpectNoRoomAsync(second, firstLarge);
        await RoundAsync(
            context,
            first,
            second,
            secondCharacter.Name,
            "the second player's",
            [(first, [firstLarge])]);

        // Round 2: the first player has no room for what the second offers.
        var secondLarge = await LargeItemAsync(second);
        await ExpectNoRoomAsync(first, secondLarge);
        await RoundAsync(
            context,
            first,
            second,
            secondCharacter.Name,
            "the first player's",
            [(second, [secondLarge])]);

        // Round 3: neither has room. The first offers a 2x2 item, which leaves a
        // 2x2 hole in its own inventory; the second offers more single items than
        // the first then has free squares, taken so that no 2x2 area of its own
        // inventory frees up.
        var firstOffer = await LargeItemAsync(first);
        var secondOffer = await SmallItemsBeyondAsync(second, await FreeSquaresAsync(first) + (firstOffer.Width * firstOffer.Height));
        await RoundAsync(
            context,
            first,
            second,
            secondCharacter.Name,
            "neither player's",
            [(first, [firstOffer]), (second, secondOffer)]);
    }

    // What a client has: its inventory by slot and its zen.
    private sealed record Holdings(IReadOnlyList<ItemSlot> Items, long Zen)
    {
        public bool SameAs(Holdings other)
            => this.Zen == other.Zen
               && this.Items.Select(item => (item.Slot, item.Name)).SequenceEqual(other.Items.Select(item => (item.Slot, item.Name)));
    }

    private static async Task<Holdings> HoldingsAsync(GameClient client)
    {
        var state = await client.StateAsync();
        return new Holdings(
            ItemSlots.Of(state, "inventory").OrderBy(item => item.Slot).ToList(),
            state.GetProperty("zen").GetInt64());
    }

    // One trade: the request, the accept, the offers, both confirms, and the
    // failure. `where` names the inventory that has no room, for the report.
    private static async Task RoundAsync(
        ScenarioContext context,
        GameClient first,
        GameClient second,
        string secondName,
        string where,
        IReadOnlyList<(GameClient Offerer, IReadOnlyList<ItemSlot> Items)> offers)
    {
        var firstHad = await HoldingsAsync(first);
        var secondHad = await HoldingsAsync(second);

        await context.StepAsync(
            "The first player asks the second for a trade",
            $"{secondName} gets the request: a dialog asks whether to trade.",
            () => Trading.RequestAsync(first, second, secondName));
        await context.StepAsync(
            "The second player accepts with Enter",
            "The trade window opens for both players.",
            () => Trading.AcceptAsync(context, first, second));
        foreach (var (offerer, items) in offers)
        {
            var partner = offerer == first ? second : first;
            var what = items.Count == 1 ? $"'{items[0].Name}' ({items[0].Width}x{items[0].Height} squares)" : $"{items.Count} single-square items";
            await context.StepAsync(
                $"The {offerer.Role} player puts {what} into the trade window with clicks",
                $"Two clicks for each item: one picks it up from the inventory, one puts it into the {offerer.Role} player's offer. "
                + $"The {partner.Role} player sees {(items.Count == 1 ? "it" : "them")} in the partner's half of the window.",
                () => OfferAsync(offerer, partner, items));
        }

        await context.StepAsync(
            "The first player presses the confirm button",
            "The first player's confirm button stays pressed.",
            () => Trading.ConfirmAsync(first));

        var firstSequence = await first.LastEventSequenceAsync();
        var secondSequence = await second.LastEventSequenceAsync();
        await context.StepAsync(
            "The second player presses the confirm button",
            $"Both have confirmed, but the offer does not fit into {where} inventory: the server answers \"inventory full\", the "
            + "trade window closes on both sides, and both keep exactly the items and the zen they had.",
            async () =>
            {
                await Trading.ConfirmAsync(second);
                var failed = new Dictionary<string, string> { ["change"] = "closed", ["result"] = "inventory_full" };
                await first.WaitForEventAsync("trade", failed, firstSequence, ServerAnswer);
                await second.WaitForEventAsync("trade", failed, secondSequence, ServerAnswer);
                await ExpectUnchangedAsync(first, firstHad);
                await ExpectUnchangedAsync(second, secondHad);
            });
    }

    // The items go into the offerer's half of the trade window one after the
    // other, each where it fits, and the partner has to see them all.
    private static async Task OfferAsync(GameClient offerer, GameClient partner, IReadOnlyList<ItemSlot> items)
    {
        var used = new HashSet<int>();
        foreach (var item in items)
        {
            var slot = TradeSlotFor(item, used);
            var before = ItemSlots.OfTrade(await offerer.StateAsync(), "my_items").Count;
            await offerer.MoveItemAsync("inventory", item.Slot, "trade", slot);
            var now = before;
            await Expect.EventuallyAsync(
                async () => (now = ItemSlots.OfTrade(await offerer.StateAsync(), "my_items").Count) == before + 1,
                ServerAnswer,
                () => $"'{item.Name}' from inventory slot {item.Slot} did not go into the trade window (it holds {now} items)");
        }

        var seen = 0;
        await Expect.EventuallyAsync(
            async () => (seen = ItemSlots.OfTrade(await partner.StateAsync(), "partner_items").Count) == items.Count,
            ServerAnswer,
            () => $"the {partner.Role} player sees {seen} of the {items.Count} offered items");
    }

    // The first free place of the 8x4 trade grid for the item; marks it used.
    private static int TradeSlotFor(ItemSlot item, HashSet<int> used)
    {
        for (var row = 0; row + item.Height <= TradeRows; row++)
        {
            for (var column = 0; column + item.Width <= TradeColumns; column++)
            {
                var squares = Enumerable.Range(0, item.Height)
                    .SelectMany(dy => Enumerable.Range(0, item.Width).Select(dx => ((row + dy) * TradeColumns) + column + dx))
                    .ToList();
                if (squares.All(square => !used.Contains(square)))
                {
                    used.UnionWith(squares);
                    return (row * TradeColumns) + column;
                }
            }
        }

        throw new ScenarioFailedException($"the trade window has no room for '{item.Name}'");
    }

    private static async Task ExpectUnchangedAsync(GameClient client, Holdings had)
    {
        Holdings? now = null;
        await Expect.EventuallyAsync(
            async () => (now = await HoldingsAsync(client)).SameAs(had),
            ServerAnswer,
            () => $"the {client.Role} player's inventory or zen changed after the failed trade: {Describe(had)} before, {Describe(now!)} after");
    }

    private static string Describe(Holdings holdings)
        => $"{holdings.Items.Count} squares used and {holdings.Zen:N0} zen";

    // The first item of the inventory that covers 2x2 squares or more.
    private static async Task<ItemSlot> LargeItemAsync(GameClient client)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        return inventory.Where(item => item.Width >= 2 && item.Height >= 2).OrderBy(item => item.Slot).FirstOrDefault()
               ?? throw new ScenarioFailedException($"the {client.Role} player has no item of 2x2 squares to offer");
    }

    private static async Task ExpectNoRoomAsync(GameClient client, ItemSlot item)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        Expect.That(
            ItemSlots.FreeArea(inventory, item.Width, item.Height) is null,
            $"the {client.Role} player has room for '{item.Name}' ({item.Width}x{item.Height}); the test data should leave none");
    }

    private static async Task<int> FreeSquaresAsync(GameClient client)
    {
        var used = ItemSlots.Of(await client.StateAsync(), "inventory").Select(item => item.Slot).ToHashSet();
        return Enumerable.Range(ItemSlots.FirstInventorySlot, ItemSlots.InventoryColumns * ItemSlots.InventoryRows)
            .Count(slot => !used.Contains(slot));
    }

    // More than <paramref name="count"/> single-square items, taken like the
    // black squares of a chess board: whatever is taken, no 2x2 area of the
    // inventory becomes free, so a 2x2 item still has no room.
    private static async Task<IReadOnlyList<ItemSlot>> SmallItemsBeyondAsync(GameClient client, int count)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        var taken = inventory
            .Where(item => item.Width == 1 && item.Height == 1)
            .Where(item => (((item.Slot - ItemSlots.FirstInventorySlot) / ItemSlots.InventoryColumns)
                            + ((item.Slot - ItemSlots.FirstInventorySlot) % ItemSlots.InventoryColumns)) % 2 == 0)
            .OrderBy(item => item.Slot)
            .Take(count + 1)
            .ToList();
        Expect.That(
            taken.Count > count && taken.Count <= TradeColumns * TradeRows,
            $"the {client.Role} player has {taken.Count} single-square items to offer on alternate squares, not more than {count}");
        var left = inventory.Where(item => !taken.Contains(item)).ToList();
        Expect.That(
            ItemSlots.FreeArea(left, 2, 2) is null,
            $"offering these items would free a 2x2 area in the {client.Role} player's inventory");
        return taken;
    }
}
