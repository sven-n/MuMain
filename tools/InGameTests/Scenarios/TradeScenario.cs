using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// sven-n/MuMain#588: an item put into the trade window with two clicks is
/// offered, and after both confirm it belongs to the partner.
/// </summary>
internal sealed class TradeScenario : Scenario
{
    private const string Seller = "seller";
    private const string Buyer = "buyer";
    private const string TradeGrid = "trade";
    private const int FirstTradeSlot = 0;

    // A free spot in Lorencia's town; the trade partner has to stand at most one tile away.
    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;
    private const int MeetingX = 135;
    private const int MeetingY = 128;
    private const int TradeDistance = 1;

    // What the buyer pays: a number that stands out in the screenshots.
    private const int ZenOffer = 123456;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // Walking across Lorencia's town, see WalkUpToAsync.
    private static readonly TimeSpan WalkTimeout = TimeSpan.FromSeconds(120);
    // After an offer changes, the confirm button waits about 150 frames: some
    // seconds normally, half a minute at five frames a second.
    private static readonly TimeSpan CooldownWait = TimeSpan.FromSeconds(120);
    private static readonly TimeSpan ConfirmWait = TimeSpan.FromSeconds(20);

    public override string Name => "trade";

    public override string Description => "an item put into the trade window with clicks, and zen typed into it, change owner (#588)";

    public override ScenarioCategory Category => ScenarioCategory.PlayerInteractions;

    public override IReadOnlyList<string> Roles => [Seller, Buyer];

    public override int StepCount => 13;

