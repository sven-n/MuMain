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

    // What the buyer pays: a number that stands out in the screenshots.
    private const int ZenOffer = 123456;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

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
            () => seller.EnterWorldAsync(sellerCharacter.Account, sellerCharacter.Password, sellerCharacter.Name),
            [Seller]);
        await context.StepAsync(
            $"The buyer logs in as {buyerCharacter.Name}",
            $"{buyerCharacter.Name} enters the world in the safe zone of its home map.",
            () => buyer.EnterWorldAsync(buyerCharacter.Account, buyerCharacter.Password, buyerCharacter.Name),
            [Buyer]);
        await context.StepAsync(
            "The seller warps to Lorencia",
            $"{sellerCharacter.Name} stands in Lorencia's town. A warp to a town lands anywhere in it.",
            () => seller.WarpAsync(LorenciaGate, LorenciaMap),
            [Seller]);
        await context.StepAsync(
            "The buyer warps to Lorencia",
            $"{buyerCharacter.Name} stands in Lorencia's town, somewhere else than the seller.",
            () => buyer.WarpAsync(LorenciaGate, LorenciaMap),
            [Buyer]);
        await context.StepAsync(
            "The seller walks to the meeting spot",
            $"{sellerCharacter.Name} stands at ({MeetingX},{MeetingY}), a free spot in Lorencia's town, or one tile from it.",
            () => Meeting.WalkToAsync(seller, MeetingX, MeetingY),
            [Seller]);
        await context.StepAsync(
            "The buyer walks up to the seller",
            $"{buyerCharacter.Name} stands on a tile next to {sellerCharacter.Name}: a trade needs the partner at most one tile away.",
            () => Meeting.WalkUpToAsync(buyer, seller, buyerCharacter.Name));

        var offer = await FindJewelAsync(seller);
        // What each one has before the trade: the buyer may have such a jewel
        // already, and the warps have cost zen.
        var sellerHadJewels = await seller.CountAsync(offer.Name);
        var buyerHadJewels = await buyer.CountAsync(offer.Name);
        var sellerHadZen = await seller.ZenAsync();
        var buyerHadZen = await buyer.ZenAsync();
        await context.StepAsync(
            "The seller asks the buyer for a trade",
            $"{buyerCharacter.Name} gets the request: a dialog asks whether to trade with {sellerCharacter.Name}.",
            () => Trading.RequestAsync(seller, buyer, buyerCharacter.Name),
            [Buyer]);
        await context.StepAsync(
            "The buyer accepts with Enter",
            "The trade window opens for both characters, each with the inventory next to it.",
            () => Trading.AcceptAsync(context, seller, buyer));
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
                await Keys.PressUntilAsync(
                    context,
                    buyer,
                    "enter",
                    async () => !(await buyer.OpenWindowsAsync()).Contains("message_box"),
                    "the zen box does not take Enter");
                var seen = 0;
                await Expect.EventuallyAsync(
                    async () => (seen = Trading.Zen(await seller.StateAsync(), "partner_zen")) == ZenOffer,
                    ServerAnswer,
                    () => $"the seller sees {seen} zen in the buyer's offer, not {ZenOffer}");
            });
        await context.StepAsync(
            "The seller presses the confirm button",
            "The seller's confirm button stays pressed. Right after the offer changed it ignores clicks for a moment, so it is "
            + "pressed until it counts.",
            () => Trading.ConfirmAsync(seller));

        var buyerSequence = await buyer.LastEventSequenceAsync();
        await context.StepAsync(
            "The buyer presses the confirm button",
            "Both have confirmed, so the trade completes and its window closes on both sides.",
            async () =>
            {
                await Trading.ConfirmAsync(buyer);
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
                    async () => (buyerHas = await buyer.CountAsync(offer.Name)) == buyerHadJewels + 1,
                    ServerAnswer,
                    () => $"the buyer has {buyerHas} '{offer.Name}' after the trade, not {buyerHadJewels + 1}");
                await Expect.EventuallyAsync(
                    async () => (sellerHas = await seller.CountAsync(offer.Name)) == sellerHadJewels - 1,
                    ServerAnswer,
                    () => $"the seller has {sellerHas} '{offer.Name}' after the trade, not {sellerHadJewels - 1}");
                var sellerInventory = ItemSlots.Of(await seller.StateAsync(), "inventory");
                Expect.That(ItemSlots.At(sellerInventory, offer.Slot)?.Name != offer.Name, $"'{offer.Name}' is still in the seller's slot {offer.Slot}");
                var buyerZen = 0L;
                var sellerZen = 0L;
                await Expect.EventuallyAsync(
                    async () => (buyerZen = await buyer.ZenAsync()) == buyerHadZen - ZenOffer,
                    ServerAnswer,
                    () => $"the buyer has {buyerZen:N0} zen after the trade, not {buyerHadZen - ZenOffer:N0}");
                await Expect.EventuallyAsync(
                    async () => (sellerZen = await seller.ZenAsync()) == sellerHadZen + ZenOffer,
                    ServerAnswer,
                    () => $"the seller has {sellerZen:N0} zen after the trade, not {sellerHadZen + ZenOffer:N0}");
            });
    }

    private static async Task<ItemSlot> FindJewelAsync(GameClient client)
    {
        var inventory = ItemSlots.Of(await client.StateAsync(), "inventory");
        return inventory.FirstOrDefault(item => item.Name.StartsWith("Jewel of", StringComparison.Ordinal))
               ?? throw new ScenarioFailedException("the seller has no jewel to trade; the OpenMU test data gives test300Dl some");
    }
}
