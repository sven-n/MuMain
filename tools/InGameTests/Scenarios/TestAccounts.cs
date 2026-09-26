namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>A test account and one of its characters.</summary>
internal sealed record TestCharacter(string Account, string Password, string Name);

/// <summary>
/// Accounts OpenMU creates with its test data (VersionSeasonSix/TestAccounts);
/// the password is the account name. Each scenario has accounts of its own, and
/// none of them is a game master account people play with, so a test and a
/// person on the same server do not get in each other's way.
/// </summary>
internal static class TestAccounts
{
    /// <summary>
    /// <c>icarus-take-off</c>: level 400 High Elf with a Wing of Illusion and a Horn of Fenrir,
    /// and room in the inventory for both.
    /// </summary>
    public static readonly TestCharacter IcarusFlyer = new("test400", "test400", "test400Elf");

    /// <summary><c>trade</c> seller: level 300 Dark Lord with Jewels of Bless in the inventory.</summary>
    public static readonly TestCharacter TradeSeller = new("test300", "test300", "test300Dl");

    /// <summary><c>trade</c> buyer: level 380 Elf.</summary>
    public static readonly TestCharacter TradeBuyer = new("socket", "socket", "socketElf");
}
