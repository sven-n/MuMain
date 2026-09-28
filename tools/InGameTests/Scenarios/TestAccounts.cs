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

    /// <summary>
    /// <c>equip-all-slots</c>: level 400 Dimension Master in Lorencia that wears something in all
    /// twelve equipment slots and has room in the inventory for each of them.
    /// </summary>
    public static readonly TestCharacter Equipper = new("testgm2", "testgm2", "testgm2Sum");

    /// <summary>
    /// <c>trade-inventory-full</c>: level 71 and 81 Dark Wizards in Lorencia. The test data leaves
    /// nine single free squares and no free 2x2 area in their inventories, and gives each 2x2 pad
    /// armour pieces and rows of jewels and potions to offer.
    /// </summary>
    public static readonly TestCharacter FullTraderFirst = new("test7", "test7", "test7Dw");

    /// <inheritdoc cref="FullTraderFirst"/>
    public static readonly TestCharacter FullTraderSecond = new("test8", "test8", "test8Dw");

    /// <summary>
    /// <c>personal-shop</c> seller: level 150 Dark Knight with 100,000,000 zen and Jewels of Bless;
    /// the quest accounts all start on one tile in Devias.
    /// </summary>
    public static readonly TestCharacter ShopSeller = new("quest1", "quest1", "quest1Dk");

    /// <summary><c>personal-shop</c> buyer: level 220 Dark Knight with 100,000,000 zen, next to the seller in Devias.</summary>
    public static readonly TestCharacter ShopBuyer = new("quest2", "quest2", "quest2Dk");

    /// <summary><c>party</c> leader: level 1 Dark Knight, at home in Lorencia.</summary>
    public static readonly TestCharacter PartyLeader = new("test0", "test0", "test0Dk");

    /// <summary><c>party</c> members: Dark Knights of level 11 to 41, at home in Lorencia.</summary>
    public static readonly TestCharacter[] PartyMembers =
    [
        new("test1", "test1", "test1Dk"),
        new("test2", "test2", "test2Dk"),
        new("test3", "test3", "test3Dk"),
        new("test4", "test4", "test4Dk"),
    ];
}
