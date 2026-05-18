#!/usr/bin/env python3
"""
bmd_converter.py
================

Pure Python converter for MU Online BMD 3D model files to GLTF Binary (GLB) format.

The BMD format has been reverse-engineered from:
  - cpp/MuClient5.2/source/ZzzBMD.cpp (Open/Open2 parsing)
  - cpp/MuClient5.2/source/ZzzBMD.h (struct definitions)
  - cpp/MuClientTools16/_src_/Core/BMD_SMD.cpp (FixUpBones, Bmd2Smd)
  - cpp/MuClientTools16/_src_/Core/BMD.h (struct definitions)

Usage:
    python3 bmd_converter.py \\
        --bmd-root cpp/MuClient5.2/bin/Data \\
        --output-root rust/assets/data \\
        --format glb \\
        --force --verbose \\
        --report rust/assets/reports/models_report.json
"""

from __future__ import annotations

import argparse
import json
import logging
import math
import os
import re
import struct
import subprocess
import sys
import tempfile
import time
from collections import Counter
from concurrent.futures import ProcessPoolExecutor
from dataclasses import dataclass, field, fields as dataclass_fields
from functools import partial
from pathlib import Path, PurePosixPath
from typing import Dict, List, Optional, Sequence, Tuple


def _image_temp_suffix_from_bytes(image_bytes: bytes) -> str:
    if image_bytes.startswith(b"\x89PNG\r\n\x1a\n"):
        return ".png"
    if image_bytes.startswith(b"\xff\xd8\xff"):
        return ".jpg"
    if image_bytes.startswith(b"BM"):
        return ".bmp"
    # Most MU legacy textures decoded from .OZT are TGA payloads.
    return ".tga"


def _image_payload_may_have_alpha(image_bytes: bytes) -> bool:
    suffix = _image_temp_suffix_from_bytes(image_bytes)
    # Legacy additive textures frequently come from OZJ/JPG payloads; those
    # formats do not carry alpha and must not be classified via fake alpha.
    return suffix in {".png", ".tga"}


def _parse_magick_gray_histogram(histogram_output: str) -> Tuple[int, Dict[int, int]]:
    bins: Dict[int, int] = {}
    total_pixels = 0
    for line in histogram_output.splitlines():
        entry = line.strip()
        if not entry or ":" not in entry:
            continue

        count_token, _ = entry.split(":", 1)
        try:
            count = int(count_token.strip())
        except ValueError:
            continue

        gray_match = re.search(
            r"gray(?:a)?\(([-+]?\d+(?:\.\d+)?)(?:,[-+]?\d+(?:\.\d+)?)?\)",
            entry,
            re.IGNORECASE,
        )
        if gray_match is None:
            continue

        gray_value = int(round(float(gray_match.group(1))))
        gray_value = max(0, min(255, gray_value))
        bins[gray_value] = bins.get(gray_value, 0) + count
        total_pixels += count
    return total_pixels, bins


def _png_alpha_profile(png_bytes: bytes) -> Tuple[bool, bool, float, float]:
    """Return (has_alpha, has_partial_alpha, transparent_ratio, opaque_ratio)."""
    try:
        import io
        from PIL import Image

        img = Image.open(io.BytesIO(png_bytes))
        img.load()

        if img.mode in ("RGBA", "LA", "PA"):
            alpha = img.getchannel("A")
        elif img.mode == "P" and "transparency" in img.info:
            # Palettized PNG with tRNS transparency.
            alpha = img.convert("RGBA").getchannel("A")
        else:
            return False, False, 0.0, 1.0

        extrema = alpha.getextrema()
        if extrema is None:
            return False, False, 0.0, 1.0
        if extrema[0] >= 255:
            return False, False, 0.0, 1.0

        histogram = alpha.histogram()
        total_pixels = sum(histogram)
        if total_pixels <= 0:
            return False, False, 0.0, 1.0
        transparent_ratio = histogram[0] / total_pixels
        opaque_ratio = histogram[255] / total_pixels
        has_partial_alpha = any(histogram[value] > 0 for value in range(1, 255))
        return True, has_partial_alpha, transparent_ratio, opaque_ratio
    except Exception:
        pass

    if not _image_payload_may_have_alpha(png_bytes):
        return False, False, 0.0, 1.0

    # Fallback for environments without Pillow (or unsupported payloads):
    # ask ImageMagick for alpha channel data only when alpha truly exists.
    temp_input_path: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            suffix=_image_temp_suffix_from_bytes(png_bytes),
            delete=False,
        ) as input_tmp:
            input_tmp.write(png_bytes)
            temp_input_path = Path(input_tmp.name)

        histogram_output = subprocess.check_output(
            [
                "magick",
                "identify",
                "-format",
                "%[channels]",
                str(temp_input_path),
            ],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
        if "a" not in histogram_output.lower():
            return False, False, 0.0, 1.0

        histogram_output = subprocess.check_output(
            [
                "magick",
                str(temp_input_path),
                "-channel",
                "a",
                "-separate",
                "-define",
                "histogram:unique-colors=true",
                "-format",
                "%c",
                "histogram:info:-",
            ],
            stderr=subprocess.DEVNULL,
            text=True,
        )

        total_pixels, bins = _parse_magick_gray_histogram(histogram_output)

        if total_pixels <= 0:
            return False, False, 0.0, 1.0

        transparent_pixels = bins.get(0, 0)
        opaque_pixels = bins.get(255, 0)
        has_partial_alpha = any(count > 0 for value, count in bins.items() if 0 < value < 255)
        transparent_ratio = transparent_pixels / total_pixels
        opaque_ratio = opaque_pixels / total_pixels
        has_alpha = transparent_pixels > 0 or has_partial_alpha
        return has_alpha, has_partial_alpha, transparent_ratio, opaque_ratio
    except Exception:
        return False, False, 0.0, 1.0
    finally:
        if temp_input_path is not None:
            temp_input_path.unlink(missing_ok=True)


# Conservative thresholds to avoid turning cutout/opaque materials into full blend.
ALPHA_BLEND_PARTIAL_MIN_RATIO = 0.35
ALPHA_BLEND_OPAQUE_MAX_RATIO = 0.20
ALPHA_BLEND_TRANSPARENT_MIN_RATIO = 0.02
ALPHA_MASK_PARTIAL_MIN_RATIO = 0.10
ALPHA_MASK_CUTOFF = 0.35

# Legacy additive inference thresholds for textures that do not expose alpha.
LEGACY_ADDITIVE_BLACK_LUMA_MAX = 0.06
LEGACY_ADDITIVE_VERY_DARK_LUMA_MAX = 0.12
LEGACY_ADDITIVE_BRIGHT_LUMA_MIN = 0.75
LEGACY_ADDITIVE_BRIGHT_ENERGY_LUMA_MIN = 0.46
LEGACY_ADDITIVE_INTENSITY_DEFAULT = 1.00
LEGACY_ADDITIVE_INTENSITY_BOOST = 1.20
LEGACY_ADDITIVE_INTENSITY_BOOST_BRIGHT_RATIO_MIN = 0.12
LEGACY_ADDITIVE_INTENSITY_MIN = 0.60
LEGACY_ADDITIVE_INTENSITY_MAX = 1.40

# Legacy additive score-based inference.
LEGACY_ADDITIVE_MEAN_LUMA_SOFT_MAX = 0.46
LEGACY_ADDITIVE_LUMA_STDDEV_MIN = 0.10
LEGACY_ADDITIVE_LUMA_STDDEV_MAX = 0.45
LEGACY_ADDITIVE_BRIGHT_ENERGY_RATIO_MIN = 0.10
LEGACY_ADDITIVE_HINT_BOOST_PER_MATCH = 0.06
LEGACY_ADDITIVE_HINT_BOOST_MAX = 0.18
LEGACY_ADDITIVE_SCORE_MIN = 0.40
LEGACY_ADDITIVE_SCORE_BLACK_WEIGHT = 0.33
LEGACY_ADDITIVE_SCORE_VERY_DARK_WEIGHT = 0.17
LEGACY_ADDITIVE_SCORE_BRIGHT_WEIGHT = 0.12
LEGACY_ADDITIVE_SCORE_DARK_MEAN_WEIGHT = 0.22
LEGACY_ADDITIVE_SCORE_LUMA_STDDEV_WEIGHT = 0.08
LEGACY_ADDITIVE_SCORE_BRIGHT_ENERGY_WEIGHT = 0.13
LEGACY_ADDITIVE_SCORE_ALPHA_PENALTY = 0.70
LEGACY_ADDITIVE_SCORE_LOW_BRIGHT_RATIO_MIN = 0.006
LEGACY_ADDITIVE_SCORE_LOW_BRIGHT_PENALTY = 0.18
LEGACY_ADDITIVE_SCORE_HIGH_LUMA_PENALTY_MIN = 0.58
LEGACY_ADDITIVE_SCORE_HIGH_LUMA_PENALTY = 0.12
LEGACY_ADDITIVE_SCORE_DARK_RATIO_MIN = 0.16
LEGACY_ADDITIVE_SCORE_VERY_DARK_RATIO_MIN = 0.24
LEGACY_ADDITIVE_SCORE_BRIGHT_SIGNAL_MIN = 0.015
LEGACY_ADDITIVE_SCORE_BRIGHT_ENERGY_SIGNAL_MIN = 0.12
LEGACY_ADDITIVE_SCORE_HINT_SIGNAL_MIN = 0.06
LEGACY_ADDITIVE_SCORE_HINT_SIGNAL_STRONG_MIN = 0.12

LEGACY_ADDITIVE_HINT_KEYWORDS: Tuple[str, ...] = (
    "glow",
    "flare",
    "spark",
    "fire",
    "smoke",
    "trail",
    "aura",
    "halo",
    "effect",
    "fx",
    "energy",
    "beam",
    "light",
    "shine",
    "flash",
    "particle",
    "laser",
)


def _clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


def _legacy_additive_hint_boost(hint_text: str) -> Tuple[float, str]:
    if not hint_text:
        return 0.0, ""

    normalized = hint_text.lower()
    matched: List[str] = []
    for keyword in LEGACY_ADDITIVE_HINT_KEYWORDS:
        if keyword in normalized:
            matched.append(keyword)

    if not matched:
        return 0.0, ""

    boost = min(
        LEGACY_ADDITIVE_HINT_BOOST_MAX,
        len(matched) * LEGACY_ADDITIVE_HINT_BOOST_PER_MATCH,
    )
    detail = f"hint_keywords={','.join(sorted(set(matched))[:4])}"
    return boost, detail


def _texture_visual_profile_from_histogram(
    histogram: Sequence[int],
) -> Tuple[float, float, float, float, float, float]:
    """Return (black_ratio, very_dark_ratio, bright_ratio, mean_luma, luma_std_dev, bright_energy_ratio)."""
    total_pixels = sum(histogram)
    if total_pixels <= 0:
        return 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

    black_bin_max = max(0, min(255, int(round(LEGACY_ADDITIVE_BLACK_LUMA_MAX * 255.0))))
    very_dark_bin_max = max(
        0,
        min(255, int(round(LEGACY_ADDITIVE_VERY_DARK_LUMA_MAX * 255.0))),
    )
    bright_bin_min = max(0, min(255, int(round(LEGACY_ADDITIVE_BRIGHT_LUMA_MIN * 255.0))))
    bright_energy_bin_min = max(
        0,
        min(255, int(round(LEGACY_ADDITIVE_BRIGHT_ENERGY_LUMA_MIN * 255.0))),
    )

    black_pixels = sum(histogram[: black_bin_max + 1])
    very_dark_pixels = sum(histogram[: very_dark_bin_max + 1])
    bright_pixels = sum(histogram[bright_bin_min:])
    weighted_sum = sum(value * count for value, count in enumerate(histogram))
    mean_luma = weighted_sum / (255.0 * total_pixels)

    variance = 0.0
    if total_pixels > 0:
        variance = sum(
            (((value / 255.0) - mean_luma) ** 2) * count
            for value, count in enumerate(histogram)
        ) / total_pixels
    luma_std_dev = math.sqrt(max(0.0, variance))

    bright_weighted_sum = sum(
        value * count for value, count in enumerate(histogram) if value >= bright_energy_bin_min
    )
    bright_energy_ratio = (bright_weighted_sum / weighted_sum) if weighted_sum > 0 else 0.0

    return (
        black_pixels / total_pixels,
        very_dark_pixels / total_pixels,
        bright_pixels / total_pixels,
        mean_luma,
        luma_std_dev,
        bright_energy_ratio,
    )


def _texture_visual_profile(image_bytes: bytes) -> Tuple[float, float, float, float, float, float]:
    """Return (black_ratio, very_dark_ratio, bright_ratio, mean_luma, luma_std_dev, bright_energy_ratio)."""

    try:
        import io
        from PIL import Image

        img = Image.open(io.BytesIO(image_bytes))
        img.load()
        gray = img.convert("RGB").convert("L")
        histogram = gray.histogram()
        return _texture_visual_profile_from_histogram(histogram)
    except Exception:
        pass

    temp_input_path: Optional[Path] = None
    try:
        with tempfile.NamedTemporaryFile(
            suffix=_image_temp_suffix_from_bytes(image_bytes),
            delete=False,
        ) as input_tmp:
            input_tmp.write(image_bytes)
            temp_input_path = Path(input_tmp.name)

        histogram_output = subprocess.check_output(
            [
                "magick",
                str(temp_input_path),
                "-colorspace",
                "Gray",
                "-depth",
                "8",
                "-format",
                "%c",
                "histogram:info:-",
            ],
            stderr=subprocess.DEVNULL,
            text=True,
        )
        total_pixels, bins = _parse_magick_gray_histogram(histogram_output)
        if total_pixels <= 0:
            return 0.0, 0.0, 0.0, 0.0, 0.0, 0.0

        histogram = [0] * 256
        for value, count in bins.items():
            histogram[value] = count
        return _texture_visual_profile_from_histogram(histogram)
    except Exception:
        return 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    finally:
        if temp_input_path is not None:
            temp_input_path.unlink(missing_ok=True)


def _legacy_additive_intensity_from_signal(bright_ratio: float) -> float:
    intensity = LEGACY_ADDITIVE_INTENSITY_DEFAULT
    if bright_ratio > LEGACY_ADDITIVE_INTENSITY_BOOST_BRIGHT_RATIO_MIN:
        intensity = LEGACY_ADDITIVE_INTENSITY_BOOST
    return max(
        LEGACY_ADDITIVE_INTENSITY_MIN,
        min(LEGACY_ADDITIVE_INTENSITY_MAX, intensity),
    )


def _legacy_additive_confidence_from_signal(
    additive_score: float,
    bright_ratio: float,
    bright_energy_ratio: float,
    hint_boost: float,
) -> float:
    bright_score = _clamp01(bright_ratio / max(LEGACY_ADDITIVE_INTENSITY_BOOST_BRIGHT_RATIO_MIN, 1e-6))
    bright_energy_score = _clamp01(bright_energy_ratio / 0.45)
    hint_score = _clamp01(hint_boost / max(LEGACY_ADDITIVE_HINT_BOOST_MAX, 1e-6))
    return _clamp01(
        additive_score * 0.75
        + bright_score * 0.10
        + bright_energy_score * 0.10
        + hint_score * 0.05
    )


def _infer_legacy_additive_from_visual_signal(
    has_alpha: bool,
    black_ratio: float,
    very_dark_ratio: float,
    bright_ratio: float,
    mean_luma: float,
    luma_std_dev: float,
    bright_energy_ratio: float,
    hint_text: str = "",
) -> Optional[Tuple[float, float, str]]:
    hint_boost, hint_detail = _legacy_additive_hint_boost(hint_text)
    dark_mean_score = _clamp01(
        (LEGACY_ADDITIVE_MEAN_LUMA_SOFT_MAX - mean_luma)
        / max(LEGACY_ADDITIVE_MEAN_LUMA_SOFT_MAX, 1e-6)
    )
    variance_score = _clamp01(
        (luma_std_dev - LEGACY_ADDITIVE_LUMA_STDDEV_MIN)
        / max(LEGACY_ADDITIVE_LUMA_STDDEV_MAX - LEGACY_ADDITIVE_LUMA_STDDEV_MIN, 1e-6)
    )
    bright_energy_score = _clamp01(
        (bright_energy_ratio - LEGACY_ADDITIVE_BRIGHT_ENERGY_RATIO_MIN)
        / max(1.0 - LEGACY_ADDITIVE_BRIGHT_ENERGY_RATIO_MIN, 1e-6)
    )

    additive_score = (
        black_ratio * LEGACY_ADDITIVE_SCORE_BLACK_WEIGHT
        + very_dark_ratio * LEGACY_ADDITIVE_SCORE_VERY_DARK_WEIGHT
        + bright_ratio * LEGACY_ADDITIVE_SCORE_BRIGHT_WEIGHT
        + dark_mean_score * LEGACY_ADDITIVE_SCORE_DARK_MEAN_WEIGHT
        + variance_score * LEGACY_ADDITIVE_SCORE_LUMA_STDDEV_WEIGHT
        + bright_energy_score * LEGACY_ADDITIVE_SCORE_BRIGHT_ENERGY_WEIGHT
        + hint_boost
    )

    if has_alpha:
        additive_score -= LEGACY_ADDITIVE_SCORE_ALPHA_PENALTY
    if bright_ratio < LEGACY_ADDITIVE_SCORE_LOW_BRIGHT_RATIO_MIN:
        additive_score -= LEGACY_ADDITIVE_SCORE_LOW_BRIGHT_PENALTY
    if mean_luma > LEGACY_ADDITIVE_SCORE_HIGH_LUMA_PENALTY_MIN:
        additive_score -= LEGACY_ADDITIVE_SCORE_HIGH_LUMA_PENALTY

    additive_score = _clamp01(additive_score)
    has_dark_signal = (
        black_ratio >= LEGACY_ADDITIVE_SCORE_DARK_RATIO_MIN
        or very_dark_ratio >= LEGACY_ADDITIVE_SCORE_VERY_DARK_RATIO_MIN
        or hint_boost >= LEGACY_ADDITIVE_SCORE_HINT_SIGNAL_MIN
    )
    has_energy_signal = (
        bright_ratio >= LEGACY_ADDITIVE_SCORE_BRIGHT_SIGNAL_MIN
        or bright_energy_ratio >= LEGACY_ADDITIVE_SCORE_BRIGHT_ENERGY_SIGNAL_MIN
        or hint_boost >= LEGACY_ADDITIVE_SCORE_HINT_SIGNAL_STRONG_MIN
    )

    if not has_alpha and has_dark_signal and has_energy_signal and additive_score >= LEGACY_ADDITIVE_SCORE_MIN:
        intensity = _legacy_additive_intensity_from_signal(bright_ratio)
        confidence = _legacy_additive_confidence_from_signal(
            additive_score=additive_score,
            bright_ratio=bright_ratio,
            bright_energy_ratio=bright_energy_ratio,
            hint_boost=hint_boost,
        )
        detail = (
            f"score={additive_score:.3f} black_ratio={black_ratio:.3f} "
            f"very_dark_ratio={very_dark_ratio:.3f} bright_ratio={bright_ratio:.3f} "
            f"bright_energy_ratio={bright_energy_ratio:.3f} mean_luma={mean_luma:.3f} "
            f"luma_std_dev={luma_std_dev:.3f}"
        )
        if hint_detail:
            detail = f"{detail} {hint_detail}"
        return intensity, confidence, detail
    return None


def _classify_alpha_mode(
    has_alpha: bool,
    has_partial_alpha: bool,
    transparent_ratio: float,
    opaque_ratio: float,
) -> Optional[str]:
    if not has_alpha:
        return None

    partial_ratio = max(0.0, 1.0 - transparent_ratio - opaque_ratio)
    if (
        has_partial_alpha
        and transparent_ratio >= ALPHA_BLEND_TRANSPARENT_MIN_RATIO
        and partial_ratio >= ALPHA_BLEND_PARTIAL_MIN_RATIO
        and opaque_ratio <= ALPHA_BLEND_OPAQUE_MAX_RATIO
    ):
        return "BLEND"

    if transparent_ratio > 0.0 or partial_ratio >= ALPHA_MASK_PARTIAL_MIN_RATIO:
        return "MASK"

    return None

class BmdParseError(Exception):
    pass

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

Q_PI = 3.14159265358979323846

# BMD struct sizes (MSVC-aligned, on-disk)
SIZEOF_VERTEX = 16      # short Node(2) + pad(2) + float Position[3](12)
SIZEOF_NORMAL = 20      # short Node(2) + pad(2) + float Normal[3](12) + short BindVertex(2) + pad(2)
SIZEOF_TEXCOORD = 8     # float U(4) + float V(4)
SIZEOF_TRIANGLE = 64    # on-disk stride (Triangle_t2 with lightmap data)

# Encryption constants (from assets_convert.py)
MAP_XOR_KEY: Tuple[int, ...] = (
    0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
    0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2,
)
MAP_KEY_SEED = 0x5E

MAX_BONES = 200
MAX_MESH = 50

# Data-table BMD files that are NOT 3D models
NON_MODEL_STEMS = {
    "item", "minimap", "itemsetting", "petdata", "gate", "movereq",
    "npcname", "quest", "skill", "filter", "dialog", "movelist",
    "serverlist", "chaosbox", "mixlist",
}

WORLD_DIR_PATTERN = re.compile(r"^world_?(\d+)$", re.IGNORECASE)
OBJECT_DIR_PATTERN = re.compile(r"^object_?(\d+)$", re.IGNORECASE)
OBJECT_MODEL_PATTERN = re.compile(r"^object_?(\d+)$", re.IGNORECASE)
NON_ALNUM_PATTERN = re.compile(r"[^a-z0-9]+")

PLAYER_DIR_NAME = "player"
CANONICAL_PLAYER_BMD_BASENAME = "player.bmd"
PLAYER_SET_ITEM_MODEL_PATTERN = re.compile(
    r"^(?:cw_|hdk_)?(?:armor|helm|pant|glove|boot)_",
    re.IGNORECASE,
)
PLAYER_CANONICAL_REQUIRED_BONES: Tuple[str, ...] = (
    "bip01",
    "bip01 spine",
    "bip01 l clavicle",
    "bip01 l upperarm",
    "bip01 l forearm",
    "bip01 l hand",
)
PLAYER_CANONICAL_MIN_BONE_MATCH_RATIO = 0.60

# Global cache for fallback PNG lookup maps, keyed by absolute root path.
_GLOBAL_PNG_INDEX_CACHE: Dict[str, Tuple[Dict[str, Path], Dict[str, Path], Dict[str, Path]]] = {}

LEGACY_TEXTURE_EXTENSIONS_BY_PREFERENCE: Tuple[str, ...] = (
    ".ozj",
    ".ozj2",
    ".jpg",
    ".jpeg",
    ".png",
    ".ozp",
    ".ozt",
    ".tga",
    ".ozb",
    ".bmp",
)

# Legacy world/object rendering hints from MuClient5.2 / muonline-cross source.
# Key: (object_dir, model_index), where model_index is from ObjectXX.bmd.
# Value: BlendMesh value used by RenderMesh, which maps to Mesh.Texture index.
# NOTE: these hints are now a fallback when automatic blend inference cannot
# confidently classify a material as additive.
#
# MuClient5.2 references:
#   cpp/MuClient5.2/source/ZzzObject.cpp
#     - CreateObject(): WD_3NORIA (types 1, 9, 17, 18, 19, 37)
#     - MoveObject():   WD_3NORIA (types 39, 41)
#     - type N maps to Object(N+1).bmd under Object4.
#
# muonline-cross reference:
#   Client.Main/Objects/Worlds/Noria/ClimberObject.cs
#     - Object4/Object07.bmd uses BlendMesh=1 with additive blend state.
LEGACY_BLEND_TEXTURE_INDEX_BY_OBJECT_MODEL: Dict[Tuple[int, int], int] = {
    # Noria (World4 / WD_3NORIA)
    (4, 2): 1,    # type 1  -> Object02, BlendMesh=1
    (4, 7): 1,    # type 6  -> Object07, BlendMesh=1 (muonline-cross)
    (4, 10): 3,   # type 9  -> Object10, BlendMesh=3
    (4, 18): 0,   # type 17 -> Object18, BlendMesh=0
    (4, 19): 2,   # type 18 -> Object19, BlendMesh=2
    (4, 20): 0,   # type 19 -> Object20, BlendMesh=0
    (4, 38): 0,   # type 37 -> Object38, BlendMesh=0
    (4, 40): 1,   # type 39 -> Object40, BlendMesh=1
    (4, 42): 0,   # type 41 -> Object42, BlendMesh=0
}

# Legacy blend hints for non-object model groups (currently item models).
# Key format: "<group>/<model_stem>" normalized to lowercase alnum only.
# NOTE: this table exists for compatibility fallback only.
LEGACY_BLEND_TEXTURE_INDEX_BY_MODEL_KEY: Dict[str, int] = {
    "item/wing01": 0,
    # MODEL_FIRE (Data\Skill\Fire01.bmd) uses BlendMesh=1 in legacy client.
    "skill/fire01": 1,
    # Keep legacy stem fallback for datasets that expose Fire.bmd.
    "skill/fire": 1,
    # Lorencia PoseBox uses a legacy transparent/additive-style render path.
    "object1/posebox01": 0,
    # Lorencia HouseWall02 maps to MODEL_HOUSE_WALL01+1 in legacy client,
    # where BlendMesh=4 is explicitly applied.
    "object1/housewall02": 4,
    # Noria Object39 (DataNorm/object_4/object_39.bmd) requires legacy blend fallback.
    "object4/object39": 0,
}

# Optional intensity hint for legacy additive materials.
# Runtime may apply this multiplier to tune perceived brightness.
LEGACY_ADDITIVE_INTENSITY_BY_OBJECT_MODEL: Dict[Tuple[int, int], float] = {
    (4, 38): 1.20,
    (4, 42): 1.20,
    (4, 40): 1.00,
}

LEGACY_ADDITIVE_INTENSITY_BY_MODEL_KEY: Dict[str, float] = {
    "item/wing01": 1.00,
    "skill/fire01": 1.00,
    "skill/fire": 1.00,
}

# Optional metadata hints for legacy additive effects that used near-black
# backgrounds in the original client. This converter currently does not rewrite
# textures with color-key alpha; values are emitted as extras metadata only.
LEGACY_ADDITIVE_COLOR_KEY_THRESHOLD_BY_OBJECT_MODEL: Dict[Tuple[int, int], int] = {
    (4, 40): 64,  # Object4/Object40 (Chaos Machine glow mesh)
}

# ---------------------------------------------------------------------------
# Decryption
# ---------------------------------------------------------------------------

def map_file_decrypt(data: bytes) -> bytes:
    out = bytearray(len(data))
    map_key = MAP_KEY_SEED
    key_len = len(MAP_XOR_KEY)
    for index, value in enumerate(data):
        out[index] = ((value ^ MAP_XOR_KEY[index % key_len]) - map_key) & 0xFF
        map_key = (value + 0x3D) & 0xFF
    return bytes(out)

# ---------------------------------------------------------------------------
# LEA-256 ECB Decryption (Season20 BMD version 0x0F)
# Port of KISA reference via xulek/muonline-bmd-viewer
# ---------------------------------------------------------------------------

_LEA_DELTA = (0xc3efe9db, 0x44626b02, 0x79e27c8a, 0x78df30ec,
              0x715ea49e, 0xc785da0a, 0xe04ef22a, 0xe5c40957)

_LEA_KEY = bytes([
    0xcc, 0x50, 0x45, 0x13, 0xc2, 0xa6, 0x57, 0x4e,
    0xd6, 0x9a, 0x45, 0x89, 0xbf, 0x2f, 0xbc, 0xd9,
    0x39, 0xb3, 0xb3, 0xbd, 0x50, 0xbd, 0xcc, 0xb6,
    0x85, 0x46, 0xd1, 0xd6, 0x16, 0x54, 0xe0, 0x87,
])

_M32 = 0xFFFFFFFF


def _rol32(x: int, n: int) -> int:
    n &= 31
    return ((x << n) | (x >> (32 - n))) & _M32


def _ror32(x: int, n: int) -> int:
    n &= 31
    return ((x >> n) | (x << (32 - n))) & _M32


def _lea256_key_schedule(key: bytes) -> List[int]:
    T = list(struct.unpack_from('<8I', key))
    rk = [0] * 192
    for i in range(32):
        d = _LEA_DELTA[i & 7]
        s = (i * 6) & 7
        T[(s+0)&7] = _rol32((T[(s+0)&7] + _rol32(d, i  )) & _M32,  1)
        T[(s+1)&7] = _rol32((T[(s+1)&7] + _rol32(d, i+1)) & _M32,  3)
        T[(s+2)&7] = _rol32((T[(s+2)&7] + _rol32(d, i+2)) & _M32,  6)
        T[(s+3)&7] = _rol32((T[(s+3)&7] + _rol32(d, i+3)) & _M32, 11)
        T[(s+4)&7] = _rol32((T[(s+4)&7] + _rol32(d, i+4)) & _M32, 13)
        T[(s+5)&7] = _rol32((T[(s+5)&7] + _rol32(d, i+5)) & _M32, 17)
        for j in range(6):
            rk[i * 6 + j] = T[(s + j) & 7]
    return rk


def _lea256_decrypt_block(block: Tuple[int, ...], rk: List[int]) -> Tuple[int, ...]:
    s = list(block)
    for r in range(31, -1, -1):
        base = r * 6
        t0 = s[3]
        t1 = (_ror32(s[0], 9) - (t0 ^ rk[base+0]) ^ rk[base+1]) & _M32
        t2 = (_rol32(s[1], 5) - (t1 ^ rk[base+2]) ^ rk[base+3]) & _M32
        t3 = (_rol32(s[2], 3) - (t2 ^ rk[base+4]) ^ rk[base+5]) & _M32
        s = [t0, t1, t2, t3]
    return tuple(s)


# Pre-compute round keys once at import time
_LEA_RK = _lea256_key_schedule(_LEA_KEY)


def lea256_ecb_decrypt(data: bytes) -> bytes:
    """Decrypt data using LEA-256 in ECB mode (16-byte block cipher)."""
    if len(data) % 16 != 0:
        raise BmdParseError(
            f"LEA-256 ECB payload must be 16-byte aligned (got {len(data)} bytes)"
        )

    out = bytearray(len(data))
    rk = _LEA_RK
    for off in range(0, len(data), 16):
        block = struct.unpack_from('<4I', data, off)
        dec = _lea256_decrypt_block(block, rk)
        struct.pack_into('<4I', out, off, *dec)
    return bytes(out)


def _find_modulus_tool() -> Optional[Path]:
    """Find the mu_terrain_decrypt tool (also works for BMD ModulusDecrypt)."""
    script_dir = Path(__file__).resolve().parent
    for name in ("mu_terrain_decrypt",):
        candidate = script_dir / name
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return candidate
    return None


def _modulus_decrypt_bmd(enc_body: bytes) -> bytes:
    """Decrypt ModulusDecrypt-encrypted BMD body using the C++ tool.

    Wraps the encrypted body with a MAP\\x01 header so the tool processes it
    (MAP applies pure ModulusDecrypt with no Xor3Byte post-processing).
    """
    tool = _find_modulus_tool()
    if tool is None:
        raise BmdParseError(
            "mu_terrain_decrypt tool not found (needed for version 0x0E). "
            "Build it with: g++ -O2 -o mu_terrain_decrypt mu_terrain_decrypt.cpp -lcryptopp"
        )
    fake_data = b'MAP\x01' + enc_body
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp_in:
        tmp_in.write(fake_data)
        tmp_in_path = tmp_in.name
    tmp_out_path = tmp_in_path + ".dec"
    try:
        result = subprocess.run(
            [str(tool), tmp_in_path, tmp_out_path],
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            raise BmdParseError(
                f"ModulusDecrypt failed (rc={result.returncode}): {result.stderr.strip()}"
            )
        return Path(tmp_out_path).read_bytes()
    finally:
        Path(tmp_in_path).unlink(missing_ok=True)
        Path(tmp_out_path).unlink(missing_ok=True)

# ---------------------------------------------------------------------------
# Math helpers (ported from ZzzMathLib.cpp)
# ---------------------------------------------------------------------------

def angle_matrix(angles: Tuple[float, float, float]) -> List[List[float]]:
    """Compute a 3x4 rotation matrix from Euler angles in degrees (ZYX convention)."""
    a = angles[2] * (Q_PI * 2.0 / 360.0)
    sy, cy = math.sin(a), math.cos(a)
    a = angles[1] * (Q_PI * 2.0 / 360.0)
    sp, cp = math.sin(a), math.cos(a)
    a = angles[0] * (Q_PI * 2.0 / 360.0)
    sr, cr = math.sin(a), math.cos(a)

    return [
        [cp * cy,           sr * sp * cy + cr * (-sy), cr * sp * cy + (-sr) * (-sy), 0.0],
        [cp * sy,           sr * sp * sy + cr * cy,    cr * sp * sy + (-sr) * cy,     0.0],
        [-sp,               sr * cp,                   cr * cp,                        0.0],
    ]


def r_concat_transforms(
    in1: List[List[float]], in2: List[List[float]]
) -> List[List[float]]:
    """Concatenate two 3x4 transforms."""
    out = [[0.0] * 4 for _ in range(3)]
    for i in range(3):
        for j in range(4):
            s = in1[i][0] * in2[0][j] + in1[i][1] * in2[1][j] + in1[i][2] * in2[2][j]
            if j == 3:
                s += in1[i][3]
            out[i][j] = s
    return out


def vector_transform(v: Tuple[float, float, float], m: List[List[float]]) -> Tuple[float, float, float]:
    """Transform a vector by a 3x4 matrix (rotate + translate)."""
    return (
        v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + m[0][3],
        v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2] + m[1][3],
        v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2] + m[2][3],
    )


