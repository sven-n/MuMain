using System.Diagnostics;
using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Runs a scenario with fresh clients and reports it. On a failure it saves, per
/// client, a screenshot, the recent events and the state into the output folder.
/// </summary>
internal sealed class ScenarioRunner(ClientOptions clientOptions, string outputFolder, TextWriter log)
{
    private const int RecentEventCount = 200;

    public async Task<bool> RunAsync(Scenario scenario, CancellationToken cancellationToken)
    {
        log.WriteLine($"{scenario.Name}: {scenario.Description}");
        var stopwatch = Stopwatch.StartNew();
        var clients = new Dictionary<string, GameClient>();
        try
        {
            foreach (var role in scenario.Roles)
            {
                log.WriteLine($"    starting client '{role}'");
                clients[role] = await GameClient.StartAsync(clientOptions, role, cancellationToken);
            }

            await scenario.RunAsync(new ScenarioContext(clients, log));
            log.WriteLine($"PASS {scenario.Name} ({stopwatch.Elapsed.TotalSeconds:0} s)");
            return true;
        }
        catch (Exception exception)
        {
            log.WriteLine($"FAIL {scenario.Name} ({stopwatch.Elapsed.TotalSeconds:0} s): {exception.Message}");
            await this.SaveFailureAsync(scenario, clients);
            return false;
        }
        finally
        {
            foreach (var client in clients.Values)
            {
                await client.DisposeAsync();
            }
        }
    }

    private async Task SaveFailureAsync(Scenario scenario, IReadOnlyDictionary<string, GameClient> clients)
    {
        var folder = Path.Combine(outputFolder, $"{scenario.Name}-{DateTime.Now:yyyyMMdd-HHmmss}");
        Directory.CreateDirectory(folder);
        foreach (var (role, client) in clients)
        {
            await TrySaveAsync(() => client.ScreenshotAsync(Path.GetFullPath(Path.Combine(folder, $"{role}.png"))));
            await TrySaveAsync(async () =>
            {
                var last = await client.LastEventSequenceAsync();
                var events = await client.EventsSinceAsync(Math.Max(0, last - RecentEventCount));
                await File.WriteAllTextAsync(Path.Combine(folder, $"{role}-events.json"), Pretty(events));
            });
            await TrySaveAsync(async () =>
                await File.WriteAllTextAsync(Path.Combine(folder, $"{role}-state.json"), Pretty(await client.StateAsync())));
        }

        log.WriteLine($"    details in {folder}");
    }

    private static string Pretty(JsonElement element)
        => JsonSerializer.Serialize(element, new JsonSerializerOptions { WriteIndented = true });

    // A client that is already gone must not hide the scenario's own failure.
    private async Task TrySaveAsync(Func<Task> save)
    {
        try
        {
            await save();
        }
        catch (Exception exception)
        {
            log.WriteLine($"    could not save failure details: {exception.Message}");
        }
    }
}
