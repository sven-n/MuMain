namespace MuMain.Tools.InGameTests.Clients;

/// <summary>
/// How to start a client: an editor build of <c>Main</c> with the control socket
/// (<c>ENABLE_CONTROL_SOCKET=ON</c>) and the server it connects to.
/// </summary>
internal sealed record ClientOptions(string ExecutablePath, string ServerHost, int ServerPort)
{
    /// <summary>How long a client may take from start until its socket answers.</summary>
    public TimeSpan StartTimeout { get; init; } = TimeSpan.FromSeconds(120);
}
