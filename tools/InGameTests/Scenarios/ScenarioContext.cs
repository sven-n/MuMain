using System.Diagnostics;
using MuMain.Tools.InGameTests.Clients;
using MuMain.Tools.InGameTests.Control;
using MuMain.Tools.InGameTests.Running;

namespace MuMain.Tools.InGameTests.Scenarios;

/// <summary>
/// The clients of a running scenario, and its steps. A scenario is written as a
/// row of steps; after each one every client takes a screenshot, and the report
/// shows the step with what should have happened and the pictures.
/// </summary>
internal sealed class ScenarioContext(
    IReadOnlyDictionary<string, GameClient> clients,
    string folder,
    TextWriter log,
    int screenshotQuality,
    Action<int, string>? stepStarted,
    Action<StepResult>? stepFinished)
{
    private readonly List<StepResult> steps = [];

    /// <summary>The steps so far, the last one possibly failed.</summary>
    public IReadOnlyList<StepResult> Steps => this.steps;

    public GameClient Client(string role) => clients[role];

    /// <summary>A line in the log that is no step, e.g. what else went wrong.</summary>
    public void Note(string text) => log.WriteLine($"        {text}");

    /// <summary>
    /// Runs one step. <paramref name="title"/> says what is done, <paramref name="expectation"/>
    /// what should happen then, for someone reading the report; <paramref name="action"/> does it
    /// and checks the outcome. A step that throws fails the scenario.
    /// </summary>
    public Task StepAsync(string title, string expectation, Func<Task> action)
        => this.StepAsync<object?>(
            title,
            expectation,
            async () =>
            {
                await action();
                return null;
            });

    /// <summary>As <see cref="StepAsync(string, string, Func{Task})"/>, for a step that finds something out.</summary>
    public async Task<T> StepAsync<T>(string title, string expectation, Func<Task<T>> action)
    {
        var number = this.steps.Count + 1;
        log.WriteLine($"    {number,2}. {title}");
        stepStarted?.Invoke(number, title);
        var stopwatch = Stopwatch.StartNew();
        try
        {
            var result = await action();
            var duration = stopwatch.Elapsed;
            this.Finish(new StepResult(number, title, expectation, true, duration, null, await this.CaptureAsync(number)));
            return result;
        }
        catch (Exception exception)
        {
            var duration = stopwatch.Elapsed;
            this.Finish(new StepResult(number, title, expectation, false, duration, exception.Message, await this.CaptureAsync(number)));
            throw;
        }
    }

    private void Finish(StepResult step)
    {
        this.steps.Add(step);
        stepFinished?.Invoke(step);
    }

    // Every client, so a trade shows both sides.
    private async Task<IReadOnlyList<StepScreenshot>> CaptureAsync(int number)
    {
        var screenshots = new List<StepScreenshot>();
        foreach (var (role, client) in clients)
        {
            var path = Path.GetFullPath(Path.Combine(folder, $"step-{number:00}-{role}.jpg"));
            try
            {
                await client.ScreenshotAsync(path, screenshotQuality);
                screenshots.Add(new StepScreenshot(role, path));
            }
            catch (Exception exception) when (exception is ControlException or TimeoutException or IOException)
            {
                log.WriteLine($"        no screenshot of '{role}': {exception.Message}");
            }
        }

        return screenshots;
    }
}
