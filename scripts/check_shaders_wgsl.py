#!/usr/bin/env python3
"""Checks that every compiled shader of the game still converts to WGSL, which a browser build needs.

    python3 scripts/check_shaders_wgsl.py [--naga <path>] [<folder of .spv files>]

The folder is src/shaders/compiled by default. Each SPIR-V file is converted with naga (cargo install naga-cli); the
check fails when one does not convert, e.g. because it passes an array between shader stages.

SDL GPU puts a texture and its sampler on the same (set, binding), which WGSL forbids. A WebGPU backend has to give
them separate bindings, so the check moves the later resource of each such pair to binding + 16 before running naga.
The files themselves are not changed.

naga does not run the uniformity analysis a browser runs, so a shader that passes here can still be rejected for
sampling in non-uniform control flow.
"""

import argparse
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

DEFAULT_FOLDER = Path(__file__).resolve().parent.parent / "src" / "shaders" / "compiled"
SPIRV_MAGIC = 0x07230203
HEADER_WORDS = 5
OP_DECORATE = 71
DECORATION_BINDING = 33
DECORATION_DESCRIPTOR_SET = 34
SAMPLER_BINDING_OFFSET = 16


def split_bindings(data):
    """The module with the later resource of each (set, binding) pair that is used twice moved up."""
    words = list(struct.unpack(f"<{len(data) // 4}I", data))
    if len(words) < HEADER_WORDS or words[0] != SPIRV_MAGIC:
        raise ValueError("not a little-endian SPIR-V module")
    sets, binding_words = {}, {}
    index = HEADER_WORDS
    while index < len(words):
        opcode, length = words[index] & 0xFFFF, words[index] >> 16
        if length == 0:
            raise ValueError("an instruction of length 0")
        if opcode == OP_DECORATE and words[index + 2] == DECORATION_DESCRIPTOR_SET:
            sets[words[index + 1]] = words[index + 3]
        if opcode == OP_DECORATE and words[index + 2] == DECORATION_BINDING:
            binding_words[words[index + 1]] = index + 3
        index += length
    used = set()
    for variable in sorted(binding_words):
        position = binding_words[variable]
        key = (sets.get(variable), words[position])
        if key in used:
            words[position] += SAMPLER_BINDING_OFFSET
        used.add(key)
    return struct.pack(f"<{len(words)}I", *words)


def convert(naga, shader, work):
    """naga's error for a shader that does not convert, else None."""
    source = work / shader.name
    source.write_bytes(split_bindings(shader.read_bytes()))
    result = subprocess.run([naga, str(source), str(source.with_suffix(".wgsl"))], capture_output=True, text=True)
    if result.returncode == 0:
        return None
    return (result.stderr or result.stdout).strip() or f"naga exited with {result.returncode}"


def main(arguments=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("folder", nargs="?", type=Path, default=DEFAULT_FOLDER)
    parser.add_argument("--naga", default="naga")
    options = parser.parse_args(arguments)

    naga = shutil.which(options.naga)
    if naga is None:
        print(f"naga not found ({options.naga}): cargo install naga-cli", file=sys.stderr)
        return 1
    shaders = sorted(options.folder.glob("*.spv"))
    if not shaders:
        print(f"No .spv files in {options.folder}", file=sys.stderr)
        return 1

    failed = 0
    with tempfile.TemporaryDirectory() as work:
        for shader in shaders:
            error = convert(naga, shader, Path(work))
            if error:
                failed += 1
                print(f"FAIL {shader.name}\n{error}")
    print(f"{len(shaders) - failed} of {len(shaders)} shaders convert to WGSL.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
