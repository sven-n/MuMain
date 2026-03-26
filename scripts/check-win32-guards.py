#!/usr/bin/env python3
"""
check-win32-guards.py — Anti-pattern checker for #ifdef _WIN32 in game logic.

Detects #ifdef _WIN32 blocks that wrap CODE (function calls, statements) rather
than #include directives. This is the forbidden pattern: it silences compilation
errors by disabling features on macOS/Linux instead of fixing the root cause.

ALLOWED:
  - #ifdef _WIN32 / #include <...> / #else / #include "PlatformTypes.h" / #endif
    (standard include-selection — this is the correct platform abstraction pattern)
  - #ifdef _WIN32 / ... / #else / fallback_impl / #endif
    where BOTH branches provide the functionality (e.g. DPAPI vs no-op-with-comment)
  - Files under Platform/, ThirdParty/, Audio/DSwaveIO*, Audio/DSplaysound*
    (platform implementation files — they ARE the abstraction boundary)

FORBIDDEN:
  - #ifdef _WIN32 / SocketClient->Foo(); / #endif   ← no #else, hides the call
  - #ifdef _WIN32 / KillGLWindow(); / #endif         ← no #else, hides exit logic

Exit code: 0 = no violations, 1 = violations found (prints each one).
"""

import os
import sys

SOURCE_ROOT = os.path.join(os.path.dirname(__file__), "..", "src", "source")

# Files/directories where #ifdef _WIN32 wrapping code is permitted.
# IMPORTANT: Only the platform abstraction layer belongs here.
# Every other module must achieve full cross-platform parity — no exemptions.
ALLOWED_PATHS = [
    "/Platform/",
    "/ThirdParty/",
    "Audio/DSwaveIO",
    "Audio/DSplaysound",
    "Audio/DSPlaySound",
]


def is_allowed(fpath: str) -> bool:
    return any(a in fpath for a in ALLOWED_PATHS)


def is_include_line(line: str) -> bool:
    s = line.strip()
    return s.startswith("#include") or s.startswith("//") or s == ""


def check_file(fpath: str) -> list[tuple[int, str]]:
    """Return list of (lineno, snippet) for each violation in fpath."""
    violations = []
    try:
        with open(fpath, encoding="utf-8", errors="replace") as f:
            lines = f.readlines()
    except OSError:
        return violations

    i = 0
    while i < len(lines):
        stripped = lines[i].strip()
        # Match #ifdef _WIN32, #if defined(_WIN32), #if _WIN32
        # Exclude #ifndef _WIN32 and #if !defined(_WIN32) (those are non-Windows guards)
        is_win32_guard = (
            stripped == "#ifdef _WIN32"
            or (
                stripped.startswith("#if ")
                and "_WIN32" in stripped
                and "!defined" not in stripped
                and "!" not in stripped.split("_WIN32")[0]
            )
        )
        if not is_win32_guard:
            i += 1
            continue

        start = i + 1  # line number (0-based index of next line)
        block_lines = []
        has_else = False
        j = i + 1
        depth = 1

        while j < len(lines) and j < i + 2000:
            s = lines[j].strip()
            if s.startswith("#if"):
                depth += 1
            elif s.startswith("#else") and depth == 1:
                has_else = True
                break
            elif s.startswith("#endif"):
                depth -= 1
                if depth == 0:
                    break
            else:
                block_lines.append((j + 1, s))  # 1-based line number
            j += 1

        # Violation: no #else AND the block contains non-include code
        if not has_else:
            code_lines = [(ln, s) for ln, s in block_lines if not is_include_line(s)]
            if code_lines:
                first_ln, first_s = code_lines[0]
                violations.append((i + 1, first_ln, first_s[:80]))

        i = j + 1

    return violations


def main() -> int:
    violations_found = False
    for dirpath, _dirs, files in os.walk(SOURCE_ROOT):
        for fname in files:
            if not (fname.endswith(".cpp") or fname.endswith(".h")):
                continue
            fpath = os.path.join(dirpath, fname)
            if is_allowed(fpath):
                continue
            viols = check_file(fpath)
            for ifdef_ln, code_ln, snippet in viols:
                rel = os.path.relpath(fpath, SOURCE_ROOT)
                print(f"VIOLATION  {rel}:{ifdef_ln}  (code at line {code_ln}): {snippet}")
                violations_found = True

    if violations_found:
        print()
        print("Fix: remove the #ifdef _WIN32 / #endif wrapper and add a stub to")
        print("PlatformCompat.h for the missing type instead.")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
