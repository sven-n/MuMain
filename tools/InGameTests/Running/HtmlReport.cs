using System.Net;
using MuMain.Tools.InGameTests.Scenarios;
using System.Text;

namespace MuMain.Tools.InGameTests.Running;

/// <summary>
/// The report of a run as one HTML file with everything in it: the screenshots,
/// the state and events of the clients of a failed scenario, and the results as
/// JSON for tools. So it can be attached to a pull request or an issue as it is. It lists every scenario;
/// each one is a collapsed section that opens on a click, with its steps and
/// screenshots, and the ones that did not run are marked as skipped.
/// </summary>
internal static class HtmlReport
{
    private const string Style = """
        :root { --bg:#f6f7f9; --card:#fff; --text:#1d2330; --muted:#5d6675; --line:#dde1e7; --hover:#eef1f5;
                --pass:#1f7a3f; --pass-bg:#e3f4e8; --fail:#b3261e; --fail-bg:#fbe5e3; --skip:#5d6675; --skip-bg:#eceef1;
                --notice:#5c4400; --notice-bg:#fff4c2; --notice-line:#e0bb00; }
        @media (prefers-color-scheme: dark) {
          :root { --bg:#15181d; --card:#1e2229; --text:#e6e9ef; --muted:#9aa3b2; --line:#303641; --hover:#262b33;
                  --pass:#6fd08f; --pass-bg:#1c3326; --fail:#ff8a80; --fail-bg:#3a1f1e; --skip:#9aa3b2; --skip-bg:#2a2f37;
                  --notice:#ffe28a; --notice-bg:#3a3000; --notice-line:#8a7200; }
        }
        * { box-sizing: border-box; }
        body { margin:0; padding:24px 16px 48px; background:var(--bg); color:var(--text);
               font:15px/1.5 system-ui, -apple-system, "Segoe UI", sans-serif; }
        main { max-width:1200px; margin:0 auto; }
        h1 { font-size:24px; margin:0 0 4px; } h2 { font-size:18px; margin:0; }
        .category-title { font-size:14px; text-transform:uppercase; letter-spacing:.06em; color:var(--muted); font-weight:700; }
        tr.category th { padding-top:14px; cursor:pointer; user-select:none; }
        tr.category th .muted { font-weight:400; margin-left:6px; }
        table.overview { table-layout:fixed; }
        table.overview col.result { width:110px; } table.overview col.steps { width:70px; }
        table.overview col.duration { width:100px; } table.overview col.pause { width:150px; } table.overview col.quality { width:150px; }
        tr.category th::before { content:"▾ "; color:var(--muted); }
        tbody.collapsed tr.category th::before { content:"▸ "; }
        tbody.collapsed tr:not(.category) { display:none; }
        tr.category:hover th { background:var(--hover); }
        details.group { margin-top:24px; }
        details.group > summary { list-style:none; cursor:pointer; display:flex; gap:12px; align-items:baseline;
                                  flex-wrap:wrap; padding:6px 4px; border-radius:6px; }
        details.group > summary::-webkit-details-marker { display:none; }
        details.group > summary::before { content:"▸"; color:var(--muted); width:1em; }
        details.group[open] > summary::before { content:"▾"; }
        details.group > summary:hover { background:var(--hover); }
        .muted { color:var(--muted); }
        .verdict { display:flex; align-items:baseline; gap:18px; flex-wrap:wrap; color:#fff; border-radius:12px;
                   padding:18px 26px; margin:0 0 20px; }
        .verdict strong { font-size:40px; font-weight:800; letter-spacing:.04em; line-height:1.1; }
        .verdict span { font-size:18px; font-weight:500; opacity:.95; }
        .notice + .notice { margin-top:-12px; }
        .notice { background:var(--notice-bg); color:var(--notice); border:1px solid var(--notice-line); border-radius:10px;
                  padding:10px 18px; margin:-8px 0 20px; font-weight:600; }
        .verdict-pass { background:#1f7a3f; } .verdict-fail { background:#b3261e; } .verdict-none { background:#5d6675; }
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
        details.raw { margin-top:10px; } details.raw > summary { cursor:pointer; color:var(--muted); }
        details.raw pre { max-height:420px; overflow:auto; background:var(--bg); border:1px solid var(--line); border-radius:6px;
                          padding:10px; font-size:12px; }
        #zoom { position:fixed; inset:0; background:rgba(0,0,0,.85); display:none; align-items:center; justify-content:center; padding:16px; cursor:zoom-out; }
        #zoom img { max-width:100%; max-height:100%; }
        """;

