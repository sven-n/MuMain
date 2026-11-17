#!/usr/bin/env python3
"""The game stops and reports errors through Core/Platform/Quit.h and ErrorDialog, never through Win32:

- No code outside Core/Platform/ quits by sending WM_DESTROY, WM_CLOSE or WM_QUIT to a window, or by PostQuitMessage.
  Under SDL a sent WM_DESTROY never reaches the game, and a quit should happen at one point in the loop.
- No code outside Core/Platform/ shows a MessageBox: errors go through the one error dialog.

Usage: check_quit_paths.py <repo root>
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cpp_text import code_only, source_files  # noqa: E402

SUFFIXES = (".cpp", ".h", ".inl")
SKIPPED_DIRS = {"ThirdParty"}
PLATFORM_DIR = "src/source/Core/Platform/"

QUIT_MESSAGE = re.compile(r"\b(?:SendMessage(?:Timeout)?|SendNotifyMessage|PostMessage|PostThreadMessage)[AW]?\s*\("
                          r"[^;]*?\bWM_(?:DESTROY|CLOSE|QUIT)\b|\bPostQuitMessage\s*\(")
MESSAGE_BOX = re.compile(r"\bMessageBox(?:Ex|Indirect)?[AW]?\s*\(")


def problems(root):
    found = []
    for file in source_files(root / "src" / "source", SKIPPED_DIRS, SUFFIXES):
        path = file.relative_to(root).as_posix()
        if path.startswith(PLATFORM_DIR):
            continue
        text = code_only(file.read_text(encoding="utf-8", errors="replace"))
        for pattern, fix in ((QUIT_MESSAGE, "Core::Platform::RequestQuit() or ShowFatalError()"),
                             (MESSAGE_BOX, "Core::Platform::ErrorDialog or ShowFatalError()")):
            found += [f"{path}:{text.count(chr(10), 0, match.start()) + 1}: {match.group(0).split('(')[0].strip()}: "
                      f"use {fix}" for match in pattern.finditer(text)]
    return found


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    found = problems(Path(argv[1]).resolve())
    for problem in found:
        print(problem)
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