def vector_rotate(v: Tuple[float, float, float], m: List[List[float]]) -> Tuple[float, float, float]:
    """Rotate a vector by the 3x3 part of a 3x4 matrix (no translation)."""
    return (
        v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2],
        v[0] * m[1][0] + v[1] * m[1][1] + v[2] * m[1][2],
        v[0] * m[2][0] + v[1] * m[2][1] + v[2] * m[2][2],
    )


def vector_normalize(v: Tuple[float, float, float]) -> Tuple[float, float, float]:
    length = math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
    if length == 0.0:
        return (0.0, 0.0, 0.0)
    return (v[0] / length, v[1] / length, v[2] / length)


# ---------------------------------------------------------------------------
# BMD data structures
# ---------------------------------------------------------------------------

@dataclass
class BmdVertex:
    node: int
    position: Tuple[float, float, float]


@dataclass
class BmdNormal:
    node: int
    normal: Tuple[float, float, float]
    bind_vertex: int


@dataclass
class BmdTexCoord:
    u: float
    v: float


@dataclass
class BmdTriangle:
    polygon: int
    vertex_index: Tuple[int, int, int, int]
    normal_index: Tuple[int, int, int, int]
    texcoord_index: Tuple[int, int, int, int]


@dataclass
class BmdMesh:
    num_vertices: int
    num_normals: int
    num_texcoords: int
    num_triangles: int
    texture: int
    vertices: List[BmdVertex]
    normals: List[BmdNormal]
    texcoords: List[BmdTexCoord]
    triangles: List[BmdTriangle]
    texture_name: str


@dataclass
class BmdAction:
    num_animation_keys: int
    lock_positions: bool
    positions: Optional[List[Tuple[float, float, float]]]


@dataclass
class BmdBone:
    name: str
    parent: int
    dummy: bool
    # Per-action: list of (positions_per_key, rotations_per_key)
    matrices: List[Tuple[List[Tuple[float, float, float]], List[Tuple[float, float, float]]]]


@dataclass
class BoneFixup:
    m: List[List[float]]
    world_org: Tuple[float, float, float]


@dataclass
class BmdModel:
    name: str
    version: int
    num_meshs: int
    num_bones: int
    num_actions: int
    meshs: List[BmdMesh]
    actions: List[BmdAction]
    bones: List[BmdBone]


@dataclass(frozen=True)
class CanonicalSkeleton:
    source_path: Path
    model: BmdModel
    fixups: List[BoneFixup]
    bone_index_by_key: Dict[str, int]
    local_bind_pose_by_index: List[Tuple[Tuple[float, float, float], Tuple[float, float, float]]]


@dataclass(frozen=True)
class ResolvedTexture:
    uri: str
    png_bytes: Optional[bytes]
    found_on_disk: bool
    has_alpha: bool = False
    has_partial_alpha: bool = False
    transparent_ratio: float = 0.0
    opaque_ratio: float = 1.0
    black_ratio: float = 0.0
    very_dark_ratio: float = 0.0
    bright_ratio: float = 0.0
    mean_luma: float = 0.0
    luma_std_dev: float = 0.0
    bright_energy_ratio: float = 0.0


@dataclass(frozen=True)
class PrimitiveInfo:
    vert_offset: int
    vert_count: int
    idx_offset: int
    idx_count: int
    texture_uri: Optional[str]
    legacy_blend_mode: Optional[str]
    mesh_index: int
    mesh_texture_index: int
    mesh_texture_name: str
    mesh_texture_reference_name: Optional[str]
    texture_candidates: Tuple[str, ...]
    texture_found_on_disk: bool
    texture_has_alpha: bool
    texture_has_partial_alpha: bool
    texture_transparent_ratio: float
    texture_opaque_ratio: float
    texture_black_ratio: float
    texture_very_dark_ratio: float
    texture_bright_ratio: float
    texture_mean_luma: float
    texture_luma_std_dev: float
    texture_bright_energy_ratio: float


# ---------------------------------------------------------------------------
# BMD Parser
# ---------------------------------------------------------------------------


def _read_c_string(data: bytes, offset: int, length: int) -> str:
    raw = data[offset:offset + length]
    null_pos = raw.find(b'\x00')
    if null_pos >= 0:
        raw = raw[:null_pos]
    return raw.decode('ascii', errors='replace')


