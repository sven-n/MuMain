"""Check every wrapped asset, shared gauge seam and retained input before install."""

from __future__ import annotations

import argparse
import io
import json
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

import numpy as np
from PIL import Image

from common import (ASSETS, BASE, BRANCH, CASH_PATH, GAUGES, PANEL_PATHS, ROOT,
                    UI_WORK, WORK, digest, mu_texture, write_json)
from paint_parts import LAYOUT
from preview_hud import hud

REPORT = WORK / "validation"
MAX_JPEG_ERROR = 5
LOW_FILL_RATIO = 0.1
VISIBLE_CHANNEL_DELTA = 16
MIN_VISIBLE_PIXELS = 16
HUE_RANGES = {"red": (0, 15), "blue": (135, 170), "SD": (20, 60), "AG": (178, 215)}
MIN_ENERGY_SATURATION = 90
MIN_ENERGY_VALUE = 45


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def master(relative):
    return Image.open((WORK / "masters" / relative).with_suffix(".png")).convert("RGB")


def verify_scope():
    branch = subprocess.check_output(["git", "branch", "--show-current"], cwd=ROOT, text=True).strip()
    require(branch == BRANCH, f"Work must stay on {BRANCH}; found {branch}")
    data = ROOT / "src/bin/Data"
    require(not data.is_symlink(), "Source Data must not be a runtime symlink")
    changed = subprocess.check_output(["git", "diff", "--name-only", BASE, "--", "src"], cwd=ROOT, text=True)
    allowed = {f"src/bin/Data/{path}" for path in ASSETS}
    require(set(changed.splitlines()) <= allowed, "Source changes exceed the 14 declared HUD files")
    for relative in ASSETS:
        require((data / relative).resolve().is_relative_to(ROOT.resolve()), "Data path escapes worktree")


def verify_originals(entries):
    require({entry["path"] for entry in entries} == set(ASSETS), "Inventory differs from asset claim")
    for entry in entries:
        path = Path(entry["path"])
        container = WORK / "original/containers" / path
        payload = WORK / "original" / path.with_suffix(".jpg")
        require(digest(container) == entry["container_sha256"], f"Retained container changed: {path}")
        require(digest(payload) == entry["payload_sha256"], f"Retained payload changed: {path}")
        require(mu_texture.unwrap_bytes(container.read_bytes(), ".ozj") == payload.read_bytes(), str(path))
    full_inventory = json.loads((UI_WORK / "inventory/interface.json").read_text())
    for entry in full_inventory:
        require(digest(UI_WORK / entry["payload"]) == entry["payload_sha256"], "Original UI payload changed")
    return len(full_inventory)


def verify_editable(relative, expected):
    path = (WORK / "source/editable" / relative).with_suffix(".ora")
    with zipfile.ZipFile(path) as archive:
        old = Image.open(io.BytesIO(archive.read("data/original.png"))).convert("RGBA")
        paint = Image.open(io.BytesIO(archive.read("data/paint.png"))).convert("RGBA")
        merged = Image.alpha_composite(old, paint).convert("RGB")
    require(merged.size == expected.size and merged.tobytes() == expected.tobytes(),
            f"Editable layer composite differs: {relative}")


def verify_export(entry):
    relative = entry["path"]
    path = WORK / "exports" / relative
    payload = (WORK / "payloads" / relative).with_suffix(".jpg")
    raw = path.read_bytes()
    image = Image.open(io.BytesIO(mu_texture.unwrap_bytes(raw, ".ozj")))
    lossless = master(relative)
    require(image.mode == "RGB" and list(image.size) == entry["size"], f"Format or dimensions changed: {relative}")
    require(lossless.size == image.size, f"Master size differs: {relative}")
    require(raw == mu_texture.wrap_bytes(payload.read_bytes(), ".ozj"), f"Wrapper mismatch: {relative}")
    require(raw[:24] == raw[24:48], f"Invalid repeated OZJ prefix: {relative}")
    warnings = mu_texture.check_file(path)
    require(warnings == entry["baseline_warnings"], f"New validator warning: {relative}")
    require(image.convert("RGBA").getchannel("A").getextrema() == (255, 255), f"Alpha changed: {relative}")
    error = np.abs(np.asarray(image).astype(int) - np.asarray(lossless).astype(int))
    require(int(error.max()) <= MAX_JPEG_ERROR, f"JPEG error exceeds budget: {relative}")
    require(digest(path) != entry["container_sha256"], f"No artwork changed: {relative}")
    verify_editable(relative, lossless)
    return {"file": relative, "status": "PASS", "size": image.size, "mode": image.mode,
            "alpha": [255, 255], "sha256": digest(path), "jpeg_max_channel_error": int(error.max()),
            "wrapper_roundtrip": "byte-identical", "editable_composite": "exact", "warnings": warnings}


