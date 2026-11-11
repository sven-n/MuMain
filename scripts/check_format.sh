#!/usr/bin/env bash
# Checks clang-format on the lines changed since BASE, for the files listed on stdin.
# Usage: changed_cpp_files.sh BASE | check_format.sh BASE
set -eu

base="${1:?usage: check_format.sh BASE < files}"
status=0
while IFS= read -r file; do
  format_args=()
  while read -r start count; do
    if [ -z "$start" ]; then
      continue
    fi
    if [ -z "$count" ]; then
      count=1
    fi
    if [ "$count" -eq 0 ]; then
      continue
    fi
    end=$((start + count - 1))
    format_args+=(--lines="${start}:${end}")
  done < <(
    git diff --unified=0 --no-color "${base}...HEAD" -- "$file" \
      | sed -nE 's/^@@ -[0-9]+(,[0-9]+)? \+([0-9]+)(,([0-9]+))? @@.*/\2 \4/p'
  )
  if [ "${#format_args[@]}" -ne 0 ]; then
    clang-format --dry-run --Werror "${format_args[@]}" "$file" || status=1
  fi
done
exit "$status"
