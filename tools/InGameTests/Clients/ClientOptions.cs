namespace MuMain.Tools.InGameTests.Clients;

/// <summary>
/// How to start a client: an editor build of <c>Main</c> with the control socket
/// (<c>ENABLE_CONTROL_SOCKET=ON</c>) and the server it connects to.
/// </summary>
internal sealed record ClientOptions(string ExecutablePath, string ServerHost, int ServerPort)
{
    /// <summary>How long a client may take from start until its socket answers.</summary>
    public TimeSpan StartTimeout { get; init; } = TimeSpan.FromSeconds(120);

    /// <summary>
    /// A pause after every action a client takes (a click, a key, a walk, …), so a
    /// person watching can follow the scenario; zero runs at full speed.
    /// </summary>
    public TimeSpan StepDelay { get; init; } = TimeSpan.Zero;

    /// <summary>JPEG quality of the step screenshots, 1 to 100: lower makes the report smaller.</summary>
    public int ScreenshotQuality { get; init; } = DefaultScreenshotQuality;

    /// <summary>Small enough to embed a picture per step and client, still easy to read.</summary>
    public const int DefaultScreenshotQuality = 70;
}
