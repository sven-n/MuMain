using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Selling to an NPC's shop and buying from it with clicks: an item dragged onto
/// the shop is sold for the price its tooltip shows, a valuable one after a
/// confirmation, and an item clicked in the shop is bought for its price.
/// </summary>
internal sealed class NpcShopScenario : Scenario
{
    private const string Player = "player";
    private const string ShopGrid = "npc_shop";
    private const string ShopWindow = "npc_shop";

    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;

    private const string PotionName = "Large Healing Potion";
    private const string JewelName = "Jewel of Bless";

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "npc-shop";

    public override string Description => "items dragged onto an NPC's shop are sold for their price, an item clicked in it is bought";

    public override ScenarioCategory Category => ScenarioCategory.NpcInteractions;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 7;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        var character = TestAccounts.NpcShopper;
        var merchant = Npcs.Hanzo;

        await context.StepAsync(
            $"Log in as {character.Name}",
            $"{character.Name} enters the world in Lorencia, its home town.",
            () => player.EnterWorldAsync(character.Account, character.Password, character.Name));
        await context.StepAsync(
            $"Walk up to {merchant.Name}",
            $"{character.Name} stands next to {merchant.Name} ({merchant.X},{merchant.Y}), who sells weapons and shields.",
            async () =>
            {
                await player.WarpAsync(LorenciaGate, LorenciaMap);
                await Npcs.WalkUpToAsync(player, merchant);
            });
        await context.StepAsync(
            $"Talk to {merchant.Name} with a click on him",
            "His shop window opens next to the inventory and lists his goods.",
            async () =>
            {
                await Npcs.TalkAsync(player, merchant, ShopWindow);
                await Expect.EventuallyAsync(
                    async () => ShopItems(await player.StateAsync()).Count > 0,
                    ServerAnswer,
                    $"{merchant.Name}'s shop lists no goods");
            });

        var potion = await FindAsync(player, PotionName);
        await context.StepAsync(
            $"Sell the '{PotionName}' stack: drag it onto the shop with two clicks",
            $"The first click picks the potions up from inventory slot {potion.Slot}, the second drops them onto the shop. They are "
            + $"sold without a question: they leave the inventory, and the zen goes up by the selling price the tooltip shows "
            + $"({potion.SellPrice:N0}).",
            () => SellAsync(player, potion));

        var jewel = await FindAsync(player, JewelName);
        await context.StepAsync(
            $"Drag a '{JewelName}' onto the shop",
            $"A jewel is valuable, so instead of selling it at once a dialog asks whether to sell it; the jewel stays in slot {jewel.Slot}.",
            async () =>
            {
                await player.MoveItemAsync("inventory", jewel.Slot, ShopGrid, await FreeShopSquareAsync(player));
                await Expect.EventuallyAsync(
                    async () => (await player.OpenWindowsAsync()).Contains("message_box"),
                    ServerAnswer,
                    "no dialog asks whether to sell the jewel");
            });
        await context.StepAsync(
            "Confirm the sale with Enter",
            $"The dialog closes and the jewel is sold: slot {jewel.Slot} is empty and the zen goes up by the selling price the "
            + $"tooltip shows ({jewel.SellPrice:N0}).",
            async () =>
            {
                var zen = await player.ZenAsync();
                await Keys.PressUntilAsync(
                    context,
                    player,
                    "enter",
                    async () => !(await player.OpenWindowsAsync()).Contains("message_box"),
                    "the sell dialog does not take Enter");
                await ExpectSoldAsync(player, jewel, zen);
            });

