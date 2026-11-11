using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// A party of five, the most a party takes: the leader invites four players one
/// after the other through the command window (D, Party, a right-click on the
/// player), and each accepts the invitation with Enter.
/// </summary>
internal sealed class PartyScenario : Scenario
{
    private const string Leader = "leader";

    // A free spot in Lorencia's town; the command window invites a player at most two tiles away.
    private const string LorenciaGate = "Lorencia";
    private const int LorenciaMap = 0;
    private const int MeetingX = 135;
    private const int MeetingY = 128;
    private const int InviteDistance = 2;

    // Where a member steps once it joined, so the next one is not behind it
    // when the leader right-clicks.
    private static readonly (int X, int Y)[] Aside = [(-4, 0), (4, 0), (0, 4), (0, -4), (-4, 4), (4, -4)];

    private static readonly TimeSpan ServerAnswer = TimeSpan.FromSeconds(10);

    private static readonly string[] Members = ["member1", "member2", "member3", "member4"];

    public override string Name => "party";

    public override string Description => "the leader invites four players through the command window, each accepts with Enter: a party of five";

    public override ScenarioCategory Category => ScenarioCategory.PlayerInteractions;

    public override IReadOnlyList<string> Roles => [Leader, .. Members];

    public override int StepCount => 3 + (3 * Members.Length) + 1;

    public override async Task RunAsync(ScenarioContext context)
    {
        var leader = context.Client(Leader);
        var leaderCharacter = TestAccounts.PartyLeader;
        var memberCharacters = TestAccounts.PartyMembers;

        await context.StepAsync(
            "All five log in",
            $"{leaderCharacter.Name} (the leader) and {string.Join(", ", memberCharacters.Select(member => member.Name))} enter the "
            + "world in Lorencia, their home town.",
            async () =>
            {
                await leader.EnterWorldAsync(leaderCharacter.Account, leaderCharacter.Password, leaderCharacter.Name);
                for (var index = 0; index < Members.Length; index++)
                {
                    var member = memberCharacters[index];
                    await context.Client(Members[index]).EnterWorldAsync(member.Account, member.Password, member.Name);
                }
            });
        await context.StepAsync(
            "The leader walks to the meeting spot",
            $"{leaderCharacter.Name} stands at ({MeetingX},{MeetingY}), a free spot in Lorencia's town, or one tile from it.",
            async () =>
            {
                await leader.WarpAsync(LorenciaGate, LorenciaMap);
                await Meeting.WalkToAsync(leader, MeetingX, MeetingY);
            },
            [Leader]);
        await context.StepAsync(
            "The leader opens the command window with D",
            "The command window opens on the right with its buttons: Trade, Buy, Party and the others.",
            async () =>
            {
                await leader.SendAsync("hotkey", new { key = "d" });
                await Expect.EventuallyAsync(
                    async () => (await leader.OpenWindowsAsync()).Contains("command"),
                    ServerAnswer,
                    "the command window does not open");
            },
            [Leader]);

        for (var index = 0; index < Members.Length; index++)
        {
            var role = Members[index];
            var member = context.Client(role);
            var name = memberCharacters[index].Name;
            var size = index + 2;
            await context.StepAsync(
                $"{name} walks up to the leader",
                $"{name} stands at most {InviteDistance} tiles from {leaderCharacter.Name}: the command window invites only a player that close.",
                async () =>
                {
                    await member.WarpAsync(LorenciaGate, LorenciaMap);
                    await Meeting.WalkUpToAsync(member, leader, name, InviteDistance);
                },
                [Leader, role]);
            await context.StepAsync(
                $"The leader clicks Party in the command window and right-clicks {name}",
                $"The click selects the Party command; the right-click on {name} invites it. {name} gets a dialog that asks "
                + $"whether to join {leaderCharacter.Name}'s party.",
                () => InviteAsync(leader, member, leaderCharacter.Name, name),
                [Leader, role]);
            await context.StepAsync(
                $"{name} accepts with Enter",
                $"The dialog closes, and the party lists of the leader and of {name} show {size} members, {leaderCharacter.Name} "
                + $"first. {name} then steps aside to make room for the next one.",
                async () =>
                {
                    await Keys.PressUntilAsync(
                        context,
                        member,
                        "enter",
                        async () => !(await member.OpenWindowsAsync()).Contains("message_box"),
                        $"{name}'s invitation dialog does not take Enter");
                    await ExpectPartyAsync(leader, leaderCharacter.Name, size);
                    await ExpectPartyAsync(member, leaderCharacter.Name, size);
                    await StepAsideAsync(member, leader, index);
                },
                [Leader, role]);
        }

        var everyone = new[] { leaderCharacter.Name }.Concat(memberCharacters.Select(member => member.Name)).ToList();
        await context.StepAsync(
            "Every player looks at the party list",
            $"All five show the same party of five, {leaderCharacter.Name} first: {string.Join(", ", everyone)}.",
            async () =>
            {
                foreach (var role in Roles)
                {
                    var client = context.Client(role);
                    IReadOnlyList<string> party = [];
                    await Expect.EventuallyAsync(
                        async () => (party = await PartyAsync(client)).SequenceEqual(everyone),
                        ServerAnswer,
                        () => $"the {role}'s party list is {(party.Count == 0 ? "empty" : string.Join(", ", party))}, "
                              + $"not {string.Join(", ", everyone)}");
                }
            });
    }

    // The Party command, then a right-click on the member where the leader's
    // client draws it; the member's client shows the invitation.
    private static async Task InviteAsync(GameClient leader, GameClient member, string leaderName, string memberName)
    {
        var sequence = await member.LastEventSequenceAsync();
        await leader.ClickElementAsync("command.party");
        var pixel = await Meeting.PixelOfAsync(leader, memberName);
        await leader.SendAsync("click-ui", new { x = pixel.X, y = pixel.Y, button = "right" });
        await member.WaitForEventAsync(
            "party",
            new Dictionary<string, string> { ["change"] = "invited", ["name"] = leaderName },
            sequence,
            ServerAnswer);
        await Expect.EventuallyAsync(
            async () => (await member.OpenWindowsAsync()).Contains("message_box"),
            ServerAnswer,
            $"{memberName} sees no invitation dialog");
    }

    private static async Task ExpectPartyAsync(GameClient client, string leaderName, int size)
    {
        IReadOnlyList<string> party = [];
        await Expect.EventuallyAsync(
            async () => (party = await PartyAsync(client)).Count == size && party[0] == leaderName,
            ServerAnswer,
            () => $"the {client.Role}'s party list is {(party.Count == 0 ? "empty" : string.Join(", ", party))}, "
                  + $"not {size} members led by {leaderName}");
    }

    private static async Task<IReadOnlyList<string>> PartyAsync(GameClient client)
        => (await client.StateAsync()).GetProperty("party").EnumerateArray()
            .Select(member => member.GetProperty("name").GetString() ?? string.Empty)
            .ToList();

    // A few tiles away from the leader, in a direction of its own for each member.
    private static async Task StepAsideAsync(GameClient member, GameClient leader, int index)
    {
        var (x, y) = await Meeting.PositionAsync(leader);
        for (var attempt = 0; attempt < Aside.Length; attempt++)
        {
            var (dx, dy) = Aside[(index + attempt) % Aside.Length];
            try
            {
                await Meeting.WalkToAsync(member, x + dx, y + dy);
                return;
            }
            catch (ControlException exception) when (exception.Error == "no_path")
            {
                // A wall; try the next direction.
            }
        }
    }
}
