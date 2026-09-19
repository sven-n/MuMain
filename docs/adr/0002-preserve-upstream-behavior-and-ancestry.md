# ADR 0002: Preserve Upstream Behavior and Ancestry

**Status:** Accepted

**Date:** 2026-09-02

## Context

MuMain carries downstream architecture and product behavior while periodically
integrating upstream work. Large direct merges make omissions difficult to
detect. Squash-only or whole-file conflict resolutions can discard behavior or
force the same conflicts to be solved again during the next synchronization.

## Decision

For each upstream synchronization:

1. Pin the downstream base, upstream tip, and shared merge base before work
   begins.
2. Use an isolated integration branch or worktree. Keep shared `main`
   unchanged until acceptance.
3. Merge chronological upstream checkpoints or bounded pull-request groups,
   preserving Git ancestry.
4. Resolve conflicts semantically: keep downstream structure, APIs, defaults,
   and platform abstractions while porting the upstream behavioral delta.
5. Preserve upstream capabilities, tests, documentation, assets, and
   configuration changes. Adapt them to downstream architecture where needed.
6. Regenerate generated files from their source schema or generator; never
   hand-edit generated output.
7. Record temporary checkpoints, conflict accounting, and command output in
   the pull request or issue, not permanent repository documentation.
8. Verify focused behavior at each checkpoint. Before publication, run the
   supported build and test gates and confirm the pinned upstream tip is an
   ancestor of the integration result.
9. Publish through a fast-forward of `main`. After publication, roll back
   with a revert rather than rewriting shared history.

## Consequences

- Synchronizations take more checkpoints but provide smaller review and
  rollback boundaries.
- Future synchronizations reuse recorded ancestry instead of repeating a
  squash-only conflict set.
- No committed ledger or session handoff is required; Git, the pull request,
  tests, and durable design documentation retain the useful record.
