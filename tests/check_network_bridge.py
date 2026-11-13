#!/usr/bin/env python3
"""Two checks on the network library's bridge (src/source/Dotnet):

- Every export the client looks up with LoadManagedSymbol is in an export list (the kManagedExports_* arrays: the
  generated PacketBindings_*.h and Dotnet/ManagedSymbols.cpp), and every listed export is looked up. The bridge
  resolves the lists at startup, so a lookup missing from them would only fail when its packet is first sent.
- Connection.h, and the headers that include it, carry no platform conditionals and load no library: the target's
  adapter (Dotnet/SymbolSources) supplies the exports.

Usage: check_network_bridge.py <repo root>
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cpp_text import code_and_source, skipped_spans, source_files, with_spans_blanked  # noqa: E402

SUFFIXES = (".cpp", ".h", ".inl")
SKIPPED_DIRS = {"ThirdParty"}

LOOKUP = re.compile(r"\bLoadManagedSymbol\s*<[^;]*?>\s*\(\s*\"([^\"]+)\"\s*\)")
EXPORT_LIST = re.compile(r"\bkManagedExports_\w+\s*\[\s*\]\s*=\s*\{(.*?)\};", re.DOTALL)
NAME = re.compile(r"\"([^\"]+)\"")

PLATFORM_CONDITIONAL = re.compile(
    r"^\s*#\s*(?:if|ifdef|ifndef|elif)\b.*\b(?:_WIN32|_WIN64|_MSC_VER|__linux__|__unix__|__APPLE__|__ANDROID__|"
    r"__EMSCRIPTEN__)\b")
LIBRARY_LOADING = re.compile(r"\b(?:LoadLibrary\w*|dlopen|dlsym|GetProcAddress|mu::platform::(?:Load|GetSymbol|"
                             r"FindSymbol))\s*\(|#\s*include\s*[\"<][^\">]*PlatformLibrary\.h")
INCLUDES_CONNECTION = re.compile(r"^\s*#\s*include\s*\"(?:Dotnet/)?Connection\.h\"", re.MULTILINE)


def read_sources(root):
    source = root / "src" / "source"
    for file in source_files(source, SKIPPED_DIRS, SUFFIXES):
        text = file.read_text(encoding="utf-8", errors="replace")
        # Comments blanked, string literals kept: the export names are literals.
        yield file.relative_to(root).as_posix(), text, code_and_source(text)[1]


def export_problems(sources):
    looked_up = {}
    listed = {}
    for path, _, code in sources:
        for name in LOOKUP.findall(code):
            looked_up.setdefault(name, path)
        for block in EXPORT_LIST.findall(code):
            for name in NAME.findall(block):
                listed.setdefault(name, path)
    problems = [f"{path}: looks up {name}, which no export list holds; startup would not check it"
                for name, path in sorted(looked_up.items()) if name not in listed]
    problems += [f"{path}: lists {name}, which nothing looks up" for name, path in sorted(listed.items())
                 if name not in looked_up]
    return problems


def without_comments(text):
    """The text with its comments blanked and its preprocessor lines kept (code_and_source blanks those, and they are
    what the header check reads); every line keeps its number."""
    comments = [span for span in skipped_spans(text) if text.startswith(("//", "/*"), span[0])]
    return with_spans_blanked(text, comments, keep_literals=True)


def header_problems(sources):
    problems = []
    for path, text, _ in sources:
        if not path.endswith(".h"):
            continue
        text = without_comments(text)
        if not (path.endswith("/Connection.h") or INCLUDES_CONNECTION.search(text)):
            continue
        for number, line in enumerate(text.splitlines(), 1):
            if PLATFORM_CONDITIONAL.search(line):
                problems.append(f"{path}:{number}: platform conditional in a header of the network bridge")
            if LIBRARY_LOADING.search(line):
                problems.append(f"{path}:{number}: loads a library in a header of the network bridge; the target's "
                                "adapter (Dotnet/SymbolSources) does that")
    return problems


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    sources = list(read_sources(Path(argv[1]).resolve()))
    problems = export_problems(sources) + header_problems(sources)
    for problem in problems:
        print(problem)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
