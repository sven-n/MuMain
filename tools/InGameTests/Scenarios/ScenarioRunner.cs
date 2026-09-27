using System.Diagnostics;
using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Running;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Runs a scenario with fresh clients. Its screenshots go into
/// <paramref name="folder"/>; on a failure the recent events and the state of
/// every client go there too.
/// </summary>
internal sealed class ScenarioRunner(
    ClientOptions clientOptions,
    string folder,
    TextWriter log,
    Action<int, string>? stepStarted,
    Action<StepResult>? stepFinished)
{
    private const int RecentEventCount = 200;

    public async Task<ScenarioResult> RunAsync(Scenario scenario, CancellationToken cancellationToken)
    {
        log.WriteLine($"{scenario.Name}: {scenario.Description}");
        Directory.CreateDirectory(folder);
        var stopwatch = Stopwatch.StartNew();
        var clients = new Dictionary<string, GameClient>();
        var context = new ScenarioContext(clients, folder, log, stepStarted, stepFinished);
        try
        {
            foreach (var role in scenario.Roles)
            {
                log.WriteLine($"    starting client '{role}'");
                clients[role] = await GameClient.StartAsync(clientOptions, role, cancellationToken);
            }

            await scenario.RunAsync(context);
            log.WriteLine($"PASS {scenario.Name} ({stopwatch.Elapsed.TotalSeconds:0} s)");
            if (context.Steps.Count != scenario.StepCount)
            {
                log.WriteLine($"    note: {scenario.Name} ran {context.Steps.Count} steps but its StepCount says {scenario.StepCount}");
            }

            return this.Result(scenario, true, stopwatch.Elapsed, null, context);
        }
        catch (Exception exception)
        {
            log.WriteLine($"FAIL {scenario.Name} ({stopwatch.Elapsed.TotalSeconds:0} s): {exception.Message}");
            await this.SaveFailureAsync(clients);
            return this.Result(scenario, false, stopwatch.Elapsed, exception.Message, context);
        }
        finally
        {
            foreach (var client in clients.Values)
            {
                await client.DisposeAsync();
            }
        }
    }

    private ScenarioResult Result(Scenario scenario, bool passed, TimeSpan duration, string? failure, ScenarioContext context)
        => new(
            scenario.Name,
            scenario.Description,
            passed ? ScenarioStatus.Passed : ScenarioStatus.Failed,
            duration,
            clientOptions.StepDelay,
            failure,
            [.. context.Steps]);

    private async Task SaveFailureAsync(IReadOnlyDictionary<string, GameClient> clients)
    {
        foreach (var (role, client) in clients)
        {
            await this.TrySaveAsync(async () =>
            {
                var last = await client.LastEventSequenceAsync();
                var events = await client.EventsSinceAsync(Math.Max(0, last - RecentEventCount));
                await File.WriteAllTextAsync(Path.Combine(folder, $"{role}-events.json"), Pretty(events));
            });
            await this.TrySaveAsync(async () =>
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
