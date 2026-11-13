#!/usr/bin/env python3
"""The game reads its content (Data/, fonts/, shaders/ ...) through the content layer (src/source/Core/Content), so a
port can serve it from anywhere: the APK, the browser, memory. This finds the calls in src/source that open, probe or
list files themselves, and fails when a file has more of a kind than check_content_io_baseline.txt allows.

The baseline lists the calls that stay on disk (settings, logs, editor saves), each with why in a fourth column, and
the calls still to move, without one; the check lists those as it runs. Each change that moves calls lowers it. A file
with fewer calls than its baseline line also fails, so the baseline keeps up. Comments and string literals are not
code and are left out.

Usage: check_content_io.py <repo root> [--update]   (--update rewrites the counts from the code, keeping the reasons)
"""
import re
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cpp_text import code_only, source_files  # noqa: E402

SUFFIXES = (".cpp", ".h", ".inl")

BASELINE = Path("tests/check_content_io_baseline.txt")

# Calls that reach a file by its path, each under the name the baseline uses.
CALLS = {
    "fopen": r"\b_?w?fopen(?:_s)?\s*\(",
    # A stream declared or built, not the type named (std::ifstream::in, an std::ifstream& parameter).
    "ifstream": r"\b(?:std::)?w?ifstream\b(?!\s*(?:::|&|\*|>))",
    # A stream that can read, built with its file (std::fstream input(path, std::ios::in)).
    "fstream": r"\b(?:std::)?w?fstream\b(?!\s*(?:::|&|\*|>))",
    # Opening a stream declared earlier.
    "stream open": r"[.]open\s*\(",
    "Win32 file": r"\b(?:CreateFile|FindFirstFile(?:Ex)?|GetFileAttributes(?:Ex)?)[AW]?\s*\(",
    "filesystem probe": r"\bstd::filesystem::(?:exists|is_regular_file|is_directory|file_size)\s*\(",
    "filesystem listing": r"\bstd::filesystem::(?:recursive_)?directory_iterator\b",
    "SDL file": r"\bSDL_(?:IOFromFile|LoadFile|LoadPNG|LoadBMP)\s*\(",
    "font file": r"\b(?:TTF_OpenFont|AddFontResourceExW?)\s*\(",
    "glTF file": r"\bcgltf_parse_file\s*\(",
    "audio file": r"\bma_(?:sound_init_from_file|decoder_init_file)(?:_w)?\s*\(",
}
PATTERNS = {name: re.compile(pattern) for name, pattern in CALLS.items()}

# The content layer itself, and code that is not the game's.
SKIPPED_DIRS = {"ThirdParty"}
LAYER = Path("src/source/Core/Content")


def count_calls(root):
    counts = Counter()
    source = root / "src" / "source"
    for file in source_files(source, SKIPPED_DIRS, SUFFIXES):
        relative = file.relative_to(root)
        if LAYER in relative.parents:
            continue
        code = code_only(file.read_text(encoding="utf-8", errors="replace"))
        for name, pattern in PATTERNS.items():
            found = len(pattern.findall(code))
            if found:
                counts[(relative.as_posix(), name)] = found
    return counts


def read_baseline(file):
    allowed = Counter()
    reasons = {}
    for line in file.read_text(encoding="utf-8").splitlines():
        if line.strip() and not line.startswith("#"):
            path, name, count, *reason = line.split("\t")
            allowed[(path, name)] = int(count)
            if reason and reason[0].strip():
                reasons[(path, name)] = reason[0].strip()
    return allowed, reasons


def write_baseline(file, counts, reasons):
    lines = ["# check_content_io.py: file<TAB>call<TAB>how many it may still have<TAB>why it stays on disk (none: "
             "still to move). Lower the count, never raise it."]
    for (path, name), count in sorted(counts.items()):
        reason = reasons.get((path, name))
        lines.append(f"{path}\t{name}\t{count}" + (f"\t{reason}" if reason else ""))
    file.write_bytes(("\n".join(lines) + "\n").encode("utf-8"))


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    root = Path(argv[1]).resolve()
    counts = count_calls(root)
    allowed, reasons = read_baseline(root / BASELINE)
    if "--update" in argv:
        write_baseline(root / BASELINE, counts, reasons)
        return 0

    for key in sorted(counts):
        if key not in reasons and counts[key] <= allowed[key]:
            print(f"still to move: {key[0]}: {counts[key]} {key[1]} call(s)")
    problems = []
    for key in sorted(set(counts) | set(allowed)):
        path, name = key
        if counts[key] > allowed[key]:
            problems.append(f"{path}: {counts[key]} {name} call(s), {allowed[key]} allowed; "
                            "read content through Core/Content (docs/content-access.md)")
        elif counts[key] < allowed[key]:
            problems.append(f"{path}: {counts[key]} {name} call(s), the baseline still allows {allowed[key]}; "
                            "lower it (check_content_io.py --update)")
    for problem in problems:
        print(problem)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
