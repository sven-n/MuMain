using System.Globalization;
using System.Net;
using System.Text;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// Compares the pictures of the look references in two reports, e.g. one run before a rendering
/// change and one after. For each scene it measures how much the first picture of one run
/// differs from the first of the other, and weighs that against the scene's own motion: how much
/// its two pictures differ within each run (grass, birds and fire move between them). The result
/// is one HTML file with both pictures and a heat map of the difference per scene.
/// </summary>
internal static class LookComparison
{
    public const string Scenario = "look-references";

    // A scene step pictures its scene twice; other steps (entering the world, changing the look) are left out.
    private const int ScenePictures = 2;

    // A difference counts as beyond the scene's motion above this many times that motion, plus a margin
    // for scenes that hardly move.
    private const double MotionTolerance = 1.25;
    private const double MeanMargin = 0.5;
    private const double ChangedPercentMargin = 0.5;

    /// <summary>One scene of the comparison.</summary>
    internal sealed record SceneComparison(
        string Title,
        byte[]? Before,
        byte[]? After,
        byte[]? HeatMap,
        PictureDifference? Change,
        PictureDifference? Motion,
        string Verdict);

    /// <summary>Writes the comparison of <paramref name="beforePath"/> and <paramref name="afterPath"/> into <paramref name="outputFolder"/>; returns its path.</summary>
    public static string Write(string beforePath, string afterPath, string outputFolder, TextWriter log)
    {
        var before = ReportPictures.Read(beforePath, Scenario);
        var after = ReportPictures.Read(afterPath, Scenario);
        var scenes = before.Steps
            .Where(step => step.Value.Count >= ScenePictures && after.Steps.ContainsKey(step.Key))
            .Select(step => CompareScene(step.Key, step.Value, after.Steps[step.Key]))
            .ToList();
        foreach (var scene in scenes)
        {
            log.WriteLine($"{scene.Verdict,-28} {scene.Title}");
        }

        Directory.CreateDirectory(outputFolder);
        var path = Path.GetFullPath(Path.Combine(outputFolder, $"look-comparison-{DateTime.Now:yyyyMMdd-HHmmss}.html"));
        File.WriteAllText(path, Render(before, after, scenes));
        return path;
    }

    private static SceneComparison CompareScene(string title, IReadOnlyDictionary<string, byte[]> before, IReadOnlyDictionary<string, byte[]> after)
    {
        var beforeFirst = before.Values.FirstOrDefault();
        var afterFirst = after.Values.FirstOrDefault();
        if (beforeFirst is null || afterFirst is null)
        {
            return new SceneComparison(title, beforeFirst, afterFirst, null, null, null, "picture missing");
        }

        var change = PictureDifference.Compare(beforeFirst, afterFirst);
        if (change is null)
        {
            return new SceneComparison(title, beforeFirst, afterFirst, null, null, null, "sizes differ");
        }

        var motion = Larger(OwnMotion(before), OwnMotion(after));
        return new SceneComparison(title, beforeFirst, afterFirst, change.Value.HeatMap, change.Value.Difference, motion, Verdict(change.Value.Difference, motion));
    }

    // How much a run's two pictures of the scene differ; null without a second picture.
    private static PictureDifference? OwnMotion(IReadOnlyDictionary<string, byte[]> pictures)
        => pictures.Count < 2 ? null : PictureDifference.Compare(pictures.Values.ElementAt(0), pictures.Values.ElementAt(1))?.Difference;

    private static PictureDifference? Larger(PictureDifference? a, PictureDifference? b)
        => a is null ? b : b is null ? a : new PictureDifference(Math.Max(a.Mean, b.Mean), Math.Max(a.ChangedPercent, b.ChangedPercent));

    private static string Verdict(PictureDifference change, PictureDifference? motion)
    {
        if (motion is null)
        {
            return "no motion measure";
        }

        var beyond = change.Mean > (motion.Mean * MotionTolerance) + MeanMargin
            || change.ChangedPercent > (motion.ChangedPercent * MotionTolerance) + ChangedPercentMargin;
        return beyond ? "beyond the scene's motion" : "within the scene's motion";
    }

    private static string Render(ReportPictures before, ReportPictures after, IReadOnlyList<SceneComparison> scenes)
    {
        var html = new StringBuilder();
        html.Append("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>Look comparison</title><style>")
            .Append(Style).Append("</style></head><body><h1>Look comparison</h1><p>Before: ").Append(Describe(before))
            .Append("<br>After: ").Append(Describe(after)).Append("</p><p class=\"muted\">")
            .Append(Encode("Per scene: the first picture of each run, and a heat map that is white where they differ. "
                + "\"Beyond the scene's motion\" means they differ more than the scene's two pictures within a run do; "
                + "it points at what to look at, it is no pass or fail: a passing monster or a server message counts too."))
            .Append("</p>");
        foreach (var scene in scenes)
        {
            RenderScene(html, scene);
        }

        return html.Append("</body></html>").ToString();
    }

    private static void RenderScene(StringBuilder html, SceneComparison scene)
    {
        html.Append("<section><h2>").Append(Encode(scene.Title)).Append("</h2><p><b>").Append(Encode(scene.Verdict)).Append("</b>");
        if (scene.Change is { } change)
        {
            html.Append(" · change ").Append(Encode(Numbers(change)));
        }

        if (scene.Motion is { } motion)
        {
            html.Append(" · own motion ").Append(Encode(Numbers(motion)));
        }

        html.Append("</p><div class=\"shots\">").Append(Figure(scene.Before, "before")).Append(Figure(scene.After, "after"))
            .Append(Figure(scene.HeatMap, "difference")).Append("</div></section>");
    }

    private static string Numbers(PictureDifference difference)
        => string.Create(CultureInfo.InvariantCulture, $"mean {difference.Mean:0.00}, {difference.ChangedPercent:0.00} % of pixels above {PictureDifference.ChangedThreshold}");

    private static string Describe(ReportPictures report)
    {
        var commit = report.ClientCommit is null ? "client commit unknown" : $"client {report.ClientCommit}{(report.ClientChanged ? " with changes" : string.Empty)}";
        return Encode($"{Path.GetFileName(report.Path)} ({commit})");
    }

    private static string Figure(byte[]? jpeg, string caption)
        => jpeg is null
            ? $"<figure><figcaption>{Encode(caption)}: none</figcaption></figure>"
            : $"<figure><img alt=\"{Encode(caption)}\" src=\"data:image/jpeg;base64,{Convert.ToBase64String(jpeg)}\"><figcaption>{Encode(caption)}</figcaption></figure>";

    private static string Encode(string text) => WebUtility.HtmlEncode(text);

    private const string Style = """
        body{font-family:system-ui,sans-serif;margin:16px;background:#fff;color:#1b1b1b}
        h1{font-size:1.4em}h2{font-size:1.1em;margin:0 0 4px}
        section{border-top:1px solid #ccc;padding:12px 0}
        .muted{color:#555}
        .shots{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:8px}
        figure{margin:0}img{width:100%;height:auto;display:block}figcaption{color:#555;font-size:.9em}
        @media (prefers-color-scheme:dark){body{background:#151515;color:#e8e8e8}.muted,figcaption{color:#aaa}section{border-color:#333}}
        """;
}