def parse_bmd(file_path: Path) -> BmdModel:
    """Parse a BMD file and return its model data."""
    raw = file_path.read_bytes()

    if len(raw) < 4:
        raise BmdParseError(f"File too small: {len(raw)} bytes")

    magic = raw[:3]
    if magic != b'BMD':
        raise BmdParseError(f"Not a BMD file (magic: {magic!r})")

    version = raw[3]

    if version == 0x00:
        raise BmdParseError("Corrupt/empty BMD (version 0x00)")

    if version == 0x0C:
        # MapFileDecrypt encrypted
        if len(raw) < 8:
            raise BmdParseError("Encrypted BMD too small for size header")
        enc_size = struct.unpack_from('<I', raw, 4)[0]
        if len(raw) < 8 + enc_size:
            raise BmdParseError(
                f"Encrypted BMD truncated (need {8 + enc_size}, have {len(raw)})"
            )
        data = map_file_decrypt(raw[8:8 + enc_size])
    elif version == 0x0A:
        # Unencrypted
        data = raw[4:]
    elif version == 0x0E:
        # Modulus encrypted (Season16+)
        if len(raw) < 8:
            raise BmdParseError("Encrypted BMD too small for size header")
        enc_size = struct.unpack_from('<I', raw, 4)[0]
        if len(raw) < 8 + enc_size:
            raise BmdParseError(
                f"Encrypted BMD truncated (need {8 + enc_size}, have {len(raw)})"
            )
        data = _modulus_decrypt_bmd(raw[8:8 + enc_size])
    elif version == 0x0F:
        # LEA-256 ECB encrypted (Season20)
        if len(raw) < 8:
            raise BmdParseError("Encrypted BMD too small for size header")
        enc_size = struct.unpack_from('<I', raw, 4)[0]
        if len(raw) < 8 + enc_size:
            raise BmdParseError(
                f"Encrypted BMD truncated (need {8 + enc_size}, have {len(raw)})"
            )
        if enc_size % 16 != 0:
            raise BmdParseError(
                f"LEA-256 encrypted BMD size must be 16-byte aligned (got {enc_size})"
            )
        data = lea256_ecb_decrypt(raw[8:8 + enc_size])
    else:
        raise BmdParseError(f"Unknown BMD version: 0x{version:02X}")

    if len(data) < 38:
        raise BmdParseError(f"BMD data too small for model header ({len(data)} < 38)")

    pos = 0

    # Model header: Name(32) + NumMeshs(2) + NumBones(2) + NumActions(2) = 38 bytes
    name = _read_c_string(data, pos, 32)
    pos += 32

    num_meshs = struct.unpack_from('<h', data, pos)[0]
    pos += 2
    num_bones = struct.unpack_from('<h', data, pos)[0]
    pos += 2
    num_actions = struct.unpack_from('<h', data, pos)[0]
    pos += 2

    if num_meshs < 0 or num_meshs > MAX_MESH:
        raise BmdParseError(f"Invalid NumMeshs: {num_meshs}")
    if num_bones < 0 or num_bones > MAX_BONES:
        raise BmdParseError(f"Invalid NumBones: {num_bones}")
    if num_actions < 0:
        raise BmdParseError(f"Invalid NumActions: {num_actions}")

    # Parse meshes
    meshs: List[BmdMesh] = []
    for i in range(num_meshs):
        if pos + 10 > len(data):
            raise BmdParseError(f"Mesh {i} header truncated at offset {pos}")

        nv = struct.unpack_from('<h', data, pos)[0]; pos += 2
        nn = struct.unpack_from('<h', data, pos)[0]; pos += 2
        ntc = struct.unpack_from('<h', data, pos)[0]; pos += 2
        nt = struct.unpack_from('<h', data, pos)[0]; pos += 2
        tex = struct.unpack_from('<h', data, pos)[0]; pos += 2

        if nv < 0: nv = 0
        if nn < 0: nn = 0
        if ntc < 0: ntc = 0
        if nt < 0: nt = 0

        # Vertices: N * 16 bytes each
        verts: List[BmdVertex] = []
        needed = nv * SIZEOF_VERTEX
        if pos + needed > len(data):
            raise BmdParseError(f"Mesh {i} vertices truncated")
        for j in range(nv):
            off = pos + j * SIZEOF_VERTEX
            node = struct.unpack_from('<h', data, off)[0]
            px, py, pz = struct.unpack_from('<3f', data, off + 4)
            verts.append(BmdVertex(node=node, position=(px, py, pz)))
        pos += needed

        # Normals: N * 20 bytes each
        norms: List[BmdNormal] = []
        needed = nn * SIZEOF_NORMAL
        if pos + needed > len(data):
            raise BmdParseError(f"Mesh {i} normals truncated")
        for j in range(nn):
            off = pos + j * SIZEOF_NORMAL
            node = struct.unpack_from('<h', data, off)[0]
            nx, ny, nz = struct.unpack_from('<3f', data, off + 4)
            bv = struct.unpack_from('<h', data, off + 16)[0]
            norms.append(BmdNormal(node=node, normal=(nx, ny, nz), bind_vertex=bv))
        pos += needed

        # TexCoords: N * 8 bytes each
        tcs: List[BmdTexCoord] = []
        needed = ntc * SIZEOF_TEXCOORD
        if pos + needed > len(data):
            raise BmdParseError(f"Mesh {i} texcoords truncated")
        for j in range(ntc):
            off = pos + j * SIZEOF_TEXCOORD
            u, v = struct.unpack_from('<2f', data, off)
            tcs.append(BmdTexCoord(u=u, v=v))
        pos += needed

        # Triangles: N * 64 bytes stride each (on-disk Triangle_t2)
        tris: List[BmdTriangle] = []
        needed = nt * SIZEOF_TRIANGLE
        if pos + needed > len(data):
            raise BmdParseError(f"Mesh {i} triangles truncated")
        for j in range(nt):
            off = pos + j * SIZEOF_TRIANGLE
            polygon = struct.unpack_from('<b', data, off)[0]
            # skip 1 byte padding
            vi = struct.unpack_from('<4h', data, off + 2)
            ni = struct.unpack_from('<4h', data, off + 10)
            ti = struct.unpack_from('<4h', data, off + 18)
            tris.append(BmdTriangle(
                polygon=polygon,
                vertex_index=vi,
                normal_index=ni,
                texcoord_index=ti,
            ))
        pos += needed

        # Texture name: 32 bytes
        if pos + 32 > len(data):
            raise BmdParseError(f"Mesh {i} texture name truncated")
        tex_name = _read_c_string(data, pos, 32)
        pos += 32

        meshs.append(BmdMesh(
            num_vertices=nv, num_normals=nn, num_texcoords=ntc,
            num_triangles=nt, texture=tex,
            vertices=verts, normals=norms, texcoords=tcs, triangles=tris,
            texture_name=tex_name,
        ))

    # Parse actions
    actions: List[BmdAction] = []
    for i in range(num_actions):
        if pos + 3 > len(data):
            raise BmdParseError(f"Action {i} header truncated")

        num_keys = struct.unpack_from('<h', data, pos)[0]; pos += 2
        lock_pos = struct.unpack_from('<B', data, pos)[0]; pos += 1
        if num_keys < 0:
            num_keys = 0

        positions = None
        if lock_pos:
            needed = num_keys * 12  # vec3_t = 3 floats
            if pos + needed > len(data):
                raise BmdParseError(f"Action {i} positions truncated")
            positions = []
            for j in range(num_keys):
                off = pos + j * 12
                px, py, pz = struct.unpack_from('<3f', data, off)
                positions.append((px, py, pz))
            pos += needed

        actions.append(BmdAction(
            num_animation_keys=num_keys,
            lock_positions=bool(lock_pos),
            positions=positions,
        ))

    # Parse bones
    bones: List[BmdBone] = []
    for i in range(num_bones):
        if pos + 1 > len(data):
            raise BmdParseError(f"Bone {i} header truncated")

        dummy = struct.unpack_from('<b', data, pos)[0]; pos += 1

        if not dummy:
            if pos + 34 > len(data):
                raise BmdParseError(f"Bone {i} data truncated")

            bone_name = _read_c_string(data, pos, 32); pos += 32
            parent = struct.unpack_from('<h', data, pos)[0]; pos += 2

            matrices: List[Tuple[List[Tuple[float, float, float]], List[Tuple[float, float, float]]]] = []
            for j in range(num_actions):
                nkeys = actions[j].num_animation_keys
                needed = nkeys * 12 * 2  # position + rotation
                if pos + needed > len(data):
                    raise BmdParseError(f"Bone {i} action {j} data truncated")

                bone_positions = []
                for k in range(nkeys):
                    off = pos + k * 12
                    px, py, pz = struct.unpack_from('<3f', data, off)
                    bone_positions.append((px, py, pz))
                pos += nkeys * 12

                bone_rotations = []
                for k in range(nkeys):
                    off = pos + k * 12
                    rx, ry, rz = struct.unpack_from('<3f', data, off)
                    bone_rotations.append((rx, ry, rz))
                pos += nkeys * 12

                matrices.append((bone_positions, bone_rotations))

            bones.append(BmdBone(
                name=bone_name, parent=parent, dummy=False, matrices=matrices,
            ))
        else:
            # Dummy bone
            bones.append(BmdBone(
                name=f"Null_{i}", parent=-1, dummy=True,
                matrices=[([( 0.0, 0.0, 0.0)], [(0.0, 0.0, 0.0)])]
            ))

    return BmdModel(
        name=name, version=version,
        num_meshs=num_meshs, num_bones=num_bones, num_actions=num_actions,
        meshs=meshs, actions=actions, bones=bones,
    )


# ---------------------------------------------------------------------------
# Bone Fixup (rest pose world transforms)
# ---------------------------------------------------------------------------

def compute_bone_fixups(model: BmdModel) -> List[BoneFixup]:
    """Compute world-space transforms for each bone at rest pose (action=0, key=0).

    Reference: BMD_SMD.cpp:156-195 (FixUpBones)
    """
    fixups: List[BoneFixup] = []
    identity_m = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]]

    for i in range(model.num_bones):
        bone = model.bones[i]

        if bone.dummy or not bone.matrices:
            fixups.append(BoneFixup(m=identity_m, world_org=(0.0, 0.0, 0.0)))
            continue

        # Get rotation and position from action=0, key=0
        positions_0, rotations_0 = bone.matrices[0]
        rot = rotations_0[0] if rotations_0 else (0.0, 0.0, 0.0)
        bpos = positions_0[0] if positions_0 else (0.0, 0.0, 0.0)

        # Convert rotation from radians to degrees (BMD stores radians)
        # Reference: BMD_SMD.cpp:165-167 — Angle = Rotation * (180/PI)
        angle_deg = (
            rot[0] * (180.0 / Q_PI),
            rot[1] * (180.0 / Q_PI),
            rot[2] * (180.0 / Q_PI),
        )

        if bone.parent >= 0 and bone.parent < len(fixups):
            local_m = angle_matrix(angle_deg)
            parent_fixup = fixups[bone.parent]
            world_m = r_concat_transforms(parent_fixup.m, local_m)
            p = vector_transform(bpos, parent_fixup.m)
            world_org = (
                p[0] + parent_fixup.world_org[0],
                p[1] + parent_fixup.world_org[1],
                p[2] + parent_fixup.world_org[2],
            )
            fixups.append(BoneFixup(m=world_m, world_org=world_org))
        else:
            m = angle_matrix(angle_deg)
            fixups.append(BoneFixup(m=m, world_org=bpos))

    return fixups


# ---------------------------------------------------------------------------
# GLTF / GLB Emission
# ---------------------------------------------------------------------------

def _swizzle_mu_to_gltf(v: Tuple[float, float, float]) -> Tuple[float, float, float]:
    """Convert MU (X, Y, Z-up) coordinates into client/glTF (X, Y-up, Z) coordinates."""
    return (v[0], v[2], v[1])


def _swizzle_mu_to_gltf_matrix3(matrix: List[List[float]]) -> List[List[float]]:
    """Convert a MU-space rotation matrix to glTF basis by swapping Y/Z rows and columns."""
    perm = (0, 2, 1)
    return [
        [matrix[perm[row]][perm[col]] for col in range(3)]
        for row in range(3)
    ]


def _rotation_matrix3_from_mu_radians(
    radians_xyz: Tuple[float, float, float],
) -> List[List[float]]:
    """Build a MU-space 3x3 rotation matrix from Euler radians stored in BMD."""
    angle_deg = (
        radians_xyz[0] * (180.0 / Q_PI),
        radians_xyz[1] * (180.0 / Q_PI),
        radians_xyz[2] * (180.0 / Q_PI),
    )
    matrix_3x4 = angle_matrix(angle_deg)
    return [
        [matrix_3x4[0][0], matrix_3x4[0][1], matrix_3x4[0][2]],
        [matrix_3x4[1][0], matrix_3x4[1][1], matrix_3x4[1][2]],
        [matrix_3x4[2][0], matrix_3x4[2][1], matrix_3x4[2][2]],
    ]


def _matrix3_to_quaternion(matrix: List[List[float]]) -> Tuple[float, float, float, float]:
    """Convert a row-major 3x3 rotation matrix into an (x, y, z, w) quaternion."""
    m00, m01, m02 = matrix[0]
    m10, m11, m12 = matrix[1]
    m20, m21, m22 = matrix[2]

    trace = m00 + m11 + m22
    if trace > 0.0:
        s = math.sqrt(trace + 1.0) * 2.0
        w = 0.25 * s
        x = (m21 - m12) / s
        y = (m02 - m20) / s
        z = (m10 - m01) / s
    elif m00 > m11 and m00 > m22:
        s = math.sqrt(1.0 + m00 - m11 - m22) * 2.0
        w = (m21 - m12) / s
        x = 0.25 * s
        y = (m01 + m10) / s
        z = (m02 + m20) / s
    elif m11 > m22:
        s = math.sqrt(1.0 + m11 - m00 - m22) * 2.0
        w = (m02 - m20) / s
        x = (m01 + m10) / s
        y = 0.25 * s
        z = (m12 + m21) / s
    else:
        s = math.sqrt(1.0 + m22 - m00 - m11) * 2.0
        w = (m10 - m01) / s
        x = (m02 + m20) / s
        y = (m12 + m21) / s
        z = 0.25 * s

    norm = math.sqrt(x * x + y * y + z * z + w * w)
    if norm == 0.0:
        return (0.0, 0.0, 0.0, 1.0)
    return (x / norm, y / norm, z / norm, w / norm)


def _inverse_rigid_matrix4(matrix: List[List[float]]) -> List[List[float]]:
    """Invert a rigid 4x4 transform matrix (rotation + translation, no scale/shear)."""
    r00, r01, r02 = matrix[0][0], matrix[0][1], matrix[0][2]
    r10, r11, r12 = matrix[1][0], matrix[1][1], matrix[1][2]
    r20, r21, r22 = matrix[2][0], matrix[2][1], matrix[2][2]
    tx, ty, tz = matrix[0][3], matrix[1][3], matrix[2][3]

    # Inverse rotation = transpose for orthonormal rigid transforms.
    ir00, ir01, ir02 = r00, r10, r20
    ir10, ir11, ir12 = r01, r11, r21
    ir20, ir21, ir22 = r02, r12, r22

    itx = -(ir00 * tx + ir01 * ty + ir02 * tz)
    ity = -(ir10 * tx + ir11 * ty + ir12 * tz)
    itz = -(ir20 * tx + ir21 * ty + ir22 * tz)

    return [
        [ir00, ir01, ir02, itx],
        [ir10, ir11, ir12, ity],
        [ir20, ir21, ir22, itz],
        [0.0, 0.0, 0.0, 1.0],
    ]


def _matrix4_to_column_major_values(matrix: List[List[float]]) -> List[float]:
    """Serialize a row-major 4x4 matrix into glTF column-major value order."""
    out: List[float] = []
    for col in range(4):
        for row in range(4):
            out.append(float(matrix[row][col]))
    return out


def _bone_local_pose(
    model: BmdModel,
    bone_index: int,
    action_index: int,
    key_index: int,
) -> Tuple[Tuple[float, float, float], Tuple[float, float, float]]:
    """Return local bone position/rotation (MU space) for an action/key, with safe fallbacks."""
    if bone_index < 0 or bone_index >= len(model.bones):
        return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0)

    bone = model.bones[bone_index]
    if bone.dummy or not bone.matrices:
        return (0.0, 0.0, 0.0), (0.0, 0.0, 0.0)

    matrix_track_index = action_index if action_index < len(bone.matrices) else 0
    positions, rotations = bone.matrices[matrix_track_index]

    if positions:
        pos = positions[min(max(key_index, 0), len(positions) - 1)]
    else:
        pos = (0.0, 0.0, 0.0)

    if rotations:
        rot = rotations[min(max(key_index, 0), len(rotations) - 1)]
    else:
        rot = (0.0, 0.0, 0.0)

    return pos, rot


def _mu_local_pose_to_gltf_trs(
    position_mu: Tuple[float, float, float],
    rotation_mu_radians: Tuple[float, float, float],
) -> Tuple[Tuple[float, float, float], Tuple[float, float, float, float]]:
    """Convert MU local pose (position + Euler radians) to glTF translation + quaternion."""
    rotation_mu = _rotation_matrix3_from_mu_radians(rotation_mu_radians)
    rotation_gltf = _swizzle_mu_to_gltf_matrix3(rotation_mu)
    quaternion = _matrix3_to_quaternion(rotation_gltf)
    translation = _swizzle_mu_to_gltf(position_mu)
    return translation, quaternion


def _fixup_to_gltf_global_matrix(fixup: BoneFixup) -> List[List[float]]:
    """Convert a MU-space bone global bind transform into a glTF-space 4x4 matrix."""
    rotation_gltf = _swizzle_mu_to_gltf_matrix3(
        [
            [fixup.m[0][0], fixup.m[0][1], fixup.m[0][2]],
            [fixup.m[1][0], fixup.m[1][1], fixup.m[1][2]],
            [fixup.m[2][0], fixup.m[2][1], fixup.m[2][2]],
        ]
    )
    tx, ty, tz = _swizzle_mu_to_gltf(fixup.world_org)
    return [
        [rotation_gltf[0][0], rotation_gltf[0][1], rotation_gltf[0][2], tx],
        [rotation_gltf[1][0], rotation_gltf[1][1], rotation_gltf[1][2], ty],
        [rotation_gltf[2][0], rotation_gltf[2][1], rotation_gltf[2][2], tz],
        [0.0, 0.0, 0.0, 1.0],
    ]


def _track_values_vary(
    values: Sequence[Sequence[float]],
    epsilon: float = 1e-6,
) -> bool:
    """Return True when any key differs from the first key beyond *epsilon*."""
    if len(values) < 2:
        return False

    first = values[0]
    first_len = len(first)
    for value in values[1:]:
        if len(value) != first_len:
            return True
        for index in range(first_len):
            if abs(value[index] - first[index]) > epsilon:
                return True
    return False


def _normalize_bone_name_key(name: str) -> str:
    """Normalize bone names for robust map lookup across BMD variants."""
    return " ".join(name.strip().lower().split())


def _build_bone_name_index_map(model: BmdModel) -> Dict[str, int]:
    out: Dict[str, int] = {}
    for index, bone in enumerate(model.bones):
        key = _normalize_bone_name_key(bone.name)
        if key and key not in out:
            out[key] = index
    return out


def _is_player_source_path(source_path: Optional[Path]) -> bool:
    if source_path is None:
        return False
    return any(part.lower() == PLAYER_DIR_NAME for part in source_path.parts)


def _is_player_set_item_source_path(source_path: Optional[Path]) -> bool:
    if source_path is None or not _is_player_source_path(source_path):
        return False
    return bool(PLAYER_SET_ITEM_MODEL_PATTERN.match(source_path.stem))


def _resolve_default_canonical_player_skeleton_path(bmd_root: Path) -> Optional[Path]:
    """Resolve default player skeleton path under bmd_root (case-insensitive)."""
    direct_candidate = bmd_root / PLAYER_DIR_NAME / CANONICAL_PLAYER_BMD_BASENAME
    if direct_candidate.is_file():
        return direct_candidate

    # Case-insensitive fallback for datasets with mixed-casing directories/files.
    for child in bmd_root.iterdir():
        if not child.is_dir() or child.name.lower() != PLAYER_DIR_NAME:
            continue
        for entry in child.iterdir():
            if entry.is_file() and entry.name.lower() == CANONICAL_PLAYER_BMD_BASENAME:
                return entry
        return None
    return None


_CANONICAL_PLAYER_SKELETON_CACHE: Dict[str, Optional[CanonicalSkeleton]] = {}


def _load_canonical_player_skeleton(path: Optional[Path]) -> Optional[CanonicalSkeleton]:
    if path is None:
        return None

    key = str(path.expanduser().resolve())
    if key in _CANONICAL_PLAYER_SKELETON_CACHE:
        return _CANONICAL_PLAYER_SKELETON_CACHE[key]

    canonical_path = Path(key)
    if not canonical_path.is_file():
        logging.warning("Canonical player skeleton file not found: %s", canonical_path)
        _CANONICAL_PLAYER_SKELETON_CACHE[key] = None
        return None

    try:
        model = parse_bmd(canonical_path)
        if model.num_bones <= 0 or model.num_actions <= 0:
            raise BmdParseError(
                f"canonical skeleton has no bindable bones/actions (bones={model.num_bones}, actions={model.num_actions})"
            )
        fixups = compute_bone_fixups(model)
        if len(fixups) != model.num_bones:
            raise BmdParseError(
                f"canonical skeleton fixups mismatch (fixups={len(fixups)}, bones={model.num_bones})"
            )

        local_bind_pose_by_index: List[Tuple[Tuple[float, float, float], Tuple[float, float, float]]] = []
        for bone_index in range(model.num_bones):
            local_bind_pose_by_index.append(
                _bone_local_pose(model, bone_index=bone_index, action_index=0, key_index=0)
            )

        loaded = CanonicalSkeleton(
            source_path=canonical_path,
            model=model,
            fixups=fixups,
            bone_index_by_key=_build_bone_name_index_map(model),
            local_bind_pose_by_index=local_bind_pose_by_index,
        )
        _CANONICAL_PLAYER_SKELETON_CACHE[key] = loaded
        logging.info(
            "Loaded canonical player skeleton: %s (%d bones, %d actions)",
            canonical_path,
            model.num_bones,
            model.num_actions,
        )
        return loaded
    except Exception as exc:  # noqa: BLE001
        logging.warning("Failed to load canonical player skeleton %s: %s", canonical_path, exc)
        _CANONICAL_PLAYER_SKELETON_CACHE[key] = None
        return None


def _build_player_canonical_bone_remap(
    model: BmdModel,
    source_path: Optional[Path],
    canonical: Optional[CanonicalSkeleton],
) -> Optional[List[Optional[int]]]:
    """Return model-bone -> canonical-bone index mapping for compatible player parts."""
    if canonical is None or not _is_player_source_path(source_path):
        return None
    if model.num_bones <= 0:
        return None

    model_index_by_key = _build_bone_name_index_map(model)
    missing_required = [
        key for key in PLAYER_CANONICAL_REQUIRED_BONES
        if key not in model_index_by_key or key not in canonical.bone_index_by_key
    ]
    if missing_required:
        return None

    remap: List[Optional[int]] = [None] * model.num_bones
    matched_count = 0
    named_bone_count = 0
    for bone_index, bone in enumerate(model.bones):
        key = _normalize_bone_name_key(bone.name)
        if not key:
            continue
        named_bone_count += 1
        canonical_index = canonical.bone_index_by_key.get(key)
        if canonical_index is None:
            continue
        remap[bone_index] = canonical_index
        matched_count += 1

    if matched_count == 0:
        return None

    match_ratio = matched_count / max(1, named_bone_count)
    if match_ratio < PLAYER_CANONICAL_MIN_BONE_MATCH_RATIO:
        logging.debug(
            "Skipping canonical skeleton for %s: match ratio %.2f below threshold %.2f",
            source_path if source_path is not None else model.name,
            match_ratio,
            PLAYER_CANONICAL_MIN_BONE_MATCH_RATIO,
        )
        return None

    return remap


def _inplace_root_bone_indices(model: BmdModel) -> List[int]:
    """Return root-bone indices (or fallback to bone 0) for in-place motion locking."""
    if model.num_bones <= 0:
        return []

    root_indices: List[int] = []
    for bone_index, bone in enumerate(model.bones):
        parent_index = bone.parent
        if parent_index < 0 or parent_index >= model.num_bones or parent_index == bone_index:
            root_indices.append(bone_index)

    if root_indices:
        return root_indices
    return [0]


