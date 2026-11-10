using System.Net;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The pictures one scenario took in a report (an <c>in-game-report-*.html</c>), by step title
/// and caption, read back from the page: its results JSON names the steps, its figures hold
/// the pictures.
/// </summary>
internal sealed partial class ReportPictures
{
    private ReportPictures(string path, string? clientCommit, bool clientChanged, IReadOnlyDictionary<string, IReadOnlyDictionary<string, byte[]>> steps)
    {
        this.Path = path;
        this.ClientCommit = clientCommit;
        this.ClientChanged = clientChanged;
        this.Steps = steps;
    }

    public string Path { get; }

    /// <summary>The commit the run's client was built from, when it said.</summary>
    public string? ClientCommit { get; }

    /// <summary>Whether the client's files differed from that commit.</summary>
    public bool ClientChanged { get; }

    /// <summary>Step title, then picture caption, to the JPEG.</summary>
    public IReadOnlyDictionary<string, IReadOnlyDictionary<string, byte[]>> Steps { get; }

    /// <summary>Reads <paramref name="scenario"/>'s pictures; throws <see cref="InvalidDataException"/> when the file is no report of it.</summary>
    public static ReportPictures Read(string path, string scenario)
    {
        var html = File.ReadAllText(path);
        var results = ResultsJson().Match(html);
        if (!results.Success)
        {
            throw new InvalidDataException($"{path} is no in-game test report");
        }

        using var json = JsonDocument.Parse(results.Groups["json"].Value);
        var titles = StepTitles(json.RootElement, scenario)
            ?? throw new InvalidDataException($"{path} has no run of {scenario}");
        var pictures = Pictures(ScenarioSection(html, scenario));
        var steps = titles.ToDictionary(
            step => step.Value,
            step => (IReadOnlyDictionary<string, byte[]>)(pictures.GetValueOrDefault(step.Key) ?? []));
        var root = json.RootElement;
        return new ReportPictures(
            path,
            root.TryGetProperty("clientCommit", out var commit) ? commit.GetString() : null,
            root.TryGetProperty("clientChanged", out var changed) && changed.ValueKind == JsonValueKind.True,
            steps);
    }

    // Step number to title, or null when the report did not run the scenario.
    private static Dictionary<int, string>? StepTitles(JsonElement root, string scenario)
    {
        var run = root.GetProperty("scenarios").EnumerateArray()
            .FirstOrDefault(entry => entry.GetProperty("name").GetString() == scenario
                && entry.GetProperty("status").GetString() != "Skipped");
        if (run.ValueKind == JsonValueKind.Undefined)
        {
            return null;
        }

        return run.GetProperty("steps").EnumerateArray()
            .ToDictionary(step => step.GetProperty("number").GetInt32(), step => step.GetProperty("title").GetString()!);
    }

    // The scenario's part of the page, up to the next scenario.
    private static string ScenarioSection(string html, string scenario)
    {
        var start = html.IndexOf($"<details class=\"card scenario\" id=\"{WebUtility.HtmlEncode(scenario)}\">", StringComparison.Ordinal);
        if (start < 0)
        {
            return string.Empty;
        }

        var end = html.IndexOf("<details class=\"card scenario\"", start + 1, StringComparison.Ordinal);
        return end < 0 ? html[start..] : html[start..end];
    }

    // Step number to caption to JPEG; the report names each picture "<caption> after step <number>".
    private static Dictionary<int, Dictionary<string, byte[]>> Pictures(string section)
    {
        var pictures = new Dictionary<int, Dictionary<string, byte[]>>();
        foreach (Match figure in Figure().Matches(section))
        {
            var alt = WebUtility.HtmlDecode(figure.Groups["alt"].Value);
            var name = StepPicture().Match(alt);
            if (!name.Success)
            {
                continue;
            }

            var number = int.Parse(name.Groups["number"].Value, System.Globalization.CultureInfo.InvariantCulture);
            if (!pictures.TryGetValue(number, out var step))
            {
                pictures[number] = step = [];
            }

            step[name.Groups["caption"].Value] = Convert.FromBase64String(figure.Groups["data"].Value);
        }

        return pictures;
    }

    [GeneratedRegex("<script type=\"application/json\" id=\"results\">(?<json>.*?)</script>", RegexOptions.Singleline)]
    private static partial Regex ResultsJson();

    [GeneratedRegex("<img loading=\"lazy\" alt=\"(?<alt>[^\"]*)\" src=\"data:image/jpeg;base64,(?<data>[^\"]*)\">")]
    private static partial Regex Figure();

    [GeneratedRegex("^(?<caption>.*) after step (?<number>[0-9]+)$")]
    private static partial Regex StepPicture();
}