def verify_gauge_backing():
    left, right = master(PANEL_PATHS[0]), master(PANEL_PATHS[2])
    for key, panel, offset, part in (("life", left, 0, "gem"), ("mana", right, 384, "gem"),
                                    ("shield", left, 0, "column"), ("ability", right, 384, "column")):
        x, y = LAYOUT[key]["position"]
        width, height = LAYOUT[key]["size"]
        crop = panel.crop((x - offset, y, x - offset + width, y + height))
        empty = Image.open(WORK / f"source/{part}-empty.png").convert("RGB")
        require(crop.tobytes() == empty.tobytes(), f"Empty gauge backing mismatch: {key}")
    for part, names in (("gem", ("red", "green", "blue")), ("column", ("SD", "AG"))):
        empty = np.asarray(Image.open(WORK / f"source/{part}-empty.png"))
        mask = np.asarray(Image.open(WORK / f"source/{part}-energy-mask.png"))
        for name in names:
            filled = np.asarray(master(GAUGES[name]["path"]))
            require(np.array_equal(empty[mask == 0], filled[mask == 0]), f"Gauge frame shifts: {name}")
    return "PASS: four empty backings and five unchanged frame masks match exactly in masters"


def verify_low_fills():
    before = hud(True, level=0)
    after = hud(True, level=LOW_FILL_RATIO)
    results = {}
    for name in ("red", "blue", "SD", "AG"):
        item = GAUGES[name]
        x, y = item["position"]
        width, height = item["size"]
        # Existing classic 1080p transform: scale=2, xOffset=320, yOffset=120.
        rectangle = (x * 2 + 320, y * 2 + 120, (x + width) * 2 + 320, (y + height) * 2 + 120)
        current = after.crop(rectangle)
        delta = np.abs(np.asarray(before.crop(rectangle)).astype(int) - np.asarray(current).astype(int))
        hsv = np.asarray(current.convert("HSV"))
        low, high = HUE_RANGES[name]
        energy = ((hsv[..., 0] >= low) & (hsv[..., 0] <= high)
                  & (hsv[..., 1] >= MIN_ENERGY_SATURATION) & (hsv[..., 2] >= MIN_ENERGY_VALUE))
        visible = int((energy & (delta.max(axis=2) > VISIBLE_CHANNEL_DELTA)).sum())
        require(visible >= MIN_VISIBLE_PIXELS, f"10% fill is not visible: {name} ({visible} pixels)")
        results[name] = visible
    return results


def verify_cash_states():
    image = np.asarray(master(CASH_PATH))
    means = [float(image[y + 4:y + 39, 2:28].mean()) for y in LAYOUT["cash"]["state_y"]]
    normal, hover, selected, selected_hover = means
    require(hover > normal > selected and hover > selected_hover > selected, "Cash state contrast order failed")
    return [round(value, 3) for value in means]


def run_loader_checks():
    files = [str(WORK / "exports" / path) for path in ASSETS]
    result = subprocess.run([sys.executable, "tools/mu_texture.py", "check", *files],
                            cwd=ROOT, text=True, capture_output=True)
    (REPORT / "mu_texture-check.txt").write_text(result.stdout + result.stderr)
    require(result.returncode == 0, "Loader validator rejected exports; installation stopped")


def verify_reassembly():
    files = [path for directory in ("masters", "payloads", "exports")
             for path in sorted((WORK / directory).rglob("*")) if path.is_file()]
    before = {str(path.relative_to(WORK)): digest(path) for path in files}
    subprocess.run([sys.executable, str(WORK / "scripts/assemble_hud.py")], check=True)
    after = {str(path.relative_to(WORK)): digest(path) for path in files}
    require(before == after, "Reassembly changed deterministic master/payload/export bytes")
    write_json(REPORT / "reproducibility.json", {"status": "PASS", "identical_files": len(files), "hashes": after})
    return len(files)


def install(entries):
    for entry in entries:
        path = entry["path"]
        target, export = ROOT / "src/bin/Data" / path, WORK / "exports" / path
        require(digest(target) in (entry["container_sha256"], digest(export)),
                f"Source Data has an unrecognized edit: {path}")
    for entry in entries:
        path = entry["path"]
        target, export = ROOT / "src/bin/Data" / path, WORK / "exports" / path
        shutil.copyfile(export, target)
        require(digest(target) == digest(export), f"Installed bytes differ: {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install", action="store_true")
    parser.add_argument("--rebuild", action="store_true")
    args = parser.parse_args()
    REPORT.mkdir(parents=True, exist_ok=True)
    verify_scope()
    entries = json.loads((WORK / "inventory/assets.json").read_text())["assets"]
    originals = verify_originals(entries)
    repeated = verify_reassembly() if args.rebuild else None
    files = [verify_export(entry) for entry in entries]
    run_loader_checks()
    gauges, fills, states = verify_gauge_backing(), verify_low_fills(), verify_cash_states()
    if args.install:
        install(entries)
    verify_scope()
    report = {"status": "OFFLINE PASS; CLIENT REVIEW PENDING", "base": BASE, "branch": BRANCH,
              "files": files, "original_payloads_verified": originals, "baseline_containers_verified": len(entries),
              "gauge_backing": gauges, "10_percent_fill_visible_energy_pixels_at_1080p": fills,
              "cash_state_mean_rgb": states, "reassembled_identical_files": repeated,
              "installed_in_this_worktree": args.install, "shared_runtime_touched": False,
              "limitations": ["No GPU or client execution.", "NPOT warnings match originals.",
                              "Fixed source sizes retained; this is not a resolution increase."]}
    write_json(REPORT / "report.json", report)
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