    private const string Script = """
        const setAll = open => {
          document.querySelectorAll('details.group, details.scenario').forEach(d => d.open = open);
          document.querySelectorAll('tbody.group').forEach(t => t.classList.toggle('collapsed', !open));
        };
        document.getElementById('expand-all').addEventListener('click', () => setAll(true));
        document.getElementById('collapse-all').addEventListener('click', () => setAll(false));
        // A category row of the table folds its scenarios in and out.
        document.querySelectorAll('tr.category').forEach(row => row.addEventListener('click', () =>
          row.closest('tbody').classList.toggle('collapsed')));
        // A scenario linked from the table opens, with its category, before the page jumps to it.
        const openTarget = () => { const d = document.getElementById(decodeURIComponent(location.hash.slice(1)));
                                   if (d && d.tagName === 'DETAILS') {
                                     const group = d.closest('details.group'); if (group) group.open = true;
                                     d.open = true; d.scrollIntoView(); } };
        window.addEventListener('hashchange', openTarget); openTarget();
        const zoom = document.getElementById('zoom');
        document.querySelectorAll('figure img').forEach(img => img.addEventListener('click', () => {
          zoom.querySelector('img').src = img.src; zoom.style.display = 'flex'; }));
        zoom.addEventListener('click', () => { zoom.style.display = 'none'; });
        """;

    public static string Render(TestRunResult run, string resultsJson)
    {
        var html = new StringBuilder();
        html.Append("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">")
            .Append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">")
            .Append("<title>In-game test report</title><style>").Append(Style).Append("</style></head><body><main>");

        var (verdictClass, verdict, detail) = Verdict(run);
        html.Append("<div class=\"verdict verdict-").Append(verdictClass).Append("\"><strong>").Append(verdict).Append("</strong><span>")
            .Append(Encode(detail)).Append("</span></div>");

        // A client built with changes that were not committed is not exactly the commit it names.
        if (run.Client is { Changed: true } changedClient)
        {
            html.Append("<div class=\"notice\">The tested client was built from commit <code>").Append(Encode(Short(changedClient.Commit)))
                .Append("</code> with changes that were not committed, so it is not exactly that commit.</div>");
        }

        // A PASSED run that left scenarios out has not tested everything.
        var skipped = run.Scenarios.Where(scenario => scenario.Status == ScenarioStatus.Skipped).ToList();
        if (skipped.Count > 0)
        {
            html.Append("<div class=\"notice\">").Append(Encode(
                $"Not all scenarios were run: {skipped.Count} of {run.Scenarios.Count} skipped ({string.Join(", ", skipped.Select(scenario => scenario.Name))})."))
                .Append("</div>");
        }

        html.Append("<h1>In-game test report</h1><p class=\"muted\">")
            .Append(Encode($"{run.Started:yyyy-MM-dd HH:mm:ss} · server {run.Options.ServerHost}:{run.Options.ServerPort}"))
            .Append(run.Options.FreshServer ? Encode(" (fresh test data)") : string.Empty)
            .Append("<br>").Append(Encode($"client {run.Options.ClientPath}"))
            .Append("<br>").Append(ClientLine(run.Client))
            .Append("<br>").Append(ServerLine(run.Server)).Append("</p>");

        html.Append("<section class=\"card\"><table class=\"overview\"><colgroup><col><col class=\"result\"><col class=\"steps\"><col class=\"duration\"><col class=\"pause\"><col class=\"quality\"></colgroup><thead><tr><th>Scenario</th><th>Result</th><th>Steps</th><th>Duration</th><th>Pause per action</th><th>Screenshot quality</th></tr></thead>");
        foreach (var group in Categories(run))
        {
            html.Append("<tbody class=\"group\"><tr class=\"category\"><th colspan=\"6\"><span class=\"category-title\">")
                .Append(Encode(group.Key.DisplayName())).Append("</span> <span class=\"muted\">").Append(Encode(CategorySummary(group)))
                .Append("</span></th></tr>");
            foreach (var scenario in group)
            {
                var ran = scenario.Status != ScenarioStatus.Skipped;
                html.Append("<tr><td><a href=\"#").Append(Encode(scenario.Name)).Append("\">").Append(Encode(scenario.Name)).Append("</a><br><span class=\"muted\">")
                    .Append(Encode(scenario.Description)).Append("</span></td><td>").Append(Badge(scenario.Status)).Append("</td><td>")
                    .Append(ran ? scenario.Steps.Count.ToString() : "–").Append("</td><td>")
                    .Append(ran ? Seconds(scenario.Duration) : "–").Append("</td><td>")
                    .Append(ran ? $"{scenario.StepDelay.TotalMilliseconds} ms" : "–").Append("</td><td>")
                    .Append(ran ? $"{scenario.ScreenshotQuality} %" : "–").Append("</td></tr>");
            }

            html.Append("</tbody>");
        }

        html.Append("</table><p class=\"muted\">")
            .Append(run.Count(ScenarioStatus.Passed)).Append(" passed, ")
            .Append(run.Count(ScenarioStatus.Failed)).Append(" failed, ")
            .Append(run.Count(ScenarioStatus.Skipped)).Append(" skipped</p></section>");

        html.Append("<div class=\"toolbar\"><button id=\"expand-all\" type=\"button\">Expand all</button>")
            .Append("<button id=\"collapse-all\" type=\"button\">Collapse all</button>")
            .Append("<span class=\"muted\">Click a category or a scenario to open or close it.</span></div>");

        // Categories start open, scenarios closed: the page shows what ran and how it went.
        foreach (var group in Categories(run))
        {
            html.Append("<details class=\"group\" open><summary><span class=\"category-title\">").Append(Encode(group.Key.DisplayName()))
                .Append("</span><span class=\"muted\">").Append(Encode(CategorySummary(group))).Append("</span></summary>");
            foreach (var scenario in group)
            {
                RenderScenario(html, scenario);
            }

            html.Append("</details>");
        }

        // For tools: the same results as data. "</" would end the script element early.
        html.Append("<script type=\"application/json\" id=\"results\">").Append(resultsJson.Replace("</", "<\\/", StringComparison.Ordinal))
            .Append("</script>");
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

        if (scenario.FailureDetails is { Count: > 0 } details)
        {
            foreach (var client in details)
            {
                html.Append("<details class=\"raw\"><summary>State of '").Append(Encode(client.Role)).Append("' when it failed</summary><pre>")
                    .Append(Encode(client.State)).Append("</pre></details>")
                    .Append("<details class=\"raw\"><summary>Recent events of '").Append(Encode(client.Role)).Append("'</summary><pre>")
                    .Append(Encode(client.Events)).Append("</pre></details>");
            }
        }

        html.Append("</div></details>");
    }

