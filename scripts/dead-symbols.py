#!/usr/bin/env python3
"""
dead-symbols.py — Lists functions the linker proved nothing calls.

Diffs the public symbols MuClient.lib defines against the ones that survived
into Main.map. With MU_REPORT_DEAD_SYMBOLS=ON the link runs /OPT:REF, so every
COMDAT nothing references is discarded; defined-but-absent therefore means "no
caller". Evidence comes from the real build and the real toolchain, so it can't
drift from what actually ships the way a hand-maintained list would.

Usage:
    # one-off: enable the option, rebuild, then run this
    cmake -S src -B out/build/windows-x64 -DMU_REPORT_DEAD_SYMBOLS=ON
    cmake --build out/build/windows-x64 --config RelWithDebInfo
    python scripts/dead-symbols.py

Writes docs/dead-symbols.txt. Commit it: the point is that a later run's diff
shows what NEWLY died, which is the signal worth acting on. The absolute list is
mostly a backlog.

READ THE RESULTS WITH THESE THREE LIMITS IN MIND — they are not incidental, and
a name appearing here is a lead to investigate, never a verdict on its own:

 1. FALSE POSITIVE, inlining. A function defined in a header and inlined at every
    call site leaves its out-of-line COMDAT unreferenced, so it lands here while
    being used constantly. BaseMap::AddMapIndex (w_BaseMap.h) is exactly this.
    Anything defined in a header deserves suspicion before deletion.

 2. FALSE POSITIVE, transitive death. /OPT:REF is recursive: a live-looking
    function whose only caller is itself dead gets discarded too. That is correct
    and useful — it means the whole subtree is dead — but it reads as surprising
    when a grep does find a call site. CPersonalItemPriceTable::AddItemPrice is
    this case.

 3. FALSE NEGATIVE, virtual dispatch. A function reachable only through a vtable
    slot counts as referenced, so an entire unused virtual override never shows
    up here at all. Most of the UI tier dispatches this way. Absence from this
    list is not evidence of life.

Verify a candidate by grepping for callers and checking whether it is
header-defined, then delete and confirm the build still links.
"""

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent

# Template instantiations and third-party namespaces dominate the raw diff
# (~18k of ~20k) and are never actionable for us.
NOISE = ("@std@@", "@Rml@@")
SPECIAL = ("??_C", "??_R", "??_7", "??_G", "??_E")  # literals, RTTI, vftables, deleting dtors


def find_msvc_bin(build_dir: Path) -> Path:
    """Locate dumpbin/undname from the compiler the build actually used."""
    cc = build_dir / "compile_commands.json"
    if not cc.exists():
        sys.exit(f"error: {cc} not found — configure the build first")
    entry = json.loads(cc.read_text(encoding="utf-8"))[0]
    command = entry.get("command") or " ".join(entry["arguments"])
    compiler = Path(command.split()[0].strip('"'))
    if not (compiler.parent / "dumpbin.exe").exists():
        sys.exit(f"error: dumpbin.exe not found beside {compiler}")
    return compiler.parent


def lib_symbols(msvc_bin: Path, lib: Path) -> set[str]:
    out = subprocess.run(
        [str(msvc_bin / "dumpbin.exe"), "/NOLOGO", "/LINKERMEMBER:1", str(lib)],
        capture_output=True, text=True, errors="replace", check=True,
    ).stdout
    return set(re.findall(r"^\s{2}[0-9A-F]+\s+(\S+)\s*$", out, re.MULTILINE))


def map_symbols(map_file: Path) -> set[str]:
    symbols, in_section = set(), False
    for line in map_file.read_text(encoding="utf-8", errors="replace").splitlines():
        if "Publics by Value" in line:
            in_section = True
            continue
        if in_section:
            if line.startswith(" entry point at"):
                break
            m = re.match(r"^\s+[0-9A-Fa-f]{4}:[0-9A-Fa-f]+\s+(\S+)\s", line)
            if m:
                symbols.add(m.group(1))
    return symbols


def demangle(msvc_bin: Path, symbols: list[str]) -> dict[str, str]:
    """undname takes symbols as argv; batch to stay under the command-line limit."""
    result = {}
    for i in range(0, len(symbols), 40):
        batch = symbols[i:i + 40]
        out = subprocess.run(
            [str(msvc_bin / "undname.exe")] + batch,
            capture_output=True, text=True, errors="replace", check=False,
        ).stdout
        pairs = re.findall(r'Undecoration of :- "(.+?)"\r?\nis :- "(.+?)"', out)
        for mangled, plain in pairs:
            result[mangled] = plain
    return result


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--build-dir", type=Path, default=REPO / "out/build/windows-x64")
    ap.add_argument("--config", default="RelWithDebInfo")
    ap.add_argument("--out", type=Path, default=REPO / "docs/dead-symbols.txt")
    args = ap.parse_args()

    out_dir = args.build_dir / "src" / args.config
    lib, map_file = out_dir / "MuClient.lib", out_dir / "Main.map"

    if not map_file.exists():
        sys.exit(
            f"error: {map_file} not found.\n"
            "Rebuild with the option on first:\n"
            f"  cmake -S src -B {args.build_dir} -DMU_REPORT_DEAD_SYMBOLS=ON\n"
            f"  cmake --build {args.build_dir} --config {args.config}"
        )
    if not lib.exists():
        sys.exit(f"error: {lib} not found")

    msvc_bin = find_msvc_bin(args.build_dir)
    defined, survived = lib_symbols(msvc_bin, lib), map_symbols(map_file)

    dead = [
        s for s in defined - survived
        if not s.startswith(SPECIAL)
        and "?$" not in s                    # template instantiation
        and not any(n in s for n in NOISE)
        and re.match(r"^\?[A-Za-z_]", s)     # ordinary named function, not a ctor/dtor/operator
    ]
    plain = demangle(msvc_bin, dead)
    dead.sort(key=lambda s: plain.get(s, s))

    lines = [
        "Functions with no caller, per the linker (/OPT:REF discarded them).",
        "Generated by scripts/dead-symbols.py — do not hand-edit.",
        "",
        f"library symbols : {len(defined)}",
        f"survived link   : {len(survived)}",
        f"listed below    : {len(dead)}",
        "",
        "A lead, not a verdict. Header-defined functions may simply have been",
        "inlined, and a whole subtree dies when its only entry point does. See the",
        "script's docstring for all three failure modes before deleting anything.",
        "",
    ]
    lines += [plain.get(s, s) for s in dead]
    args.out.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"{len(dead)} unreferenced functions -> {args.out.relative_to(REPO)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
