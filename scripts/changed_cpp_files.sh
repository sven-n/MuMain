#!/usr/bin/env bash
# Lists the C++ sources changed since BASE that formatting and static analysis check.
# Usage: changed_cpp_files.sh BASE
set -eu

git diff --name-only --diff-filter=d "${1:?usage: changed_cpp_files.sh BASE}...HEAD" -- \
  'src/source/*.cpp' 'src/source/*.h' 'src/source/*.hpp' \
  | grep -v ThirdParty/ | grep -v 'src/source/Dotnet/' || true
