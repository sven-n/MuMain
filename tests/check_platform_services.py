#!/usr/bin/env python3
"""Saved logins, shop downloads and CPU use reach the platform only through their adapters:

- No file outside an Adapters/ folder includes OpenSSL, curl or the DPAPI stand-in, or calls the OS for process times
  or DPAPI. Each target compiles its own adapters, so a call elsewhere would tie every target to that platform.
- An adapter serves one platform and the services' headers serve all: neither branches on the platform with the
  preprocessor. The target's CMake picks the adapter.

Usage: check_platform_services.py <repo root>
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cpp_text import skipped_spans, source_files, with_spans_blanked  # noqa: E402

SUFFIXES = (".cpp", ".h", ".inl")
SKIPPED_DIRS = {"ThirdParty"}
# The DPAPI stand-in off Windows, which the credentials adapter calls.
ALLOWED = {"src/source/Core/Platform/Dpapi.h"}
SERVICE_HEADERS = {"src/source/Core/Platform/PlatformCrypto.h", "src/source/Core/Utilities/CpuUsage.h",
                   "src/source/GameShop/FileDownloader/ShopDownload.h", "src/source/Render/Renderer/GpuDevice.h"}

PLATFORM_USE = re.compile(r"^\s*#\s*include\s*[<\"](?:openssl/|curl/|(?:Core/Platform/)?[Dd]papi\.h|wincrypt\.h|"
                          r"sys/resource\.h)"
                          r"|\b(?:GetProcessTimes|getrusage|CryptProtectData|CryptUnprotectData|GetComputerNameA|"
                          r"gethostname)\s*\(", re.MULTILINE)
PLATFORM_CONDITIONAL = re.compile(
    r"^\s*#\s*(?:if|ifdef|ifndef|elif)\b.*\b(?:_WIN32|_WIN64|_MSC_VER|__linux__|__unix__|__APPLE__|__ANDROID__|"
    r"__EMSCRIPTEN__)\b.*$", re.MULTILINE)


def without_comments(text):
    comments = [span for span in skipped_spans(text) if text.startswith(("//", "/*"), span[0])]
    return with_spans_blanked(text, comments, keep_literals=True)


def line_of(text, match):
    return text.count("\n", 0, match.start()) + 1


def problems(root):
    found = []
    for file in source_files(root / "src" / "source", SKIPPED_DIRS, SUFFIXES):
        path = file.relative_to(root).as_posix()
        if path in ALLOWED:
            continue
        text = without_comments(file.read_text(encoding="utf-8", errors="replace"))
        in_adapter = "/Adapters/" in path
        if in_adapter or path in SERVICE_HEADERS:
            found += [f"{path}:{line_of(text, match)}: {match.group(0).strip()}: one adapter per platform, chosen by "
                      "the target's CMake, instead of a preprocessor branch"
                      for match in PLATFORM_CONDITIONAL.finditer(text)]
        if not in_adapter:
            found += [f"{path}:{line_of(text, match)}: {match.group(0).strip()} belongs in a platform service's "
                      "Adapters/ folder" for match in PLATFORM_USE.finditer(text)]
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
