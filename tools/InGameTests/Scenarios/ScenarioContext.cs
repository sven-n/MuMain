using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>The clients of a running scenario and a log for its steps.</summary>
internal sealed class ScenarioContext(IReadOnlyDictionary<string, GameClient> clients, TextWriter log)
{
    public GameClient Client(string role) => clients[role];

    public void Log(string step) => log.WriteLine($"    {step}");
}
