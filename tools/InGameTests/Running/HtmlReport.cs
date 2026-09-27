using System.Net;
using System.Text;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The report of a run as one HTML file with the screenshots embedded, so it can
/// be attached to a pull request or an issue as it is.
/// </summary>
internal static class HtmlReport
{
    private const string Style = """
        :root { --bg:#f6f7f9; --card:#fff; --text:#1d2330; --muted:#5d6675; --line:#dde1e7;
                --pass:#1f7a3f; --pass-bg:#e3f4e8; --fail:#b3261e; --fail-bg:#fbe5e3; }
        @media (prefers-color-scheme: dark) {
          :root { --bg:#15181d; --card:#1e2229; --text:#e6e9ef; --muted:#9aa3b2; --line:#303641;
                  --pass:#6fd08f; --pass-bg:#1c3326; --fail:#ff8a80; --fail-bg:#3a1f1e; }
        }
        * { box-sizing: border-box; }
        body { margin:0; padding:24px 16px 48px; background:var(--bg); color:var(--text);
               font:15px/1.5 system-ui, -apple-system, "Segoe UI", sans-serif; }
        main { max-width:1200px; margin:0 auto; }
        h1 { font-size:24px; margin:0 0 4px; } h2 { font-size:20px; margin:0; }
        .muted { color:var(--muted); }
        .card { background:var(--card); border:1px solid var(--line); border-radius:10px; padding:16px 18px; margin:16px 0; }
        table { border-collapse:collapse; width:100%; }
        th, td { text-align:left; padding:6px 8px; border-bottom:1px solid var(--line); vertical-align:top; }
        .badge { display:inline-block; padding:1px 8px; border-radius:999px; font-size:13px; font-weight:600; }
        .pass { color:var(--pass); background:var(--pass-bg); } .fail { color:var(--fail); background:var(--fail-bg); }
        .scenario-head { display:flex; gap:12px; align-items:baseline; flex-wrap:wrap; }
        .step { border-top:1px solid var(--line); padding:14px 0 6px; }
        .step-title { display:flex; gap:10px; align-items:baseline; flex-wrap:wrap; font-weight:600; }
        .expect { margin:4px 0 0; } .failure { margin:6px 0 0; color:var(--fail); white-space:pre-wrap; }
        .shots { display:flex; gap:12px; flex-wrap:wrap; margin-top:10px; }
        figure { margin:0; flex:1 1 360px; max-width:560px; }
        figure img { width:100%; height:auto; border:1px solid var(--line); border-radius:6px; cursor:zoom-in; display:block; }
        figcaption { font-size:13px; color:var(--muted); margin-top:2px; }
        #zoom { position:fixed; inset:0; background:rgba(0,0,0,.85); display:none; align-items:center; justify-content:center; padding:16px; cursor:zoom-out; }
        #zoom img { max-width:100%; max-height:100%; }
        """;

    private const string Script = """
        const zoom = document.getElementById('zoom');
        document.querySelectorAll('figure img').forEach(img => img.addEventListener('click', () => {
          zoom.querySelector('img').src = img.src; zoom.style.display = 'flex'; }));
        zoom.addEventListener('click', () => { zoom.style.display = 'none'; });
        """;