def _apply_player_inplace_root_motion(
    pos_mu: Tuple[float, float, float],
    bone_index: int,
    root_start_positions_by_bone: Dict[int, Tuple[float, float, float]],
) -> Tuple[float, float, float]:
    """Lock root horizontal motion in MU space (X,Y plane), keep vertical (Z)."""
    start_pos = root_start_positions_by_bone.get(bone_index)
    if start_pos is None:
        return pos_mu
    return (start_pos[0], start_pos[1], pos_mu[2])


def _texture_name_to_candidate_uris(
    texture_name: str,
    prefer_remaster_texture_order: bool = False,
) -> List[str]:
    """Normalize a BMD texture name to candidate texture URIs in output assets."""
    cleaned = texture_name.strip().replace("\\", "/")
    if not cleaned:
        return []

    basename = Path(cleaned).name
    if not basename:
        return []

    texture_path = Path(basename)
    stem = texture_path.stem
    if not stem:
        return []

    suffix = texture_path.suffix.lower()
    candidates: List[str] = []

    if prefer_remaster_texture_order:
        # Remaster ready textures may keep legacy JPEG payloads; always prefer
        # PNG first but allow JPEG fallback when PNG is missing.
        if suffix in (".tga", ".ozt"):
            candidates.extend([f"{stem}.png", f"{stem}.jpg", f"{stem}.jpeg", f"{stem}.tga"])
        elif suffix in (".jpg", ".jpeg", ".bmp", ".ozj", ".ozj2", ".ozb", ".ozp", ".png"):
            candidates.extend([f"{stem}.png", f"{stem}.jpg", f"{stem}.jpeg"])
        else:
            candidates.extend([f"{stem}.png", f"{stem}.jpg", f"{stem}.jpeg", f"{stem}.tga"])
    else:
        # Assets are normalized to PNG in conversion output. Keep TGA fallback
        # for compatibility with stale/mixed outputs from older conversions.
        if suffix in (".tga", ".ozt"):
            candidates.append(f"{stem}.png")
            candidates.append(f"{stem}.tga")
        elif suffix in (".jpg", ".jpeg", ".bmp", ".ozj", ".ozj2", ".ozb", ".ozp", ".png"):
            candidates.append(f"{stem}.png")
        else:
            candidates.append(f"{stem}.png")
            candidates.append(f"{stem}.tga")

    # De-duplicate while preserving order.
    deduped: List[str] = []
    seen: set[str] = set()
    for candidate in candidates:
        key = candidate.lower()
        if key in seen:
            continue
        seen.add(key)
        deduped.append(candidate)
    return deduped


def _texture_name_to_remaster_candidate_uris(texture_name: str) -> List[str]:
    return _texture_name_to_candidate_uris(
        texture_name,
        prefer_remaster_texture_order=True,
    )


def _mime_type_for_texture_uri(texture_uri: str) -> str:
    suffix = Path(texture_uri).suffix.lower()
    if suffix == ".tga":
        return "image/x-tga"
    if suffix in (".jpg", ".jpeg"):
        return "image/jpeg"
    return "image/png"


def _force_texture_uri_png(texture_uri: str) -> str:
    """Return *texture_uri* rewritten with a `.png` suffix."""
    uri = PurePosixPath(texture_uri)
    if uri.suffix:
        return str(uri.with_suffix(".png"))
    return f"{texture_uri}.png"


def _normalize_texture_lookup_key(raw_name: str) -> str:
    return NON_ALNUM_PATTERN.sub("", raw_name.lower())


def _build_global_png_index(
    root: Path,
) -> Tuple[Dict[str, Path], Dict[str, Path], Dict[str, Path]]:
    """Build fallback lookup maps for texture files under *root*."""
    key = str(root.resolve())
    cached = _GLOBAL_PNG_INDEX_CACHE.get(key)
    if cached is not None:
        return cached

    by_lower_name: Dict[str, Path] = {}
    by_lower_stem: Dict[str, Path] = {}
    by_normalized_stem: Dict[str, Path] = {}

    if root.exists() and root.is_dir():
        texture_files = sorted(
            (
                path
                for path in root.rglob("*")
                if path.is_file() and path.suffix.lower() in {".png", ".tga", ".jpg", ".jpeg"}
            ),
            key=lambda path: path.as_posix().lower(),
        )
        for path in texture_files:
            lower_name = path.name.lower()
            lower_stem = path.stem.lower()
            normalized_stem = _normalize_texture_lookup_key(path.stem)

            by_lower_name.setdefault(lower_name, path)
            by_lower_stem.setdefault(lower_stem, path)
            if normalized_stem:
                by_normalized_stem.setdefault(normalized_stem, path)

    cached_maps = (by_lower_name, by_lower_stem, by_normalized_stem)
    _GLOBAL_PNG_INDEX_CACHE[key] = cached_maps
    return cached_maps


class TextureResolver:
    """Resolve BMD texture names against migrated textures in local/fallback output."""

    def __init__(
        self,
        texture_dir: Path,
        embed_textures: bool,
        fallback_root: Optional[Path] = None,
        legacy_texture_dir: Optional[Path] = None,
        prefer_remaster_texture_order: bool = False,
    ) -> None:
        self.texture_dir = texture_dir.resolve()
        self.embed_textures = embed_textures
        self.fallback_root = fallback_root.resolve() if fallback_root else None
        self.legacy_texture_dir = (
            legacy_texture_dir.resolve() if legacy_texture_dir else None
        )
        self.prefer_remaster_texture_order = prefer_remaster_texture_order
        self._index_built = False
        self._by_lower_name: Dict[str, Path] = {}
        self._by_lower_stem: Dict[str, Path] = {}
        self._by_normalized_stem: Dict[str, Path] = {}
        self._fallback_by_lower_name: Optional[Dict[str, Path]] = None
        self._fallback_by_lower_stem: Optional[Dict[str, Path]] = None
        self._fallback_by_normalized_stem: Optional[Dict[str, Path]] = None
        self._legacy_by_lower_name: Optional[Dict[str, Path]] = None
        self._legacy_by_lower_stem: Optional[Dict[str, List[Path]]] = None
        self._legacy_by_normalized_stem: Optional[Dict[str, List[Path]]] = None
        self._cache: Dict[str, ResolvedTexture] = {}

    def _legacy_texture_suffix_order(self, requested_suffix: str) -> List[str]:
        suffix = requested_suffix.lower()
        remaster_preferred: List[str] = []
        if self.prefer_remaster_texture_order:
            remaster_preferred = [".png", ".ozp"]

        if suffix in {".jpg", ".jpeg", ".ozj", ".ozj2"}:
            preferred = [".ozj", ".ozj2", ".jpg", ".jpeg"]
        elif suffix in {".tga", ".ozt"}:
            preferred = [".ozt", ".tga"]
        elif suffix in {".png", ".ozp"}:
            preferred = [".png", ".ozp"]
        elif suffix in {".bmp", ".ozb"}:
            preferred = [".ozb", ".bmp"]
        else:
            preferred = []

        ordered: List[str] = []
        seen: set[str] = set()
        for entry in remaster_preferred + preferred + list(LEGACY_TEXTURE_EXTENSIONS_BY_PREFERENCE):
            if entry in seen:
                continue
            seen.add(entry)
            ordered.append(entry)
        return ordered

    def _decode_legacy_texture_payload(
        self,
        source_path: Path,
    ) -> Optional[Tuple[bytes, str]]:
        try:
            raw = source_path.read_bytes()
        except OSError:
            return None

        suffix = source_path.suffix.lower()
        stem = source_path.stem

        if suffix in {".ozj", ".ozj2"}:
            if len(raw) <= 24:
                return None
            return raw[24:], f"{stem}.jpg"
        if suffix == ".ozp":
            if len(raw) <= 4:
                return None
            return raw[4:], f"{stem}.png"
        if suffix == ".ozt":
            if len(raw) <= 4:
                return None
            return raw[4:], f"{stem}.tga"
        if suffix == ".ozb":
            if len(raw) <= 4:
                return None
            return raw[4:], f"{stem}.bmp"
        if suffix in {".jpg", ".jpeg", ".png", ".tga", ".bmp"}:
            return raw, source_path.name
        return None

    def _build_legacy_index(self) -> None:
        if (
            self._legacy_by_lower_name is not None
            and self._legacy_by_lower_stem is not None
            and self._legacy_by_normalized_stem is not None
        ):
            return

        by_lower_name: Dict[str, Path] = {}
        by_lower_stem: Dict[str, List[Path]] = {}
        by_normalized_stem: Dict[str, List[Path]] = {}
        if self.legacy_texture_dir and self.legacy_texture_dir.exists() and self.legacy_texture_dir.is_dir():
            legacy_files = sorted(
                (child for child in self.legacy_texture_dir.iterdir() if child.is_file()),
                key=lambda path: path.name.lower(),
            )
            for child in legacy_files:
                if not child.is_file():
                    continue
                by_lower_name.setdefault(child.name.lower(), child)
                by_lower_stem.setdefault(child.stem.lower(), []).append(child)
                normalized_stem = _normalize_texture_lookup_key(child.stem)
                if normalized_stem:
                    by_normalized_stem.setdefault(normalized_stem, []).append(child)
        self._legacy_by_lower_name = by_lower_name
        self._legacy_by_lower_stem = by_lower_stem
        self._legacy_by_normalized_stem = by_normalized_stem

    def _legacy_candidate_from_stem_maps(
        self,
        stem: str,
        suffix: str,
    ) -> Optional[Path]:
        self._build_legacy_index()
        assert self._legacy_by_lower_stem is not None
        assert self._legacy_by_normalized_stem is not None

        lower_stem = stem.lower()
        for candidate in self._legacy_by_lower_stem.get(lower_stem, []):
            if candidate.suffix.lower() == suffix:
                return candidate

        normalized_stem = _normalize_texture_lookup_key(stem)
        if normalized_stem:
            for candidate in self._legacy_by_normalized_stem.get(normalized_stem, []):
                if candidate.suffix.lower() == suffix:
                    return candidate
        return None

    def _resolve_legacy_texture_source(self, texture_name: str) -> Optional[ResolvedTexture]:
        if self.legacy_texture_dir is None:
            return None
        if not self.legacy_texture_dir.exists() or not self.legacy_texture_dir.is_dir():
            return None

        requested = texture_name.strip().replace("\\", "/")
        stem = Path(requested).stem
        if not stem:
            return None
        requested_suffix = Path(requested).suffix.lower()

        for suffix in self._legacy_texture_suffix_order(requested_suffix):
            candidate = self.legacy_texture_dir / f"{stem}{suffix}"
            if not candidate.exists() or not candidate.is_file():
                self._build_legacy_index()
                assert self._legacy_by_lower_name is not None
                candidate = self._legacy_by_lower_name.get(f"{stem}{suffix}".lower())
            if candidate is None or not candidate.exists() or not candidate.is_file():
                candidate = self._legacy_candidate_from_stem_maps(stem=stem, suffix=suffix)
            if candidate is None or not candidate.exists() or not candidate.is_file():
                continue

            decoded = self._decode_legacy_texture_payload(candidate)
            if decoded is None:
                continue

            payload_bytes, virtual_uri = decoded
            has_alpha, has_partial_alpha, transparent_ratio, opaque_ratio = _png_alpha_profile(
                payload_bytes
            )
            (
                black_ratio,
                very_dark_ratio,
                bright_ratio,
                mean_luma,
                luma_std_dev,
                bright_energy_ratio,
            ) = _texture_visual_profile(payload_bytes)
            return ResolvedTexture(
                uri=virtual_uri,
                png_bytes=payload_bytes if self.embed_textures else None,
                found_on_disk=True,
                has_alpha=has_alpha,
                has_partial_alpha=has_partial_alpha,
                transparent_ratio=transparent_ratio,
                opaque_ratio=opaque_ratio,
                black_ratio=black_ratio,
                very_dark_ratio=very_dark_ratio,
                bright_ratio=bright_ratio,
                mean_luma=mean_luma,
                luma_std_dev=luma_std_dev,
                bright_energy_ratio=bright_energy_ratio,
            )

        return None

    def _build_index(self) -> None:
        if self._index_built:
            return
        self._index_built = True

        if not self.texture_dir.exists() or not self.texture_dir.is_dir():
            return

        indexed_suffixes = {".png", ".tga"}
        if self.prefer_remaster_texture_order:
            indexed_suffixes.update({".jpg", ".jpeg"})

        texture_files = sorted(
            (
                child for child in self.texture_dir.iterdir()
                if child.is_file() and child.suffix.lower() in indexed_suffixes
            ),
            key=lambda path: path.name.lower(),
        )

        for path in texture_files:
            lower_name = path.name.lower()
            lower_stem = path.stem.lower()
            normalized_stem = _normalize_texture_lookup_key(path.stem)

            self._by_lower_name.setdefault(lower_name, path)
            self._by_lower_stem.setdefault(lower_stem, path)
            if normalized_stem:
                self._by_normalized_stem.setdefault(normalized_stem, path)

    def _build_fallback_index(self) -> None:
        if (
            self._fallback_by_lower_name is not None
            and self._fallback_by_lower_stem is not None
            and self._fallback_by_normalized_stem is not None
        ):
            return

        if self.fallback_root is None:
            self._fallback_by_lower_name = {}
            self._fallback_by_lower_stem = {}
            self._fallback_by_normalized_stem = {}
            return

        (
            self._fallback_by_lower_name,
            self._fallback_by_lower_stem,
            self._fallback_by_normalized_stem,
        ) = _build_global_png_index(self.fallback_root)

    def _resolve_texture_path(
        self,
        requested_uri: str,
        allow_fallback_root: bool = True,
    ) -> Optional[Path]:
        direct = self.texture_dir / requested_uri
        if direct.exists() and direct.is_file():
            return direct

        self._build_index()

        lower_name = requested_uri.lower()
        if lower_name in self._by_lower_name:
            return self._by_lower_name[lower_name]

        requested_stem = Path(requested_uri).stem
        lower_stem = requested_stem.lower()
        if lower_stem in self._by_lower_stem:
            return self._by_lower_stem[lower_stem]

        normalized_stem = _normalize_texture_lookup_key(requested_stem)
        if normalized_stem and normalized_stem in self._by_normalized_stem:
            return self._by_normalized_stem[normalized_stem]

        if allow_fallback_root and self.fallback_root is not None:
            preferred_dirs: List[Path] = []
            if self.legacy_texture_dir is not None:
                legacy_dir_name = self.legacy_texture_dir.name
                if legacy_dir_name:
                    preferred_dirs.append(self.fallback_root / legacy_dir_name)
                    preferred_dirs.append(self.fallback_root / legacy_dir_name.lower())

            for preferred_dir in preferred_dirs:
                if not preferred_dir.exists() or not preferred_dir.is_dir():
                    continue

                preferred_direct = preferred_dir / requested_uri
                if preferred_direct.exists() and preferred_direct.is_file():
                    return preferred_direct

                (
                    preferred_by_lower_name,
                    preferred_by_lower_stem,
                    preferred_by_normalized_stem,
                ) = _build_global_png_index(preferred_dir)
                if lower_name in preferred_by_lower_name:
                    return preferred_by_lower_name[lower_name]
                if lower_stem in preferred_by_lower_stem:
                    return preferred_by_lower_stem[lower_stem]
                if (
                    normalized_stem
                    and normalized_stem in preferred_by_normalized_stem
                ):
                    return preferred_by_normalized_stem[normalized_stem]

            fallback_direct = self.fallback_root / requested_uri
            if fallback_direct.exists() and fallback_direct.is_file():
                return fallback_direct

            self._build_fallback_index()
            assert self._fallback_by_lower_name is not None
            assert self._fallback_by_lower_stem is not None
            assert self._fallback_by_normalized_stem is not None

            if lower_name in self._fallback_by_lower_name:
                return self._fallback_by_lower_name[lower_name]
            if lower_stem in self._fallback_by_lower_stem:
                return self._fallback_by_lower_stem[lower_stem]
            if (
                normalized_stem
                and normalized_stem in self._fallback_by_normalized_stem
            ):
                return self._fallback_by_normalized_stem[normalized_stem]

        return None

    def _uri_for_path(self, path: Path) -> str:
        if self.embed_textures:
            if path.is_relative_to(self.texture_dir):
                return path.relative_to(self.texture_dir).as_posix()
            if self.fallback_root and path.is_relative_to(self.fallback_root):
                return path.relative_to(self.fallback_root).as_posix()
            return path.name

        try:
            return Path(os.path.relpath(path, self.texture_dir)).as_posix()
        except ValueError:
            return path.name

    def _resolved_texture_from_path(self, resolved_path: Path) -> ResolvedTexture:
        payload_bytes: Optional[bytes] = None
        alpha_probe_payload: Optional[bytes] = None
        if self.embed_textures:
            try:
                payload_bytes = resolved_path.read_bytes()
                alpha_probe_payload = payload_bytes
            except OSError as exc:
                logging.warning(
                    "Failed to read texture '%s' for embedding: %s",
                    resolved_path,
                    exc,
                )
                payload_bytes = None
        else:
            try:
                alpha_probe_payload = resolved_path.read_bytes()
            except OSError as exc:
                logging.warning(
                    "Failed to read texture '%s' for alpha analysis: %s",
                    resolved_path,
                    exc,
                )

        has_alpha = False
        has_partial_alpha = False
        transparent_ratio = 0.0
        opaque_ratio = 1.0
        black_ratio = 0.0
        very_dark_ratio = 0.0
        bright_ratio = 0.0
        mean_luma = 0.0
        luma_std_dev = 0.0
        bright_energy_ratio = 0.0
        if alpha_probe_payload is not None:
            (
                has_alpha,
                has_partial_alpha,
                transparent_ratio,
                opaque_ratio,
            ) = _png_alpha_profile(alpha_probe_payload)
            (
                black_ratio,
                very_dark_ratio,
                bright_ratio,
                mean_luma,
                luma_std_dev,
                bright_energy_ratio,
            ) = _texture_visual_profile(alpha_probe_payload)

        resolved_uri = self._uri_for_path(resolved_path)
        return ResolvedTexture(
            uri=resolved_uri,
            png_bytes=payload_bytes,
            found_on_disk=True,
            has_alpha=has_alpha,
            has_partial_alpha=has_partial_alpha,
            transparent_ratio=transparent_ratio,
            opaque_ratio=opaque_ratio,
            black_ratio=black_ratio,
            very_dark_ratio=very_dark_ratio,
            bright_ratio=bright_ratio,
            mean_luma=mean_luma,
            luma_std_dev=luma_std_dev,
            bright_energy_ratio=bright_energy_ratio,
        )

    def resolve(self, texture_name: str) -> Optional[ResolvedTexture]:
        if self.prefer_remaster_texture_order:
            texture_uris = _texture_name_to_remaster_candidate_uris(texture_name)
        else:
            texture_uris = _texture_name_to_candidate_uris(texture_name)
        if not texture_uris:
            return None

        cache_key = "|".join(uri.lower() for uri in texture_uris)
        cached = self._cache.get(cache_key)
        if cached is not None:
            return cached

        unresolved = ResolvedTexture(
            uri=texture_uris[0],
            png_bytes=None,
            found_on_disk=False,
        )

        for texture_uri in texture_uris:
            resolved_path = self._resolve_texture_path(
                texture_uri,
                allow_fallback_root=False,
            )
            if resolved_path is None:
                continue

            resolved = self._resolved_texture_from_path(resolved_path)
            self._cache[cache_key] = resolved
            return resolved

        legacy_resolved = self._resolve_legacy_texture_source(texture_name)
        if legacy_resolved is not None:
            self._cache[cache_key] = legacy_resolved
            return legacy_resolved

        for texture_uri in texture_uris:
            resolved_path = self._resolve_texture_path(
                texture_uri,
                allow_fallback_root=True,
            )
            if resolved_path is None:
                continue

            resolved = self._resolved_texture_from_path(resolved_path)
            self._cache[cache_key] = resolved
            return resolved

        self._cache[cache_key] = unresolved
        return unresolved


def _resolve_mesh_texture(
    meshes: List[BmdMesh],
    mesh_index: int,
    texture_resolver: Optional[TextureResolver],
) -> Optional[ResolvedTexture]:
    """Resolve the effective texture for a mesh, honoring BMD texture indirection."""
    mesh = meshes[mesh_index]
    candidates: List[str] = []

    # In BMD, Mesh.Texture can point to another entry in the texture table.
    if 0 <= mesh.texture < len(meshes):
        candidates.append(meshes[mesh.texture].texture_name)
    candidates.append(mesh.texture_name)

    fallback: Optional[ResolvedTexture] = None
    for candidate in candidates:
        if texture_resolver is None:
            candidate_uris = _texture_name_to_candidate_uris(candidate)
            if candidate_uris:
                return ResolvedTexture(
                    uri=candidate_uris[0],
                    png_bytes=None,
                    found_on_disk=False,
                )
            continue

        resolved = texture_resolver.resolve(candidate)
        if resolved is None:
            continue
        if resolved.found_on_disk:
            return resolved
        if fallback is None:
            fallback = resolved

    return fallback


