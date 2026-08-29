# Client Benchmark

The client can measure itself. `$bench` runs a fixed list of scenarios, records every frame,
and writes a report you can put next to another one — so a graphics change is judged against a
measurement instead of an impression.

It is a developer tool: available in every build, but it reconfigures the scene while it runs
and writes files next to the client, so it is not something a player would leave on.

For the per-frame overlays this builds on, see the `$details` and `$glstats` commands.

---

## 1. Running one

Stand where you want to measure, then type in chat:

```
$bench label glp09-ring-allocator
$bench run
```

The client freezes nothing and takes over nothing — it plays on. A line at the top of the
screen shows which segment is running and how far along it is. When the run finishes, the chat
prints the directory it was written to.

| Command | What it does |
|---|---|
| `$bench list` | Prints the segment catalog and the tags you can select by. |
| `$bench run` | Every segment, 3 repeats of 300 frames each. |
| `$bench run fx.*` | Only the segments whose name matches the pattern. |
| `$bench run #particles` | Only the segments carrying that tag. |
| `$bench run fx.* 5 600` | 5 repeats of 600 frames each. |
| `$bench quick fx.particles.off` | One repeat of 120 frames — for iterating, not for reporting. |
| `$bench label <text>` | Names the next run. It ends up in the directory name and the report. |
| `$bench stop` | Ends the run early. What was measured is still written out, marked aborted. |

A full run is a couple of minutes. `$bench quick` is seconds and is deliberately too short to
trust a small difference — use it to see whether a change did anything at all, then re-measure
properly.

### Getting numbers worth comparing

- **Turn vsync off** (`$vsync off`). With it on, every frame time is pinned to the display
  refresh, so the segments come out looking alike no matter what they switch off. The run says
  so in chat as it starts, and the report flags it again — but neither gets the run back.
- **Stand in the same place**, facing the same way, for both runs. The segments hold the
  effect configuration steady; they cannot hold the world steady.
- **Don't alt-tab.** A frame longer than half a second discards its whole repeat — the client
  was not running normally, so what it measured was the interruption.
- **Compare like with like.** Two runs are comparable when their manifest hash, GPU, resolution
  and build configuration match. All four are in the report header.

## 2. What a segment is

A segment is one named scenario, measured on its own. Splitting a run into segments is what
lets the result say *where* a change had an effect, rather than only whether the frame got
faster: a change that helps particle-heavy scenes and does nothing elsewhere reads very
differently from one that shifts every segment by the same amount.

The current catalog varies the effect surfaces of whatever scene you are standing in. Each
segment switches one surface off, so the difference between it and the baseline is what that
surface costs at that spot.

| Segment | What it isolates |
|---|---|
| `scene.full` | Nothing disabled. The reference every other row is read against. |
| `fx.all.off` | Every effect surface at once, exactly as `$effects off` does it — the upper bound. |
| `fx.sprites.off` | The sprite draw path. |
| `fx.particles.off` | The particle draw path. |
| `fx.joints.off` | Beam and tail-trail effects. |
| `fx.skillmodels.off` | Skill effect models, leaving impact effects rendering. |
| `fx.boids.off` | Ambient wildlife — an always-on cost in towns. |
| `fx.wingshadow.off` | The extra per-wing body shadow draw. |
| `fx.winglayers.off` | Wing glow overlay passes. Visibly changes the wing while it runs. |

`scene.full` is always measured first and again last, whatever you selected. The difference
between those two is drift: the machine getting hotter, or busier, under the run. If it is
large, nothing measured in between is trustworthy, and the report says so.

Each repeat discards a warmup window before recording. The first frames of a segment pay for
shader compiles, texture uploads and buffer growth — real hitching, but not the steady state a
comparison is about.

## 3. What you get

Each run writes one directory under `bench/runs/`, named after the timestamp and your label.

| File | What it is for |
|---|---|
| `run.json` | The canonical result. Everything else is a view of this. Versioned, so an old run stays readable. |
| `report.md` | The human view: environment, per-segment summary, pacing, findings, pass breakdown. |
| `frames.csv` | One row per measured frame — frame time and per-pass CPU/GPU ms. For spreadsheets and plots. |
| `passes.csv` | Per segment, repeat and pass: timings plus every GL counter, averaged per frame. |

`report.md` opens with what makes the run comparable: the build configuration, the GPU and
driver, the GL context version the client actually got, the capability flags the driver
reported, the resolution, and whether vsync was on. Those decide which code path the client is
even running, and a frame time without them next to it is not something anyone can act on
later.

### Reading the summary

Alongside the median it reports the **1% low** (the mean of the worst 1% of frames) and the
**pacing** numbers. A change can lower the average frame time and make the stream stutter more,
and an average on its own will call that a win. If the mean delta between consecutive frames
climbs while the median falls, that is what happened.

**Repeat spread** is the noise band. It is the gap between the repeat medians of the same
segment on the same machine measuring the same thing. A difference between two runs that is
smaller than this is not a result.

### Findings

The report ends with a table of rule-based observations — each one a named rule with a fixed
threshold, so you can see what was actually tested. They describe the run; they do not judge a
code change. The rules cover things like a segment whose frame time is mostly outside every
profiled pass (work running unmeasured), a batching path merging nothing, a buffer ring
wrapping repeatedly, drift across the run, and repeats that had to be discarded.

One of them is worth knowing before you read a table: **`segment-inert`** fires when a segment
submitted the same number of draw calls as the baseline. That segment disabled nothing here —
either the scene had none of what it targets (no wings equipped, no particles on screen) or its
configuration does not reach the path it names. Its timing is then not evidence that what it
targets is cheap, and the report says so rather than leaving the row to be misread.

## 4. Recording which commit a run came from

The report has a commit field, and the build system does not fill it in. To get it, define it
when you build:

```
-DMU_GIT_DESCRIBE="\"$(git describe --always --dirty)\"" -DMU_GIT_DIRTY
```

Without it the field reads `(unknown)`, which is honest — better than a commit that was stale
by the time the binary was built. Pass `MU_GIT_DIRTY` only when the tree really was dirty; the
report warns about it, because a run of uncommitted code cannot be reproduced from the commit
alone.
