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
