using MuMain.Tools.InGameTests.Clients;

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

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // After an offer changes, the confirm button waits about 150 frames.
    private static readonly TimeSpan ConfirmWait = TimeSpan.FromSeconds(20);

    public override string Name => "trade";

    public override string Description => "an item put into the trade window with clicks changes owner (#588)";

    public override IReadOnlyList<string> Roles => [Seller, Buyer];

    public override async Task RunAsync(ScenarioContext context)
    {
        var seller = context.Client(Seller);
        var buyer = context.Client(Buyer);
        var sellerCharacter = TestAccounts.TradeSeller;
        var buyerCharacter = TestAccounts.TradeBuyer;

        context.Log("both characters enter Lorencia and meet");
        await seller.EnterWorldAsync(sellerCharacter.Account, sellerCharacter.Password, sellerCharacter.Name);
        await buyer.EnterWorldAsync(buyerCharacter.Account, buyerCharacter.Password, buyerCharacter.Name);
        await MeetAsync(seller, buyer);

        var offer = await FindJewelAsync(seller);
        context.Log($"the seller offers '{offer.Name}' from inventory slot {offer.Slot}");

        await OpenTradeAsync(context, seller, buyer, buyerCharacter.Name);

        context.Log("the seller puts the item into the trade window with two clicks");
        await seller.OpenInventoryAsync();
        await seller.MoveItemAsync("inventory", offer.Slot, TradeGrid, FirstTradeSlot);
        await Expect.EventuallyAsync(
            async () => ItemSlots.OfTrade(await buyer.StateAsync(), "partner_items").Any(item => item.Name == offer.Name),
            ServerAnswer,
            $"the buyer does not see '{offer.Name}' in the seller's offer");

        context.Log("both press the confirm button");
        var buyerSequence = await buyer.LastEventSequenceAsync();
        await ConfirmAsync(seller);
        await ConfirmAsync(buyer);
        await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "closed", ["result"] = "completed" }, buyerSequence, ServerAnswer);

        var buyerInventory = ItemSlots.Of(await buyer.StateAsync(), "inventory");
        Expect.That(buyerInventory.Any(item => item.Name == offer.Name), $"the buyer's inventory has no '{offer.Name}' after the trade");
        var sellerInventory = ItemSlots.Of(await seller.StateAsync(), "inventory");
        Expect.That(ItemSlots.At(sellerInventory, offer.Slot)?.Name != offer.Name, $"'{offer.Name}' is still in the seller's slot {offer.Slot}");
    }

    // Warps both to Lorencia and walks the buyer next to where the seller stands.
    private static async Task MeetAsync(GameClient seller, GameClient buyer)
    {
        var warpTimeout = TimeSpan.FromSeconds(60);
        await WarpToLorenciaAsync(seller, warpTimeout);
        await WarpToLorenciaAsync(buyer, warpTimeout);
        await seller.SendAsync("move", new { x = MeetingX, y = MeetingY }, warpTimeout);
        var (sellerX, sellerY) = await PositionAsync(seller);
        await buyer.SendAsync("move", new { x = sellerX + 1, y = sellerY }, warpTimeout);

        var (buyerX, buyerY) = await PositionAsync(buyer);
        Expect.That(
            Math.Abs(buyerX - sellerX) <= TradeDistance && Math.Abs(buyerY - sellerY) <= TradeDistance,
            $"the buyer ({buyerX},{buyerY}) did not get next to the seller ({sellerX},{sellerY})");
    }

    // The client refuses a warp to the map the character is on.
    private static async Task WarpToLorenciaAsync(GameClient client, TimeSpan timeout)
    {
        if ((await client.StateAsync()).GetProperty("map").GetInt32() != LorenciaMap)
        {
            await client.SendAsync("warp", new { gate = LorenciaGate }, timeout);
        }
    }

    private static async Task<(int X, int Y)> PositionAsync(GameClient client)
    {
        var position = (await client.StateAsync()).GetProperty("position");
        return (position[0].GetInt32(), position[1].GetInt32());
    }

    private static async Task<ItemSlot> FindJewelAsync(GameClient client)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        return inventory.FirstOrDefault(item => item.Name.StartsWith("Jewel of", StringComparison.Ordinal))
               ?? throw new ScenarioFailedException("the seller has no jewel to trade; the OpenMU test data gives test300Dl some");
    }

    // Setup: the request is a direct command; accepting it is the Enter key on the dialog.
    private static async Task OpenTradeAsync(ScenarioContext context, GameClient seller, GameClient buyer, string buyerName)
    {
        context.Log("the seller asks for a trade, the buyer accepts with Enter");
        var sellerSequence = await seller.LastEventSequenceAsync();
        var buyerSequence = await buyer.LastEventSequenceAsync();
        await seller.SendAsync("trade", new { action = "request", target = buyerName });
        await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "requested" }, buyerSequence, ServerAnswer);
        // Enter only reaches the dialog once it is on screen; before that it opens the chat.
        await Expect.EventuallyAsync(
            async () => (await buyer.OpenWindowsAsync()).Contains("message_box"),
            ServerAnswer,
            "the buyer sees no trade request dialog");
        await buyer.SendAsync("hotkey", new { key = "enter" });
        await seller.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, sellerSequence, ServerAnswer);
        await buyer.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, buyerSequence, ServerAnswer);
    }

    // The button ignores clicks while it waits after a change, so it is pressed until it counts.
    private static Task ConfirmAsync(GameClient client)
        => Expect.EventuallyAsync(
            async () =>
            {
                await client.ClickElementAsync("trade.confirm");
                var trade = (await client.StateAsync()).GetProperty("trade");
                return trade.ValueKind != System.Text.Json.JsonValueKind.Object || trade.GetProperty("my_confirmed").GetBoolean();
            },
            ConfirmWait,
            $"the {client.Role}'s confirm button does not stay pressed");
}
