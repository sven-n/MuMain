#!/usr/bin/env bash
# Lists the C++ sources changed since BASE that formatting and static analysis check.
# Usage: changed_cpp_files.sh BASE
set -euo pipefail

# awk, unlike grep -v, succeeds on empty input, so only a git diff failure fails the pipeline.
git diff --name-only --diff-filter=d "${1:?usage: changed_cpp_files.sh BASE}...HEAD" -- \
  'src/source/*.cpp' 'src/source/*.h' 'src/source/*.hpp' \
  | awk '!index($0, "ThirdParty/") && !index($0, "src/source/Dotnet/")'