        var goods = await CheapestFittingAsync(player);
        await context.StepAsync(
            $"Buy '{goods.Name}' with a click on it in the shop",
            $"The cheapest of {merchant.Name}'s goods that fits into the inventory ({goods.Width}x{goods.Height} squares) goes into "
            + $"the inventory, and the zen goes down by its price ({goods.Price:N0}).",
            async () =>
            {
                var zen = await player.ZenAsync();
                var had = await player.CountAsync(goods.Name);
                await player.ClickSlotAsync(ShopGrid, goods.Slot);
                var has = 0;
                await Expect.EventuallyAsync(
                    async () => (has = await player.CountAsync(goods.Name)) == had + 1,
                    ServerAnswer,
                    () => $"the inventory has {has} '{goods.Name}' after buying, not {had + 1}");
                var now = 0L;
                await Expect.EventuallyAsync(
                    async () => (now = await player.ZenAsync()) == zen - goods.Price,
                    ServerAnswer,
                    () => $"{now:N0} zen after buying for {goods.Price:N0}, not {zen - goods.Price:N0}");
            });
    }

    // An item of the inventory with what the open shop pays for it.
    private sealed record Sellable(int Slot, string Name, long SellPrice);

    // Something the shop sells, where it lies in the shop and what it costs.
    private sealed record Goods(int Slot, string Name, int Width, int Height, long Price);

    private static async Task<Sellable> FindAsync(GameClient client, string name)
    {
        foreach (var item in (await client.StateAsync()).GetProperty("inventory").EnumerateArray())
        {
            if (item.GetProperty("name").GetString() == name && item.TryGetProperty("sell_price", out var price))
            {
                return new Sellable(item.GetProperty("slot").GetInt32(), name, price.GetInt64());
            }
        }

        throw new ScenarioFailedException($"the inventory has no '{name}' to sell; the OpenMU test data gives {TestAccounts.NpcShopper.Name} some");
    }

    // Two clicks: the item is picked up and dropped onto the shop, which buys it.
    private static async Task SellAsync(GameClient client, Sellable item)
    {
        var zen = await client.ZenAsync();
        await client.MoveItemAsync("inventory", item.Slot, ShopGrid, await FreeShopSquareAsync(client));
        await ExpectSoldAsync(client, item, zen);
    }

    private static async Task ExpectSoldAsync(GameClient client, Sellable item, long zenBefore)
    {
        await Expect.EventuallyAsync(
            async () => ItemSlots.At(ItemSlots.Of(await client.StateAsync(), "inventory"), item.Slot)?.Name != item.Name,
            ServerAnswer,
            $"'{item.Name}' is still in slot {item.Slot}");
        var zen = 0L;
        await Expect.EventuallyAsync(
            async () => (zen = await client.ZenAsync()) == zenBefore + item.SellPrice,
            ServerAnswer,
            () => $"{zen:N0} zen after selling '{item.Name}' for {item.SellPrice:N0}, not {zenBefore + item.SellPrice:N0}");
    }

    // A square of the shop no goods cover: a drop there sells the item.
    private static async Task<int> FreeShopSquareAsync(GameClient client)
    {
        var used = new HashSet<int>();
        foreach (var goods in ShopItems(await client.StateAsync()))
        {
            for (var dy = 0; dy < goods.Height; dy++)
            {
                for (var dx = 0; dx < goods.Width; dx++)
                {
                    used.Add(goods.Slot + (dy * ShopColumns) + dx);
                }
            }
        }

        return Enumerable.Range(0, ShopColumns * ShopRows).Reverse().First(square => !used.Contains(square));
    }

    private const int ShopColumns = 8;
    private const int ShopRows = 15;

    private static async Task<Goods> CheapestFittingAsync(GameClient client)
    {
        var state = await client.StateAsync();
        var inventory = ItemSlots.Of(state, "inventory");
        return ShopItems(state)
                   .Where(goods => ItemSlots.FreeArea(inventory, goods.Width, goods.Height) is not null)
                   .OrderBy(goods => goods.Price)
                   .FirstOrDefault()
               ?? throw new ScenarioFailedException("none of the shop's goods fits into the inventory");
    }

    private static IReadOnlyList<Goods> ShopItems(JsonElement state)
        => state.GetProperty("npc_shop") is { ValueKind: JsonValueKind.Object } shop
            ? shop.GetProperty("items").EnumerateArray()
                .Select(item => new Goods(
                    item.GetProperty("slot").GetInt32(),
                    item.GetProperty("name").GetString() ?? string.Empty,
                    item.GetProperty("width").GetInt32(),
                    item.GetProperty("height").GetInt32(),
                    item.GetProperty("price").GetInt64()))
                .ToList()
            : [];
}
