#!/usr/bin/env python3
"""
validate_assets.py
==================

Structural validation script for all converted assets in rust/assets/.
Validates PNGs, GLBs, and JSON sidecar files and generates a report.

Usage:
    python3 validate_assets.py \\
        --assets-root rust/assets \\
        --report rust/assets/reports/validation_report.json \\
        --verbose
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import struct
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    from PIL import Image
    HAS_PILLOW = True
except ImportError:
    HAS_PILLOW = False

# ---------------------------------------------------------------------------
# Expected JSON top-level keys per sidecar type
# ---------------------------------------------------------------------------

EXPECTED_JSON_KEYS: Dict[str, List[str]] = {
    "terrain_height.json": ["width", "height"],
    "terrain_attributes.json": ["width", "height"],
    "terrain_config.json": [],
    "camera_tour.json": [],
    "scene_objects.json": [],
}


# ---------------------------------------------------------------------------
# Validation functions
# ---------------------------------------------------------------------------

def validate_png(path: Path) -> Tuple[bool, str]:
    """Validate a PNG file: opens with Pillow, has non-zero dimensions."""
    if not HAS_PILLOW:
        # Fallback: check PNG signature
        try:
            header = path.read_bytes()[:8]
        except OSError as exc:
            return False, f"cannot read: {exc}"
        if header[:4] != b'\x89PNG':
            return False, "invalid PNG signature"
        return True, "ok (no Pillow)"

    try:
        with Image.open(path) as img:
            w, h = img.size
            if w <= 0 or h <= 0:
                return False, f"zero dimensions ({w}x{h})"
            img.verify()
        return True, "ok"
    except Exception as exc:
        return False, f"invalid PNG: {exc}"


def validate_glb(path: Path) -> Tuple[bool, str]:
    """Validate a GLB file: size >= 128, valid glTF header magic."""
    try:
        size = path.stat().st_size
    except OSError as exc:
        return False, f"cannot stat: {exc}"

    if size < 128:
        return False, f"too small ({size} bytes)"

    try:
        with open(path, 'rb') as f:
            header = f.read(12)
    except OSError as exc:
        return False, f"cannot read: {exc}"

    if len(header) < 12:
        return False, f"header too short ({len(header)} bytes)"

    magic, version, total_len = struct.unpack_from('<III', header, 0)
    if magic != 0x46546C67:  # "glTF"
        return False, f"invalid magic: 0x{magic:08X}"
    if version != 2:
        return False, f"unsupported version: {version}"

    return True, "ok"


def validate_gltf(path: Path) -> Tuple[bool, str]:
    """Validate a GLTF file: valid JSON, has meshes/buffers/accessors/bufferViews."""
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return False, f"invalid JSON: {exc}"

    meshes = payload.get("meshes", [])
    buffers = payload.get("buffers", [])
    accessors = payload.get("accessors", [])
    buffer_views = payload.get("bufferViews", [])

    if not isinstance(meshes, list) or len(meshes) == 0:
        return False, f"no meshes (found {type(meshes).__name__})"
    if not isinstance(buffers, list) or len(buffers) == 0:
        return False, "no buffers"
    if not isinstance(accessors, list) or len(accessors) == 0:
        return False, "no accessors"
    if not isinstance(buffer_views, list) or len(buffer_views) == 0:
        return False, "no bufferViews"

    # Check that at least one primitive has a POSITION attribute
    has_position = False
    for mesh in meshes:
        if not isinstance(mesh, dict):
            continue
        for prim in mesh.get("primitives", []):
            if not isinstance(prim, dict):
                continue
            attrs = prim.get("attributes", {})
            if isinstance(attrs, dict) and "POSITION" in attrs:
                has_position = True
                break
        if has_position:
            break

    if not has_position:
        return False, "no POSITION attribute in any primitive"

    # Check buffer URIs resolve to non-empty files
    for buf in buffers:
        if not isinstance(buf, dict):
            continue
        uri = buf.get("uri")
        if isinstance(uri, str) and not uri.startswith("data:"):
            bin_path = path.parent / uri
            if not bin_path.exists():
                return False, f"missing buffer: {uri}"
            if bin_path.stat().st_size == 0:
                return False, f"empty buffer: {uri}"

    return True, "ok"


def validate_terrain_map_payload(payload: dict) -> Tuple[bool, str]:
    if "width" in payload and "height" in payload:
        return True, "ok"
    if "terrain_size" in payload:
        return True, "ok"
    return False, "missing expected key: width/height or terrain_size"


def validate_expected_json_keys(fname: str, payload: dict) -> Tuple[bool, str]:
    if fname == "terrain_map.json":
        return validate_terrain_map_payload(payload)

    expected = EXPECTED_JSON_KEYS.get(fname)
    if not expected:
        return True, "ok"

    for key in expected:
        if key not in payload:
            return False, f"missing expected key: {key}"
    return True, "ok"


def validate_json_sidecar(path: Path) -> Tuple[bool, str]:
    """Validate a JSON sidecar file: valid JSON, expected top-level keys present."""
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return False, f"invalid JSON: {exc}"

    if not isinstance(payload, (dict, list)):
        return False, f"unexpected root type: {type(payload).__name__}"

    fname = path.name.lower()
    if isinstance(payload, dict):
        ok, reason = validate_expected_json_keys(fname, payload)
        if not ok:
            return False, reason

    return True, "ok"


# ---------------------------------------------------------------------------
# Batch validation
# ---------------------------------------------------------------------------

@dataclass
class ValidationStats:
    total: int = 0
    passed: int = 0
    failed: int = 0
    by_type: Dict[str, Dict[str, int]] = field(default_factory=lambda: {})
    failures: List[Dict] = field(default_factory=list)


def validate_all(
    assets_root: Path,
    report_path: Optional[Path],
    verbose: bool,
) -> ValidationStats:
    stats = ValidationStats()

    for dirpath, _dirnames, filenames in os.walk(assets_root):
        for fname in filenames:
            fpath = Path(dirpath) / fname
            suffix = fpath.suffix.lower()

            ok = False
            reason = "unknown type"
            ftype = "other"

            if suffix == ".png":
                ftype = "png"
                ok, reason = validate_png(fpath)
            elif suffix == ".glb":
                ftype = "glb"
                ok, reason = validate_glb(fpath)
            elif suffix == ".gltf":
                ftype = "gltf"
                ok, reason = validate_gltf(fpath)
            elif suffix == ".json":
                ftype = "json"
                ok, reason = validate_json_sidecar(fpath)
            else:
                continue  # skip non-asset files

            stats.total += 1
            if ftype not in stats.by_type:
                stats.by_type[ftype] = {"total": 0, "passed": 0, "failed": 0}
            stats.by_type[ftype]["total"] += 1

            if ok:
                stats.passed += 1
                stats.by_type[ftype]["passed"] += 1
            else:
                stats.failed += 1
                stats.by_type[ftype]["failed"] += 1
                stats.failures.append({
                    "path": str(fpath.relative_to(assets_root)),
                    "type": ftype,
                    "reason": reason,
                })
                logging.warning("FAIL: %s — %s", fpath.relative_to(assets_root), reason)

    logging.info(
        "Validation: %d total, %d passed, %d failed",
        stats.total, stats.passed, stats.failed,
    )
    for ftype, counts in sorted(stats.by_type.items()):
        logging.info(
            "  %s: %d total, %d passed, %d failed",
            ftype, counts["total"], counts["passed"], counts["failed"],
        )

    if report_path:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report = {
            "total": stats.total,
            "passed": stats.passed,
            "failed": stats.failed,
            "by_type": stats.by_type,
            "failures": stats.failures,
        }
        report_path.write_text(json.dumps(report, indent=2))
        logging.info("Report written to %s", report_path)

    return stats


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate all converted MU Online assets."
    )
    parser.add_argument(
        "--assets-root", type=Path, default=Path("rust/assets"),
        help="Root directory of converted assets",
    )
    parser.add_argument(
        "--report", type=Path, default=None,
        help="Path for JSON validation report",
    )
    parser.add_argument("--verbose", action="store_true", help="Verbose output")

    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%H:%M:%S",
    )

    if not args.assets_root.is_dir():
        logging.error("Assets root not found: %s", args.assets_root)
        return 1

    stats = validate_all(
        assets_root=args.assets_root,
        report_path=args.report,
        verbose=args.verbose,
    )

    if stats.failed > 0:
        logging.warning("%d files failed validation", stats.failed)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