    public static string Render(TestRunResult run)
    {
        var html = new StringBuilder();
        var passed = run.Scenarios.Count(scenario => scenario.Passed);
        var failed = run.Scenarios.Count - passed;

        html.Append("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">")
            .Append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
            .Append("<title>In-game test report</title><style>").Append(Style).Append("</style></head><body><main>");

        html.Append("<h1>In-game test report</h1><p class=\"muted\">")
            .Append(Encode($"{run.Started:yyyy-MM-dd HH:mm:ss} · server {run.Options.ServerHost}:{run.Options.ServerPort}"))
            .Append(run.Options.FreshServer ? Encode(" (fresh test data)") : string.Empty)
            .Append("<br>").Append(Encode($"client {run.Options.ClientPath}")).Append("</p>");

        html.Append("<section class=\"card\"><table><thead><tr><th>Scenario</th><th>Result</th><th>Steps</th><th>Duration</th><th>Pause per action</th></tr></thead><tbody>");
        foreach (var scenario in run.Scenarios)
        {
            html.Append("<tr><td><a href=\"#").Append(Encode(scenario.Name)).Append("\">").Append(Encode(scenario.Name)).Append("</a><br><span class=\"muted\">")
                .Append(Encode(scenario.Description)).Append("</span></td><td>").Append(Badge(scenario.Passed)).Append("</td><td>")
                .Append(scenario.Steps.Count).Append("</td><td>").Append(Seconds(scenario.Duration)).Append("</td><td>")
                .Append(scenario.StepDelay.TotalMilliseconds).Append(" ms</td></tr>");
        }

        html.Append("</tbody></table><p class=\"muted\">").Append(passed).Append(" passed, ").Append(failed).Append(" failed</p></section>");

        foreach (var scenario in run.Scenarios)
        {
            RenderScenario(html, scenario);
        }

        html.Append("</main><div id=\"zoom\"><img alt=\"\"></div><script>").Append(Script).Append("</script></body></html>");
        return html.ToString();
    }

    private static void RenderScenario(StringBuilder html, ScenarioResult scenario)
    {
        html.Append("<section class=\"card\" id=\"").Append(Encode(scenario.Name)).Append("\"><div class=\"scenario-head\"><h2>")
            .Append(Encode(scenario.Name)).Append("</h2>").Append(Badge(scenario.Passed)).Append("<span class=\"muted\">")
            .Append(Seconds(scenario.Duration)).Append("</span></div><p class=\"muted\">").Append(Encode(scenario.Description)).Append("</p>");

        // A failure before the first step (a client that did not start) has no step to show it.
        if (!scenario.Passed && scenario.Steps.All(step => step.Passed))
        {
            html.Append("<p class=\"failure\">").Append(Encode(scenario.Failure ?? "failed")).Append("</p>");
        }

        foreach (var step in scenario.Steps)
        {
            html.Append("<div class=\"step\"><div class=\"step-title\"><span>").Append(step.Number).Append(". ").Append(Encode(step.Title))
                .Append("</span>").Append(Badge(step.Passed)).Append("<span class=\"muted\">").Append(Seconds(step.Duration)).Append("</span></div>")
                .Append("<p class=\"expect\"><span class=\"muted\">Expected:</span> ").Append(Encode(step.Expectation)).Append("</p>");
            if (step.Failure is not null)
            {
                html.Append("<p class=\"failure\">").Append(Encode(step.Failure)).Append("</p>");
            }

            if (step.Screenshots.Count > 0)
            {
                html.Append("<div class=\"shots\">");
                foreach (var screenshot in step.Screenshots)
                {
                    html.Append("<figure><img loading=\"lazy\" alt=\"").Append(Encode($"{screenshot.Role} after step {step.Number}")).Append("\" src=\"")
                        .Append(ImageSource(screenshot.Path)).Append("\"><figcaption>").Append(Encode(screenshot.Role)).Append("</figcaption></figure>");
                }

                html.Append("</div>");
            }

            html.Append("</div>");
        }

        html.Append("</section>");
    }

    private static string Badge(bool passed) => passed ? "<span class=\"badge pass\">PASS</span>" : "<span class=\"badge fail\">FAIL</span>";

    private static string Seconds(TimeSpan duration) => $"{duration.TotalSeconds:0.0} s";

    private static string Encode(string text) => WebUtility.HtmlEncode(text);

    private static string ImageSource(string path)
        => File.Exists(path) ? "data:image/jpeg;base64," + Convert.ToBase64String(File.ReadAllBytes(path)) : string.Empty;
}
