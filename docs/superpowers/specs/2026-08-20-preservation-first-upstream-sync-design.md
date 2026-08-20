# Preservation-First Upstream Sync Design

## Goal

Merge the current `fork/main` snapshot into downstream `main` while preserving downstream product behavior and retaining every incoming upstream capability, test, document, asset, and configuration change. When implementations conflict, port upstream intent into the downstream architecture instead of discarding either side wholesale.

## Planning Snapshot

- Downstream: `main` and `origin/main` at `db2d3c7a94b6be43240ac37f6809104ddd078ea2`.
- Upstream: `fork/main` at `c0d74c4ed5edfad527b71899337f72d39bfc49f3`.
- Shared base: `685936c10a8ee50982d0e1ca8d9430bf88186dd1`.
- Divergence: 93 downstream-only commits and 76 upstream-only commits.
- Upstream scope: 142 changed files, 13,706 insertions, and 2,361 deletions.
- Direct-merge simulation: 54 content conflicts, concentrated in Render, UI, Scenes, Engine, Camera, and World code.

This synchronization pins `c0d74c4e`. Later upstream commits belong to a subsequent synchronization so the reviewed scope cannot move during integration.

## Chosen Strategy

Merge chronological upstream mainline checkpoints into a dated integration branch. Each checkpoint becomes a reviewable merge commit and advances the recorded upstream ancestry. Resolve conflicts semantically: retain downstream structure and behavior, then port the incoming delta from the previous upstream checkpoint into that structure.

This replaces one 54-conflict merge state with smaller ancestry-preserving checkpoints. It also permits validation, rollback, and completeness review after each upstream pull request.

## Branch Isolation

Follow `../docs/upstream-sync.md` from `MuMain/`:

- Keep `main` unchanged during integration.
- Create `rollback/pre-upstream-merge-2026-08-20` at the pinned downstream SHA.
- Create `integration/upstream-2026-08-20` from the same SHA.
- Perform all checkpoint merges and validation on the integration branch.
- Keep `rerere.enabled` and `rerere.autoupdate` enabled.
- Advance `main` only through `--ff-only` after every acceptance gate passes.
- Never rebase or force-push shared `main`.

## Checkpoint Order

Merge these upstream mainline commits in chronological order:

| Order | Checkpoint | Incoming intent | Direct-merge conflict exposure |
|---:|---|---|---:|
| 1 | `5cf17732` / PR #543 | Initialize render matrices; fix Blood Castle quest-item rendering | 2 |
| 2 | `1a678b30` / PR #544 | GPU bone skinning; retire fixed-function GL; portability fixes | 49 |
| 3 | `9fdcdb08` / PR #545 | GPU skinning and ImmediateRenderer documentation | 0 |
| 4 | `b330c33c` / PR #546 | Core Profile and AnimationTaskPool documentation corrections | 0 |
| 5 | `39107a81` / PR #551 | Disable expensive GL debug callback | 1 |
| 6 | `9a6eb943` / PR #555 | Carry `blendMeshAlpha` into the Core Profile shader | 1 |
| 7 | `8b5ae75e` / PR #553 | Repair fallback frame limiting and VSync-off behavior | 3 |
| 8 | `5cfebed3` / PR #557 | Derive terrain UV scale from each texture | 1 |
| 9 | `60d4efed` / PR #560 | UBO, RHI, terrain, ImmediateRenderer, profiling, and GL performance work | 8 |
| 10 | `56fb3636` / PR #540 | Server-provided chat commands, paged UI, history, favorites, templates, docs | 4 |
| 11 | `c0d74c4e` / PR #566 | Attribute ImmediateRenderer batch breaks in `$glstats` | 2 |

Conflict counts describe each pull request's overlap with today's downstream tree. Sequential counts may be lower because each accepted checkpoint becomes the next merge base.

## Preservation Ledger

Create `docs/porting/upstream-sync-2026-08-20-ledger.md` during implementation. Add one row for every upstream non-merge commit between `685936c1` and `c0d74c4e`.

Each row records:

- Upstream commit and intent.
- Upstream paths changed.
- Downstream target paths.
- Resolution: `merged`, `ported`, `equivalent`, or `regenerated`.
- Verification command or test covering the change.
- Checkpoint merge containing the result.

