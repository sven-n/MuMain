# Coding Rules

These rules apply to all code changes in this project. Read before you commit.

The rules apply to both the C++ client and the C# `ClientLibrary`. Where the
right idiom differs between the two languages, the rule below calls out each
language explicitly.

---

## Design Principle: Data-Driven by Design — Data and Editor

Content and settings that a designer, admin or modder would want to change are **data, not code**. Rule 3 gives a value a name; this principle moves it out of the code. It comes before the numbered rules because it decides how a feature is designed, not only how its code is written.

### New features

A new feature with content or settings ships as **two parts**:

1. **Data** — its content and settings live in JSON files under `src/bin/Data/<Area>/`, loaded once at startup into in-memory tables. The code holds the rules for using the data, not the values or the lists.
2. **Editor** — a MuEditor tool window that shows, changes and saves that data, with Undo/Redo, and reports its problems in **Data Checks**.

- The editor has **no rules of its own**: it changes the data only through the area's edit operations (`ItemEdits` for items), which check every change.
- The data, its loader and its docs may land first; the editor may follow in its own PR. The feature is **not done until both are merged**.
- The editor exists only in editor builds (`_EDITOR`). Player builds load the data and nothing else.

### Areas that are already data-driven

Items, monsters and their models, effects and looks, the server list, tooltips, ground labels, ... — and any part of an area that has already moved to data:

- **New or changed behaviour is a field in the data, not a check in code.** Don't add `if (type == ITEM_...)` or another `case` for one monster number; add a field, a tag or a rule to the data.
- **A field works for every entry of its kind.** An option one sword needed is available to every item; a setting one monster needed is available to every monster. No field that only one hard-coded entry reads.
- **A new field is complete when it is everywhere the data is:** loading and saving, the editor tool, Data Checks, the area's docs (`docs/<area>-data.md`), and — where the server has the same value — the exchange with OpenMU, under OpenMU's name.
- **Special cases you touch move into data.** When your change affects a hard-coded case for one entry, turn it into data as part of the change. Cases you don't touch stay until someone works on them (no retroactive cleanups, see [`AGENTS.md`](../AGENTS.md)).
- **Moving game behaviour into data must not change it.** A test that compares the old code with the new data for every entry ships with the move; changes you mean to make go in commits of their own.

### How the data looks

Follow the existing data files ([`item-data.md`](item-data.md), [`monster-data.md`](monster-data.md)):

- JSON, UTF-8, LF, with a `formatVersion`. Fields with their default value are left out; entries are sorted and fields come in a fixed order, so saving unchanged data gives the same file (test it).
- Texts players see are per language (`{"en": ..., "de": ...}`), as item and monster names are.
- Anything several entries use is **defined once, with a name**, and referenced by that name (glow colors, looks, effect types). Names are resolved when loading, never per frame (rule 12).
- Unknown names and invalid values are reported when loading and in Data Checks, never ignored silently.
- Logic becomes data where someone would want to adjust it: tables, rules and formulas with parameters (how stats grow with item level, which items a rule applies to). Logic without a meaningful knob stays code.

### What stays code

Engine and platform internals, the rendering pipeline, network protocol decoding, build tooling, and fixes that need no new special case.

---

## 1. Keep Functions Short and Focused — Break Apart Monoliths

- A function does **one thing**. If you can describe it with "and", split it.
- When touching a large function, **extract at least one** logical block into its own function before you're done. You don't have to refactor everything — just leave it better than you found it.
- If the extracted code doesn't belong in the current file, **move it to a new file** in an appropriate folder. Over time this breaks apart monolithic files naturally.
- Aim for functions that fit on one screen (~40 lines). Logic-heavy functions get shorter limits.

## 2. Exit Early

- Handle error cases and edge conditions **at the top** of a function.
- Avoid deep nesting. Prefer `if (bad) return;` over wrapping the entire body in `if (good) { ... }`.
- Each early exit should have a clear reason — don't silently swallow errors.

## 3. No Hard-Coded Values

- Numbers, strings, paths, sizes, timers, colors — if it's not self-evident, **give it a name**.
- Use constants, enums, or config values. A magic number buried in logic is a bug waiting to happen.
- `0` and `1` in boolean/trivial contexts are fine. `3.14f`, `1000`, `0xFF00FF` are not.

## 4. Write Reusable Code

- Before writing new logic, check if something similar already exists. Don't duplicate.
- Extract shared logic into utility functions or helpers when two or more callers need it.
- Design parameters to be general where it costs nothing, but don't over-abstract for hypothetical future use.

## 5. Separate Concerns by Layer

- **Rendering**, **game logic**, **UI**, **data**, **networking**, and **input** are distinct layers. Don't cross these boundaries.
- UI must not contain game logic. Rendering must not parse network packets. Keep each layer self-contained.
- When touching existing code that mixes concerns, extract toward the correct layer.
- New systems should be composable, self-contained components with clear ownership hierarchies.
- Group related data and behavior together. Don't scatter related logic across unrelated files.
- Name things precisely — if a name needs a comment to explain it, pick a better name.

## 6. One Class Per File