def _legacy_object_identity_from_source_path(
    source_path: Optional[Path],
) -> Optional[Tuple[int, int]]:
    if source_path is None:
        return None

    stem_match = OBJECT_MODEL_PATTERN.fullmatch(source_path.stem)
    if not stem_match:
        return None

    model_index = int(stem_match.group(1))
    for part in reversed(source_path.parts[:-1]):
        object_match = OBJECT_DIR_PATTERN.fullmatch(part)
        if object_match:
            object_dir = int(object_match.group(1))
            return object_dir, model_index
    return None


def _legacy_model_key_from_source_path(source_path: Optional[Path]) -> Optional[str]:
    if source_path is None:
        return None

    normalized_stem = NON_ALNUM_PATTERN.sub("", source_path.stem.lower())
    if not normalized_stem:
        return None

    for raw_part in reversed(source_path.parts[:-1]):
        normalized_part = NON_ALNUM_PATTERN.sub("", raw_part.lower())
        if not normalized_part:
            continue
        if normalized_part == "item":
            return f"item/{normalized_stem}"
        if normalized_part == "skill":
            return f"skill/{normalized_stem}"
        if normalized_part.startswith("object") and normalized_part[6:].isdigit():
            return f"{normalized_part}/{normalized_stem}"
    return None


def _material_decision_from_inputs(
    texture_uri: Optional[str],
    legacy_blend_mode: Optional[str],
    texture_signal_profile_by_uri: Dict[
        str,
        Tuple[bool, bool, float, float, float, float, float, float, float, float],
    ],
    material_hint_text: str = "",
    force_opaque_for_set_item: bool = False,
    disable_additive_for_png: bool = False,
) -> Dict[str, object]:
    if not texture_uri:
        return {
            "material_kind": "untextured",
            "alpha_mode": "OPAQUE",
            "double_sided": False,
            "decision_source": "default",
            "decision_detail": "primitive has no resolved texture URI",
            "inference_mode": "untextured",
            "inference_confidence": 1.0,
            "inference_source": "default",
        }

    (
        has_alpha,
        has_partial_alpha,
        transparent_ratio,
        opaque_ratio,
        black_ratio,
        very_dark_ratio,
        bright_ratio,
        mean_luma,
        luma_std_dev,
        bright_energy_ratio,
    ) = texture_signal_profile_by_uri.get(
        texture_uri,
        (False, False, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
    )

    if force_opaque_for_set_item:
        return {
            "material_kind": "textured_pbr",
            "alpha_mode": "OPAQUE",
            "double_sided": False,
            "decision_source": "player_set_override",
            "decision_detail": "set item materials are forced to OPAQUE",
            "inference_mode": "opaque",
            "inference_confidence": 1.0,
            "inference_source": "player_set_override",
        }

    alpha_mode = _classify_alpha_mode(
        has_alpha=has_alpha,
        has_partial_alpha=has_partial_alpha,
        transparent_ratio=transparent_ratio,
        opaque_ratio=opaque_ratio,
    )

    if alpha_mode == "BLEND":
        return {
            "material_kind": "textured_pbr",
            "alpha_mode": "BLEND",
            "double_sided": True,
            "decision_source": "alpha_profile",
            "decision_detail": "partial alpha coverage classified as BLEND",
            "inference_mode": "blend",
            "inference_confidence": 0.95,
            "inference_source": "alpha_profile",
        }
    if alpha_mode == "MASK":
        return {
            "material_kind": "textured_pbr",
            "alpha_mode": "MASK",
            "double_sided": True,
            "decision_source": "alpha_profile",
            "decision_detail": "binary/threshold alpha coverage classified as MASK",
            "inference_mode": "mask",
            "inference_confidence": 0.90,
            "inference_source": "alpha_profile",
        }

    if disable_additive_for_png and Path(texture_uri).suffix.lower() == ".png":
        return {
            "material_kind": "textured_pbr",
            "alpha_mode": "OPAQUE",
            "double_sided": False,
            "decision_source": "remaster_png_override",
            "decision_detail": "remaster PNG disables legacy additive inference/fallback",
            "inference_mode": "opaque",
            "inference_confidence": 1.0,
            "inference_source": "remaster_png_override",
        }

    inferred_additive = _infer_legacy_additive_from_visual_signal(
        has_alpha=has_alpha,
        black_ratio=black_ratio,
        very_dark_ratio=very_dark_ratio,
        bright_ratio=bright_ratio,
        mean_luma=mean_luma,
        luma_std_dev=luma_std_dev,
        bright_energy_ratio=bright_energy_ratio,
        hint_text=material_hint_text,
    )
    if inferred_additive is not None:
        additive_intensity, confidence, detail = inferred_additive
        return {
            "material_kind": "additive_emissive",
            "alpha_mode": "OPAQUE",
            "double_sided": True,
            "decision_source": "legacy_rgb_key",
            "decision_detail": f"inferred additive from RGB profile ({detail})",
            "inference_mode": "additive",
            "inference_confidence": confidence,
            "inference_source": "legacy_rgb_key",
            "additive_intensity": additive_intensity,
            "legacy_blend_reason": "rgb_key_inference",
        }

    if legacy_blend_mode == "additive":
        return {
            "material_kind": "additive_emissive",
            "alpha_mode": "OPAQUE",
            "double_sided": True,
            "decision_source": "legacy_blend_texture_index",
            "decision_detail": "mesh.texture matched legacy BlendMesh texture index fallback",
            "inference_mode": "additive",
            "inference_confidence": 0.65,
            "inference_source": "legacy_blend_texture_index",
            "additive_intensity": _legacy_additive_intensity_from_signal(bright_ratio),
            "legacy_blend_reason": "blend_mesh_texture_index",
        }

    return {
        "material_kind": "textured_pbr",
        "alpha_mode": "OPAQUE",
        "double_sided": False,
        "decision_source": "default",
        "decision_detail": "no alpha signal requiring BLEND/MASK",
        "inference_mode": "opaque",
        "inference_confidence": 0.90,
        "inference_source": "default",
    }


def bmd_to_glb(
    model: BmdModel,
    texture_resolver: Optional[TextureResolver] = None,
    source_path: Optional[Path] = None,
    canonical_player_skeleton: Optional[CanonicalSkeleton] = None,
    blend_probe_records: Optional[List[Dict[str, object]]] = None,
    force_player_inplace: bool = True,
    remaster_mode: bool = False,
) -> Optional[bytes]:
    """Convert a parsed BMD model to GLB (GLTF Binary) bytes.

    Returns None if the model has no renderable geometry.
    """
    if model.num_meshs == 0:
        return None

    # Check if there are any triangles at all
    total_tris = sum(m.num_triangles for m in model.meshs)
    if total_tris == 0:
        return None

    # Compute bone fixups for world-space transform.
    if model.num_bones > 0 and model.num_actions > 0:
        fixups = compute_bone_fixups(model)
    else:
        fixups = []

    export_skinning = bool(fixups) and len(fixups) == model.num_bones
    bind_fixups = fixups
    bind_local_pose_overrides: Optional[
        List[Tuple[Tuple[float, float, float], Tuple[float, float, float]]]
    ] = None

    if export_skinning:
        force_inplace_for_this_model = force_player_inplace and _is_player_source_path(source_path)
        canonical_remap = _build_player_canonical_bone_remap(
            model=model,
            source_path=source_path,
            canonical=canonical_player_skeleton,
        )
        if canonical_remap is not None and canonical_player_skeleton is not None:
            bind_fixups = []
            bind_local_pose_overrides = []
            remapped_count = 0
            for bone_index in range(model.num_bones):
                canonical_index = canonical_remap[bone_index]
                if canonical_index is None:
                    bind_fixups.append(fixups[bone_index])
                    bind_local_pose_overrides.append(
                        _bone_local_pose(model, bone_index, action_index=0, key_index=0)
                    )
                    continue
                bind_fixups.append(canonical_player_skeleton.fixups[canonical_index])
                bind_local_pose_overrides.append(
                    canonical_player_skeleton.local_bind_pose_by_index[canonical_index]
                )
                remapped_count += 1

            logging.debug(
                "Using canonical player skeleton for %s (%d/%d bones remapped from %s)",
                source_path if source_path is not None else model.name,
                remapped_count,
                model.num_bones,
                canonical_player_skeleton.source_path,
            )

    legacy_object_identity = _legacy_object_identity_from_source_path(source_path)
    legacy_model_key = _legacy_model_key_from_source_path(source_path)
    legacy_blend_texture_index = None
    if legacy_object_identity is not None:
        legacy_blend_texture_index = LEGACY_BLEND_TEXTURE_INDEX_BY_OBJECT_MODEL.get(
            legacy_object_identity
        )
        if legacy_blend_texture_index is not None:
            logging.debug(
                "Applying legacy blend hint for %s: object%d/object%d texture_index=%d",
                model.name,
                legacy_object_identity[0],
                legacy_object_identity[1],
                legacy_blend_texture_index,
            )
    if legacy_blend_texture_index is None and legacy_model_key is not None:
        legacy_blend_texture_index = LEGACY_BLEND_TEXTURE_INDEX_BY_MODEL_KEY.get(legacy_model_key)
        if legacy_blend_texture_index is not None:
            logging.debug(
                "Applying legacy blend hint for %s: model=%s texture_index=%d",
                model.name,
                legacy_model_key,
                legacy_blend_texture_index,
            )

    legacy_additive_intensity_override: Optional[float] = None
    if legacy_object_identity is not None:
        legacy_additive_intensity_override = LEGACY_ADDITIVE_INTENSITY_BY_OBJECT_MODEL.get(
            legacy_object_identity
        )
    if legacy_additive_intensity_override is None and legacy_model_key is not None:
        legacy_additive_intensity_override = LEGACY_ADDITIVE_INTENSITY_BY_MODEL_KEY.get(
            legacy_model_key
        )

    # Build unified vertex buffer per mesh, then combine into GLTF primitives.
    all_positions: List[Tuple[float, float, float]] = []
    all_normals: List[Tuple[float, float, float]] = []
    all_texcoords: List[Tuple[float, float]] = []
    all_indices: List[int] = []
    all_joint_indices: List[Tuple[int, int, int, int]] = []
    all_joint_weights: List[Tuple[float, float, float, float]] = []

    primitives_info: List[PrimitiveInfo] = []
    embedded_texture_payloads: Dict[str, bytes] = {}
    texture_signal_profile_by_uri: Dict[
        str,
        Tuple[bool, bool, float, float, float, float, float, float, float, float],
    ] = {}
    missing_textures: set[str] = set()
    warned_vertex_nodes: set[int] = set()
    warned_normal_nodes: set[int] = set()

    for mesh_index, mesh in enumerate(model.meshs):
        if mesh.num_triangles == 0:
            continue

        # De-index: build combined vertices.
        vert_map: Dict[Tuple[int, int, int], int] = {}
        mesh_positions: List[Tuple[float, float, float]] = []
        mesh_normals: List[Tuple[float, float, float]] = []
        mesh_texcoords: List[Tuple[float, float]] = []
        mesh_indices: List[int] = []
        mesh_joint_indices: List[Tuple[int, int, int, int]] = []
        mesh_joint_weights: List[Tuple[float, float, float, float]] = []

        for tri in mesh.triangles:
            n_corners = min(tri.polygon, 4) if tri.polygon >= 3 else 3

            corners: List[int] = []
            for k in range(n_corners):
                vi = tri.vertex_index[k]
                ni = tri.normal_index[k]
                ti = tri.texcoord_index[k]

                # Bounds check.
                if vi < 0 or vi >= mesh.num_vertices:
                    continue
                if ni < 0 or ni >= mesh.num_normals:
                    continue
                if ti < 0 or ti >= mesh.num_texcoords:
                    continue

                key = (vi, ni, ti)
                if key in vert_map:
                    corners.append(vert_map[key])
                    continue

                idx = len(mesh_positions)
                vert_map[key] = idx

                vert = mesh.vertices[vi]
                norm = mesh.normals[ni]
                tc = mesh.texcoords[ti]
                vnode = vert.node
                nnode = norm.node

                if bind_fixups:
                    if 0 <= vnode < len(bind_fixups):
                        vertex_fixup = bind_fixups[vnode]
                    else:
                        if vnode not in warned_vertex_nodes:
                            logging.warning(
                                "Vertex node %d out of range [0,%d) in %s, clamping to 0",
                                vnode,
                                len(bind_fixups),
                                model.name,
                            )
                            warned_vertex_nodes.add(vnode)
                        vertex_fixup = bind_fixups[0]

                    wp = vector_transform(vert.position, vertex_fixup.m)
                    world_pos = (
                        wp[0] + vertex_fixup.world_org[0],
                        wp[1] + vertex_fixup.world_org[1],
                        wp[2] + vertex_fixup.world_org[2],
                    )
                else:
                    world_pos = vert.position

                if bind_fixups:
                    if 0 <= nnode < len(bind_fixups):
                        normal_fixup = bind_fixups[nnode]
                    else:
                        if nnode not in warned_normal_nodes:
                            logging.warning(
                                "Normal node %d out of range [0,%d) in %s, clamping to 0",
                                nnode,
                                len(bind_fixups),
                                model.name,
                            )
                            warned_normal_nodes.add(nnode)
                        normal_fixup = bind_fixups[0]

                    wn = vector_rotate(norm.normal, normal_fixup.m)
                    world_norm = vector_normalize(wn)
                else:
                    world_norm = vector_normalize(norm.normal)

                mesh_positions.append(_swizzle_mu_to_gltf(world_pos))
                mesh_normals.append(vector_normalize(_swizzle_mu_to_gltf(world_norm)))
                mesh_texcoords.append((tc.u, tc.v))

                if export_skinning:
                    joint_index = vnode if 0 <= vnode < model.num_bones else 0
                    mesh_joint_indices.append((joint_index, 0, 0, 0))
                    mesh_joint_weights.append((1.0, 0.0, 0.0, 0.0))

                corners.append(idx)

            # Triangulate.
            if len(corners) >= 3:
                # Axis swizzle flips handedness, so we reverse winding for correct front faces.
                mesh_indices.extend([corners[0], corners[2], corners[1]])
            if len(corners) >= 4:
                # Quad -> two triangles: (0,1,2) and (0,2,3)
                mesh_indices.extend([corners[0], corners[3], corners[2]])

        if not mesh_indices:
            continue

        vert_offset = len(all_positions)
        idx_offset = len(all_indices)

        all_positions.extend(mesh_positions)
        all_normals.extend(mesh_normals)
        all_texcoords.extend(mesh_texcoords)
        all_indices.extend(i + vert_offset for i in mesh_indices)
        if export_skinning:
            all_joint_indices.extend(mesh_joint_indices)
            all_joint_weights.extend(mesh_joint_weights)

        mesh_texture_reference_name: Optional[str] = None
        texture_candidates: List[str] = []
        if 0 <= mesh.texture < len(model.meshs):
            mesh_texture_reference_name = model.meshs[mesh.texture].texture_name
            texture_candidates.append(mesh_texture_reference_name)
        texture_candidates.append(mesh.texture_name)

        resolved_texture = _resolve_mesh_texture(
            model.meshs,
            mesh_index,
            texture_resolver,
        )

        texture_uri: Optional[str] = None
        texture_found_on_disk = False
        texture_has_alpha = False
        texture_has_partial_alpha = False
        texture_transparent_ratio = 0.0
        texture_opaque_ratio = 1.0
        texture_black_ratio = 0.0
        texture_very_dark_ratio = 0.0
        texture_bright_ratio = 0.0
        texture_mean_luma = 0.0
        texture_luma_std_dev = 0.0
        texture_bright_energy_ratio = 0.0
        if resolved_texture is not None:
            texture_found_on_disk = resolved_texture.found_on_disk
            texture_has_alpha = resolved_texture.has_alpha
            texture_has_partial_alpha = resolved_texture.has_partial_alpha
            texture_transparent_ratio = resolved_texture.transparent_ratio
            texture_opaque_ratio = resolved_texture.opaque_ratio
            texture_black_ratio = resolved_texture.black_ratio
            texture_very_dark_ratio = resolved_texture.very_dark_ratio
            texture_bright_ratio = resolved_texture.bright_ratio
            texture_mean_luma = resolved_texture.mean_luma
            texture_luma_std_dev = resolved_texture.luma_std_dev
            texture_bright_energy_ratio = resolved_texture.bright_energy_ratio
            if resolved_texture.found_on_disk:
                resolved_texture_uri = resolved_texture.uri
                resolved_texture_payload = resolved_texture.png_bytes
                resolved_texture_profile = (
                    resolved_texture.has_alpha,
                    resolved_texture.has_partial_alpha,
                    resolved_texture.transparent_ratio,
                    resolved_texture.opaque_ratio,
                    resolved_texture.black_ratio,
                    resolved_texture.very_dark_ratio,
                    resolved_texture.bright_ratio,
                    resolved_texture.mean_luma,
                    resolved_texture.luma_std_dev,
                    resolved_texture.bright_energy_ratio,
                )

                texture_uri = resolved_texture_uri
                (
                    texture_has_alpha,
                    texture_has_partial_alpha,
                    texture_transparent_ratio,
                    texture_opaque_ratio,
                    texture_black_ratio,
                    texture_very_dark_ratio,
                    texture_bright_ratio,
                    texture_mean_luma,
                    texture_luma_std_dev,
                    texture_bright_energy_ratio,
                ) = resolved_texture_profile
                texture_signal_profile_by_uri[texture_uri] = resolved_texture_profile
                if resolved_texture_payload is not None:
                    embedded_texture_payloads.setdefault(texture_uri, resolved_texture_payload)
            else:
                missing_textures.add(resolved_texture.uri)
                if texture_resolver is None or not texture_resolver.embed_textures:
                    texture_uri = resolved_texture.uri

        legacy_blend_mode: Optional[str] = None
        if (
            legacy_blend_texture_index is not None
            and mesh.texture == legacy_blend_texture_index
        ):
            # Legacy RenderMesh uses additive blending for BlendMesh surfaces.
            legacy_blend_mode = "additive"

        primitives_info.append(
            PrimitiveInfo(
                vert_offset=vert_offset,
                vert_count=len(mesh_positions),
                idx_offset=idx_offset,
                idx_count=len(mesh_indices),
                texture_uri=texture_uri,
                legacy_blend_mode=legacy_blend_mode,
                mesh_index=mesh_index,
                mesh_texture_index=mesh.texture,
                mesh_texture_name=mesh.texture_name,
                mesh_texture_reference_name=mesh_texture_reference_name,
                texture_candidates=tuple(texture_candidates),
                texture_found_on_disk=texture_found_on_disk,
                texture_has_alpha=texture_has_alpha,
                texture_has_partial_alpha=texture_has_partial_alpha,
                texture_transparent_ratio=texture_transparent_ratio,
                texture_opaque_ratio=texture_opaque_ratio,
                texture_black_ratio=texture_black_ratio,
                texture_very_dark_ratio=texture_very_dark_ratio,
                texture_bright_ratio=texture_bright_ratio,
                texture_mean_luma=texture_mean_luma,
                texture_luma_std_dev=texture_luma_std_dev,
                texture_bright_energy_ratio=texture_bright_energy_ratio,
            )
        )

    if not all_positions or not all_indices:
        return None

    num_verts = len(all_positions)
    use_uint32 = num_verts > 65535

    # Compute bounding box for POSITION accessor.
    min_pos = [float('inf')] * 3
    max_pos = [float('-inf')] * 3
    for p in all_positions:
        for c in range(3):
            if p[c] < min_pos[c]:
                min_pos[c] = p[c]
            if p[c] > max_pos[c]:
                max_pos[c] = p[c]

    # Build binary payloads.
    pos_data = b''.join(struct.pack('<3f', *p) for p in all_positions)
    norm_data = b''.join(struct.pack('<3f', *n) for n in all_normals)
    tc_data = b''.join(struct.pack('<2f', *t) for t in all_texcoords)
    if use_uint32:
        idx_data = b''.join(struct.pack('<I', i) for i in all_indices)
    else:
        idx_data = b''.join(struct.pack('<H', i) for i in all_indices)

    joint_data = b''
    weight_data = b''
    if export_skinning:
        joint_data = b''.join(struct.pack('<4H', *values) for values in all_joint_indices)
        weight_data = b''.join(struct.pack('<4f', *values) for values in all_joint_weights)

    pos_offset = 0
    pos_size = len(pos_data)
    norm_offset = pos_offset + pos_size
    norm_size = len(norm_data)
    tc_offset = norm_offset + norm_size
    tc_size = len(tc_data)
    idx_offset_buf = tc_offset + tc_size
    idx_size = len(idx_data)

    binary_buffer = bytearray(pos_data + norm_data + tc_data + idx_data)
    base_padding = (4 - len(binary_buffer) % 4) % 4
    if base_padding:
        binary_buffer.extend(b"\x00" * base_padding)

    POSITION_BUFFER_VIEW = 0
    NORMAL_BUFFER_VIEW = 1
    TEXCOORD_BUFFER_VIEW = 2
    INDEX_BUFFER_VIEW = 3

    buffer_views: List[Dict[str, object]] = [
        {"buffer": 0, "byteOffset": pos_offset, "byteLength": pos_size, "target": 34962},
        {"buffer": 0, "byteOffset": norm_offset, "byteLength": norm_size, "target": 34962},
        {"buffer": 0, "byteOffset": tc_offset, "byteLength": tc_size, "target": 34962},
        {"buffer": 0, "byteOffset": idx_offset_buf, "byteLength": idx_size, "target": 34963},
    ]

    accessors: List[Dict[str, object]] = [
        {
            "bufferView": POSITION_BUFFER_VIEW,
            "componentType": 5126,
            "count": num_verts,
            "type": "VEC3",
            "max": max_pos,
            "min": min_pos,
        },
        {
            "bufferView": NORMAL_BUFFER_VIEW,
            "componentType": 5126,
            "count": num_verts,
            "type": "VEC3",
        },
        {
            "bufferView": TEXCOORD_BUFFER_VIEW,
            "componentType": 5126,
            "count": num_verts,
            "type": "VEC2",
        },
    ]

    def append_binary_buffer_view(
        payload: bytes,
        target: Optional[int] = None,
    ) -> int:
        byte_offset = len(binary_buffer)
        binary_buffer.extend(payload)
        payload_padding = (4 - len(binary_buffer) % 4) % 4
        if payload_padding:
            binary_buffer.extend(b"\x00" * payload_padding)

        view: Dict[str, object] = {
            "buffer": 0,
            "byteOffset": byte_offset,
            "byteLength": len(payload),
        }
        if target is not None:
            view["target"] = target

        buffer_view_index = len(buffer_views)
        buffer_views.append(view)
        return buffer_view_index

    joint_accessor_index: Optional[int] = None
    weight_accessor_index: Optional[int] = None
    if export_skinning:
        joint_buffer_view = append_binary_buffer_view(joint_data, target=34962)
        joint_accessor_index = len(accessors)
        accessors.append(
            {
                "bufferView": joint_buffer_view,
                "componentType": 5123,  # UNSIGNED_SHORT
                "count": num_verts,
                "type": "VEC4",
            }
        )

        weight_buffer_view = append_binary_buffer_view(weight_data, target=34962)
        weight_accessor_index = len(accessors)
        accessors.append(
            {
                "bufferView": weight_buffer_view,
                "componentType": 5126,
                "count": num_verts,
                "type": "VEC4",
            }
        )

    images: List[Dict[str, object]] = []
    textures: List[Dict[str, object]] = []
    materials: List[Dict[str, object]] = []
    material_index_by_texture: Dict[Tuple[Optional[str], Optional[str]], int] = {}
    image_buffer_view_by_texture: Dict[Tuple[str, Optional[str], Optional[int]], int] = {}
    uses_khr_materials_unlit = False

    if missing_textures:
        missing_preview = ", ".join(sorted(missing_textures)[:4])
        if texture_resolver is not None:
            logging.debug(
                "Model '%s': %d missing textures in %s (sample: %s)",
                model.name,
                len(missing_textures),
                texture_resolver.texture_dir,
                missing_preview,
            )
        else:
            logging.debug(
                "Model '%s': %d missing textures (sample: %s)",
                model.name,
                len(missing_textures),
                missing_preview,
            )

    model_blend_hint_text = str(source_path) if source_path is not None else model.name
    force_opaque_for_set_item = _is_player_set_item_source_path(source_path)

    def material_decision_for_primitive(
        texture_uri: Optional[str],
        legacy_blend_mode: Optional[str],
    ) -> Dict[str, object]:
        material_hint_text = model_blend_hint_text
        if texture_uri:
            material_hint_text = f"{material_hint_text} {texture_uri}"
        return _material_decision_from_inputs(
            texture_uri=texture_uri,
            legacy_blend_mode=legacy_blend_mode,
            texture_signal_profile_by_uri=texture_signal_profile_by_uri,
            material_hint_text=material_hint_text,
            force_opaque_for_set_item=force_opaque_for_set_item,
            disable_additive_for_png=remaster_mode,
        )

    def ensure_material(
        texture_uri: Optional[str],
        legacy_blend_mode: Optional[str],
    ) -> int:
        cache_key = (texture_uri, legacy_blend_mode)
        cached = material_index_by_texture.get(cache_key)
        if cached is not None:
            return cached

        material: Dict[str, object] = {
            "pbrMetallicRoughness": {
                "metallicFactor": 0.0,
                "roughnessFactor": 1.0,
            }
        }
        additive_alpha_key_threshold: Optional[int] = None
        if legacy_blend_mode == "additive" and legacy_object_identity is not None:
            additive_alpha_key_threshold = LEGACY_ADDITIVE_COLOR_KEY_THRESHOLD_BY_OBJECT_MODEL.get(
                legacy_object_identity
            )

        def apply_legacy_additive_mode(
            alpha_key_applied: bool = False,
            additive_intensity: Optional[float] = None,
            blend_reason: str = "blend_mesh_texture_index",
        ) -> None:
            material["alphaMode"] = "OPAQUE"
            material["doubleSided"] = True
            pbr_add = material.get("pbrMetallicRoughness")
            if isinstance(pbr_add, dict):
                # Keep a visible fallback in generic glTF viewers.
                # Runtime still applies legacy additive behavior via extras.
                pbr_add["baseColorFactor"] = [1.0, 1.0, 1.0, 1.0]
            effective_intensity = legacy_additive_intensity_override
            if effective_intensity is None:
                effective_intensity = additive_intensity
            if effective_intensity is None:
                effective_intensity = LEGACY_ADDITIVE_INTENSITY_DEFAULT
            extras: Dict[str, object] = {
                "legacy_blend_mode": "additive",
                "legacy_blend_mode_reason": blend_reason,
            }
            if additive_alpha_key_threshold is not None:
                extras["mu_legacy_alpha_key_threshold"] = additive_alpha_key_threshold
                if alpha_key_applied:
                    extras["mu_legacy_alpha_key_applied"] = True
            extras["mu_legacy_additive_intensity"] = float(effective_intensity)
            if legacy_blend_texture_index is not None:
                extras["mu_legacy_blend_texture_index"] = legacy_blend_texture_index
            if legacy_object_identity is not None:
                extras["mu_legacy_object_dir"] = legacy_object_identity[0]
                extras["mu_legacy_object_model"] = legacy_object_identity[1]
            material["extras"] = extras

        material_decision = material_decision_for_primitive(texture_uri, legacy_blend_mode)
        if texture_uri:
            image_index = len(images)
            embedded_payload = embedded_texture_payloads.get(texture_uri)
            if embedded_payload is not None:
                image_name_uri = texture_uri
                image_payload_key = (
                    image_name_uri,
                    legacy_blend_mode if legacy_blend_mode == "additive" else None,
                    None,
                )
                image_buffer_view = image_buffer_view_by_texture.get(image_payload_key)
                if image_buffer_view is None:
                    image_buffer_view = append_binary_buffer_view(embedded_payload)
                    image_buffer_view_by_texture[image_payload_key] = image_buffer_view

                images.append(
                    {
                        "bufferView": image_buffer_view,
                        "mimeType": _mime_type_for_texture_uri(image_name_uri),
                        "name": image_name_uri,
                    }
                )
            else:
                images.append({"uri": texture_uri})

            texture_index = len(textures)
            textures.append({"source": image_index})

            if material_decision["material_kind"] == "additive_emissive":
                material["emissiveFactor"] = [1.0, 1.0, 1.0]
                material["emissiveTexture"] = {"index": texture_index}
                pbr = material.get("pbrMetallicRoughness")
                if isinstance(pbr, dict):
                    pbr["baseColorTexture"] = {"index": texture_index}
                apply_legacy_additive_mode(
                    alpha_key_applied=False,
                    additive_intensity=material_decision.get("additive_intensity"),
                    blend_reason=str(
                        material_decision.get(
                            "legacy_blend_reason",
                            "blend_mesh_texture_index",
                        )
                    ),
                )
            else:
                pbr = material["pbrMetallicRoughness"]
                if isinstance(pbr, dict):
                    pbr["baseColorTexture"] = {"index": texture_index}
                alpha_mode = material_decision["alpha_mode"]
                if alpha_mode == "BLEND":
                    material["alphaMode"] = "BLEND"
                    material["doubleSided"] = True
                elif alpha_mode == "MASK":
                    material["alphaMode"] = "MASK"
                    material["alphaCutoff"] = ALPHA_MASK_CUTOFF
                    material["doubleSided"] = True
        elif material_decision["material_kind"] == "additive_emissive":
            apply_legacy_additive_mode(
                alpha_key_applied=False,
                additive_intensity=material_decision.get("additive_intensity"),
                blend_reason=str(
                    material_decision.get(
                        "legacy_blend_reason",
                        "blend_mesh_texture_index",
                    )
                ),
            )

        material_index = len(materials)
        materials.append(material)
        material_index_by_texture[cache_key] = material_index
        return material_index

    mesh_primitives: List[Dict[str, object]] = []
    index_component_type = 5125 if use_uint32 else 5123
    index_component_size = 4 if use_uint32 else 2
    for primitive in primitives_info:
        index_accessor: Dict[str, object] = {
            "bufferView": INDEX_BUFFER_VIEW,
            "componentType": index_component_type,
            "count": primitive.idx_count,
            "type": "SCALAR",
        }

        byte_offset = primitive.idx_offset * index_component_size
        if byte_offset:
            index_accessor["byteOffset"] = byte_offset

        index_accessor_index = len(accessors)
        accessors.append(index_accessor)

        attributes: Dict[str, int] = {
            "POSITION": 0,
            "NORMAL": 1,
            "TEXCOORD_0": 2,
        }
        if (
            export_skinning
            and joint_accessor_index is not None
            and weight_accessor_index is not None
        ):
            attributes["JOINTS_0"] = joint_accessor_index
            attributes["WEIGHTS_0"] = weight_accessor_index

        material_decision = material_decision_for_primitive(
            primitive.texture_uri,
            primitive.legacy_blend_mode,
        )

        mesh_primitives.append(
            {
                "attributes": attributes,
                "indices": index_accessor_index,
                "material": ensure_material(primitive.texture_uri, primitive.legacy_blend_mode),
            }
        )

        if blend_probe_records is not None:
            source_value = str(source_path) if source_path is not None else None
            object_dir = None
            object_model = None
            if legacy_object_identity is not None:
                object_dir, object_model = legacy_object_identity
            blend_probe_records.append(
                {
                    "source_path": source_value,
                    "model_name": model.name,
                    "model_version": model.version,
                    "object_dir": object_dir,
                    "object_model": object_model,
                    "mesh_index": primitive.mesh_index,
                    "mesh_texture_index": primitive.mesh_texture_index,
                    "mesh_texture_name": primitive.mesh_texture_name,
                    "mesh_texture_reference_name": primitive.mesh_texture_reference_name,
                    "texture_candidates": list(primitive.texture_candidates),
                    "resolved_texture_uri": primitive.texture_uri,
                    "resolved_texture_found_on_disk": primitive.texture_found_on_disk,
                    "resolved_texture_has_alpha": primitive.texture_has_alpha,
                    "resolved_texture_has_partial_alpha": primitive.texture_has_partial_alpha,
                    "resolved_texture_transparent_ratio": primitive.texture_transparent_ratio,
                    "resolved_texture_opaque_ratio": primitive.texture_opaque_ratio,
                    "resolved_texture_black_ratio": primitive.texture_black_ratio,
                    "resolved_texture_very_dark_ratio": primitive.texture_very_dark_ratio,
                    "resolved_texture_bright_ratio": primitive.texture_bright_ratio,
                    "resolved_texture_mean_luma": primitive.texture_mean_luma,
                    "resolved_texture_luma_std_dev": primitive.texture_luma_std_dev,
                    "resolved_texture_bright_energy_ratio": primitive.texture_bright_energy_ratio,
                    "legacy_blend_texture_index": legacy_blend_texture_index,
                    "legacy_blend_mode": primitive.legacy_blend_mode,
                    "material_kind": material_decision["material_kind"],
                    "material_alpha_mode": material_decision["alpha_mode"],
                    "material_double_sided": material_decision["double_sided"],
                    "material_decision_source": material_decision["decision_source"],
                    "material_decision_detail": material_decision["decision_detail"],
                    "material_inference_mode": material_decision.get("inference_mode"),
                    "material_inference_confidence": material_decision.get(
                        "inference_confidence"
                    ),
                    "material_inference_source": material_decision.get("inference_source"),
                    "bmd_explicit_blend_flag_present": False,
                    "bmd_explicit_blend_flag_note": (
                        "Parser currently exposes mesh texture indirection only; "
                        "no explicit per-material blend state field is decoded."
                    ),
                }
            )

    nodes: List[Dict[str, object]] = [{"mesh": 0, "name": model.name}]
    scene_nodes: List[int] = [0]
    skins: List[Dict[str, object]] = []
    animations: List[Dict[str, object]] = []

    if export_skinning:
        bone_node_offset = len(nodes)
        bone_nodes: List[Dict[str, object]] = []

        for bone_index, bone in enumerate(model.bones):
            if bind_local_pose_overrides is not None:
                bind_pos, bind_rot = bind_local_pose_overrides[bone_index]
            else:
                bind_pos, bind_rot = _bone_local_pose(model, bone_index, action_index=0, key_index=0)
            translation, quaternion = _mu_local_pose_to_gltf_trs(bind_pos, bind_rot)

            bone_name = bone.name.strip() if bone.name else ""
            if not bone_name:
                bone_name = f"Bone_{bone_index:03d}"

            bone_nodes.append(
                {
                    "name": bone_name,
                    "translation": [translation[0], translation[1], translation[2]],
                    "rotation": [quaternion[0], quaternion[1], quaternion[2], quaternion[3]],
                }
            )

        root_bones: List[int] = []
        for child_index, bone in enumerate(model.bones):
            parent_index = bone.parent
            if 0 <= parent_index < model.num_bones and parent_index != child_index:
                children = bone_nodes[parent_index].setdefault("children", [])
                if isinstance(children, list):
                    children.append(bone_node_offset + child_index)
            else:
                root_bones.append(child_index)

        if not root_bones and model.num_bones > 0:
            root_bones.append(0)

        nodes.extend(bone_nodes)
        scene_nodes.extend(bone_node_offset + root_index for root_index in root_bones)

        joint_nodes = [bone_node_offset + bone_index for bone_index in range(model.num_bones)]

        inverse_bind_values: List[float] = []
        for fixup in bind_fixups:
            bind_global = _fixup_to_gltf_global_matrix(fixup)
            inverse_bind = _inverse_rigid_matrix4(bind_global)
            inverse_bind_values.extend(_matrix4_to_column_major_values(inverse_bind))

        inverse_bind_data = b''.join(struct.pack('<f', value) for value in inverse_bind_values)
        inverse_bind_view = append_binary_buffer_view(inverse_bind_data)
        inverse_bind_accessor = len(accessors)
        accessors.append(
            {
                "bufferView": inverse_bind_view,
                "componentType": 5126,
                "count": model.num_bones,
                "type": "MAT4",
            }
        )

        skin: Dict[str, object] = {
            "joints": joint_nodes,
            "inverseBindMatrices": inverse_bind_accessor,
        }
        if root_bones:
            skin["skeleton"] = bone_node_offset + root_bones[0]

        skins.append(skin)
        nodes[0]["skin"] = 0

        # Export one glTF animation per BMD action.
        for action_index, action in enumerate(model.actions):
            key_count = action.num_animation_keys
            if key_count <= 0:
                continue

            root_start_positions_by_bone: Dict[int, Tuple[float, float, float]] = {}
            if force_inplace_for_this_model:
                for root_bone_index in _inplace_root_bone_indices(model):
                    root_start_positions_by_bone[root_bone_index], _ = _bone_local_pose(
                        model,
                        bone_index=root_bone_index,
                        action_index=action_index,
                        key_index=0,
                    )

            time_values = [float(key) / 30.0 for key in range(key_count)]
            time_accessor: Optional[int] = None

            samplers: List[Dict[str, object]] = []
            channels: List[Dict[str, object]] = []

            for bone_index in range(model.num_bones):
                translation_values: List[Tuple[float, float, float]] = []
                rotation_values: List[Tuple[float, float, float, float]] = []

                for key_index in range(key_count):
                    pos_mu, rot_mu = _bone_local_pose(
                        model,
                        bone_index=bone_index,
                        action_index=action_index,
                        key_index=key_index,
                    )
                    if root_start_positions_by_bone:
                        pos_mu = _apply_player_inplace_root_motion(
                            pos_mu=pos_mu,
                            bone_index=bone_index,
                            root_start_positions_by_bone=root_start_positions_by_bone,
                        )
                    translation, quaternion = _mu_local_pose_to_gltf_trs(pos_mu, rot_mu)
                    translation_values.append(translation)
                    rotation_values.append(quaternion)

                # Always emit TR tracks per bone for every action.
                # Omitting constant tracks causes pose leakage when switching clips
                # (skill/action leaves transforms that idle clip never overrides).
                emit_translation = True
                emit_rotation = True

                if time_accessor is None:
                    time_data = b''.join(struct.pack('<f', value) for value in time_values)
                    time_view = append_binary_buffer_view(time_data)
                    time_accessor = len(accessors)
                    accessors.append(
                        {
                            "bufferView": time_view,
                            "componentType": 5126,
                            "count": key_count,
                            "type": "SCALAR",
                            "min": [time_values[0]],
                            "max": [time_values[-1]],
                        }
                    )

                if emit_translation:
                    translation_data = b''.join(
                        struct.pack('<3f', value[0], value[1], value[2])
                        for value in translation_values
                    )
                    translation_view = append_binary_buffer_view(translation_data)
                    translation_accessor = len(accessors)
                    accessors.append(
                        {
                            "bufferView": translation_view,
                            "componentType": 5126,
                            "count": key_count,
                            "type": "VEC3",
                        }
                    )

                    translation_sampler = len(samplers)
                    samplers.append(
                        {
                            "input": time_accessor,
                            "output": translation_accessor,
                            "interpolation": "LINEAR",
                        }
                    )
                    channels.append(
                        {
                            "sampler": translation_sampler,
                            "target": {
                                "node": bone_node_offset + bone_index,
                                "path": "translation",
                            },
                        }
                    )

                if emit_rotation:
                    rotation_data = b''.join(
                        struct.pack('<4f', value[0], value[1], value[2], value[3])
                        for value in rotation_values
                    )
                    rotation_view = append_binary_buffer_view(rotation_data)
                    rotation_accessor = len(accessors)
                    accessors.append(
                        {
                            "bufferView": rotation_view,
                            "componentType": 5126,
                            "count": key_count,
                            "type": "VEC4",
                        }
                    )

                    rotation_sampler = len(samplers)
                    samplers.append(
                        {
                            "input": time_accessor,
                            "output": rotation_accessor,
                            "interpolation": "LINEAR",
                        }
                    )
                    channels.append(
                        {
                            "sampler": rotation_sampler,
                            "target": {
                                "node": bone_node_offset + bone_index,
                                "path": "rotation",
                            },
                        }
                    )

            if not channels:
                continue

            animation: Dict[str, object] = {
                "name": f"Action{action_index:02d}",
                "samplers": samplers,
                "channels": channels,
            }
            if action.lock_positions:
                animation["extras"] = {"lock_positions": True}
            animations.append(animation)

    # Build GLTF JSON.
    gltf = {
        "asset": {"version": "2.0", "generator": "mu-rust bmd_converter.py"},
        "scene": 0,
        "scenes": [{"nodes": scene_nodes}],
        "nodes": nodes,
        "buffers": [{"byteLength": len(binary_buffer)}],
        "bufferViews": buffer_views,
        "accessors": accessors,
        "meshes": [{
            "name": model.name,
            "primitives": mesh_primitives,
        }],
    }

    if skins:
        gltf["skins"] = skins
    if animations:
        gltf["animations"] = animations
    if images:
        gltf["images"] = images
    if textures:
        gltf["textures"] = textures
    if materials:
        gltf["materials"] = materials
    if uses_khr_materials_unlit:
        gltf["extensionsUsed"] = ["KHR_materials_unlit"]

    # Encode GLB.
    json_bytes = json.dumps(gltf, indent=2).encode('ascii')
    json_pad = (4 - len(json_bytes) % 4) % 4
    json_bytes += b' ' * json_pad

    bin_pad = (4 - len(binary_buffer) % 4) % 4
    binary_buffer += b'\x00' * bin_pad

    total_length = 12 + 8 + len(json_bytes) + 8 + len(binary_buffer)
    glb = bytearray()
    glb += struct.pack('<III', 0x46546C67, 2, total_length)
    glb += struct.pack('<II', len(json_bytes), 0x4E4F534A)
    glb += json_bytes
    glb += struct.pack('<II', len(binary_buffer), 0x004E4942)
    glb += binary_buffer

    return bytes(glb)


def bmd_to_skeleton_glb(
    model: BmdModel,
    source_path: Optional[Path] = None,
    force_player_inplace: bool = True,
) -> Optional[bytes]:
    """Convert a parsed BMD model with bones+animations but 0 meshes to GLB.

    Creates a skeleton-only GLB containing bone nodes and all animation clips.
    This is used for files like player.bmd which carry a skeleton and animations
    but no renderable geometry (body parts are in separate BMDs).

    Returns None if the model has no bones or no actions.
    """
    if model.num_bones == 0 or model.num_actions <= 1:
        return None

    fixups = compute_bone_fixups(model)
    if len(fixups) != model.num_bones:
        return None

    # Binary buffer for GLB
    binary_buffer = bytearray()
    buffer_views: List[Dict[str, object]] = []
    accessors: List[Dict[str, object]] = []

    def append_binary_buffer_view(data: bytes) -> int:
        alignment = (4 - len(binary_buffer) % 4) % 4
        binary_buffer.extend(b'\x00' * alignment)
        offset = len(binary_buffer)
        binary_buffer.extend(data)
        view_index = len(buffer_views)
        buffer_views.append({
            "buffer": 0,
            "byteOffset": offset,
            "byteLength": len(data),
        })
        return view_index

    # Build bone nodes
    bone_node_offset = 0  # No mesh node — bones start at index 0
    bone_nodes: List[Dict[str, object]] = []

    for bone_index, bone in enumerate(model.bones):
        bind_pos, bind_rot = _bone_local_pose(model, bone_index, action_index=0, key_index=0)
        translation, quaternion = _mu_local_pose_to_gltf_trs(bind_pos, bind_rot)

        bone_name = bone.name.strip() if bone.name else ""
        if not bone_name:
            bone_name = f"Bone_{bone_index:03d}"

        bone_nodes.append({
            "name": bone_name,
            "translation": [translation[0], translation[1], translation[2]],
            "rotation": [quaternion[0], quaternion[1], quaternion[2], quaternion[3]],
        })

    root_bones: List[int] = []
    for child_index, bone in enumerate(model.bones):
        parent_index = bone.parent
        if 0 <= parent_index < model.num_bones and parent_index != child_index:
            children = bone_nodes[parent_index].setdefault("children", [])
            if isinstance(children, list):
                children.append(bone_node_offset + child_index)
        else:
            root_bones.append(child_index)

    if not root_bones and model.num_bones > 0:
        root_bones.append(0)

    nodes: List[Dict[str, object]] = list(bone_nodes)
    scene_nodes: List[int] = [bone_node_offset + ri for ri in root_bones]

    # Build skin with inverse bind matrices
    joint_nodes = [bone_node_offset + bi for bi in range(model.num_bones)]

    inverse_bind_values: List[float] = []
    for fixup in fixups:
        bind_global = _fixup_to_gltf_global_matrix(fixup)
        inverse_bind = _inverse_rigid_matrix4(bind_global)
        inverse_bind_values.extend(_matrix4_to_column_major_values(inverse_bind))

    inverse_bind_data = b''.join(struct.pack('<f', v) for v in inverse_bind_values)
    inverse_bind_view = append_binary_buffer_view(inverse_bind_data)
    inverse_bind_accessor = len(accessors)
    accessors.append({
        "bufferView": inverse_bind_view,
        "componentType": 5126,
        "count": model.num_bones,
        "type": "MAT4",
    })

    skin: Dict[str, object] = {
        "joints": joint_nodes,
        "inverseBindMatrices": inverse_bind_accessor,
    }
    if root_bones:
        skin["skeleton"] = bone_node_offset + root_bones[0]

    skins = [skin]

    # Export animations (same logic as bmd_to_glb)
    animations: List[Dict[str, object]] = []
    force_inplace_for_this_model = force_player_inplace and _is_player_source_path(source_path)
    for action_index, action in enumerate(model.actions):
        key_count = action.num_animation_keys
        if key_count <= 0:
            continue

        root_start_positions_by_bone: Dict[int, Tuple[float, float, float]] = {}
        if force_inplace_for_this_model:
            for root_bone_index in _inplace_root_bone_indices(model):
                root_start_positions_by_bone[root_bone_index], _ = _bone_local_pose(
                    model,
                    bone_index=root_bone_index,
                    action_index=action_index,
                    key_index=0,
                )

        time_values = [float(key) / 30.0 for key in range(key_count)]
        time_accessor: Optional[int] = None

        samplers: List[Dict[str, object]] = []
        channels: List[Dict[str, object]] = []

        for bone_index in range(model.num_bones):
            translation_values: List[Tuple[float, float, float]] = []
            rotation_values: List[Tuple[float, float, float, float]] = []

            for key_index in range(key_count):
                pos_mu, rot_mu = _bone_local_pose(
                    model,
                    bone_index=bone_index,
                    action_index=action_index,
                    key_index=key_index,
                )
                if root_start_positions_by_bone:
                    pos_mu = _apply_player_inplace_root_motion(
                        pos_mu=pos_mu,
                        bone_index=bone_index,
                        root_start_positions_by_bone=root_start_positions_by_bone,
                    )
                translation, quaternion = _mu_local_pose_to_gltf_trs(pos_mu, rot_mu)
                translation_values.append(translation)
                rotation_values.append(quaternion)

            # Always emit TR tracks per bone for every action.
            # Omitting constant tracks causes pose leakage when switching clips
            # (skill/action leaves transforms that idle clip never overrides).
            emit_translation = True
            emit_rotation = True

            if time_accessor is None:
                time_data = b''.join(struct.pack('<f', v) for v in time_values)
                time_view = append_binary_buffer_view(time_data)
                time_accessor = len(accessors)
                accessors.append({
                    "bufferView": time_view,
                    "componentType": 5126,
                    "count": key_count,
                    "type": "SCALAR",
                    "min": [time_values[0]],
                    "max": [time_values[-1]],
                })

            if emit_translation:
                translation_data = b''.join(
                    struct.pack('<3f', v[0], v[1], v[2])
                    for v in translation_values
                )
                translation_view = append_binary_buffer_view(translation_data)
                translation_accessor = len(accessors)
                accessors.append({
                    "bufferView": translation_view,
                    "componentType": 5126,
                    "count": key_count,
                    "type": "VEC3",
                })

                translation_sampler = len(samplers)
                samplers.append({
                    "input": time_accessor,
                    "output": translation_accessor,
                    "interpolation": "LINEAR",
                })
                channels.append({
                    "sampler": translation_sampler,
                    "target": {
                        "node": bone_node_offset + bone_index,
                        "path": "translation",
                    },
                })

            if emit_rotation:
                rotation_data = b''.join(
                    struct.pack('<4f', v[0], v[1], v[2], v[3])
                    for v in rotation_values
                )
                rotation_view = append_binary_buffer_view(rotation_data)
                rotation_accessor = len(accessors)
                accessors.append({
                    "bufferView": rotation_view,
                    "componentType": 5126,
                    "count": key_count,
                    "type": "VEC4",
                })

                rotation_sampler = len(samplers)
                samplers.append({
                    "input": time_accessor,
                    "output": rotation_accessor,
                    "interpolation": "LINEAR",
                })
                channels.append({
                    "sampler": rotation_sampler,
                    "target": {
                        "node": bone_node_offset + bone_index,
                        "path": "rotation",
                    },
                })

        if not channels:
            continue

        animation: Dict[str, object] = {
            "name": f"Action{action_index:03d}",
            "samplers": samplers,
            "channels": channels,
        }
        if action.lock_positions:
            animation["extras"] = {"lock_positions": True}
        animations.append(animation)

    if not animations:
        return None

    # Build GLTF JSON
    gltf: Dict[str, object] = {
        "asset": {"version": "2.0", "generator": "mu-rust bmd_converter.py (skeleton-only)"},
        "scene": 0,
        "scenes": [{"nodes": scene_nodes}],
        "nodes": nodes,
        "buffers": [{"byteLength": len(binary_buffer)}],
        "bufferViews": buffer_views,
        "accessors": accessors,
        "skins": skins,
        "animations": animations,
    }

    # Encode GLB
    json_bytes = json.dumps(gltf, indent=2).encode('ascii')
    json_pad = (4 - len(json_bytes) % 4) % 4
    json_bytes += b' ' * json_pad

    bin_pad = (4 - len(binary_buffer) % 4) % 4
    binary_buffer += b'\x00' * bin_pad

    total_length = 12 + 8 + len(json_bytes) + 8 + len(binary_buffer)
    glb = bytearray()
    glb += struct.pack('<III', 0x46546C67, 2, total_length)
    glb += struct.pack('<II', len(json_bytes), 0x4E4F534A)
    glb += json_bytes
    glb += struct.pack('<II', len(binary_buffer), 0x004E4942)
    glb += binary_buffer

    return bytes(glb)


# ---------------------------------------------------------------------------
# Batch conversion
# ---------------------------------------------------------------------------

@dataclass
class ConversionStats:
    total_found: int = 0
    converted: int = 0
    skipped_no_geometry: int = 0
    skipped_non_model: int = 0
    skipped_existing: int = 0
    skipped_corrupt: int = 0
    pruned_embedded_pngs: int = 0
    kept_pngs_not_embedded: int = 0
    failed: int = 0
    failures: List[Dict] = field(default_factory=list)
    remaster_variants_generated: int = 0
    remaster_variants_skipped_no_ready_texture: int = 0
    remaster_variants_failed: int = 0
    remaster_failures: List[Dict] = field(default_factory=list)
    blend_probe_entries: List[Dict[str, object]] = field(default_factory=list)


def merge_stats(target: ConversionStats, source: ConversionStats) -> None:
    """Merge *source* into *target* by summing int fields and extending list fields."""
    for f in dataclass_fields(ConversionStats):
        src_val = getattr(source, f.name)
        if isinstance(src_val, int):
            setattr(target, f.name, getattr(target, f.name) + src_val)
        elif isinstance(src_val, list):
            getattr(target, f.name).extend(src_val)


def is_non_model_bmd(file_path: Path) -> bool:
    """Check if a BMD file is actually a data table, not a 3D model."""
    stem = file_path.stem.lower()
    return stem in NON_MODEL_STEMS


def _convert_model_to_glb_bytes(
    model: BmdModel,
    source: Path,
    texture_dir: Path,
    fallback_root: Path,
    embed_textures: bool,
    canonical_player_skeleton: Optional[CanonicalSkeleton],
    force_player_inplace: bool,
    blend_probe_records: Optional[List[Dict[str, object]]],
    conversion_label: str,
) -> Optional[bytes]:
    remaster_mode = conversion_label == "remaster"
    texture_resolver = TextureResolver(
        texture_dir=texture_dir,
        embed_textures=embed_textures,
        fallback_root=fallback_root,
        legacy_texture_dir=source.parent,
        prefer_remaster_texture_order=remaster_mode,
    )
    glb_bytes = bmd_to_glb(
        model,
        texture_resolver=texture_resolver,
        source_path=source,
        canonical_player_skeleton=canonical_player_skeleton,
        blend_probe_records=blend_probe_records,
        force_player_inplace=force_player_inplace,
        remaster_mode=remaster_mode,
    )

    if glb_bytes is None and model.num_bones > 0 and model.num_actions > 1:
        glb_bytes = bmd_to_skeleton_glb(
            model,
            source_path=source,
            force_player_inplace=force_player_inplace,
        )
        if glb_bytes is not None:
            logging.info(
                "Skeleton-only GLB (%s) for %s: %d bones, %d actions, %d bytes",
                conversion_label,
                source,
                model.num_bones,
                model.num_actions,
                len(glb_bytes),
            )

    return glb_bytes


def convert_single_bmd(
    source: Path,
    output_path: Path,
    output_root: Path,
    force: bool,
    embed_textures: bool,
    canonical_player_skeleton_path: Optional[Path],
    force_player_inplace: bool,
    blend_probe: bool,
    remaster_variant_enabled: bool,
    remaster_ready_root: Path,
    remaster_output_root: Path,
    stats: ConversionStats,
) -> None:
    """Convert a single BMD file to GLB."""
    stats.total_found += 1

    if is_non_model_bmd(source):
        stats.skipped_non_model += 1
        logging.debug("Skipping non-model BMD: %s", source)
        return

    # Check magic without full parse
    try:
        with open(source, 'rb') as f:
            header = f.read(4)
        if len(header) < 3 or header[:3] != b'BMD':
            stats.skipped_non_model += 1
            logging.debug("Skipping non-BMD file: %s (magic: %r)", source, header[:3])
            return
    except OSError as exc:
        stats.failed += 1
        stats.failures.append({"source": str(source), "error": str(exc)})
        logging.error("Cannot read %s: %s", source, exc)
        return

    base_output_exists = not force and output_path.exists() and output_path.stat().st_size >= 128
    if base_output_exists:
        stats.skipped_existing += 1
        logging.debug("Skipping existing base GLB: %s", output_path)
        if not remaster_variant_enabled:
            return

    try:
        canonical_player_skeleton = _load_canonical_player_skeleton(canonical_player_skeleton_path)
    except Exception as exc:
        stats.failed += 1
        stats.failures.append({"source": str(source), "error": str(exc), "type": "canonical_skeleton"})
        logging.error("Canonical skeleton setup error for %s: %s", source, exc)
        return

    try:
        model = parse_bmd(source)
    except BmdParseError as exc:
        stats.skipped_corrupt += 1
        stats.failures.append({"source": str(source), "error": str(exc), "type": "parse"})
        logging.warning("Parse error for %s: %s", source, exc)
        return
    except Exception as exc:
        stats.failed += 1
        stats.failures.append({"source": str(source), "error": str(exc), "type": "unexpected"})
        logging.error("Unexpected error parsing %s: %s", source, exc)
        return

    if not base_output_exists:
        try:
            blend_probe_records: Optional[List[Dict[str, object]]] = [] if blend_probe else None
            glb_bytes = _convert_model_to_glb_bytes(
                model=model,
                source=source,
                texture_dir=output_path.parent,
                fallback_root=output_root,
                embed_textures=embed_textures,
                canonical_player_skeleton=canonical_player_skeleton,
                force_player_inplace=force_player_inplace,
                blend_probe_records=blend_probe_records,
                conversion_label="base",
            )
        except Exception as exc:
            stats.failed += 1
            stats.failures.append({"source": str(source), "error": str(exc), "type": "convert"})
            logging.error("Conversion error for %s: %s", source, exc)
            return

        if blend_probe and blend_probe_records:
            stats.blend_probe_entries.extend(blend_probe_records)

        if glb_bytes is None:
            stats.skipped_no_geometry += 1
            logging.debug("No geometry in %s (meshs=%d)", source, model.num_meshs)
            return

        if len(glb_bytes) < 128:
            stats.failed += 1
            stats.failures.append({
                "source": str(source), "error": f"GLB too small ({len(glb_bytes)} bytes)",
                "type": "validation",
            })
            logging.warning("GLB output too small for %s: %d bytes", source, len(glb_bytes))
            return

        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_bytes(glb_bytes)
        stats.converted += 1
        logging.debug("Converted %s -> %s (%d bytes)", source, output_path, len(glb_bytes))

    if not remaster_variant_enabled:
        return

    relative_output = _relative_output_glb_path(output_root, output_path)
    remaster_texture_dir = remaster_ready_root / relative_output.parent
    if not _model_references_remaster_ready_texture(model, remaster_texture_dir):
        stats.remaster_variants_skipped_no_ready_texture += 1
        logging.debug(
            "Remaster variant skipped (no matching ready textures) for %s in %s",
            source,
            remaster_texture_dir,
        )
        return

    remaster_output_path = remaster_output_root / relative_output
    try:
        remaster_glb_bytes = _convert_model_to_glb_bytes(
            model=model,
            source=source,
            texture_dir=remaster_texture_dir,
            fallback_root=output_root,
            embed_textures=True,
            canonical_player_skeleton=canonical_player_skeleton,
            force_player_inplace=force_player_inplace,
            blend_probe_records=None,
            conversion_label="remaster",
        )
    except Exception as exc:
        stats.remaster_variants_failed += 1
        stats.remaster_failures.append(
            {"source": str(source), "error": str(exc), "type": "remaster_convert"}
        )
        logging.warning("Remaster variant conversion error for %s: %s", source, exc)
        return

    if remaster_glb_bytes is None:
        stats.remaster_variants_failed += 1
        stats.remaster_failures.append(
            {
                "source": str(source),
                "error": "no renderable geometry in remaster variant",
                "type": "remaster_no_geometry",
            }
        )
        logging.warning("Remaster variant returned no geometry for %s", source)
        return

    if len(remaster_glb_bytes) < 128:
        stats.remaster_variants_failed += 1
        stats.remaster_failures.append(
            {
                "source": str(source),
                "error": f"GLB too small ({len(remaster_glb_bytes)} bytes)",
                "type": "remaster_validation",
            }
        )
        logging.warning(
            "Remaster variant GLB too small for %s: %d bytes",
            source,
            len(remaster_glb_bytes),
        )
        return

    try:
        remaster_output_path.parent.mkdir(parents=True, exist_ok=True)
        remaster_output_path.write_bytes(remaster_glb_bytes)
    except OSError as exc:
        stats.remaster_variants_failed += 1
        stats.remaster_failures.append(
            {"source": str(source), "error": str(exc), "type": "remaster_write"}
        )
        logging.warning("Failed to write remaster variant for %s: %s", source, exc)
        return

    stats.remaster_variants_generated += 1
    logging.debug(
        "Generated remaster variant %s -> %s (%d bytes)",
        source,
        remaster_output_path,
        len(remaster_glb_bytes),
    )


def _bmd_convert_worker(
    source: Path,
    output_path: Path,
    output_root: Path,
    force: bool,
    embed_textures: bool,
    canonical_player_skeleton_path: Optional[Path],
    force_player_inplace: bool,
    blend_probe: bool,
    remaster_variant_enabled: bool,
    remaster_ready_root: Path,
    remaster_output_root: Path,
) -> ConversionStats:
    """Worker function for parallel BMD conversion. Returns local stats."""
    stats = ConversionStats()
    try:
        convert_single_bmd(
            source,
            output_path,
            output_root,
            force,
            embed_textures,
            canonical_player_skeleton_path,
            force_player_inplace,
            blend_probe,
            remaster_variant_enabled,
            remaster_ready_root,
            remaster_output_root,
            stats,
        )
    except Exception as exc:  # noqa: BLE001
        stats.failed += 1
        stats.failures.append({"source": str(source), "error": str(exc), "type": "worker"})
        logging.error("BMD worker error for %s: %s", source, exc)
    return stats


def parse_world_token(raw_value: str) -> int:
    token = raw_value.strip()
    if not token:
        raise ValueError("empty world token")

    world_match = WORLD_DIR_PATTERN.fullmatch(token)
    if world_match:
        return int(world_match.group(1))

    object_match = OBJECT_DIR_PATTERN.fullmatch(token)
    if object_match:
        return int(object_match.group(1))

    if token.isdigit():
        return int(token)

    raise ValueError(
        f"invalid world token '{raw_value}'. Use values like '74' or 'World74'."
    )


def parse_world_filters(raw_values: List[str]) -> Optional[set[int]]:
    if not raw_values:
        return None

    worlds: set[int] = set()
    for raw in raw_values:
        for token in raw.split(","):
            stripped = token.strip()
            if not stripped:
                continue
            worlds.add(parse_world_token(stripped))
    return worlds or None


def path_matches_world_filter(path: Path, world_filter: Optional[set[int]]) -> bool:
    if not world_filter:
        return True

    for part in path.parts:
        world_match = WORLD_DIR_PATTERN.fullmatch(part)
        if world_match and int(world_match.group(1)) in world_filter:
            return True
        object_match = OBJECT_DIR_PATTERN.fullmatch(part)
        if object_match and int(object_match.group(1)) in world_filter:
            return True
    return False


def canonicalize_output_rel_path(path: Path) -> Path:
    parts = list(path.parts)
    if parts and parts[0].lower() == "data":
        parts = parts[1:]
    if not parts:
        return Path()

    normalized: List[str] = []
    for part in parts:
        world_match = WORLD_DIR_PATTERN.fullmatch(part)
        if world_match:
            normalized.append(f"world_{int(world_match.group(1))}")
            continue
        object_match = OBJECT_DIR_PATTERN.fullmatch(part)
        if object_match:
            normalized.append(f"object_{int(object_match.group(1))}")
            continue
        normalized.append(part)

    # Normalize model filenames (e.g. Object40.bmd -> object40.bmd) so output
    # paths stay consistent with scene references that use lowercase names.
    filename = Path(normalized[-1])
    model_match = OBJECT_MODEL_PATTERN.fullmatch(filename.stem)
    if model_match:
        normalized[-1] = f"object_{int(model_match.group(1))}{filename.suffix.lower()}"

    return Path(*normalized)


def _infer_assets_root_from_output_root(output_root: Path) -> Path:
    resolved = output_root.resolve()
    if resolved.name.lower() == "data":
        return resolved.parent
    if resolved.name.lower() == "assets":
        return resolved

    parts = list(resolved.parts)
    for index in range(len(parts) - 1, -1, -1):
        if parts[index].lower() == "assets":
            return Path(*parts[: index + 1])

    return resolved


def _default_remaster_ready_root(output_root: Path) -> Path:
    assets_root = _infer_assets_root_from_output_root(output_root)
    return assets_root / "remaster" / "textures" / "ready"


def _default_remaster_output_root(output_root: Path) -> Path:
    assets_root = _infer_assets_root_from_output_root(output_root)
    return assets_root / "remaster" / "data"


def _relative_output_glb_path(output_root: Path, output_path: Path) -> Path:
    try:
        return output_path.relative_to(output_root)
    except ValueError:
        return Path(output_path.name)


def _model_references_remaster_ready_texture(
    model: BmdModel,
    remaster_texture_dir: Path,
) -> bool:
    if not remaster_texture_dir.exists() or not remaster_texture_dir.is_dir():
        return False

    texture_names: List[str] = []
    for mesh in model.meshs:
        texture_names.append(mesh.texture_name)
        if 0 <= mesh.texture < len(model.meshs):
            texture_names.append(model.meshs[mesh.texture].texture_name)

    for texture_name in texture_names:
        for candidate_uri in _texture_name_to_remaster_candidate_uris(texture_name):
            candidate_path = remaster_texture_dir / candidate_uri
            if candidate_path.exists() and candidate_path.is_file():
                return True

    return False


def discover_bmd_files(root: Path, world_filter: Optional[set[int]] = None) -> List[Path]:
    """Discover all .bmd files under root, case-insensitive."""
    result = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for fname in filenames:
            if fname.lower().endswith('.bmd'):
                candidate = Path(dirpath) / fname
                rel = canonicalize_output_rel_path(candidate.relative_to(root))
                if not path_matches_world_filter(rel, world_filter):
                    continue
                result.append(candidate)
    result.sort()
    return result


def _read_glb_json_chunk(path: Path) -> Dict[str, object]:
    data = path.read_bytes()
    if len(data) < 20:
        raise ValueError("GLB payload too small")

    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != 0x46546C67:
        raise ValueError("invalid GLB magic")
    if version != 2:
        raise ValueError(f"unsupported GLB version: {version}")
    if total_length > len(data):
        raise ValueError("GLB truncated")

    json_length, json_type = struct.unpack_from("<II", data, 12)
    if json_type != 0x4E4F534A:
        raise ValueError("GLB missing JSON chunk")

    json_start = 20
    json_end = json_start + json_length
    if json_end > len(data):
        raise ValueError("GLB JSON chunk truncated")

    payload = json.loads(data[json_start:json_end].decode("utf-8").rstrip(" \t\r\n\x00"))
    if not isinstance(payload, dict):
        raise ValueError("GLB JSON root is not an object")
    return payload


def _external_png_uris_from_glb(path: Path) -> set[str]:
    payload = _read_glb_json_chunk(path)
    images = payload.get("images")
    if not isinstance(images, list):
        return set()

    uris: set[str] = set()
    for image in images:
        if not isinstance(image, dict):
            continue
        uri = image.get("uri")
        if not isinstance(uri, str) or not uri or uri.startswith("data:"):
            continue
        uri_path = uri.replace("\\", "/")
        uris.add(Path(uri_path).name.lower())
    return uris


def _embedded_png_names_from_glb(path: Path) -> set[str]:
    """Extract names of embedded (non-external) PNG images from a GLB."""
    payload = _read_glb_json_chunk(path)
    images = payload.get("images")
    if not isinstance(images, list):
        return set()
    names: set[str] = set()
    for image in images:
        if not isinstance(image, dict):
            continue
        if "bufferView" not in image:
            continue
        name = image.get("name")
        if isinstance(name, str) and name:
            names.add(Path(name).name.lower())
    return names


def prune_embedded_texture_pngs(
    model_dirs: List[Path],
    dry_run: bool,
) -> Tuple[int, int]:
    """Remove PNGs that are confirmed embedded in GLBs."""
    pruned = 0
    kept = 0

    for model_dir in sorted(set(model_dirs)):
        if not model_dir.exists() or not model_dir.is_dir():
            continue

        glb_files = sorted(model_dir.glob("*.glb"))
        if not glb_files:
            continue

        embedded_refs: set[str] = set()
        parse_failed = False
        for glb_path in glb_files:
            try:
                embedded_refs.update(_embedded_png_names_from_glb(glb_path))
            except Exception as exc:  # noqa: BLE001
                logging.warning(
                    "Skipping PNG prune in %s due to unreadable GLB %s (%s)",
                    model_dir,
                    glb_path.name,
                    exc,
                )
                parse_failed = True
                break
        if parse_failed:
            continue

        for png_path in sorted(model_dir.glob("*.png")):
            if png_path.name.lower() not in embedded_refs:
                kept += 1
                continue
            if dry_run:
                logging.info("[DRY-RUN] Would prune embedded texture PNG: %s", png_path)
                pruned += 1
                continue
            try:
                png_path.unlink()
                pruned += 1
            except OSError as exc:
                logging.warning("Failed to prune PNG %s: %s", png_path, exc)

    return pruned, kept


def write_blend_probe_report(entries: List[Dict[str, object]], output_path: Path) -> None:
    decision_source_counts: Counter[str] = Counter()
    material_kind_counts: Counter[str] = Counter()
    alpha_mode_counts: Counter[str] = Counter()
    inference_mode_counts: Counter[str] = Counter()
    inference_source_counts: Counter[str] = Counter()
    legacy_object_counts: Counter[str] = Counter()

    for entry in entries:
        decision_source_counts[str(entry.get("material_decision_source", "unknown"))] += 1
        material_kind_counts[str(entry.get("material_kind", "unknown"))] += 1
        alpha_mode_counts[str(entry.get("material_alpha_mode", "unknown"))] += 1
        inference_mode_counts[str(entry.get("material_inference_mode", "unknown"))] += 1
        inference_source_counts[str(entry.get("material_inference_source", "unknown"))] += 1

        object_dir = entry.get("object_dir")
        object_model = entry.get("object_model")
        if isinstance(object_dir, int) and isinstance(object_model, int):
            legacy_object_counts[f"object{object_dir}/object{object_model}"] += 1

    payload = {
        "entry_count": len(entries),
        "summary": {
            "material_decision_source_counts": dict(decision_source_counts),
            "material_kind_counts": dict(material_kind_counts),
            "material_alpha_mode_counts": dict(alpha_mode_counts),
            "material_inference_mode_counts": dict(inference_mode_counts),
            "material_inference_source_counts": dict(inference_source_counts),
            "legacy_object_counts": dict(legacy_object_counts),
        },
        "entries": entries,
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2))