There is no `dropped` or `not applicable` resolution. If upstream code cannot fit the downstream architecture directly, implement an adapter, translation layer, or downstream-compatible equivalent. If upstream default behavior conflicts with downstream product behavior, preserve the downstream default while keeping the upstream capability reachable through the compatible interface or existing configuration model.

## Conflict Resolution Policy

For each checkpoint:

1. Compare the previous upstream checkpoint, incoming checkpoint, and current integration branch.
2. Identify the behavioral delta introduced upstream.
3. Keep downstream directories, APIs, platform abstractions, configuration conventions, and product defaults.
4. Apply the upstream behavioral delta to those downstream integration points.
5. Preserve upstream tests and documentation, adapting paths and terminology where architecture differs.
6. Inspect automatic merges with the same rigor as textual conflicts.
7. Never accept a whole conflicted file as `ours` or `theirs` without a hunk-level accounting in the ledger.
8. Avoid unrelated refactors and mass formatting.

Generated packet bindings must not be hand-edited. Port the source schema, generator, or ClientLibrary change first, then regenerate the bindings using the repository's documented code-generation flow.

## Checkpoint Workflow

For each checkpoint SHA:

1. Merge with `--no-ff --no-commit`.
2. Inventory unresolved and automatically merged files.
3. Resolve by subsystem, starting with build and shared interfaces before dependent render, UI, scene, and world code.
4. Update the preservation ledger before completing the merge.
5. Run conflict-marker and whitespace checks.
6. Run focused tests for the incoming behavior.
7. Build the macOS target and run the registered test suite for every code-bearing checkpoint.
8. Commit the merge only after its ledger rows and validation pass.

PR #544 is the dominant checkpoint. Resolve it in dependency order inside its merge state:

1. CMake and platform entry points.
2. Render interfaces, RHI, shaders, and GPU resource ownership.
3. Model and bone-animation paths.
4. Effects, sprites, terrain, camera, scenes, UI, and world call sites.
5. Tests and renderer documentation.

## Validation

After every code-bearing checkpoint:

```bash
git diff --check
git diff --name-only --diff-filter=U
git grep -n -E '^(<<<<<<< |>>>>>>> )' -- 'src/source/*'
cmake --preset macos-arm64
cmake --build --preset macos-arm64-debug
cmake -S . -B build-test -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-test -j8
ctest --test-dir build-test --output-on-failure
```

Run `../ctl check` from `MuMain/` at major renderer checkpoints (#544 and #560), after the chat-command checkpoint (#540), and before publication. Record any pre-existing whole-tree failure separately; do not hide a new checkpoint regression behind existing debt.

Final acceptance requires:

- Every ledger row resolved and verified.
- `git merge-base --is-ancestor c0d74c4e HEAD` exits successfully.
- `git rev-list --left-right --count c0d74c4e...HEAD` reports zero upstream-only commits.
- Native build, tests, and `./ctl check` pass.
- The integration worktree is clean.
- Downstream product defaults and known local behavior remain intact.

## Publication And Rollback

After final validation:

1. Record the integration head SHA.
2. Switch to `main`.
3. Fast-forward `main` to `integration/upstream-2026-08-20`.
4. Push `main` to `origin`.
5. Retain the rollback branch until the merged result is confirmed stable.

Before publication, abandon only the integration branch if the work cannot be completed safely. After publication, revert the relevant merge commit; never reset and force-push shared history.

## Rejected Strategies

- **Single final merge:** valid ancestry, but 54 simultaneous conflicts make behavioral omission difficult to detect and prevent per-pull-request checkpoint validation.
- **Cherry-pick or squash only:** can port behavior, but does not make `fork/main` an ancestor; later synchronizations repeat conflict work.
- **Reset downstream to upstream and replay local commits:** rewrites shared history, risks losing downstream behavior, and violates the synchronization runbook.
- **Whole-tree `ours` ancestry marker:** can conceal missing upstream behavior and cannot prove the preservation requirement.

## Scope Ceiling

This synchronization integrates upstream through `c0d74c4e`. It does not redesign unrelated downstream systems, mass-format files, clean historical debt, or include later upstream commits. Any architectural extraction must be the minimum needed to preserve both downstream behavior and the incoming capability.