- Each new class or significant struct/type gets its **own file**, named after the class.
- **C++:** header and implementation separated (`.h` / `.cpp`). Small helper structs tightly coupled to a single class may stay in that class's header.
- **C#:** one public type per `.cs` file. Small helper types (private nested types, tightly coupled `record`s/`enum`s) may stay in the owning class's file.

## 7. Consistent Style

- Follow the style of the surrounding code. Don't introduce a new convention in one file.
- Use the existing naming conventions for the module you're working in.
- Keep includes minimal — only what the file actually uses.

### Namespaces and file names for extracted free functions (new files only)

When extracting free functions from a monolithic file into a new file:

1. **Wrap them in a namespace** following `<Layer>::<Concern>[::<SubConcern>]`.
   The hierarchy is **domain-first**: code about skills lives under `UI::Skills`
   regardless of which HUD area renders it; code about combat lives under
   `UI::Combat`; etc. Examples (fully-qualified function calls, parentheses
   show where the function name ends and the namespace ends):
   `UI::Skills::Tooltip::Render()`, `UI::Items::Tooltip::Render()`,
   `GameLogic::Combat::CalculateDamage()`. The last `::Tooltip` / `::Combat`
   segment is a sub-concern namespace; the verb (`Render`, `CalculateDamage`)
   is the function. Don't nest a `Render` namespace just to hold a `Render()`
   function.

2. **Drop the legacy `NewUI` prefix** from the new file name. New extractions
   use bare names like `SkillTooltip.cpp` / `SkillTooltip.h`, not
   `NewUISkillTooltip.cpp`. The `NewUI` prefix on existing files is technical
   debt from a prior UI generation and conveys nothing useful; new files start
   clean. Existing files keep their names until a separate broader rename.

These conventions apply only to **new files containing extracted free
functions**. Existing global functions and existing `C*`-style classes stay
as they are; they would be migrated as part of a future broader modernization,
not piecemeal. Anonymous namespaces for file-private helpers remain unchanged.

The first file using this convention is
`src/source/UI/HUD/Skills/SkillTooltip.cpp` (`namespace
UI::Skills::Tooltip`). Subsequent extractions should follow the same shape so
the pattern accretes consistently.

## 8. Guard Against Leaks and Dangling State

- Every allocation/resource has a clear owner and a clear cleanup path.
- **C++:** prefer RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`) over manual `new` / `delete`.
- **C#:** implement `IDisposable` for types that own unmanaged resources, and consume them with `using` statements/blocks.
- Don't leave resources (handles, connections, textures, streams) open on early-exit paths.

## 9. Readable Over Clever

- Write code that is obvious to read, even if a "clever" version is shorter.
- Complex expressions get broken into named intermediate variables.
- If a block of code needs a comment to explain *what* it does, refactor it so it doesn't.

## 10. Think Before You Add

- Every new dependency, abstraction, or pattern has a maintenance cost. Justify it.
- Smaller diffs are easier to review. Keep changes focused on one concern per commit.
- If you're unsure about a structural decision, ask before building on top of it.

## 11. Document Usage, Not Internals

- New, changed, or removed systems get a docs update, but only at the **usage** level: what the feature does and how players or admins use it.
  - In scope: gameplay features, admin/editor workflows (e.g. editor usage), UI behaviour, configuration the user touches.
  - Out of scope: function signatures, class diagrams, internal data flow. The code is the source of truth for those.
- The one exception is **game/business logic and non-obvious calculations** - the rules a system enforces (drop rates, EXP curves, combat resolution, trade flow) and the math behind them (damage formulas, collision math). Document the rules and the *why* of any formula, so a reader understands the behaviour without having to read the code.
  - When you change a rule or formula like this (not just edit nearby code), add the docs if they're missing or update them if behaviour shifts. Most of this is currently undocumented, so treat first contact as the chance to capture it.
  - Where it helps, note how the original S6 client handled it vs. how we do it now - the contrast is often the most useful part for a future reader.
- Extend an existing file in `docs/` for related content rather than starting a new one. A distinct, self-contained system (e.g. `camera-system.md`, `options-window.md`) can have its own file.
- If it doesn't help a player, an admin, or someone reaching for a formula, don't write it.

## 12. Performance: be deliberate on hot paths

- Don't micro-optimize cold paths (startup, one-shot setup, save/load) - clarity wins over saving 1-iteration-out-of-10.
- DO be careful on **hot paths**. When adding or touching code, ask: is this called per-frame, per-network-packet, per-character-tick, per-entity, or inside another tight loop?
- On hot paths, avoid:
  - **Heap allocations** (`new`, `std::vector` / `std::string` constructed per call) - prefer small stack arrays (keep them small to avoid stack-overflow risk), thread-local or pooled buffers, or caches that only rebuild on state change. Plain file-scope `static` buffers are fine for known single-threaded paths (e.g. the main render loop) but risky in worker threads - default to thread-local when in doubt.
  - **I/O inside the loop** (file reads, registry queries, network calls).
  - **O(N²) when O(N log N) or O(N)** is straightforward.
  - **String formatting in inner loops** - cache or pre-build where possible.
- When existing hot-path code is wasteful on a path you're touching, fix it or file a follow-up. Don't add new abstractions that slow it further without justification.
- "Hot path" is judged by call frequency and N, not by code prominence. A render function called 60×/sec for 50 visible objects is hot. A config reader called once at boot is not.
