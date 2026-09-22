"""Shared helpers for the Blender scripts in this folder.

These scripts run inside Blender (``blender -b ... --python <script> -- <args>``). They
drive the Blender Source Tools add-on (SMD import/export) and the ``bmdconv`` tool that
converts between SMD and the client's BMD models. See docs/asset-pipeline.md.
"""

from __future__ import annotations

import glob
import os
import shutil
import subprocess
import sys

SOURCE_TOOLS_MODULE = "io_scene_valvesource"
TOOLS_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO_ROOT = os.path.dirname(TOOLS_DIR)


def script_args() -> list[str]:
    """Arguments after ``--`` on the Blender command line."""
    argv = sys.argv
    return argv[argv.index("--") + 1 :] if "--" in argv else []


def find_bmdconv(explicit: str | None) -> str:
    """Locate the bmdconv executable: explicit path, MU_BMDCONV, a build tree, then PATH."""
    candidates = []
    if explicit:
        candidates.append(explicit)
    if os.environ.get("MU_BMDCONV"):
        candidates.append(os.environ["MU_BMDCONV"])
    candidates += sorted(glob.glob(os.path.join(REPO_ROOT, "out", "build", "*", "tools", "bmdconv", "*", "bmdconv*")))
    candidates += sorted(glob.glob(os.path.join(REPO_ROOT, "build*", "tools", "bmdconv", "*", "bmdconv*")))
    for candidate in candidates:
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK) and not candidate.endswith(".cmake"):
            return candidate
    on_path = shutil.which("bmdconv")
    if on_path:
        return on_path
    raise SystemExit(
        "bmdconv not found. Build it (cmake --build --preset macos-arm64-release --target bmdconv) or pass --bmdconv / set MU_BMDCONV."
    )


def run_bmdconv(bmdconv: str, *arguments: str) -> str:
    """Run bmdconv, echo its output and fail loudly on a non-zero exit."""
    command = [bmdconv, *arguments]
    print("+", " ".join(command))
    completed = subprocess.run(command, capture_output=True, text=True)
    if completed.stdout:
        print(completed.stdout, end="")
    if completed.stderr:
        print(completed.stderr, end="", file=sys.stderr)
    if completed.returncode != 0:
        raise SystemExit(f"bmdconv failed with exit code {completed.returncode}")
    return completed.stdout


def ensure_source_tools():
    """Enable Blender Source Tools and return its utils module."""
    import bpy  # noqa: PLC0415 - only available inside Blender

    if SOURCE_TOOLS_MODULE not in bpy.context.preferences.addons:
        try:
            bpy.ops.preferences.addon_enable(module=SOURCE_TOOLS_MODULE)
        except Exception as error:  # noqa: BLE001 - report any add-on failure the same way
            raise SystemExit(
                "Blender Source Tools is not installed. Install the add-on (io_scene_valvesource) first; "
                f"enable failed with: {error}"
            ) from error
    import importlib

    return importlib.import_module(f"{SOURCE_TOOLS_MODULE}.utils")


def sanitise_action_stem(name: str) -> str:
    """Mirror Source Tools' file-name sanitising (SmdExporter.sanitiseFilename) closely enough
    to find the exported file for an action."""
    import re

    return re.sub(r"[/?<>\\:*|\"]", "_", name)


# --- SMD bone order -------------------------------------------------------------------------
# Blender stores bones as a tree and Source Tools writes them depth-first, so an exported SMD
# numbers bones differently from the file that was imported. The game addresses bones by
# index, therefore the importer records the original order and the exporter restores it.

import re as _re

_NODE_LINE = _re.compile(r'^\s*(\d+)\s+"(.*)"\s+(-?\d+)\s*$')


def read_smd_node_names(path: str) -> list[str]:
    """Bone names in node-id order from an SMD file."""
    names: dict[int, str] = {}
    section = None
    with open(path, encoding="utf-8", errors="replace") as smd:
        for line in smd:
            token = line.strip().split(" ", 1)[0] if line.strip() else ""
            if token in ("nodes", "skeleton", "triangles"):
                section = token
                continue
            if token == "end":
                section = None
                continue
            if section == "nodes":
                match = _NODE_LINE.match(line)
                if match:
                    names[int(match.group(1))] = match.group(2)
    return [names[index] for index in sorted(names)]


def reorder_smd_bones(path: str, desired_order: list[str]) -> int:
    """Renumber the bones of an SMD file so that names in ``desired_order`` get indices
    0..n-1 (bones unknown to the list follow in their current order). Rewrites the file in
    place and returns how many bones changed index."""
    with open(path, encoding="utf-8", errors="replace") as smd:
        lines = smd.read().splitlines()

    nodes: dict[int, tuple[str, int]] = {}
    for line in lines:
        match = _NODE_LINE.match(line)
        if match:
            nodes[int(match.group(1))] = (match.group(2), int(match.group(3)))
    if not nodes:
        return 0
    by_name = {name: old for old, (name, _parent) in nodes.items()}
    ordered = [by_name[name] for name in desired_order if name in by_name]
    ordered += [old for old in sorted(nodes) if old not in ordered]
    remap = {old: new for new, old in enumerate(ordered)}
    changed = sum(1 for old, new in remap.items() if old != new)
    if changed == 0:
        return 0

    def remap_parent(parent: int) -> int:
        return remap.get(parent, parent) if parent >= 0 else parent

    output: list[str] = []
    section = None
    frame: list[tuple[int, str]] = []
    node_lines: list[tuple[int, str]] = []

    def flush_frame():
        for _new, text in sorted(frame):
            output.append(text)
        frame.clear()

    triangle_line = 0
    for line in lines:
        stripped = line.strip()
        token = stripped.split(" ", 1)[0] if stripped else ""
        if section == "nodes":
            if token == "end":
                for _new, text in sorted(node_lines):
                    output.append(text)
                node_lines.clear()
                section = None
                output.append(line)
                continue
            match = _NODE_LINE.match(line)
            if match:
                old = int(match.group(1))
                node_lines.append((remap[old], f'{remap[old]} "{match.group(2)}" {remap_parent(int(match.group(3)))}'))
                continue
        if section == "skeleton":
            if token in ("time", "end"):
                flush_frame()
                if token == "end":
                    section = None
                output.append(line)
                continue
            parts = stripped.split()
            if parts and parts[0].isdigit():
                old = int(parts[0])
                frame.append((remap[old], " ".join([str(remap[old])] + parts[1:])))
                continue
        if section == "triangles":
            if token == "end":
                section = None
                output.append(line)
                continue
            parts = stripped.split()
            if len(parts) >= 9 and parts[0].lstrip("-").isdigit():
                parts[0] = str(remap.get(int(parts[0]), int(parts[0])))
                if len(parts) > 9 and parts[9].isdigit():  # weight links: count, then (bone, weight) pairs
                    links = int(parts[9])
                    for link in range(links):
                        index = 10 + link * 2
                        if index < len(parts) and parts[index].isdigit():
                            parts[index] = str(remap.get(int(parts[index]), int(parts[index])))
                output.append(" ".join(parts))
                triangle_line += 1
                continue
        if token in ("nodes", "skeleton", "triangles"):
            section = token
        output.append(line)

    with open(path, "w", encoding="utf-8") as smd:
        smd.write("\n".join(output) + "\n")
    return changed
