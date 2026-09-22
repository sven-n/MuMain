#!/usr/bin/env python3
"""Validate payloads, wrapping, atlas guards and allowed scope before installation.

Use --install to copy only the five validated exports into THIS worktree's Data.
Requires Pillow and numpy. No build or shared-runtime operation is performed.
"""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[3]
WORK = ROOT / "assets-work/UI"
LAYOUT = json.loads((WORK / "source/layout.json").read_text())
REPORT = WORK / "validation"
sys.path.insert(0, str(ROOT / "tools"))
sys.dont_write_bytecode = True
import mu_texture  # noqa: E402

JPEG_MAX_ERROR = 5
BUTTON_INTERIOR = (slice(4, 39), slice(2, 28))


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def selected_paths():
    return [LAYOUT["panel"]["path"]] + [f"Interface/partCharge1/{item['file']}"
                                          for item in LAYOUT["buttons"]["icons"]]


def expected_warnings(size):
    width, height = size
    return [f"{width}x{height} is not a power of two; the loader pads to the next power of two "
            "and UVs will not cover the image"]


def verify_states(master):
    means = []
    for state in range(4):
        cell = np.asarray(master)[state * 41:(state + 1) * 41]
        means.append(float(cell[BUTTON_INTERIOR].mean()))
    normal, hover, selected, selected_hover = means
    require(hover > normal > selected and hover > selected_hover > selected,
            "Button state luminance order is not normal/hover/selected/selected-hover")
    return [round(value, 3) for value in means]


def inspect_export(relative):
    path = Path(relative)
    original = WORK / "original" / path.with_suffix(".jpg")
    payload = WORK / "payloads" / path.with_suffix(".jpg")
    export = WORK / "exports" / path
    master = Image.open(WORK / "masters" / path.with_suffix(".png")).convert("RGB")
    before = Image.open(original).convert("RGB")
    wrapped = export.read_bytes()
    decoded = Image.open(io.BytesIO(mu_texture.unwrap_bytes(wrapped, ".ozj")))
    require(decoded.mode == "RGB", f"{path}: RGB JPEG required")
    require(before.size == master.size == decoded.size, f"{path}: dimensions changed")
    require(wrapped == mu_texture.wrap_bytes(payload.read_bytes(), ".ozj"), f"{path}: wrapper differs")
    require(wrapped[:24] == wrapped[24:48], f"{path}: invalid repeated prefix")
    warnings = mu_texture.check_file(export)
    require(warnings == mu_texture.check_file(original) == expected_warnings(before.size),
            f"{path}: unexpected validator warning or rejection")
    mask = np.asarray(Image.open(WORK / "source/masks" / f"{path.stem}.png"))
    old, new = np.asarray(before), np.asarray(master)
    require(np.array_equal(old[mask == 0], new[mask == 0]), f"{path}: protected pixels changed")
    error = np.abs(np.asarray(decoded).astype(int) - new.astype(int))
    require(int(error.max()) <= JPEG_MAX_ERROR, f"{path}: JPEG error exceeds {JPEG_MAX_ERROR}")
    require(decoded.convert("RGBA").getchannel("A").getextrema() == (255, 255), f"{path}: alpha changed")
    require(np.any(old != new), f"{path}: no artwork changed")
    result = {"file": relative, "status": "PASS (baseline NPOT warning preserved)", "size": list(master.size),
              "alpha": [255, 255], "wrapper_roundtrip": "byte-identical", "sha256": sha256(export),
              "protected_pixels": int((mask == 0).sum()), "protected_master_delta": 0,
              "jpeg_max_channel_error": int(error.max()), "jpeg_mean_channel_error": float(error.mean()),
              "changed_master_pixels": int(np.any(old != new, axis=2).sum()), "warnings": warnings}
    if path.stem.startswith("newui_menu_Bt"):
        require(master.size == (30, 164), "Four 30x41 cells required")
        result["state_mean_rgb"] = verify_states(master)
    return result


def verify_originals():
    entries = json.loads((WORK / "inventory/interface.json").read_text())
    hashes = {entry["path"]: entry["container_sha256"] for entry in entries}
    for entry in entries:
        require(sha256(WORK / entry["payload"]) == entry["payload_sha256"],
                f"Untouched original changed: {entry['payload']}")
    for relative in selected_paths():
        container = WORK / "original/containers" / relative
        payload = WORK / "original" / Path(relative).with_suffix(".jpg")
        require(sha256(container) == hashes[f"src/bin/Data/{relative}"], "Original container hash differs")
        require(mu_texture.unwrap_bytes(container.read_bytes(), ".ozj") == payload.read_bytes(),
                "Original payload differs from original container")
    return len(entries)


def verify_scope():
    branch = subprocess.check_output(["git", "branch", "--show-current"], cwd=ROOT, text=True).strip()
    expected_branch = LAYOUT.get("work_branch", "art/ui-pilot")
    require(branch == expected_branch, f"Refusing installation outside {expected_branch}: {branch}")
    require(not (ROOT / "src/bin/Data").is_symlink(), "Source Data must not be a runtime symlink")
    for relative in selected_paths():
        target = ROOT / "src/bin/Data" / relative
        require(target.resolve().is_relative_to(ROOT.resolve()), "Resolved source Data escapes worktree")


def run_tool_checks():
    files = [str((WORK / "exports" / path).relative_to(ROOT)) for path in selected_paths()]
    command = [sys.executable, "tools/mu_texture.py", "check", *files]
    result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
    (REPORT / "mu_texture-check.txt").write_text(result.stdout + result.stderr)
    require(result.returncode == 0, "mu_texture.py rejected an export; not installing")


def install():
    verify_scope()
    for relative in selected_paths():
        target = ROOT / "src/bin/Data" / relative
        shutil.copyfile(WORK / "exports" / relative, target)
        require(sha256(target) == sha256(WORK / "exports" / relative), "Installed file differs")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    REPORT.mkdir(parents=True, exist_ok=True)
    verify_scope()
    originals = verify_originals()
    files = [inspect_export(path) for path in selected_paths()]
    run_tool_checks()
    if args.install:
        install()
    report = {"status": "OFFLINE PASS; CLIENT VERIFICATION PENDING", "original_payloads_verified": originals,
              "installed_in_this_worktree": args.install, "shared_runtime_touched": False,
              "files": files, "limitations": ["No client capture; no GPU execution in this check.",
              "NPOT warnings match shipped originals; no dimension change.",
              "JPEG is lossy: protected pixels are exact in masters, within measured error in exports."]}
    (REPORT / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
