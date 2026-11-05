#!/usr/bin/env python3
"""Fails a CI step whose output holds a warning, note or deprecation.

    python3 scripts/ci_warnings.py run [--] <command> [arguments...]
        Runs the command, passes its output through as it comes, and exits non-zero when the command failed
        or printed a line that reports a problem. Each such line becomes a GitHub error annotation.
    python3 scripts/ci_warnings.py report [--] <command> [arguments...]
        The same, but only the command's own failure fails the step: the lines are listed, not enforced
        (builds the rule does not cover, such as Fork CI's editor builds).
    python3 scripts/ci_warnings.py scan <log file>...
        Lists those lines in saved logs, e.g. a downloaded job log.

The compilers and linkers of the game's own code turn their warnings into errors themselves
(MU_WARNINGS_AS_ERRORS); this check also covers what they cannot: CMake, the third-party libraries' CMake
scripts, vcpkg, the shader compilers, wayland-scanner and the .NET build.
"""

import os
import re
import subprocess
import sys

# Each pattern names one kind of report. A line matches when any pattern finds it.
PATTERNS = (
    # GCC, Clang, ld, DXC, glslang ("WARNING:"), CMake modules ("-- Note: ..."), MSVC ("note: see ...").
    ("diagnostic", re.compile(r"\b(?:warning|note|remark)\s*(?:\[[^\]\n]*\]\s*)?:", re.IGNORECASE)),
    # MSVC, MSBuild and dotnet codes: "file(12): warning C4996:", "warning NU1603:", "info NETSDK1057:".
    ("coded diagnostic", re.compile(r"\b(?:warning|info|message)\s+[A-Z]{1,8}\d{3,6}\b")),
    ("CMake warning", re.compile(r"^\s*CMake (?:Deprecation )?Warning\b")),
    # libxml2's DTD check inside wayland-scanner: not fatal, but the protocol does not match the scanner.
    ("validity error", re.compile(r"\bvalidity error\b")),
    ("Node.js deprecation", re.compile(r"\bDeprecationWarning\b")),
    # Python's warning categories, e.g. from the build's generator scripts: "gen.py:12: SyntaxWarning: ...".
    ("Python warning", re.compile(r"\b[A-Z][A-Za-z]*Warning:")),
    # NuGet outside MSBuild: "warn : NU1603: ...".
    ("NuGet warning", re.compile(r"^\s*warn\s*:")),
    ("Clang warning count", re.compile(r"\b\d+ warnings? generated\b")),
    # A line that opens with an upper-case WARN: loggers ("WARN ", "[WARN]") and CMake's message() with a mode it
    # does not know, which it prints as text ("message(WARN \"x\")" -> "WARNx").
    ("warning line", re.compile(r"^\s*\[?WARN(?![a-z])")),
)

ANSI_ESCAPE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
# GitHub job logs start each line with a timestamp.
LOG_TIMESTAMP = re.compile(r"^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d(?:\.\d+)?Z ")
MAX_ANNOTATIONS = 50


def find_problem(line):
    """The kind of problem the line reports, or None."""
    text = LOG_TIMESTAMP.sub("", ANSI_ESCAPE.sub("", line))
    for kind, pattern in PATTERNS:
        if pattern.search(text):
            return kind
    return None


def escape_annotation(text):
    return text.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")


def escape_property(text):
    """A workflow command's property value, e.g. its title: unescaped, ':' and ',' would end it."""
    return escape_annotation(text).replace(":", "%3A").replace(",", "%2C")


def report(problems, what, enforce=True):
    """Prints the problems (as error annotations when enforced) and a step summary; returns whether there were any."""
    if not problems:
        return False
    unique = list(dict.fromkeys(problems))
    for kind, line in unique[:MAX_ANNOTATIONS] if enforce else ():
        print(f"::error title={escape_property(f'{kind} in {what}')}::{escape_annotation(line)}", flush=True)
    print(f"{len(problems)} line(s) report a warning, note or deprecation ({len(unique)} different).", flush=True)
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as out:
            out.write(f"### Warnings in {what}\n\n```text\n")
            out.writelines(f"{line}\n" for _, line in unique)
            out.write("```\n")
    return True


def run(command, enforce=True):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    problems = []
    for raw in process.stdout:
        line = raw.decode("utf-8", errors="replace").rstrip("\r\n")
        print(line, flush=True)
        kind = find_problem(line)
        if kind:
            problems.append((kind, ANSI_ESCAPE.sub("", line).strip()))
    status = process.wait()
    found = report(problems, " ".join(command[:2]), enforce)
    if status != 0:
        return status
    return 1 if found and enforce else 0


def scan(paths):
    problems = []
    for path in paths:
        with open(path, encoding="utf-8", errors="replace") as log:
            for line in log:
                line = line.rstrip("\r\n")
                kind = find_problem(line)
                if kind:
                    problems.append((kind, LOG_TIMESTAMP.sub("", ANSI_ESCAPE.sub("", line)).strip()))
    return 1 if report(problems, ", ".join(paths)) else 0


def main(arguments):
    # The build prints UTF-8 (e.g. the Korean of old data files); a Windows pipe would take only the code page.
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    if len(arguments) >= 2 and arguments[0] in ("run", "report"):
        # "--" is optional: PowerShell may drop it before a native command's arguments.
        command = arguments[2:] if arguments[1] == "--" else arguments[1:]
        if command:
            return run(command, enforce=arguments[0] == "run")
    if len(arguments) >= 2 and arguments[0] == "scan":
        return scan(arguments[1:])
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
