using System.Globalization;
using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// A personal shop: the seller prices an item, names the shop and opens it; the
/// buyer opens it through the command window and buys the item. The item and
/// the zen change owner, and the shop is gone once it is empty.
/// </summary>
internal sealed class PersonalShopScenario : Scenario
{
    private const string Seller = "seller";
    private const string Buyer = "buyer";

    private const string JewelName = "Jewel of Bless";
    private const string ShopTitle = "In-game test shop";
    // The jewel's full value: a price below it makes the client ask again.
    private const int Price = 9_000_000;

    // The command window buys from a player at most two tiles away.
    private const int PurchaseDistance = 2;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "personal-shop";

    public override string Description => "a player prices an item and opens a personal shop; another buys it there for the price";

    public override ScenarioCategory Category => ScenarioCategory.PlayerInteractions;

    public override IReadOnlyList<string> Roles => [Seller, Buyer];

    public override int StepCount => 12;

    public override async Task RunAsync(ScenarioContext context)
    {
        var seller = context.Client(Seller);
        var buyer = context.Client(Buyer);
        var sellerCharacter = TestAccounts.ShopSeller;
        var buyerCharacter = TestAccounts.ShopBuyer;
        var price = Price.ToString("N0", CultureInfo.InvariantCulture);

        await context.StepAsync(
            $"The seller logs in as {sellerCharacter.Name}",
            $"{sellerCharacter.Name} enters the world in Devias, where the test data puts the quest characters.",
            () => seller.EnterWorldAsync(sellerCharacter.Account, sellerCharacter.Password, sellerCharacter.Name),
            [Seller]);
        await context.StepAsync(
            $"The buyer logs in as {buyerCharacter.Name}",
            $"{buyerCharacter.Name} enters the world next to {sellerCharacter.Name}.",
            async () =>
            {
                await buyer.EnterWorldAsync(buyerCharacter.Account, buyerCharacter.Password, buyerCharacter.Name);
                await Meeting.WalkUpToAsync(buyer, seller, buyerCharacter.Name, PurchaseDistance);
            },
            [Buyer]);

        await context.StepAsync(
            "The seller opens the inventory with I and presses its personal shop button",
            "The personal shop window opens next to the inventory: an empty grid, a title field and the Open and Close buttons.",
            async () =>
            {
                await seller.OpenInventoryAsync();
                await seller.ClickElementAsync("inventory.my_shop");
                await Expect.EventuallyAsync(
                    async () => (await seller.OpenWindowsAsync()).Contains("my_shop"),
                    ServerAnswer,
                    "the personal shop window does not open");
            },
            [Seller]);

        var jewel = ItemSlots.Of(await seller.StateAsync(), "inventory").FirstOrDefault(item => item.Name == JewelName)
                    ?? throw new ScenarioFailedException($"the seller has no '{JewelName}'; the OpenMU test data gives {sellerCharacter.Name} some");
        var shopSlot = 0;
        await context.StepAsync(
            $"The seller puts a '{JewelName}' into the shop with two clicks",
            $"The first click picks the jewel up from inventory slot {jewel.Slot}, the second puts it into the shop; a dialog asks for its price.",
            async () =>
            {
                shopSlot = await FirstShopSlotAsync(seller);
                await seller.MoveItemAsync("inventory", jewel.Slot, "my_shop", shopSlot);
                await Expect.EventuallyAsync(
                    async () => (await seller.OpenWindowsAsync()).Contains("message_box"),
                    ServerAnswer,
                    "no dialog asks for the price");
            },
            [Seller]);
        await context.StepAsync(
            $"The seller types the price {price} and confirms with Enter",
            $"The dialog closes and the jewel lies in the shop with the price {price} zen.",
            async () =>
            {
                await seller.SendAsync("type", new { text = Price.ToString(CultureInfo.InvariantCulture) });
                await Keys.PressUntilAsync(
                    context,
                    seller,
                    "enter",
                    async () => MyShopItems(await seller.StateAsync()).Any(item => item.Name == JewelName && item.Price == Price),
                    $"the jewel is not in the shop for {price} zen after Enter");
            },
            [Seller]);
        await context.StepAsync(
            $"The seller clicks the title field and types \"{ShopTitle}\"",
            "The title field shows the shop's name.",
            async () =>
            {
                await seller.ClickElementAsync("my_shop.title");
                await seller.SendAsync("type", new { text = ShopTitle });
                var title = string.Empty;
                await Expect.EventuallyAsync(
                    async () => (title = MyShop(await seller.StateAsync()).GetProperty("title").GetString() ?? string.Empty) == ShopTitle,
                    ServerAnswer,
                    () => $"the title field shows \"{title}\"");
            },
            [Seller]);
        await context.StepAsync(
            "The seller presses Open",
            "Every item has a price and the shop a name, so a dialog asks whether to open the shop.",
            async () =>
            {
                await seller.ClickElementAsync("my_shop.open");
                await Expect.EventuallyAsync(
                    async () => (await seller.OpenWindowsAsync()).Contains("message_box"),
                    ServerAnswer,
                    "no dialog asks whether to open the shop");
            },
            [Seller]);
        await context.StepAsync(
            "The seller confirms with Enter",
            $"The shop is open: the server confirms it, and \"{ShopTitle}\" shows above {sellerCharacter.Name}'s head.",
            async () =>
            {
                await Keys.PressUntilAsync(
                    context,
                    seller,
                    "enter",
                    async () => !(await seller.OpenWindowsAsync()).Contains("message_box"),
                    "the open dialog does not take Enter");
                await Expect.EventuallyAsync(
                    async () => MyShop(await seller.StateAsync()).GetProperty("open").GetBoolean(),
                    ServerAnswer,
                    "the server does not open the shop");
            });

        await context.StepAsync(
            "The buyer opens the command window with D",
            "The command window opens with its Buy button.",
            async () =>
            {
                await buyer.SendAsync("hotkey", new { key = "d" });
                await Expect.EventuallyAsync(
                    async () => (await buyer.OpenWindowsAsync()).Contains("command"),
                    ServerAnswer,
                    "the command window does not open");
            },
            [Buyer]);
        await context.StepAsync(
            $"The buyer clicks Buy and right-clicks {sellerCharacter.Name}",
            $"The seller's shop opens for the buyer: \"{ShopTitle}\" by {sellerCharacter.Name}, with the jewel for {price} zen.",
            async () =>
            {
                await buyer.ClickElementAsync("command.purchase");
                var pixel = await Meeting.PixelOfAsync(buyer, sellerCharacter.Name);
                await buyer.SendAsync("click-ui", new { x = pixel.X, y = pixel.Y, button = "right" });
                JsonElement shop = default;
                await Expect.EventuallyAsync(
                    async () => (shop = (await buyer.StateAsync()).GetProperty("purchase_shop")).ValueKind == JsonValueKind.Object,
                    ServerAnswer,
                    "the seller's shop does not open for the buyer");
                Expect.That(shop.GetProperty("seller").GetString() == sellerCharacter.Name, $"the shop is {shop.GetProperty("seller").GetString()}'s");
                Expect.That(shop.GetProperty("title").GetString() == ShopTitle, $"the shop is called \"{shop.GetProperty("title").GetString()}\"");
                Expect.That(
                    ShopItems(shop).Any(item => item.Slot == shopSlot && item.Name == JewelName && item.Price == Price),
                    $"the shop does not offer the jewel in slot {shopSlot} for {price} zen");
            });

        var buyerHadJewels = await buyer.CountAsync(JewelName);
        var buyerHadZen = await buyer.ZenAsync();
        var sellerHadZen = await seller.ZenAsync();
        await context.StepAsync(
            "The buyer clicks the jewel in the shop",
            $"A dialog shows the jewel and asks whether to buy it for {price} zen.",
            async () =>
            {
                await buyer.ClickSlotAsync("purchase_shop", shopSlot);
                await Expect.EventuallyAsync(
                    async () => (await buyer.OpenWindowsAsync()).Contains("message_box"),
                    ServerAnswer,
                    "no dialog asks whether to buy the jewel");
            },
            [Buyer]);
        await context.StepAsync(
            "The buyer confirms with Enter",
            $"The jewel and the zen change owner: {buyerCharacter.Name} has one more jewel and {price} zen less, "
            + $"{sellerCharacter.Name} {price} zen more; the shop is empty now and closes.",
            async () =>
            {
                await Keys.PressUntilAsync(
                    context,
                    buyer,
                    "enter",
                    async () => !(await buyer.OpenWindowsAsync()).Contains("message_box"),
                    "the buy dialog does not take Enter");
                var has = 0;
                await Expect.EventuallyAsync(
                    async () => (has = await buyer.CountAsync(JewelName)) == buyerHadJewels + 1,
                    ServerAnswer,
                    () => $"the buyer has {has} '{JewelName}' after buying, not {buyerHadJewels + 1}");
                await ExpectZenAsync(buyer, buyerHadZen - Price);
                await ExpectZenAsync(seller, sellerHadZen + Price);
                await Expect.EventuallyAsync(
                    async () => MyShopItems(await seller.StateAsync()).Count == 0,
                    ServerAnswer,
                    "the jewel is still in the seller's shop");
            });
    }

