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
    /// <c>icarus-flying-item-take-off</c>: level 400 High Elf with a Wing of Illusion and a Horn of Fenrir,
    /// and room in the inventory for both.
    /// </summary>
    public static readonly TestCharacter IcarusFlyer = new("test400", "test400", "test400Elf");

    /// <summary><c>trade</c> seller: level 300 Dark Lord with Jewels of Bless in the inventory.</summary>
    public static readonly TestCharacter TradeSeller = new("test300", "test300", "test300Dl");

    /// <summary><c>trade</c> buyer: level 380 Elf.</summary>
    public static readonly TestCharacter TradeBuyer = new("socket", "socket", "socketElf");

    /// <summary><c>chat</c>: level 51 Dark Knight, at home in Lorencia.</summary>
    public static readonly TestCharacter ChatFirst = new("test5", "test5", "test5Dk");

    /// <summary><c>chat</c>: level 61 Dark Knight, at home in Lorencia.</summary>
    public static readonly TestCharacter ChatSecond = new("test6", "test6", "test6Dk");

    /// <summary>
    /// <c>npc-shop</c>: level 71 Dark Knight in Lorencia with potions and Jewels of Bless to sell,
    /// and 25 free squares for what it buys.
    /// </summary>
    public static readonly TestCharacter NpcShopper = new("test7", "test7", "test7Dk");

    /// <summary>
    /// <c>repair</c>: level 91 Dark Knight in Lorencia, high enough for the inventory's repair
    /// button. The test data gives its excellent gloves and boots 30 of 45 durability.
    /// </summary>
    public static readonly TestCharacter Repairer = new("test9", "test9", "test9Dk");
}
