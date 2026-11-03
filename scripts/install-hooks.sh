#!/usr/bin/env bash
# Enables the repository's git hooks (.githooks/) for this clone and all its worktrees.
# Run from any directory inside the repo.
set -eu

git -C "$(dirname "$0")" config core.hooksPath .githooks
echo "Git hooks enabled from .githooks/ (pre-push runs the static checks)."