    public override async Task RunAsync(ScenarioContext context)
    {
        var seller = context.Client(Seller);
        var buyer = context.Client(Buyer);
        var sellerCharacter = TestAccounts.TradeSeller;
        var buyerCharacter = TestAccounts.TradeBuyer;

        await context.StepAsync(
            $"The seller logs in as {sellerCharacter.Name}",
            $"{sellerCharacter.Name} enters the world; the test data puts it in the safe zone of its home map.",
            () => seller.EnterWorldAsync(sellerCharacter.Account, sellerCharacter.Password, sellerCharacter.Name));
        await context.StepAsync(
            $"The buyer logs in as {buyerCharacter.Name}",
            $"{buyerCharacter.Name} enters the world in the safe zone of its home map.",
            () => buyer.EnterWorldAsync(buyerCharacter.Account, buyerCharacter.Password, buyerCharacter.Name));
        await context.StepAsync(
            "The seller warps to Lorencia",
            $"{sellerCharacter.Name} stands in Lorencia's town. A warp to a town lands anywhere in it.",
            () => seller.WarpAsync(LorenciaGate, LorenciaMap));
        await context.StepAsync(
            "The buyer warps to Lorencia",
            $"{buyerCharacter.Name} stands in Lorencia's town, somewhere else than the seller.",
            () => buyer.WarpAsync(LorenciaGate, LorenciaMap));
        await context.StepAsync(
            "The seller walks to the meeting spot",
            $"{sellerCharacter.Name} stands at ({MeetingX},{MeetingY}), a free spot in Lorencia's town, or one tile from it.",
            () => WalkToAsync(seller, MeetingX, MeetingY));
        await context.StepAsync(
            "The buyer walks up to the seller",
            $"{buyerCharacter.Name} stands on a tile next to {sellerCharacter.Name}: a trade needs the partner at most one tile away.",
            () => WalkUpToAsync(buyer, seller, buyerCharacter.Name));

        var offer = await FindJewelAsync(seller);
        // What each one has before the trade: the buyer may have such a jewel
        // already, and the warps have cost zen.
        var sellerHadJewels = await CountAsync(seller, offer.Name);
        var buyerHadJewels = await CountAsync(buyer, offer.Name);
        var sellerHadZen = await ZenAsync(seller);
        var buyerHadZen = await ZenAsync(buyer);
        await context.StepAsync(
            "The seller asks the buyer for a trade",
            $"{buyerCharacter.Name} gets the request: a dialog asks whether to trade with {sellerCharacter.Name}.",
            () => RequestTradeAsync(seller, buyer, buyerCharacter.Name));
        await context.StepAsync(
            "The buyer accepts with Enter",
            "The trade window opens for both characters, each with the inventory next to it.",
            () => AcceptTradeAsync(seller, buyer));
        await context.StepAsync(
            $"The seller puts '{offer.Name}' into the trade window with two clicks",
            $"The first click picks '{offer.Name}' up from inventory slot {offer.Slot}, the second puts it into the first square of the "
            + $"seller's offer. {buyerCharacter.Name} then sees it in the seller's half of the trade window (sven-n/MuMain#588).",
            async () =>
            {
                await seller.OpenInventoryAsync();
                await seller.MoveItemAsync("inventory", offer.Slot, TradeGrid, FirstTradeSlot);
                await Expect.EventuallyAsync(
                    async () => ItemSlots.OfTrade(await buyer.StateAsync(), "partner_items").Any(item => item.Name == offer.Name),
                    ServerAnswer,
                    $"the buyer does not see '{offer.Name}' in the seller's offer");
            });
        await context.StepAsync(
            $"The buyer offers {ZenOffer:N0} zen",
            $"The buyer clicks the zen button of the trade window, types {ZenOffer} into the box that opens and confirms with "
            + $"Enter. The seller sees {ZenOffer:N0} zen in the buyer's half of the trade window.",
            async () =>
            {
                await buyer.ClickElementAsync("trade.zen");
                await Expect.EventuallyAsync(
                    async () => (await buyer.OpenWindowsAsync()).Contains("message_box"),
                    ServerAnswer,
                    "the zen box does not open");
                await buyer.SendAsync("type", new { text = ZenOffer.ToString(System.Globalization.CultureInfo.InvariantCulture) });
                await buyer.SendAsync("hotkey", new { key = "enter" });
                var seen = 0;
                await Expect.EventuallyAsync(
                    async () => (seen = TradeZen(await seller.StateAsync(), "partner_zen")) == ZenOffer,
                    ServerAnswer,
                    () => $"the seller sees {seen} zen in the buyer's offer, not {ZenOffer}");
            });
        await context.StepAsync(
            "The seller presses the confirm button",
            "The seller's confirm button stays pressed. Right after the offer changed it ignores clicks for a moment, so it is "
            + "pressed until it counts.",
            () => ConfirmAsync(seller));

        var buyerSequence = await buyer.LastEventSequenceAsync();
        await context.StepAsync(
            "The buyer presses the confirm button",
            "Both have confirmed, so the trade completes and its window closes on both sides.",
            async () =>
            {
                await ConfirmAsync(buyer);
                await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "closed", ["result"] = "completed" }, buyerSequence, ServerAnswer);
            });
        await context.StepAsync(
            "Both look into their inventories",
            $"'{offer.Name}' and the zen have changed owner: {buyerCharacter.Name} has one more jewel ({buyerHadJewels + 1} now) "
            + $"and {ZenOffer:N0} zen less ({buyerHadZen - ZenOffer:N0}); {sellerCharacter.Name} one jewel fewer "
            + $"({sellerHadJewels - 1} left, slot {offer.Slot} empty) and {ZenOffer:N0} zen more ({sellerHadZen + ZenOffer:N0}).",
            async () =>
            {
                await seller.OpenInventoryAsync();
                await buyer.OpenInventoryAsync();
                // The inventories follow the server's answer to the trade.
                var buyerHas = 0;
                var sellerHas = 0;
                await Expect.EventuallyAsync(
                    async () => (buyerHas = await CountAsync(buyer, offer.Name)) == buyerHadJewels + 1,
                    ServerAnswer,
                    () => $"the buyer has {buyerHas} '{offer.Name}' after the trade, not {buyerHadJewels + 1}");
                await Expect.EventuallyAsync(
                    async () => (sellerHas = await CountAsync(seller, offer.Name)) == sellerHadJewels - 1,
                    ServerAnswer,
                    () => $"the seller has {sellerHas} '{offer.Name}' after the trade, not {sellerHadJewels - 1}");
                var sellerInventory = ItemSlots.Of(await seller.StateAsync(), "inventory");
                Expect.That(ItemSlots.At(sellerInventory, offer.Slot)?.Name != offer.Name, $"'{offer.Name}' is still in the seller's slot {offer.Slot}");
                var buyerZen = 0L;
                var sellerZen = 0L;
                await Expect.EventuallyAsync(
                    async () => (buyerZen = await ZenAsync(buyer)) == buyerHadZen - ZenOffer,
                    ServerAnswer,
                    () => $"the buyer has {buyerZen:N0} zen after the trade, not {buyerHadZen - ZenOffer:N0}");
                await Expect.EventuallyAsync(
                    async () => (sellerZen = await ZenAsync(seller)) == sellerHadZen + ZenOffer,
                    ServerAnswer,
                    () => $"the seller has {sellerZen:N0} zen after the trade, not {sellerHadZen + ZenOffer:N0}");
            });
    }

    private static async Task<long> ZenAsync(GameClient client) => (await client.StateAsync()).GetProperty("zen").GetInt64();

    // The zen of one side of the open trade; 0 without a trade.
    private static int TradeZen(System.Text.Json.JsonElement state, string side)
        => state.GetProperty("trade") is { ValueKind: System.Text.Json.JsonValueKind.Object } trade ? trade.GetProperty(side).GetInt32() : 0;

    // How many items named <paramref name="name"/> the inventory holds. `state`
    // lists an item under every square it covers, so the squares are divided by
    // the item's size.
    private static async Task<int> CountAsync(GameClient client, string name)
        => (int)Math.Round(ItemSlots.Of(await client.StateAsync(), "inventory")
            .Where(item => item.Name == name)
            .Sum(item => 1.0 / (item.Width * item.Height)));

    // Walks the buyer next to where the seller stands, as the seller's client
    // sees it: that client checks the distance of a trade request.
    //
    // A warp to a town lands anywhere in it, and a character that is in Lorencia
    // already starts wherever it stood, so how far each one walks changes from run
    // to run: across the whole town at worst. The walk gets time for that. And
    // `move` answers within a tile of its target while the walk may still take
    // its last step, so the buyer heads for where the seller stands now, until
    // the two are next to each other.
    //
    // The buyer's own client can also show it a step further than the others see
    // it, when its last step did not reach them. Then the buyer walks a few tiles
    // away and comes back: a `move` onto a tile in reach sends no walk at all.
    private static async Task WalkUpToAsync(GameClient buyer, GameClient seller, string buyerName)
    {
        var sellerPosition = (X: 0, Y: 0);
        var buyerPosition = (X: 0, Y: 0);
        (int X, int Y)? seenPosition = null;
        await Expect.EventuallyAsync(
            async () =>
            {
                sellerPosition = await PositionAsync(seller);
                buyerPosition = await PositionAsync(buyer);
                seenPosition = await SeenPositionAsync(seller, buyerName);
                var nextToSeller = IsNextTo(buyerPosition, sellerPosition);
                if (nextToSeller && seenPosition is { } seen && IsNextTo(seen, sellerPosition))
                {
                    return true;
                }

                if (nextToSeller)
                {
                    await StepAwayAsync(buyer, sellerPosition);
                    return false;
                }

                await WalkToAsync(buyer, sellerPosition.X, sellerPosition.Y);
                return false;
            },
            WalkTimeout,
            () => $"the buyer ({buyerPosition.X},{buyerPosition.Y}; the seller's client sees it at "
                  + $"{(seenPosition is { } seen ? $"({seen.X},{seen.Y})" : "no place")}) did not get next to the seller "
                  + $"({sellerPosition.X},{sellerPosition.Y})");
    }

    // The client ends one `move` after 30 s; a longer walk (across the town, or
    // with a slowly drawing client) goes on with the next one from where the
    // last stopped, until the walk's own time is up.
    private static async Task WalkToAsync(GameClient client, int x, int y)
    {
        var deadline = DateTime.UtcNow + WalkTimeout;
        while (true)
        {
            try
            {
                await client.SendAsync("move", new { x, y }, WalkTimeout);
                return;
            }
            catch (ControlException exception) when (exception.Error == "timeout" && DateTime.UtcNow < deadline)
            {
                // Walk on.
            }
        }
    }

    private static bool IsNextTo((int X, int Y) position, (int X, int Y) other)
        => Math.Abs(position.X - other.X) <= TradeDistance && Math.Abs(position.Y - other.Y) <= TradeDistance;

    // Three tiles away from the seller, in the first direction that can be walked.
    private static async Task StepAwayAsync(GameClient buyer, (int X, int Y) sellerPosition)
    {
        const int Away = 3;
        foreach (var (dx, dy) in new[] { (Away, 0), (-Away, 0), (0, Away), (0, -Away) })
        {
            try
            {
                await WalkToAsync(buyer, sellerPosition.X + dx, sellerPosition.Y + dy);
                return;
            }
            catch (ControlException exception) when (exception.Error == "no_path")
            {
                // A wall or a building; try the next direction.
            }
        }
    }

    private static async Task<(int X, int Y)> PositionAsync(GameClient client)
    {
        var position = (await client.StateAsync()).GetProperty("position");
        return (position[0].GetInt32(), position[1].GetInt32());
    }

    // Where the client of <paramref name="observer"/> sees the player <paramref name="name"/>.
    private static async Task<(int X, int Y)?> SeenPositionAsync(GameClient observer, string name)
    {
        foreach (var entry in (await observer.StateAsync()).GetProperty("nearby").EnumerateArray())
        {
            if (entry.TryGetProperty("name", out var entryName) && entryName.GetString() == name
                && entry.TryGetProperty("position", out var position))
            {
                return (position[0].GetInt32(), position[1].GetInt32());
            }
        }

        return null;
    }

    private static async Task<ItemSlot> FindJewelAsync(GameClient client)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        return inventory.FirstOrDefault(item => item.Name.StartsWith("Jewel of", StringComparison.Ordinal))
               ?? throw new ScenarioFailedException("the seller has no jewel to trade; the OpenMU test data gives test300Dl some");
    }

    // Setup: the request is a direct command; accepting it is the Enter key on the dialog.
    private static async Task RequestTradeAsync(GameClient seller, GameClient buyer, string buyerName)
    {
        var buyerSequence = await buyer.LastEventSequenceAsync();
        // The seller's client learns where the buyer stands from the server, a
        // moment after the buyer's own; until then it refuses the request as too far.
        await Expect.EventuallyAsync(
            async () =>
            {
                try
                {
                    await seller.SendAsync("trade", new { action = "request", target = buyerName });
                    return true;
                }
                catch (ControlException exception) when (exception.Error == "not_allowed")
                {
                    return false;
                }
            },
            ServerAnswer,
            "the seller's client does not see the buyer next to it");
        await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "requested" }, buyerSequence, ServerAnswer);
        // Enter only reaches the dialog once it is on screen; before that it opens the chat.
        await Expect.EventuallyAsync(
            async () => (await buyer.OpenWindowsAsync()).Contains("message_box"),
            ServerAnswer,
            "the buyer sees no trade request dialog");
    }

    private static async Task AcceptTradeAsync(GameClient seller, GameClient buyer)
    {
        var sellerSequence = await seller.LastEventSequenceAsync();
        var buyerSequence = await buyer.LastEventSequenceAsync();
        await buyer.SendAsync("hotkey", new { key = "enter" });
        await seller.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, sellerSequence, ServerAnswer);
        await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, buyerSequence, ServerAnswer);
    }

    // After an offer changes the button ignores clicks for some frames; how long
    // that takes depends on the frame rate, so it waits for the button, then
    // presses it until the press counts.
    private static async Task ConfirmAsync(GameClient client)
    {
        await Expect.EventuallyAsync(
            async () => (await client.StateAsync()).GetProperty("trade") is not { ValueKind: System.Text.Json.JsonValueKind.Object } trade
                        || trade.GetProperty("my_confirm_wait").GetInt32() <= 0,
            CooldownWait,
            $"the {client.Role}'s confirm button does not take clicks again");
        await Expect.EventuallyAsync(
            async () =>
            {
                await client.ClickElementAsync("trade.confirm");
                var trade = (await client.StateAsync()).GetProperty("trade");
                return trade.ValueKind != System.Text.Json.JsonValueKind.Object || trade.GetProperty("my_confirmed").GetBoolean();
            },
            ConfirmWait,
            $"the {client.Role}'s confirm button does not stay pressed");
    }
}