    // Which client was tested: the git commit it was built from.
    private static string ClientLine(ClientVersion? client)
        => client is null
            ? "tested client: commit unknown (the client did not say)"
            : "tested client: commit <code title=\"" + Encode(client.Commit) + "\">" + Encode(Short(client.Commit)) + "</code>"
              + (client.Changed ? Encode(" (built with changes that were not committed)") : string.Empty);

    // What served the run: the test server's version and commit, linked to its source.
    private static string ServerLine(ServerVersion? server)
    {
        if (server is null)
        {
            return Encode("server: unknown (not the test server)");
        }

        var commit = server.Commit is null
            ? "commit unknown"
            : server.Source is { } source
                ? "commit <a href=\"" + Encode($"{source}/commit/{server.Commit}") + "\"><code>" + Encode(Short(server.Commit)) + "</code></a>"
                : "commit <code>" + Encode(Short(server.Commit)) + "</code>";
        return Encode($"server: {server.Name} {server.Version ?? "version unknown"}, ") + commit;
    }

    private static string Short(string commit) => commit.Length > 10 ? commit[..10] : commit;

    // The run in one word, for the banner on top: FAILED when a scenario failed,
    // STOPPED when the run did not get to all it was to run, NOTHING RAN, or
    // PASSED. Scenarios that were not selected do not count.
    private static (string Class, string Word, string Detail) Verdict(TestRunResult run)
    {
        var ran = run.Scenarios.Where(scenario => scenario.Status != ScenarioStatus.Skipped).ToList();
        var failed = ran.Count(scenario => scenario.Failed);
        var notRun = run.Options.Scenarios.Count - ran.Count;
        var steps = ran.Sum(scenario => scenario.Steps.Count);
        if (failed > 0)
        {
            return ("fail", "FAILED", $"{failed} of {Tests(ran.Count)} failed");
        }

        if (notRun > 0)
        {
            return ("none", "STOPPED", $"{ran.Count} of {Tests(run.Options.Scenarios.Count)} ran and passed; the run was stopped");
        }

        return ran.Count == 0
            ? ("none", "NOTHING RAN", "no test was selected")
            : ("pass", "PASSED", $"{ran.Count} of {Tests(ran.Count)} · {steps} steps");

        static string Tests(int count) => count == 1 ? "1 test" : $"{count} tests";
    }

    // The scenarios by category, in the categories' order.
    private static IEnumerable<IGrouping<ScenarioCategory, ScenarioResult>> Categories(TestRunResult run)
        => run.Scenarios.GroupBy(scenario => scenario.Category).OrderBy(group => group.Key);

    // "2 scenarios · 1 passed · 1 skipped": next to a category's name.
    private static string CategorySummary(IGrouping<ScenarioCategory, ScenarioResult> group)
    {
        var parts = new List<string> { group.Count() == 1 ? "1 scenario" : $"{group.Count()} scenarios" };
        foreach (var (status, word) in new[] { (ScenarioStatus.Passed, "passed"), (ScenarioStatus.Failed, "failed"), (ScenarioStatus.Skipped, "skipped") })
        {
            var count = group.Count(scenario => scenario.Status == status);
            if (count > 0)
            {
                parts.Add($"{count} {word}");
            }
        }

        return string.Join(" · ", parts);
    }

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
