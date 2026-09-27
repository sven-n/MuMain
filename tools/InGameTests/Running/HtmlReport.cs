using System.Net;
using MuMain.Tools.InGameTests.Scenarios;
using System.Text;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The report of a run as one HTML file with the screenshots embedded, so it can
/// be attached to a pull request or an issue as it is. It lists every scenario;
/// each one is a collapsed section that opens on a click, with its steps and
/// screenshots, and the ones that did not run are marked as skipped.
/// </summary>
internal static class HtmlReport
{
    private const string Style = """
        :root { --bg:#f6f7f9; --card:#fff; --text:#1d2330; --muted:#5d6675; --line:#dde1e7; --hover:#eef1f5;
                --pass:#1f7a3f; --pass-bg:#e3f4e8; --fail:#b3261e; --fail-bg:#fbe5e3; --skip:#5d6675; --skip-bg:#eceef1; }
        @media (prefers-color-scheme: dark) {
          :root { --bg:#15181d; --card:#1e2229; --text:#e6e9ef; --muted:#9aa3b2; --line:#303641; --hover:#262b33;
                  --pass:#6fd08f; --pass-bg:#1c3326; --fail:#ff8a80; --fail-bg:#3a1f1e; --skip:#9aa3b2; --skip-bg:#2a2f37; }
        }
        * { box-sizing: border-box; }
        body { margin:0; padding:24px 16px 48px; background:var(--bg); color:var(--text);
               font:15px/1.5 system-ui, -apple-system, "Segoe UI", sans-serif; }
        main { max-width:1200px; margin:0 auto; }
        h1 { font-size:24px; margin:0 0 4px; } h2 { font-size:18px; margin:0; }
        h3.category { font-size:15px; text-transform:uppercase; letter-spacing:.06em; color:var(--muted); margin:28px 0 -4px; }
        tr.category th { font-size:13px; text-transform:uppercase; letter-spacing:.06em; color:var(--muted); padding-top:14px; }
        .muted { color:var(--muted); }
        .card { background:var(--card); border:1px solid var(--line); border-radius:10px; padding:16px 18px; margin:16px 0; }
        table { border-collapse:collapse; width:100%; }
        th, td { text-align:left; padding:6px 8px; border-bottom:1px solid var(--line); vertical-align:top; }
        a { color:inherit; }
        .badge { display:inline-block; padding:1px 8px; border-radius:999px; font-size:13px; font-weight:600; white-space:nowrap; }
        .pass { color:var(--pass); background:var(--pass-bg); } .fail { color:var(--fail); background:var(--fail-bg); }
        .skip { color:var(--skip); background:var(--skip-bg); }
        .toolbar { display:flex; gap:8px; align-items:center; margin:24px 0 0; flex-wrap:wrap; }
        button { font:inherit; font-size:14px; padding:5px 14px; border-radius:6px; border:1px solid var(--line);
                 background:var(--card); color:var(--text); cursor:pointer; }
        button:hover { background:var(--hover); }
        details.scenario { padding:0; }
        details.scenario > summary { list-style:none; cursor:pointer; padding:14px 18px; display:flex; gap:12px;
                                     align-items:baseline; flex-wrap:wrap; border-radius:10px; }
        details.scenario > summary::-webkit-details-marker { display:none; }
        details.scenario > summary::before { content:"▸"; color:var(--muted); width:1em; }
        details.scenario[open] > summary::before { content:"▾"; }
        details.scenario > summary:hover { background:var(--hover); }
        .scenario-body { padding:0 18px 16px; }
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
        const sections = () => document.querySelectorAll('details.scenario');
        document.getElementById('expand-all').addEventListener('click', () => sections().forEach(d => d.open = true));
        document.getElementById('collapse-all').addEventListener('click', () => sections().forEach(d => d.open = false));
        // A scenario linked from the table opens before the page jumps to it.
        const openTarget = () => { const d = document.getElementById(decodeURIComponent(location.hash.slice(1)));
                                   if (d && d.tagName === 'DETAILS') { d.open = true; d.scrollIntoView(); } };
        window.addEventListener('hashchange', openTarget); openTarget();
        const zoom = document.getElementById('zoom');
        document.querySelectorAll('figure img').forEach(img => img.addEventListener('click', () => {
          zoom.querySelector('img').src = img.src; zoom.style.display = 'flex'; }));
        zoom.addEventListener('click', () => { zoom.style.display = 'none'; });
        """;