def convert_all(
    bmd_root: Path,
    output_root: Path,
    fmt: str,
    world_filter: Optional[set[int]],
    force: bool,
    dry_run: bool,
    verbose: bool,
    report_path: Optional[Path],
    embed_textures: bool,
    prune_embedded_textures: bool,
    canonical_player_skeleton_path: Optional[Path],
    force_player_inplace: bool,
    remaster_variant_enabled: bool,
    remaster_ready_root: Path,
    remaster_output_root: Path,
    blend_probe: bool = False,
    workers: int = 1,
) -> ConversionStats:
    """Convert all BMD files found under bmd_root."""
    stats = ConversionStats()

    if blend_probe and workers > 1:
        logging.info(
            "Blend probe mode enabled: forcing workers=1 for deterministic probe output"
        )
        workers = 1

    bmd_files = discover_bmd_files(bmd_root, world_filter=world_filter)
    total = len(bmd_files)
    logging.info("Found %d BMD files under %s (workers=%d)", total, bmd_root, workers)

    if dry_run:
        for f in bmd_files:
            rel = canonicalize_output_rel_path(f.relative_to(bmd_root))
            out = output_root / rel.with_suffix('.glb')
            logging.info("[DRY-RUN] Would convert %s -> %s", f, out)
            if remaster_variant_enabled:
                remaster_out = remaster_output_root / rel.with_suffix(".glb")
                logging.info(
                    "[DRY-RUN] Would evaluate remaster variant for %s -> %s (ready root: %s)",
                    f,
                    remaster_out,
                    remaster_ready_root / rel.parent,
                )
        stats.total_found = total
        return stats

    # Pre-compute (source, output) pairs
    jobs: List[Tuple[Path, Path]] = []
    for bmd_path in bmd_files:
        rel = canonicalize_output_rel_path(bmd_path.relative_to(bmd_root))
        out_path = output_root / rel.with_suffix('.glb')
        jobs.append((bmd_path, out_path))

    start_time = time.time()

    if workers <= 1:
        for idx, (bmd_path, out_path) in enumerate(jobs):
            convert_single_bmd(
                bmd_path,
                out_path,
                output_root,
                force,
                embed_textures,
                canonical_player_skeleton_path,
                force_player_inplace,
                blend_probe,
                remaster_variant_enabled,
                remaster_ready_root,
                remaster_output_root,
                stats,
            )

            if (idx + 1) % 500 == 0 or (idx + 1) == total:
                elapsed = time.time() - start_time
                logging.info(
                    "Progress: %d/%d (%.1f%%) — converted=%d skipped=%d failed=%d [%.1fs]",
                    idx + 1, total, 100.0 * (idx + 1) / total,
                    stats.converted,
                    stats.skipped_no_geometry + stats.skipped_non_model + stats.skipped_existing + stats.skipped_corrupt,
                    stats.failed,
                    elapsed,
                )
    else:
        completed = 0
        chunksize = max(1, total // (workers * 4))
        with ProcessPoolExecutor(max_workers=workers) as executor:
            futures_iter = executor.map(
                _bmd_convert_worker,
                [src for src, _ in jobs],
                [dst for _, dst in jobs],
                [output_root] * total,
                [force] * total,
                [embed_textures] * total,
                [canonical_player_skeleton_path] * total,
                [force_player_inplace] * total,
                [blend_probe] * total,
                [remaster_variant_enabled] * total,
                [remaster_ready_root] * total,
                [remaster_output_root] * total,
                chunksize=chunksize,
            )
            for worker_stats in futures_iter:
                merge_stats(stats, worker_stats)
                completed += 1
                if completed % 500 == 0 or completed == total:
                    elapsed = time.time() - start_time
                    logging.info(
                        "Progress: %d/%d (%.1f%%) — converted=%d skipped=%d failed=%d [%.1fs]",
                        completed, total, 100.0 * completed / total,
                        stats.converted,
                        stats.skipped_no_geometry + stats.skipped_non_model + stats.skipped_existing + stats.skipped_corrupt,
                        stats.failed,
                        elapsed,
                    )

    elapsed = time.time() - start_time
    logging.info(
        "Conversion complete in %.1fs: %d converted, %d skipped (no_geom=%d, non_model=%d, existing=%d, corrupt=%d), %d failed",
        elapsed, stats.converted,
        stats.skipped_no_geometry + stats.skipped_non_model + stats.skipped_existing + stats.skipped_corrupt,
        stats.skipped_no_geometry, stats.skipped_non_model, stats.skipped_existing, stats.skipped_corrupt,
        stats.failed,
    )
    if remaster_variant_enabled:
        logging.info(
            "Remaster variants: %d generated, %d skipped(no ready texture), %d failed",
            stats.remaster_variants_generated,
            stats.remaster_variants_skipped_no_ready_texture,
            stats.remaster_variants_failed,
        )

    if embed_textures and prune_embedded_textures:
        touched_dirs = [out.parent for _, out in jobs]
        pruned, kept = prune_embedded_texture_pngs(
            model_dirs=touched_dirs,
            dry_run=dry_run,
        )
        stats.pruned_embedded_pngs += pruned
        stats.kept_pngs_not_embedded += kept
        logging.info(
            "Embedded-texture cleanup: %d PNGs pruned, %d kept (not embedded in any GLB)",
            pruned,
            kept,
        )

    if report_path:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report = {
            "total_found": stats.total_found,
            "world_filter": sorted(world_filter) if world_filter else None,
            "embed_textures": embed_textures,
            "force_player_inplace": force_player_inplace,
            "prune_embedded_textures": prune_embedded_textures,
            "remaster_variant_enabled": remaster_variant_enabled,
            "remaster_ready_root": str(remaster_ready_root),
            "remaster_output_root": str(remaster_output_root),
            "converted": stats.converted,
            "skipped_no_geometry": stats.skipped_no_geometry,
            "skipped_non_model": stats.skipped_non_model,
            "skipped_existing": stats.skipped_existing,
            "skipped_corrupt": stats.skipped_corrupt,
            "pruned_embedded_pngs": stats.pruned_embedded_pngs,
            "kept_pngs_not_embedded": stats.kept_pngs_not_embedded,
            "failed": stats.failed,
            "failures": stats.failures,
            "remaster_variants_generated": stats.remaster_variants_generated,
            "remaster_variants_skipped_no_ready_texture": stats.remaster_variants_skipped_no_ready_texture,
            "remaster_variants_failed": stats.remaster_variants_failed,
            "remaster_failures": stats.remaster_failures,
            "blend_probe_enabled": blend_probe,
            "blend_probe_entries": len(stats.blend_probe_entries),
        }
        report_path.write_text(json.dumps(report, indent=2))
        logging.info("Report written to %s", report_path)

    return stats


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert MU Online BMD 3D model files to GLTF Binary (GLB)."
    )
    parser.add_argument(
        "--bmd-root", type=Path, required=True,
        help="Root directory containing legacy BMD files",
    )
    parser.add_argument(
        "--output-root", type=Path, required=True,
        help="Output directory for converted GLB files",
    )
    parser.add_argument(
        "--disable-remaster-variant",
        action="store_true",
        help=(
            "Disable generation of additional remaster GLB variants under assets/remaster/data "
            "when matching textures exist in remaster/textures/ready."
        ),
    )
    parser.add_argument(
        "--remaster-ready-root",
        type=Path,
        default=None,
        help=(
            "Root directory of remastered ready textures used by remaster variants "
            "(default: inferred from --output-root)."
        ),
    )
    parser.add_argument(
        "--remaster-output-root",
        type=Path,
        default=None,
        help=(
            "Root directory for additional remaster GLB outputs "
            "(default: inferred from --output-root)."
        ),
    )
    parser.add_argument(
        "--format", choices=["glb"], default="glb",
        help="Output format (default: glb)",
    )
    parser.add_argument(
        "--world",
        action="append",
        default=[],
        metavar="WORLD",
        help=(
            "Restrict conversion to specific worlds. Accepts values like '74' or "
            "'World74'. Can be repeated and also supports comma-separated values "
            "(example: --world 74,75)."
        ),
    )
    parser.add_argument("--force", action="store_true", help="Force reconversion")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be done")
    parser.add_argument(
        "--no-embed-textures",
        action="store_true",
        help=(
            "Keep external PNG references in GLB instead of embedding textures. "
            "Default behavior is to embed PNG payloads into the GLB."
        ),
    )
    parser.add_argument(
        "--keep-object-png-textures",
        action="store_true",
        help=(
            "When embedding textures, keep PNG files in object*/ directories. "
            "Default behavior prunes redundant PNGs after GLB generation."
        ),
    )
    parser.add_argument(
        "--disable-embedded-texture-cleanup",
        action="store_true",
        help=(
            "Keep embedded PNG textures in output folders after GLB generation."
        ),
    )
    parser.add_argument("--verbose", action="store_true", help="Enable verbose logging")
    parser.add_argument(
        "--report", type=Path, default=None,
        help="Path for JSON conversion report",
    )
    parser.add_argument(
        "--workers", type=int, default=os.cpu_count() or 4,
        help="Number of parallel worker processes (default: number of CPUs).",
    )
    parser.add_argument(
        "--canonical-player-skeleton",
        type=Path,
        default=None,
        help=(
            "Path to canonical player skeleton BMD used to rebase player-part skin bind pose. "
            "When omitted, auto-detects <bmd-root>/player/player.bmd."
        ),
    )
    parser.add_argument(
        "--disable-canonical-player-skeleton",
        action="store_true",
        help="Disable canonical player skeleton rebasing for player-part models.",
    )
    parser.add_argument(
        "--disable-player-inplace",
        action="store_true",
        help=(
            "Disable in-place root-motion locking for models under Player/. "
            "By default, player animations are exported in-place (horizontal root motion removed)."
        ),
    )
    parser.add_argument(
        "--blend-probe",
        action="store_true",
        help=(
            "Collect per-primitive material blend diagnostics and emit a JSON probe report. "
            "Useful to validate additive/alpha decisions during conversion."
        ),
    )
    parser.add_argument(
        "--probe-output",
        type=Path,
        default=None,
        help=(
            "Path for blend probe JSON output. Enables --blend-probe automatically "
            "(default: assets/reports/conversion_blend_probe.json)."
        ),
    )

    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%H:%M:%S",
    )

    bmd_root = args.bmd_root.resolve()
    output_root = args.output_root.resolve()

    if not bmd_root.is_dir():
        logging.error("BMD root directory not found: %s", bmd_root)
        return 1

    try:
        world_filter = parse_world_filters(args.world)
    except ValueError as exc:
        logging.error("Invalid --world filter: %s", exc)
        return 1

    if world_filter:
        logging.info(
            "World filter active: %s",
            ", ".join(f"World{number}" for number in sorted(world_filter)),
        )

    remaster_variant_enabled = not args.disable_remaster_variant
    remaster_ready_root = (
        args.remaster_ready_root.resolve()
        if args.remaster_ready_root is not None
        else _default_remaster_ready_root(output_root)
    )
    remaster_output_root = (
        args.remaster_output_root.resolve()
        if args.remaster_output_root is not None
        else _default_remaster_output_root(output_root)
    )
    if remaster_variant_enabled:
        logging.info(
            "Remaster variant mode: enabled (ready=%s, output=%s)",
            remaster_ready_root,
            remaster_output_root,
        )
    else:
        logging.info("Remaster variant mode: disabled by CLI flag")

    embed_textures = not args.no_embed_textures
    prune_embedded_textures = (
        embed_textures
        and (not args.keep_object_png_textures)
        and (not args.disable_embedded_texture_cleanup)
    )
    logging.info(
        "Texture mode: %s",
        "embedded in GLB" if embed_textures else "external PNG URIs",
    )
    if prune_embedded_textures:
        logging.info("PNG cleanup mode: prune redundant object textures")
    elif embed_textures and args.keep_object_png_textures:
        logging.info("PNG cleanup mode: keep object texture PNG files")
    elif embed_textures and args.disable_embedded_texture_cleanup:
        logging.info("PNG cleanup mode: disabled by --disable-embedded-texture-cleanup")
    elif embed_textures:
        logging.info("PNG cleanup mode: keep object texture PNG files")

    canonical_player_skeleton_path: Optional[Path] = None
    if not args.disable_canonical_player_skeleton:
        if args.canonical_player_skeleton is not None:
            canonical_player_skeleton_path = args.canonical_player_skeleton
        else:
            canonical_player_skeleton_path = _resolve_default_canonical_player_skeleton_path(
                bmd_root
            )
        if canonical_player_skeleton_path is not None:
            logging.info(
                "Canonical player skeleton mode: enabled (%s)",
                canonical_player_skeleton_path,
            )
        else:
            logging.info("Canonical player skeleton mode: disabled (no player/player.bmd found)")
    else:
        logging.info("Canonical player skeleton mode: disabled by CLI flag")

    workers = max(1, args.workers)
    blend_probe_enabled = args.blend_probe or args.probe_output is not None
    blend_probe_output = args.probe_output or Path("assets/reports/conversion_blend_probe.json")
    if blend_probe_enabled:
        logging.info("Blend probe mode: enabled (output: %s)", blend_probe_output)
    else:
        logging.info("Blend probe mode: disabled")
    force_player_inplace = not args.disable_player_inplace
    if force_player_inplace:
        logging.info("Player animation in-place mode: enabled")
    else:
        logging.info("Player animation in-place mode: disabled by CLI flag")

    stats = convert_all(
        bmd_root=bmd_root,
        output_root=output_root,
        fmt=args.format,
        world_filter=world_filter,
        force=args.force,
        dry_run=args.dry_run,
        verbose=args.verbose,
        report_path=args.report,
        embed_textures=embed_textures,
        prune_embedded_textures=prune_embedded_textures,
        canonical_player_skeleton_path=canonical_player_skeleton_path,
        force_player_inplace=force_player_inplace,
        remaster_variant_enabled=remaster_variant_enabled,
        remaster_ready_root=remaster_ready_root,
        remaster_output_root=remaster_output_root,
        blend_probe=blend_probe_enabled,
        workers=workers,
    )

    if blend_probe_enabled:
        write_blend_probe_report(stats.blend_probe_entries, blend_probe_output)
        logging.info(
            "Blend probe report written to %s (%d entries)",
            blend_probe_output,
            len(stats.blend_probe_entries),
        )

    if stats.failed > 0:
        logging.warning("%d files failed conversion", stats.failed)

    return 0


if __name__ == "__main__":
    sys.exit(main())
