# Rust Client Evidence

This project keeps comparison evidence in a stable directory layout so CI,
local smoke tests, and legacy-vs-Rust diffs are easy to find.

## Layout

Each scenario gets its own case root. The case name is slugified once and then
reused for every artifact type.

```text
<artifacts-root>/<case-slug>/
  logs/
  screenshots/
  reports/
  diffs/
```

Example:

```text
/tmp/evidence/login-main-window/
  logs/boot-finished.log
  screenshots/loading-screen.png
  reports/state-snapshot.json
  diffs/legacy-vs-rust.diff
```

## Naming Rules

- Case names are slugified to lower-case hyphenated paths.
- Artifact labels are also slugified before file creation.
- Logs use `.log`.
- Screenshots use `.png`.
- Diffs use `.diff`.
- Reports keep the extension supplied by the report writer.

## Usage

- Use one case root per comparison scenario.
- Keep the case name, artifact label, and artifact type stable between runs.
- Reuse the same root for legacy and Rust outputs when you want a direct diff.
- Prefer short, descriptive labels such as `Boot Finished` or `State Snapshot`.

## Legacy Source Classification

The canonical audit lives in `port_rust/crates/mu_test_support/src/source_inventory.rs`
and is exercised by `port_rust/tests/rust/source_inventory.rs`.

- `reference-only`: keep the legacy path as behavior or protocol reference.
- `fixture-only`: use the path as test or asset fixture input only.
- `replaced`: the Rust client owns the equivalent behavior.
- `ported`: the legacy path is fully mirrored by Rust code.
- `rejected`: keep the path out of the Rust client and packaging flow.

When a new top-level legacy path appears under `ClientLibrary`, `src/source`,
`src/MuEditor`, `src/bin`, `src/ThirdParty`, `src/dependencies` or `tests`,
update the inventory helper and its test in the same change.
