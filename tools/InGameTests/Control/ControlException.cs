namespace MuMain.Tools.InGameTests.Control;

/// <summary>
/// The client answered a control request with <c>{"ok":false,...}</c>.
/// </summary>
internal sealed class ControlException(string command, string error, string message)
    : Exception($"`{command}` failed: {error}: {message}")
{
    /// <summary>The command that failed.</summary>
    public string Command { get; } = command;

    /// <summary>The protocol's error code, e.g. <c>not_open</c> or <c>timeout</c>.</summary>
    public string Error { get; } = error;
}
