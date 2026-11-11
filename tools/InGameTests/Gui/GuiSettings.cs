using System.Text.Json;

namespace MuMain.Tools.InGameTests.Gui;

/// <summary>What the window remembers between runs, in the user's application data.</summary>
internal sealed class GuiSettings
{
    private static readonly string FilePath = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "MuMain", "in-game-tests.json");

    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };

    public string? ClientPath { get; set; }

    public string? Server { get; set; }

    public bool FreshServer { get; set; } = true;

    public int StepDelayMilliseconds { get; set; }

    public int ScreenshotQuality { get; set; } = Clients.ClientOptions.DefaultScreenshotQuality;

    public Dictionary<string, ScenarioSettings> Scenarios { get; set; } = [];

    /// <summary>The categories folded in in the list, by name.</summary>
    public List<string> CollapsedCategories { get; set; } = [];

    /// <summary>The saved settings, or the defaults when there are none or they cannot be read.</summary>
    public static GuiSettings Load()
    {
        try
        {
            return File.Exists(FilePath)
                ? JsonSerializer.Deserialize<GuiSettings>(File.ReadAllText(FilePath)) ?? new GuiSettings()
                : new GuiSettings();
        }
        catch (Exception exception) when (exception is IOException or JsonException or UnauthorizedAccessException)
        {
            return new GuiSettings();
        }
    }

    /// <summary>Saves the settings; a failure only costs remembering them.</summary>
    public void Save()
    {
        try
        {
            Directory.CreateDirectory(Path.GetDirectoryName(FilePath)!);
            File.WriteAllText(FilePath, JsonSerializer.Serialize(this, JsonOptions));
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            // Not remembered this time.
        }
    }

    /// <summary>A scenario's row: checked, and its own wait if it has one.</summary>
    public sealed class ScenarioSettings
    {
        public bool Checked { get; set; } = true;

        public string? Delay { get; set; }

        public string? Quality { get; set; }
    }
}
