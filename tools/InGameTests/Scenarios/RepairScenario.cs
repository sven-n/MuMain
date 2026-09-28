using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Repairing worn items: one with the inventory's own repair button, the rest
/// at a blacksmith with "Repair all". Both bring the durability back to its
/// maximum for zen.
/// </summary>
internal sealed class RepairScenario : Scenario
{
    private const string Player = "player";
    private const int GlovesSlot = 5;
    private const int BootsSlot = 6;

    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    public override string Name => "repair";

    public override string Description => "worn items are repaired with the inventory's repair button and at a blacksmith with Repair all";

    public override ScenarioCategory Category => ScenarioCategory.NpcInteractions;

    public override IReadOnlyList<string> Roles => [Player];

    public override int StepCount => 7;

    public override async Task RunAsync(ScenarioContext context)
    {
        var player = context.Client(Player);
        var character = TestAccounts.Repairer;
        var blacksmith = Npcs.Hanzo;

        await context.StepAsync(
            $"Log in as {character.Name}",
            $"{character.Name} enters the world in Lorencia, its home town.",
            () => player.EnterWorldAsync(character.Account, character.Password, character.Name));

        var gloves = await WornAsync(player, GlovesSlot);
        var boots = await WornAsync(player, BootsSlot);
        await context.StepAsync(
            "Open the inventory with I",
            $"The inventory shows '{gloves.Name}' ({gloves.Durability} of {gloves.MaxDurability}) and '{boots.Name}' "
            + $"({boots.Durability} of {boots.MaxDurability}) worn, their slots tinted.",
            () => player.OpenInventoryAsync());
        await context.StepAsync(
            "Press the inventory's repair button",
            "The repair mode is on: the pointer turns into a hammer, and a click on an item repairs it instead of picking it up.",
            async () =>
            {
                await player.ClickElementAsync("inventory.repair");
                await Expect.EventuallyAsync(
                    async () => (await player.StateAsync()).GetProperty("repair_mode").GetBoolean(),
                    ServerAnswer,
                    "the repair mode does not come on");
            });

        await context.StepAsync(
            $"Click '{gloves.Name}' to repair it",
            $"Its durability is back at {gloves.MaxDurability}, and zen went down by the repair price; a repair from the "
            + "inventory costs two and a half times what a blacksmith asks.",
            async () =>
            {
                var shown = (await WornAsync(player, GlovesSlot)).RepairPrice;
                var zen = await player.ZenAsync();
                await player.ClickSlotAsync("equipment", GlovesSlot);
                await ExpectRepairedAsync(player, GlovesSlot, gloves.MaxDurability);
                await NoteChargeAsync(context, player, zen, shown, gloves.Name);
            });
        await context.StepAsync(
            $"Close the inventory and walk up to {blacksmith.Name}",
            $"{character.Name} stands next to {blacksmith.Name} ({blacksmith.X},{blacksmith.Y}), who repairs.",
            async () =>
            {
                await player.SendAsync("hotkey", new { key = "i" });
                await Expect.EventuallyAsync(
                    async () => !(await player.OpenWindowsAsync()).Contains("inventory"),
                    ServerAnswer,
                    "the inventory does not close");
                await player.WarpAsync(LorenciaGate, LorenciaMap);
                await Npcs.WalkUpToAsync(player, blacksmith);
            });
        await context.StepAsync(
            $"Talk to {blacksmith.Name} with a click on him",
            "His shop opens with its Repair and Repair all buttons, and shows what repairing everything costs.",
            async () =>
            {
                await Npcs.TalkAsync(player, blacksmith, "npc_shop");
                await Expect.EventuallyAsync(
                    async () => (await player.StateAsync()).GetProperty("npc_shop") is { ValueKind: JsonValueKind.Object } shop
                                && shop.GetProperty("repair_shop").GetBoolean(),
                    ServerAnswer,
                    $"{blacksmith.Name}'s shop has no repair buttons");
            });
        await context.StepAsync(
            "Press Repair all",
            $"'{boots.Name}' has its durability back at {boots.MaxDurability}, and zen went down by what the shop showed for "
            + "repairing everything.",
            async () =>
            {
                var shown = (await player.StateAsync()).GetProperty("npc_shop").GetProperty("repair_all_price").GetInt64();
                var zen = await player.ZenAsync();
                await player.ClickElementAsync("npc_shop.repair_all");
                await ExpectRepairedAsync(player, BootsSlot, boots.MaxDurability);
                await NoteChargeAsync(context, player, zen, shown, "everything");
            });
    }

    // An equipped item and what repairing it costs now, if the client shows a price.
    private sealed record Worn(string Name, int Durability, int MaxDurability, long? RepairPrice);

    private static async Task<Worn> WornAsync(GameClient client, int slot)
    {
        foreach (var item in (await client.StateAsync()).GetProperty("equipment").EnumerateArray())
        {
            if (item.GetProperty("slot").GetInt32() != slot)
            {
                continue;
            }

            var worn = new Worn(
                item.GetProperty("name").GetString() ?? string.Empty,
                item.GetProperty("durability").GetInt32(),
                item.GetProperty("max_durability").GetInt32(),
                item.TryGetProperty("repair_price", out var price) ? price.GetInt64() : null);
            Expect.That(
                worn.Durability < worn.MaxDurability,
                $"'{worn.Name}' in slot {slot} is not worn ({worn.Durability} of {worn.MaxDurability}); nothing to repair");
            return worn;
        }

        throw new ScenarioFailedException($"nothing is equipped in slot {slot}; the OpenMU test data equips {TestAccounts.Repairer.Name} there");
    }

    private static async Task ExpectRepairedAsync(GameClient client, int slot, int maxDurability)
    {
        var durability = 0;
        await Expect.EventuallyAsync(
            async () =>
            {
                var item = (await client.StateAsync()).GetProperty("equipment").EnumerateArray()
                    .FirstOrDefault(entry => entry.GetProperty("slot").GetInt32() == slot);
                durability = item.ValueKind == JsonValueKind.Object ? item.GetProperty("durability").GetInt32() : -1;
                return durability == maxDurability;
            },
            ServerAnswer,
            () => $"slot {slot} has {durability} durability after the repair, not {maxDurability}");
    }

    // The repair has to cost zen; what it cost and what the client showed go into the log.
    private static async Task NoteChargeAsync(ScenarioContext context, GameClient client, long zenBefore, long? shown, string what)
    {
        var zen = zenBefore;
        await Expect.EventuallyAsync(
            async () => (zen = await client.ZenAsync()) < zenBefore,
            ServerAnswer,
            $"repairing {what} cost no zen");
        context.Note($"repairing {what} cost {zenBefore - zen:N0} zen; the client showed {(shown is { } price ? $"{price:N0}" : "no price")}");
    }
}
