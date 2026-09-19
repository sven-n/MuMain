#!/bin/bash
# Install git hooks for the MuMain repository.
# Run from any directory inside the repo.

REPO_ROOT=$(git -C "$(dirname "$0")" rev-parse --show-toplevel)
HOOKS_DIR="$REPO_ROOT/.git/hooks"

cp "$(dirname "$0")/pre-commit" "$HOOKS_DIR/pre-commit"
chmod +x "$HOOKS_DIR/pre-commit"

echo "Git hooks installed successfully."
