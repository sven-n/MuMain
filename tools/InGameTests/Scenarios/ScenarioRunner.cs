using System.Diagnostics;
using System.Text.Json;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Running;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// Runs a scenario with fresh clients. Its screenshots go into
/// <paramref name="folder"/>; on a failure every client's state and recent
/// events go into the result, for the report.
/// </summary>
internal sealed class ScenarioRunner(
    ClientOptions clientOptions,
    string folder,
    TextWriter log,
    Action<int, string>? stepStarted,
    Action<StepResult>? stepFinished,
    Action<GameClient>? clientStarted = null)
{
    private const int RecentEventCount = 200;

    public async Task<ScenarioResult> RunAsync(Scenario scenario, CancellationToken cancellationToken)
    {
        log.WriteLine($"{scenario.Name}: {scenario.Description}");
        Directory.CreateDirectory(folder);
        var stopwatch = Stopwatch.StartNew();
        var clients = new Dictionary<string, GameClient>();
        var context = new ScenarioContext(clients, folder, log, clientOptions.ScreenshotQuality, stepStarted, stepFinished);
        try
        {
            foreach (var role in scenario.Roles)
            {
                log.WriteLine($"    starting client '{role}'");
                clients[role] = await GameClient.StartAsync(clientOptions, role, cancellationToken);
                clientStarted?.Invoke(clients[role]);
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
            var details = await this.FailureDetailsAsync(clients);
            return this.Result(scenario, false, stopwatch.Elapsed, exception.Message, context) with { FailureDetails = details };
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
            scenario.Category,
            passed ? ScenarioStatus.Passed : ScenarioStatus.Failed,
            duration,
            clientOptions.StepDelay,
            failure,
            [.. context.Steps],
            ScreenshotQuality: clientOptions.ScreenshotQuality);

    private async Task<IReadOnlyList<ClientDetails>> FailureDetailsAsync(IReadOnlyDictionary<string, GameClient> clients)
    {
        var details = new List<ClientDetails>();
        foreach (var (role, client) in clients)
        {
            var state = await this.TryReadAsync(async () => Pretty(await client.StateAsync()));
            var events = await this.TryReadAsync(async () =>
            {
                var last = await client.LastEventSequenceAsync();
                return Pretty(await client.EventsSinceAsync(Math.Max(0, last - RecentEventCount)));
            });
            details.Add(new ClientDetails(role, state, events));
        }

        return details;
    }

    private static string Pretty(JsonElement element)
        => JsonSerializer.Serialize(element, new JsonSerializerOptions { WriteIndented = true });

    // A client that is already gone must not hide the scenario's own failure.
    private async Task<string> TryReadAsync(Func<Task<string>> read)
    {
        try
        {
            return await read();
        }
        catch (Exception exception)
        {
            log.WriteLine($"    could not read failure details: {exception.Message}");
            return $"(could not be read: {exception.Message})";
        }
    }
}
