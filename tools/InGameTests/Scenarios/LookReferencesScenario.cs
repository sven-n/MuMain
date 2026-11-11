namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// The look's reference scenes: a game master stands on fixed spots of fixed maps and each
/// spot is pictured in the Enhanced and the Classic look, twice a few seconds apart. Two runs
/// (e.g. before and after a rendering change) are compared with <c>--compare</c>; the two
/// pictures of one run show how much the scene changes by itself (grass, birds, fire), so a
/// difference between the runs is weighed against that.
/// </summary>
internal sealed class LookReferencesScenario : Scenario
{
    private const string Viewer = "viewer";
    private const string FirstPicture = "picture";
    private const string SecondPicture = "picture, a moment later";
    private const string EnhancedLook = "Enhanced";
    private const string ClassicLook = "Classic";

    // How long a scene takes to settle after a move: the map loads and its name fades out.
    private static readonly TimeSpan SceneSettle = TimeSpan.FromSeconds(8);
    private static readonly TimeSpan CharacterListSettle = TimeSpan.FromSeconds(5);
    private static readonly TimeSpan PictureInterval = TimeSpan.FromSeconds(2);
    // The overlapping effects are short-lived: picture them while they burn.
    private static readonly TimeSpan EffectSettle = TimeSpan.FromSeconds(0.5);
    private const int EffectCount = 3;
    private const string BrightEffect = "BITMAP_FIRE+1";
    private const string DarkEffect = "BITMAP_CRATER";
    // The cloud shadows drift with the clock; held still they fall alike in every run, for the session only.
    private const string StillClouds = "$clouds speed 0";

    private static readonly Scene[] Scenes =
    [
        new("Lorencia's town: roofs, walls and the road", 0, 130, 125),
        new("Noria: trees, leaves and grass", 3, 175, 110),
        new("Devias: snow, stairs and a wall", 2, 220, 40),
        new("Dungeon: an enclosed map with torches", 1, 108, 247),
        new("Kalima: its moving ground", 24, 100, 99),
        new("Atlans: the underwater environment", 7, 20, 20),
        new("Lorencia: fire next to a darkening effect", 0, 135, 128, WithEffects: true),
    ];

    public override string Name => "look-references";

    public override string Description =>
        "fixed scenes in the Enhanced and the Classic look, pictured to compare the rendering of two builds";

    public override ScenarioCategory Category => ScenarioCategory.LookReferences;

    public override IReadOnlyList<string> Roles => [Viewer];

    // The character list in each look, entering the world, the scenes in each look, and the switch between looks.
    public override int StepCount => 2 + 1 + (2 * Scenes.Length) + 1;

    // A fixed size and every Enhanced feature on, with no player tuning: the pictures show the shipped look.
    public override string? ClientConfig => """
        [Window]
        Width=1920
        Height=1080
        Windowed=1
        [Render]
        Look=Enhanced
        Antialiasing=4
        Anisotropy=8
        EnhancedLighting=1
        DynamicLights=1
        Sunlight=1
        LightMood=1
        PostProcessing=1
        """;

    public override async Task RunAsync(ScenarioContext context)
    {
        var viewer = context.Client(Viewer);
        var character = TestAccounts.LookViewer;

        await context.StepAsync(
            $"{EnhancedLook}: the character list",
            $"{character.Account} is logged in; the character list shows its characters in the {EnhancedLook} look.",
            async () =>
            {
                await viewer.LogInAsync(character.Account, character.Password);
                await Task.Delay(CharacterListSettle);
                await PictureTwiceAsync(context);
            },
            []);
        await context.StepAsync(
            $"{character.Name} enters the world, and the clouds stand still",
            $"{character.Name} stands in the world; $clouds speed 0 is accepted, so the cloud shadows lie in the same place in every run.",
            async () =>
            {
                await viewer.SelectCharacterAsync(character.Name);
                await viewer.DeveloperCommandAsync(StillClouds);
            });

        await PictureScenesAsync(context, EnhancedLook);
        await context.StepAsync(
            $"The look changes to {ClassicLook}",
            $"$look classic is accepted; the scene shows in the {ClassicLook} look.",
            () => viewer.DeveloperCommandAsync("$look classic"));
        await PictureScenesAsync(context, ClassicLook);

        await context.StepAsync(
            $"{ClassicLook}: the character list",
            $"Back on the character list, which shows the characters in the {ClassicLook} look.",
            async () =>
            {
                await viewer.LogOutAsync();
                await Task.Delay(CharacterListSettle);
                await PictureTwiceAsync(context);
            },
            []);
    }

    private static async Task PictureScenesAsync(ScenarioContext context, string look)
    {
        foreach (var scene in Scenes)
        {
            await context.StepAsync(
                $"{look}: {scene.Title}",
                $"{TestAccounts.LookViewer.Name} stands at ({scene.X},{scene.Y}) on map {scene.Map}; the scene shows in the {look} look.",
                () => PictureSceneAsync(context, scene),
                []);
        }
    }

    private static async Task PictureSceneAsync(ScenarioContext context, Scene scene)
    {
        var viewer = context.Client(Viewer);
        await viewer.TeleportAsync(scene.Map, scene.X, scene.Y);
        await Task.Delay(SceneSettle);
        var map = (await viewer.StateAsync()).GetProperty("map").GetInt32();
        Expect.That(map == scene.Map, $"the server put {TestAccounts.LookViewer.Name} on map {map}, not on map {scene.Map}");
        if (scene.WithEffects)
        {
            await SpawnAsync(viewer, BrightEffect);
            await SpawnAsync(viewer, DarkEffect);
            await Task.Delay(EffectSettle);
        }

        await PictureTwiceAsync(context);
    }

    private static async Task SpawnAsync(Clients.GameClient viewer, string type)
    {
        for (var index = 0; index < EffectCount; index++)
        {
            await viewer.SendAsync("spawn-effect", new { type });
        }
    }

    private static async Task PictureTwiceAsync(ScenarioContext context)
    {
        await context.PictureAsync(Viewer, FirstPicture);
        await Task.Delay(PictureInterval);
        await context.PictureAsync(Viewer, SecondPicture);
    }

    // A spot to picture: a map index and a tile on it.
    private sealed record Scene(string Title, int Map, int X, int Y, bool WithEffects = false);
}
