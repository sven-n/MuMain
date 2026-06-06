# Agent Instructions

This file is the entry point for AI coding assistants (Claude Code, Cursor, Codex,
etc.) working in this repository. It also applies to human contributors.

## Coding rules — read first

**The coding rules in [`docs/CODING_RULES.md`](docs/CODING_RULES.md) apply to all
code changes in this repository, by humans and by AI agents alike.**

Before writing or modifying code:

1. Read [`docs/CODING_RULES.md`](docs/CODING_RULES.md).
2. Apply the rules to any new or changed code.
3. When reviewing a change (including via `/review` or `/ultrareview`), treat
   `docs/CODING_RULES.md` as the authoritative style and quality baseline.

The rules are not exhaustive — they capture the conventions that matter most for
this codebase. Follow the style of surrounding code where the rules don't speak.

## Build setup

The project uses CMake. Supported build environments and setup steps are
documented in [`docs/build-guide.md`](docs/build-guide.md).

Quick references:

- Requirements and IDE-specific instructions: [`README.md`](README.md).
- Cross-platform / WSL build details: [`docs/build-guide.md`](docs/build-guide.md).

## Branch and PR conventions

- Target branch for PRs is `main`.
- Keep changes focused — one concern per commit (see rule 10 in the coding
  rules). Smaller diffs review faster.
- Match the style of existing commit messages in `git log`.
- Reference the related issue in the PR description when applicable.

## Out of scope for AI changes

Don't perform large retroactive cleanups of existing code to fit the rules unless
the user explicitly asks for it. Apply the rules going forward; pre-existing
code can be refactored opportunistically when you're already touching it.
