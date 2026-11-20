#!/usr/bin/env python3
"""Every model file under the game data must be one the model loader can read (BMD::Open2 in
src/source/Render/Models/ZzzBMD.cpp): a known version and a body after the header. A header-only file loads as
nothing: with an unknown version it logs an error on every load of its map, with a known one it is an empty model.

A model is a .bmd file that starts with "BMD"; other .bmd files are data tables (Data/Local/MacroConfig.bmd) and
are left alone. An empty .bmd file is neither and fails too.

Usage: check_model_files.py <data folder>
"""
import sys
from pathlib import Path

# The versions BMD::Open2 reads, with the size of a file that holds only the header ("BMD", the version, a 32-byte
# name and the mesh, bone and action counts): 0xA plain; 0xC adds the encrypted size before it; 0xE adds the cipher's
# own header.
HEADER_ONLY_BYTES = {0x0A: 42, 0x0C: 46, 0x0E: 80}


def problem(head, size):
    """Why a file that starts with `head` and has `size` bytes is no readable model; None when it is, or no model."""
    if size == 0:
        return "empty"
    if not head.startswith(b"BMD"):
        return None
    if len(head) < 4 or head[3] not in HEADER_ONLY_BYTES:
        return f"version {head[3] if len(head) > 3 else 'missing'} is none the loader reads"
    if size <= HEADER_ONLY_BYTES[head[3]]:
        return "only the header, no meshes, bones or actions"
    return None


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    data = Path(sys.argv[1])
    sys.stdout.reconfigure(errors="backslashreplace")
    failures = []
    for path in sorted(data.rglob("*")):
        if path.suffix.lower() != ".bmd" or not path.is_file():
            continue
        with open(path, "rb") as file:
            head = file.read(4)
        reason = problem(head, path.stat().st_size)
        if reason:
            failures.append(f"{path.relative_to(data).as_posix()}: {reason}")
    for failure in failures:
        print(failure)
    if failures:
        print(f"{len(failures)} model files the loader cannot read; replace them or remove them and what names them")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