    public static string Render(TestRunResult run)
    {
        var html = new StringBuilder();
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
            if (IsFirstOfCategory(run, scenario))
            {
                html.Append("<tr class=\"category\"><th colspan=\"5\">").Append(Encode(scenario.Category.DisplayName())).Append("</th></tr>");
            }

            var ran = scenario.Status != ScenarioStatus.Skipped;
            html.Append("<tr><td><a href=\"#").Append(Encode(scenario.Name)).Append("\">").Append(Encode(scenario.Name)).Append("</a><br><span class=\"muted\">")
                .Append(Encode(scenario.Description)).Append("</span></td><td>").Append(Badge(scenario.Status)).Append("</td><td>")
                .Append(ran ? scenario.Steps.Count.ToString() : "–").Append("</td><td>")
                .Append(ran ? Seconds(scenario.Duration) : "–").Append("</td><td>")
                .Append(ran ? $"{scenario.StepDelay.TotalMilliseconds} ms" : "–").Append("</td></tr>");
        }

        html.Append("</tbody></table><p class=\"muted\">")
            .Append(run.Count(ScenarioStatus.Passed)).Append(" passed, ")
            .Append(run.Count(ScenarioStatus.Failed)).Append(" failed, ")
            .Append(run.Count(ScenarioStatus.Skipped)).Append(" skipped</p></section>");

        html.Append("<div class=\"toolbar\"><button id=\"expand-all\" type=\"button\">Expand all</button>")
            .Append("<button id=\"collapse-all\" type=\"button\">Collapse all</button>")
            .Append("<span class=\"muted\">Click a scenario to see its steps and screenshots.</span></div>");

        foreach (var scenario in run.Scenarios)
        {
            if (IsFirstOfCategory(run, scenario))
            {
                html.Append("<h3 class=\"category\">").Append(Encode(scenario.Category.DisplayName())).Append("</h3>");
            }

            RenderScenario(html, scenario);
        }

        html.Append("</main><div id=\"zoom\"><img alt=\"\"></div><script>").Append(Script).Append("</script></body></html>");
        return html.ToString();
    }

    private static void RenderScenario(StringBuilder html, ScenarioResult scenario)
    {
        html.Append("<details class=\"card scenario\" id=\"").Append(Encode(scenario.Name)).Append("\"><summary><h2>")
            .Append(Encode(scenario.Name)).Append("</h2>").Append(Badge(scenario.Status)).Append("<span class=\"muted\">")
            .Append(Encode(Summary(scenario))).Append("</span></summary><div class=\"scenario-body\"><p class=\"muted\">")
            .Append(Encode(scenario.Description)).Append("</p>");

        if (scenario.Status == ScenarioStatus.Skipped)
        {
            html.Append("<p>Not run: ").Append(Encode(scenario.SkipReason ?? "skipped")).Append(".</p></div></details>");
            return;
        }

        // A failure outside a step (a client that did not start, a check between two steps) has no step to show it.
        if (scenario.Failed && scenario.Steps.All(step => step.Passed))
        {
            html.Append("<p class=\"failure\">").Append(Encode(scenario.Failure ?? "failed")).Append("</p>");
        }

        foreach (var step in scenario.Steps)
        {
            html.Append("<div class=\"step\"><div class=\"step-title\"><span>").Append(step.Number).Append(". ").Append(Encode(step.Title))
                .Append("</span>").Append(Badge(step.Passed ? ScenarioStatus.Passed : ScenarioStatus.Failed)).Append("<span class=\"muted\">")
                .Append(Seconds(step.Duration)).Append("</span></div>")
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

        html.Append("</div></details>");
    }

    // The scenarios come grouped by category (TestRun orders them so); a
    // category's heading goes before its first one.
    private static bool IsFirstOfCategory(TestRunResult run, ScenarioResult scenario)
        => run.Scenarios.First(other => other.Category == scenario.Category) == scenario;

    // The line next to the name while the section is collapsed.
    private static string Summary(ScenarioResult scenario) => scenario.Status switch
    {
        ScenarioStatus.Skipped => scenario.SkipReason ?? "skipped",
        ScenarioStatus.Failed when scenario.Steps.FirstOrDefault(step => !step.Passed) is { } failed
            => $"failed at step {failed.Number} of {scenario.Steps.Count} · {Seconds(scenario.Duration)}",
        ScenarioStatus.Failed when scenario.Steps.Count > 0 => $"failed after step {scenario.Steps.Count} · {Seconds(scenario.Duration)}",
        ScenarioStatus.Failed => $"failed before step 1 · {Seconds(scenario.Duration)}",
        _ => $"{scenario.Steps.Count} steps · {Seconds(scenario.Duration)}",
    };

    private static string Badge(ScenarioStatus status) => status switch
    {
        ScenarioStatus.Passed => "<span class=\"badge pass\">PASS</span>",
        ScenarioStatus.Failed => "<span class=\"badge fail\">FAIL</span>",
        _ => "<span class=\"badge skip\">SKIPPED</span>",
    };

    private static string Seconds(TimeSpan duration) => $"{duration.TotalSeconds:0.0} s";

    private static string Encode(string text) => WebUtility.HtmlEncode(text);

    private static string ImageSource(string path)
        => File.Exists(path) ? "data:image/jpeg;base64," + Convert.ToBase64String(File.ReadAllBytes(path)) : string.Empty;
}
