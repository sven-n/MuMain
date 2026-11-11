using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>The steps of a trade between two clients that every trade scenario takes.</summary>
internal static class Trading
{
    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);
    // After an offer changes, the confirm button waits about 150 frames: some
    // seconds normally, half a minute at five frames a second.
    private static readonly TimeSpan CooldownWait = TimeSpan.FromSeconds(120);
    private static readonly TimeSpan ConfirmWait = TimeSpan.FromSeconds(20);

    /// <summary>
    /// Setup: the request is a direct command. Waits until <paramref name="partner"/>'s client
    /// shows the request's dialog, which accepting it with Enter needs.
    /// </summary>
    public static async Task RequestAsync(GameClient requester, GameClient partner, string partnerName)
    {
        var partnerSequence = await partner.LastEventSequenceAsync();
        // The requester's client learns where the partner stands from the server, a
        // moment after the partner's own; until then it refuses the request as too far.
        await Expect.EventuallyAsync(
            async () =>
            {
                try
                {
                    await requester.SendAsync("trade", new { action = "request", target = partnerName });
                    return true;
                }
                catch (ControlException exception) when (exception.Error == "not_allowed")
                {
                    return false;
                }
            },
            ServerAnswer,
            $"the {requester.Role}'s client does not see the {partner.Role} next to it");
        await partner.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "requested" }, partnerSequence, ServerAnswer);
        // Enter only reaches the dialog once it is on screen; before that it opens the chat.
        await Expect.EventuallyAsync(
            async () => (await partner.OpenWindowsAsync()).Contains("message_box"),
            ServerAnswer,
            $"the {partner.Role} sees no trade request dialog");
    }

    /// <summary>The partner accepts with Enter; the trade window opens on both sides.</summary>
    public static async Task AcceptAsync(ScenarioContext context, GameClient requester, GameClient partner)
    {
        var requesterSequence = await requester.LastEventSequenceAsync();
        var partnerSequence = await partner.LastEventSequenceAsync();
        await Keys.PressUntilAsync(
            context,
            partner,
            "enter",
            async () => !(await partner.OpenWindowsAsync()).Contains("message_box"),
            $"the {partner.Role}'s trade request dialog does not take Enter");
        await requester.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, requesterSequence, ServerAnswer);
        await partner.WaitForEventAsync("trade", new Dictionary<string, string> { ["change"] = "opened" }, partnerSequence, ServerAnswer);
    }

    /// <summary>
    /// Presses the confirm button. After an offer changes the button ignores clicks for some
    /// frames; how long that takes depends on the frame rate, so it waits for the button,
    /// then presses it until the press counts.
    /// </summary>
    public static async Task ConfirmAsync(GameClient client)
    {
        await Expect.EventuallyAsync(
            async () => (await client.StateAsync()).GetProperty("trade") is not { ValueKind: JsonValueKind.Object } trade
                        || trade.GetProperty("my_confirm_wait").GetInt32() <= 0,
            CooldownWait,
            $"the {client.Role}'s confirm button does not take clicks again");
        await Expect.EventuallyAsync(
            async () =>
            {
                await client.ClickElementAsync("trade.confirm");
                var trade = (await client.StateAsync()).GetProperty("trade");
                return trade.ValueKind != JsonValueKind.Object || trade.GetProperty("my_confirmed").GetBoolean();
            },
            ConfirmWait,
            $"the {client.Role}'s confirm button does not stay pressed");
    }

    /// <summary>The zen of one side of the open trade (<c>my_zen</c> or <c>partner_zen</c>); 0 without a trade.</summary>
    public static int Zen(JsonElement state, string side)
        => state.GetProperty("trade") is { ValueKind: JsonValueKind.Object } trade ? trade.GetProperty(side).GetInt32() : 0;
}