    // Something in a personal shop, where it lies and what it costs.
    private sealed record ShopItem(int Slot, string Name, long? Price);

    private static JsonElement MyShop(JsonElement state) => state.GetProperty("my_shop");

    private static IReadOnlyList<ShopItem> MyShopItems(JsonElement state) => ShopItems(MyShop(state));

    private static IReadOnlyList<ShopItem> ShopItems(JsonElement shop)
        => shop.GetProperty("items").EnumerateArray()
            .Select(item => new ShopItem(
                item.GetProperty("slot").GetInt32(),
                item.GetProperty("name").GetString() ?? string.Empty,
                item.GetProperty("price").ValueKind == JsonValueKind.Number ? item.GetProperty("price").GetInt64() : null))
            .ToList();

    // The first square of the seller's shop grid; the slots start after the inventory's.
    private static async Task<int> FirstShopSlotAsync(GameClient seller)
    {
        var used = MyShopItems(await seller.StateAsync()).Select(item => item.Slot).ToHashSet();
        const int FirstShopSlot = 204;
        return Enumerable.Range(FirstShopSlot, 32).First(slot => !used.Contains(slot));
    }

    private static async Task ExpectZenAsync(GameClient client, long expected)
    {
        var zen = 0L;
        await Expect.EventuallyAsync(
            async () => (zen = await client.ZenAsync()) == expected,
            ServerAnswer,
            () => $"the {client.Role} has {zen:N0} zen, not {expected:N0}");
    }
}
