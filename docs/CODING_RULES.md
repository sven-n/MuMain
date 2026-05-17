# Coding Rules

These rules apply to all code changes in this project. Read before you commit.

The rules apply to both the C++ client and the C# `ClientLibrary`. Where the
right idiom differs between the two languages, the rule below calls out each
language explicitly.

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
`src/source/UI/NewUI/HUD/Skills/SkillTooltip.cpp` (`namespace
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
