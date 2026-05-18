#!/usr/bin/env python3
"""
assets_convert.py
=================

Utility script to batch convert the legacy MU Online asset library into a format
that is consumable by the Rust/Bevy client. It covers:

* Geometry: every `.bmd` file is forwarded to an external converter
  to produce `.glb` assets with the same relative structure.
* Textures and images: proprietary `.OZ*` containers and classic image formats
  (`.tga`, `.jpg`, `.bmp`) are normalised to `.png`, preserving the directory layout.

Example usage:

    python assets_convert.py \\
        --legacy-root \"cpp/MuClient5.2/bin/Data\" \\
        --output-root \"converted_assets\" \\
        --bmd-converter ./tools/bmd2glb \\
        --bmd-arg --optimize \\
        --report reports/conversion.json

If you do not provide `--bmd-converter`, the script copies `.bmd` files verbatim
into the output tree (useful when the converter is unavailable yet). Textures are
always converted to PNG as long as Pillow (`pip install Pillow`) is available.
"""

from __future__ import annotations

import argparse
from collections import deque
import hashlib
import io
import json
import logging
import math
import os
import re
import shutil
import struct
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
from dataclasses import dataclass, field, fields as dataclass_fields
from functools import partial
from pathlib import Path
from typing import Dict, Iterable, Iterator, List, Optional, Sequence, Tuple

try:
    from PIL import Image, ImageFile, UnidentifiedImageError  # type: ignore

    ImageFile.LOAD_TRUNCATED_IMAGES = True
except ImportError:  # pragma: no cover - handled at runtime.
    Image = None  # type: ignore
    UnidentifiedImageError = Exception  # type: ignore


DEFAULT_LEGACY_ROOT = Path("cpp/MuClient5.2/bin/Data")
DEFAULT_OUTPUT_ROOT = Path("converted_assets")

IMAGE_SPECS: Dict[str, Dict[str, object]] = {
    ".ozj": {"strip": 24, "inner": "jpeg"},
    ".ozj2": {"strip": 24, "inner": "jpeg"},
    ".ozt": {"strip": 4, "inner": "tga"},
    ".ozb": {"strip": 4, "inner": "bmp"},
    ".ozp": {"strip": 4, "inner": "png"},
    ".tga": {"strip": 0, "inner": "tga"},
    ".jpg": {"strip": 0, "inner": "jpeg"},
    ".jpeg": {"strip": 0, "inner": "jpeg"},
    ".bmp": {"strip": 0, "inner": "bmp"},
    ".png": {"strip": 0, "inner": "png"},
}

TEXTURE_EXT_PRIORITY: Dict[str, int] = {
    ".ozt": 0,
    ".tga": 1,
    ".png": 2,
    ".ozp": 3,
    ".ozj": 4,
    ".ozj2": 4,
    ".jpg": 5,
    ".jpeg": 5,
    ".bmp": 6,
    ".ozb": 7,
}

TERRAIN_COLOR_TEXTURE_EXTENSIONS = {".ozj", ".ozj2", ".jpg", ".jpeg", ".png", ".ozp", ".bmp", ".ozb"}
TERRAIN_ALPHA_TEXTURE_EXTENSIONS = {".ozt", ".tga"}
TERRAIN_COLOR_PREFERRED_PREFIXES = (
    "tilegrass",
    "tileground",
    "tilerock",
    "tilewater",
    "tilewood",
    "tilem",
)

TERRAIN_SLOT_ALIASES: Dict[str, Tuple[str, ...]] = {
    # Some datasets expose TileGround01x as the usable ground diffuse when
    # TileGround01 is an alpha mask-like strip.
    "tileground01": ("TileGround01x",),
}

TERRAIN_HEIGHT_STEMS = {"terrainheight", "terrain_height"}
TERRAIN_SIZE = 256
LEGACY_OZB_TERRAIN_HEADER = 1080
LEGACY_OZB_TERRAIN_PIXELS = TERRAIN_SIZE * TERRAIN_SIZE
LOGIN_WORLD_HEIGHT_MULTIPLIER = 3.0
DEFAULT_WORLD_HEIGHT_MULTIPLIER = 1.5
LEGACY_TERRAIN_SCALE = 100.0

ENCRYPTED_EXT_HINTS = {
    ".att",
    ".att1",
    ".map",
}

MAP_XOR_KEY: Tuple[int, ...] = (
    0xD1,
    0x73,
    0x52,
    0xF6,
    0xD2,
    0x9A,
    0xCB,
    0x27,
    0x3E,
    0xAF,
    0x59,
    0x31,
    0x37,
    0xB3,
    0xE7,
    0xA2,
)
BUX_KEY: Tuple[int, ...] = (0xFC, 0xCF, 0xAB)
MAP_KEY_SEED = 0x5E
TERRAIN_TILE_COUNT = TERRAIN_SIZE * TERRAIN_SIZE
ENC_TERRAIN_PATTERN = re.compile(r"^enc_?terrain_?(\d+)$")
# Matches ANY terrain stem: encrypted (enc_terrain*) and unencrypted (terrain*)
TERRAIN_STEM_PATTERN = re.compile(r"^(enc_?)?terrain", re.IGNORECASE)
WORLD_DIR_PATTERN = re.compile(r"^world_?(\d+)$")
OBJECT_MODEL_PATTERN = re.compile(r"^object_(\d+)$")
OBJECT_DIR_PATTERN = re.compile(r"^object_(\d+)$", re.IGNORECASE)
# Client.Data/OBJS/MapObjectV* (muonline-cross Season20-compatible reader).
SCENE_OBJECT_BASE_ENTRY_SIZE = 30
SCENE_OBJECT_ENTRY_SIZE_BY_VERSION: Dict[int, int] = {
    0: 30,
    1: 32,
    2: 33,
    3: 45,
    4: 46,
    5: 54,
}
SCENE_OBJECT_FALLBACK_ENTRY_SIZES: Tuple[int, ...] = tuple(
    sorted({size for size in SCENE_OBJECT_ENTRY_SIZE_BY_VERSION.values()})
)

_LEGACY_OBJECT_MODEL_INDEX_CACHE: Dict[str, Dict[int, Tuple[int, ...]]] = {}
_LEGACY_OBJECT_DIR_INDEX_CACHE: Dict[str, Tuple[int, ...]] = {}
_WORLD1_NAMED_MODEL_MAP_CACHE: Optional[Dict[int, str]] = None

# World 1 (Lorencia) does not follow ObjectXX naming.
# Canonical source: MuClient5.2 MapManager.cpp WorldActive==0 block.
WORLD1_NAMED_MODEL_GROUPS: Tuple[Tuple[int, int, str, int], ...] = (
    (0, 13, "Tree", 1),
    (20, 8, "Grass", 1),
    (30, 5, "Stone", 1),
    (40, 3, "StoneStatue", 1),
    (43, 1, "SteelStatue", 1),
    (44, 3, "Tomb", 1),
    (50, 2, "FireLight", 1),
    (52, 1, "Bonfire", 1),
    (55, 1, "DoungeonGate", 1),
    (56, 2, "MerchantAnimal", 1),
    (58, 1, "TreasureDrum", 1),
    (59, 1, "TreasureChest", 1),
    (60, 1, "Ship", 1),
    (65, 3, "SteelWall", 1),
    (68, 1, "SteelDoor", 1),
    (69, 6, "StoneWall", 1),
    (75, 4, "StoneMuWall", 1),
    (80, 1, "Bridge", 1),
    (81, 4, "Fence", 1),
    (85, 1, "BridgeStone", 1),
    (90, 1, "StreetLight", 1),
    (91, 3, "Cannon", 1),
    (95, 1, "Curtain", 1),
    (96, 2, "Sign", 1),
    (98, 4, "Carriage", 1),
    (102, 2, "Straw", 1),
    (105, 1, "Waterspout", 1),
    (106, 4, "Well", 1),
    (110, 1, "Hanging", 1),
    (111, 1, "Stair", 1),
    (115, 5, "House", 1),
    (120, 1, "Tent", 1),
    (121, 6, "HouseWall", 1),
    (127, 3, "HouseEtc", 1),
    (130, 3, "Light", 1),
    (133, 1, "PoseBox", 1),
    (140, 7, "Furniture", 1),
    (150, 1, "Candle", 1),
    (151, 3, "Beer", 1),
)
DEFAULT_TERRAIN_TEXTURE_SLOT_FILES: Dict[int, str] = {
    0: "TileGrass01.png",
    1: "TileGrass02.png",
    2: "TileGround01.png",
    3: "TileGround02.png",
    4: "TileGround03.png",
    5: "TileWater01.png",
    6: "TileWood01.png",
    7: "TileRock01.png",
    8: "TileRock02.png",
    9: "TileRock03.png",
    10: "TileRock04.png",
    11: "TileRock05.png",
    12: "TileRock06.png",
    13: "TileRock07.png",
    30: "TileGrass01.png",
    31: "TileGrass02.png",
    32: "TileGrass03.png",
}

@dataclass
class ConversionStats:
    textures_converted: int = 0
    textures_skipped: int = 0
    textures_failed: int = 0
    terrain_height_json_emitted: int = 0
    terrain_height_json_failed: int = 0
    terrain_map_json_emitted: int = 0
    terrain_map_json_failed: int = 0
    terrain_map_canonical_emitted: int = 0
    terrain_map_canonical_failed: int = 0
    terrain_att_json_emitted: int = 0
    terrain_att_json_failed: int = 0
    terrain_config_json_emitted: int = 0
    terrain_config_json_failed: int = 0
    terrain_texture_slots_json_emitted: int = 0
    terrain_texture_slots_json_failed: int = 0
    camera_tour_json_emitted: int = 0
    camera_tour_json_failed: int = 0
    scene_objects_json_emitted: int = 0
    scene_objects_json_failed: int = 0
    models_converted: int = 0
    models_skipped: int = 0
    models_failed: int = 0
    others_copied: int = 0
    others_skipped: int = 0
    others_failed: int = 0
    failures: List[str] = field(default_factory=list)
    encrypted_candidates: List[str] = field(default_factory=list)


def merge_stats(target: ConversionStats, source: ConversionStats) -> None:
    """Merge *source* into *target* by summing int fields and extending list fields."""
    for f in dataclass_fields(ConversionStats):
        src_val = getattr(source, f.name)
        if isinstance(src_val, int):
            setattr(target, f.name, getattr(target, f.name) + src_val)
        elif isinstance(src_val, list):
            getattr(target, f.name).extend(src_val)


@dataclass(frozen=True)
class ModelJob:
    source: Path
    target: Path


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Batch convert legacy MU Online assets (geometry + textures)."
    )
    parser.add_argument(
        "--legacy-root",
        type=Path,
        default=DEFAULT_LEGACY_ROOT,
        help="Root directory containing the legacy Data/ assets (default: %(default)s).",
    )
    parser.add_argument(
        "--legacy-fallback-root",
        type=Path,
        action="append",
        default=[],
        help=(
            "Optional fallback Data roots used for missing sidecar sources "
            "(for example enc_terrain*.obj or cw_script*.cws). "
            "Can be supplied multiple times."
        ),
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
    parser.add_argument(
        "--output-root",
        type=Path,
        default=DEFAULT_OUTPUT_ROOT,
        help="Directory where converted assets will be written (default: %(default)s).",
    )
    parser.add_argument(
        "--bmd-converter",
        type=Path,
        help="Optional external converter executable for .bmd files. "
        "It must accept (input_bmd output_file) arguments.",
    )
    parser.add_argument(
        "--bmd-arg",
        action="append",
        default=[],
        help="Extra arguments passed to the BMD converter. Can be supplied multiple times.",
    )
    parser.add_argument(
        "--bmd-output-format",
        default="glb",
        choices=["glb"],
        help="Extension produced by the BMD converter (default: %(default)s).",
    )
    parser.add_argument(
        "--copy-raw-bmd",
        action="store_true",
        help="When no converter is provided, copy .bmd files to the output tree (instead of skipping).",
    )
    parser.add_argument(
        "--skip-models",
        action="store_true",
        help="Skip BMD conversion.",
    )
    parser.add_argument(
        "--skip-textures",
        action="store_true",
        help="Skip texture/image conversion.",
    )
    parser.add_argument(
        "--disable-terrain-height-json",
        action="store_true",
        help="Disable emission of normalized terrain_height.json from TerrainHeight.OZB files.",
    )
    parser.add_argument(
        "--skip-others",
        action="store_true",
        help="Do not copy auxiliary files (map data, encrypted archives, etc.).",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force re-processing even when output files already exist.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show planned operations without modifying the filesystem.",
    )
    parser.add_argument(
        "--report",
        type=Path,
        help="Optional path to store a JSON summary of the conversion results.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose logging.",
    )
    parser.add_argument(
        "--hash",
        action="store_true",
        help="Include SHA256 hashes of emitted files in the JSON report.",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=os.cpu_count() or 4,
        help="Number of parallel worker processes (default: number of CPUs).",
    )
    return parser.parse_args(argv)


def parse_world_token(raw_value: str) -> int:
    token = raw_value.strip()
    if not token:
        raise ValueError("empty world token")

    world_match = WORLD_DIR_PATTERN.fullmatch(token.lower())
    if world_match:
        return int(world_match.group(1))

    object_match = OBJECT_DIR_PATTERN.fullmatch(token.lower())
    if object_match:
        return int(object_match.group(1))

    if token.isdigit():
        return int(token)

    raise ValueError(
        f"invalid world token '{raw_value}'. Use values like '74' or 'World74'."
    )


def parse_world_filters(raw_values: Sequence[str]) -> Optional[set[int]]:
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
        world_match = WORLD_DIR_PATTERN.fullmatch(part.lower())
        if world_match and int(world_match.group(1)) in world_filter:
            return True

        object_match = OBJECT_DIR_PATTERN.fullmatch(part.lower())
        if object_match and int(object_match.group(1)) in world_filter:
            return True

    return False


def canonical_world_dir_from_path(path: Path) -> Optional[Path]:
    normalized = normalize_legacy_rel_path(path)
    for part in normalized.parts:
        world_match = WORLD_DIR_PATTERN.fullmatch(part.lower())
        if world_match:
            return Path(f"world_{int(world_match.group(1))}")
    return None


def normalize_legacy_rel_path(path: Path) -> Path:
    parts = list(path.parts)
    if parts and parts[0].lower() == "data":
        parts = parts[1:]
    if not parts:
        return Path()
    return Path(*parts)


def canonicalize_rel_path(path: Path) -> Path:
    normalized = normalize_legacy_rel_path(path)
    parts = list(normalized.parts)
    for index, part in enumerate(parts):
        world_match = WORLD_DIR_PATTERN.fullmatch(part.lower())
        if world_match:
            world_part = f"world_{int(world_match.group(1))}"
            tail = parts[index + 1 :]
            if not tail:
                return Path(world_part)
            return Path(world_part).joinpath(*tail)
    return normalized


def find_world_dir_in_root(root: Path, world_number: int) -> Optional[Path]:
    world_name = f"world_{world_number}"
    direct = find_case_insensitive_child_dir(root, world_name)
    if direct is not None:
        return direct

    nested_data = find_case_insensitive_child_dir(root, "data")
    if nested_data is not None:
        nested = find_case_insensitive_child_dir(nested_data, world_name)
        if nested is not None:
            return nested

    return None


def ensure_dir(path: Path, dry_run: bool) -> None:
    if dry_run:
        return
    path.mkdir(parents=True, exist_ok=True)


def prepare_model_jobs(
    legacy_root: Path,
    output_root: Path,
    world_filter: Optional[set[int]] = None,
) -> Iterator[ModelJob]:
    for src in legacy_root.rglob("*.bmd"):
        if not src.is_file():
            continue
        rel = canonicalize_rel_path(src.relative_to(legacy_root))
        if not path_matches_world_filter(rel, world_filter):
            continue
        dst = output_root / "models" / rel
        yield ModelJob(source=src, target=dst)


def execute_bmd_job(
    job: ModelJob,
    converter: Optional[Path],
    converter_args: Sequence[str],
    copy_raw: bool,
    dry_run: bool,
    stats: ConversionStats,
    target_extension: str,
    force: bool,
) -> bool:
    if target_extension.startswith("."):
        target_ext = target_extension
    else:
        target_ext = f".{target_extension}"

    if converter:
        target_path = job.target.with_suffix(target_ext)
    elif copy_raw:
        target_path = job.target.with_suffix(".bmd")
    else:
        target_path = job.target.with_suffix(target_ext)

    if converter is None and not copy_raw:
        stats.models_skipped += 1
        logging.debug(
            "Skipping BMD (no converter/copy mode): %s", job.source
        )
        return False

    if target_path.exists() and not dry_run and not force:
        stats.models_skipped += 1
        logging.debug("Model already exists: %s", target_path)
        return True

    ensure_dir(target_path.parent, dry_run=dry_run)

    if dry_run:
        logging.info("[dry-run][model] %s -> %s", job.source, target_path)
        return False

    if converter:
        cmd = [str(converter), str(job.source), str(target_path)]
        if converter_args:
            cmd = [str(converter)] + list(converter_args) + [str(job.source), str(target_path)]
        logging.debug("Executing converter: %s", " ".join(cmd))
        completed = subprocess.run(cmd, check=False, capture_output=True, text=True)
        if completed.returncode != 0:
            stats.models_failed += 1
            stats.failures.append(
                f"{job.source} -> {target_path}: converter exit code {completed.returncode}\n{completed.stderr}"
            )
            logging.error(
                "Converter failed for %s (exit %d).", job.source, completed.returncode
            )
            return False
        stats.models_converted += 1
        return True
    else:
        if copy_raw:
            logging.debug("Copying raw BMD %s -> %s", job.source, target_path)
            shutil.copy2(job.source, target_path)
            stats.models_converted += 1
            return True
        else:
            logging.warning(
                "No converter provided for %s. Use --copy-raw-bmd to copy the original file.",
                job.source,
            )
            stats.models_skipped += 1
            return False


def _bmd_worker(
    job: ModelJob,
    converter: Optional[Path],
    converter_args: Sequence[str],
    copy_raw: bool,
    dry_run: bool,
    target_extension: str,
    force: bool,
) -> Tuple[ConversionStats, Optional[Path]]:
    """Worker function for parallel BMD processing. Returns (stats, resolved_path_or_None)."""
    stats = ConversionStats()
    try:
        success = execute_bmd_job(
            job=job,
            converter=converter,
            converter_args=converter_args,
            copy_raw=copy_raw,
            dry_run=dry_run,
            stats=stats,
            target_extension=target_extension,
            force=force,
        )
        return stats, job.source.resolve() if success else None
    except Exception as exc:  # noqa: BLE001
        stats.models_failed += 1
        stats.failures.append(f"{job.source}: unhandled worker error: {exc}")
        logging.error("BMD worker error for %s: %s", job.source, exc)
        return stats, None


def discover_textures(
    legacy_root: Path,
    world_filter: Optional[set[int]] = None,
) -> Iterator[Path]:
    for path in legacy_root.rglob("*"):
        if not path.is_file():
            continue
        rel = canonicalize_rel_path(path.relative_to(legacy_root))
        if not path_matches_world_filter(rel, world_filter):
            continue
        ext = path.suffix.lower()
        if ext in IMAGE_SPECS:
            yield path


def _compact_texture_stem(stem: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", stem.lower())


def _is_map_grass_alpha_source(path: Path) -> bool:
    if path.suffix.lower() not in TERRAIN_ALPHA_TEXTURE_EXTENSIONS:
        return False
    return _compact_texture_stem(path.stem) in {
        "tilegrass01",
        "tilegrass02",
        "tilegrass03",
    }


def _has_color_sibling_texture(path: Path) -> bool:
    stem = _compact_texture_stem(path.stem)
    try:
        for sibling in path.parent.iterdir():
            if not sibling.is_file():
                continue
            if sibling.suffix.lower() not in TERRAIN_COLOR_TEXTURE_EXTENSIONS:
                continue
            if _compact_texture_stem(sibling.stem) == stem:
                return True
    except Exception:  # noqa: BLE001
        return False
    return False


def texture_target_stem(path: Path) -> str:
    # Keep dedicated billboard alpha variants alongside terrain diffuse variants
    # only when a color sibling exists for the same stem.
    if _is_map_grass_alpha_source(path) and _has_color_sibling_texture(path):
        return f"{path.stem}_alpha"
    return path.stem


def _terrain_prefers_color_texture(stem: str) -> bool:
    compact = _compact_texture_stem(stem)
    return any(compact.startswith(prefix) for prefix in TERRAIN_COLOR_PREFERRED_PREFIXES)


def texture_priority(path: Path) -> int:
    ext = path.suffix.lower()
    priority = TEXTURE_EXT_PRIORITY.get(ext, 100)

    if _terrain_prefers_color_texture(path.stem):
        if ext in TERRAIN_COLOR_TEXTURE_EXTENSIONS:
            return priority - 20
        if ext in TERRAIN_ALPHA_TEXTURE_EXTENSIONS:
            return priority + 20

    return priority


def select_texture_sources(legacy_root: Path, textures: Iterable[Path]) -> List[Path]:
    """
    Select a single best source per target stem to avoid ambiguous overwrites.
    Example: TileGrass01.OZJ and TileGrass01.OZT both map to TileGrass01.png.
    """
    selected: Dict[Tuple[Path, str], Path] = {}
    skipped_duplicates = 0

    for texture in textures:
        rel = canonicalize_rel_path(texture.relative_to(legacy_root))
        canonical_rel = canonicalize_rel_path(rel)
        key = (canonical_rel.parent, texture_target_stem(texture).lower())
        current = selected.get(key)
        if current is None:
            selected[key] = texture
            continue

        if texture_priority(texture) < texture_priority(current):
            selected[key] = texture
            skipped_duplicates += 1
        else:
            skipped_duplicates += 1

    if skipped_duplicates:
        logging.info(
            "Resolved %d ambiguous texture source(s) using extension priority.",
            skipped_duplicates,
        )

    return sorted(
        selected.values(),
        key=lambda path: (
            str(canonicalize_rel_path(path.relative_to(legacy_root)).parent).lower(),
            texture_target_stem(path).lower(),
            texture_priority(path),
            path.name.lower(),
        ),
    )


def load_image_payload(path: Path, spec: Dict[str, object]) -> bytes:
    data = path.read_bytes()
    offset = int(spec.get("strip", 0))

    # Check if file is empty or too small
    if len(data) <= offset:
        raise ValueError(
            f"Asset {path} smaller than expected header ({offset} bytes)."
        )

    # Strip header bytes if needed
    if offset:
        data = data[offset:]

    # Check if the remaining payload is empty or all zeros (corrupted/placeholder file)
    if not data or len(data) == 0:
        raise ValueError(f"Asset {path} has no data after header removal.")

    # Check if file is all zeros (corrupted or placeholder)
    if all(b == 0 for b in data[:min(256, len(data))]):
        raise ValueError(f"Asset {path} appears to be empty or corrupted (all zero bytes).")

    return data


def convert_image_to_png(data: bytes, mode_hint: str) -> Image.Image:
    if Image is None:
        raise RuntimeError("Pillow is required to convert textures. Install with `pip install Pillow`.")
    stream = io.BytesIO(data)
    try:
        image = Image.open(stream)
        image.load()
    except UnidentifiedImageError as exc:  # type: ignore[arg-type]
        raise ValueError("Unsupported or corrupted image payload") from exc

    # Legacy TGA/OZT assets commonly encode transparency as a black background.
    # Preserve authored alpha when present, otherwise key out pure-black pixels
    # connected to texture borders to avoid opaque black quads.
    if mode_hint.lower() == "tga":
        rgba = image.convert("RGBA")
        if _has_opaque_alpha_only(rgba):
            rgba = _key_out_black_border_background(rgba)
        return rgba

    if image.mode not in ("RGBA", "RGB", "LA", "L"):
        image = image.convert("RGBA")
    elif image.mode in ("RGB", "L"):
        image = image.convert("RGBA")

    return image


def _has_opaque_alpha_only(image: Image.Image) -> bool:
    alpha = image.getchannel("A")
    min_alpha, _max_alpha = alpha.getextrema()
    return min_alpha == 255


def _key_out_black_border_background(image: Image.Image) -> Image.Image:
    width, height = image.size
    if width == 0 or height == 0:
        return image

    pixels = image.load()
    if pixels is None:
        return image

    visited = bytearray(width * height)
    queue: deque[Tuple[int, int]] = deque()

    def is_black(x: int, y: int) -> bool:
        r_val, g_val, b_val, _a_val = pixels[x, y]
        return r_val == 0 and g_val == 0 and b_val == 0

    def enqueue_if_black(x: int, y: int) -> None:
        idx = y * width + x
        if visited[idx]:
            return
        visited[idx] = 1
        if is_black(x, y):
            queue.append((x, y))

    for x in range(width):
        enqueue_if_black(x, 0)
        enqueue_if_black(x, height - 1)
    for y in range(height):
        enqueue_if_black(0, y)
        enqueue_if_black(width - 1, y)

    while queue:
        x, y = queue.popleft()
        r_val, g_val, b_val, _a_val = pixels[x, y]
        if r_val == 0 and g_val == 0 and b_val == 0:
            pixels[x, y] = (r_val, g_val, b_val, 0)

        if x > 0:
            enqueue_if_black(x - 1, y)
        if x + 1 < width:
            enqueue_if_black(x + 1, y)
        if y > 0:
            enqueue_if_black(x, y - 1)
        if y + 1 < height:
            enqueue_if_black(x, y + 1)

    return image


def is_terrain_height_asset(source: Path) -> bool:
    return source.suffix.lower() == ".ozb" and source.stem.lower() in TERRAIN_HEIGHT_STEMS


def infer_height_multiplier(source: Path) -> float:
    world_parts = {part.lower() for part in source.parts}
    if "world55" in world_parts:
        return LOGIN_WORLD_HEIGHT_MULTIPLIER
    return DEFAULT_WORLD_HEIGHT_MULTIPLIER


def extract_terrain_height_samples(payload: bytes) -> Tuple[List[int], int]:
    """Extract terrain height samples from a BMP payload.

    Returns (samples, effective_bpp) where:
      - samples: list of TERRAIN_SIZE*TERRAIN_SIZE height values
      - effective_bpp: 8 if heights are raw byte values (0-255),
                       24 if heights are BGR composites (R + G*256 + B*65536)

    The C++ client has two loading modes (ZzzLodTerrain.cpp):
      - OpenTerrainHeight():    8-bit BMP, reads raw bytes from fixed offset 1080
      - OpenTerrainHeightNew(): 24-bit BMP, reads packed BGR triples from fixed offset 54

    Important: MU loaders do not apply BMP row-order flipping for TerrainHeight.
    We intentionally mimic that legacy behavior for parity.
    """
    if len(payload) < 54:
        raise ValueError(f"Terrain payload too small for BMP header ({len(payload)} < 54).")

    if payload[:2] != b"BM":
        raise ValueError("TerrainHeight payload does not start with BMP signature 'BM'.")

    try:
        width = struct.unpack_from("<i", payload, 18)[0]
        height = struct.unpack_from("<i", payload, 22)[0]
        bits_per_pixel = struct.unpack_from("<H", payload, 28)[0]
        compression = struct.unpack_from("<I", payload, 30)[0]
    except struct.error as exc:
        raise ValueError(f"invalid BMP header while reading TerrainHeight: {exc}") from exc

    abs_width = abs(width)
    abs_height = abs(height)
    if abs_width < TERRAIN_SIZE or abs_height < TERRAIN_SIZE:
        raise ValueError(
            f"TerrainHeight bitmap too small ({abs_width}x{abs_height}); expected at least "
            f"{TERRAIN_SIZE}x{TERRAIN_SIZE}."
        )

    if compression not in (0,):
        raise ValueError(
            f"unsupported TerrainHeight BMP compression: {compression} (expected BI_RGB/0)"
        )

    # OpenTerrainHeight(): 8-bit mode, fixed offset 1080.
    if bits_per_pixel <= 8:
        legacy_required = LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS
        if len(payload) < legacy_required:
            raise ValueError(
                "TerrainHeight payload shorter than expected for legacy 8-bit layout "
                f"({len(payload)} < {legacy_required})."
            )
        pixels = payload[
            LEGACY_OZB_TERRAIN_HEADER:LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS
        ]
        if len(pixels) != LEGACY_OZB_TERRAIN_PIXELS:
            raise ValueError("Unexpected terrain sample count while reading 8-bit TerrainHeight.")
        return list(pixels), 8

    # OpenTerrainHeightNew(): 24-bit mode, fixed offset 54 (BMP header only),
    # then contiguous BGR triples for a 256x256 grid.
    bytes_per_pixel = max(bits_per_pixel, 1) // 8
    if bytes_per_pixel < 3:
        raise ValueError(
            f"unsupported TerrainHeight bit depth: {bits_per_pixel} (expected 8 or >=24)"
        )

    fixed_offset = 54
    required = fixed_offset + LEGACY_OZB_TERRAIN_PIXELS * bytes_per_pixel

    # For malformed/truncated 24-bit payloads (observed in legacy assets), fall
    # back to 8-bit mode if enough bytes exist at offset 1080.
    if required > len(payload):
        if bits_per_pixel >= 24:
            logging.warning(
                "TerrainHeight has %d-bit BMP header but truncated data (%d < %d); "
                "falling back to legacy 8-bit reading.",
                bits_per_pixel,
                len(payload),
                required,
            )
        legacy_required = LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS
        if len(payload) >= legacy_required:
            pixels = payload[
                LEGACY_OZB_TERRAIN_HEADER:LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS
            ]
            if len(pixels) == LEGACY_OZB_TERRAIN_PIXELS:
                return list(pixels), 8
        raise ValueError(
            "TerrainHeight payload shorter than expected for fixed 24-bit layout "
            f"({len(payload)} < {required})."
        )

    samples: List[int] = []
    for index in range(LEGACY_OZB_TERRAIN_PIXELS):
        px_offset = fixed_offset + index * bytes_per_pixel
        if px_offset + 2 >= len(payload):
            raise ValueError(
                f"TerrainHeight pixel out of bounds at index={index} "
                f"(offset {px_offset} >= {len(payload)})."
            )
        b_val = payload[px_offset]
        g_val = payload[px_offset + 1]
        r_val = payload[px_offset + 2]
        samples.append(r_val + g_val * 256 + b_val * 65536)

    if len(samples) != LEGACY_OZB_TERRAIN_PIXELS:
        raise ValueError("Unexpected terrain sample count while reading 24-bit TerrainHeight.")

    return samples, 24


def emit_terrain_height_json(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    payload: bytes,
    dry_run: bool,
    stats: ConversionStats,
) -> None:
    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    world_dir = canonical_world_dir_from_path(rel)
    target_parent = world_dir if world_dir is not None else rel.parent
    target = output_root / target_parent / "terrain_height.json"
    height_multiplier = infer_height_multiplier(source)

    effective_bpp = 8

    try:
        samples, effective_bpp = extract_terrain_height_samples(payload)
    except Exception as exc:  # noqa: BLE001
        # Fallback: try raw pixel extraction from known OZB layout.
        # Allow slightly truncated files (pad missing bytes with 0).
        legacy_min = LEGACY_OZB_TERRAIN_HEADER + (LEGACY_OZB_TERRAIN_PIXELS - 256)
        if len(payload) >= legacy_min:
            logging.warning(
                "BMP parsing failed for %s (%s); attempting raw pixel fallback.", source, exc,
            )
            pixels = payload[LEGACY_OZB_TERRAIN_HEADER:LEGACY_OZB_TERRAIN_HEADER + LEGACY_OZB_TERRAIN_PIXELS]
            if len(pixels) < LEGACY_OZB_TERRAIN_PIXELS:
                missing = LEGACY_OZB_TERRAIN_PIXELS - len(pixels)
                logging.warning(
                    "TerrainHeight %s is %d bytes short; padding with zeros.", source, missing,
                )
                pixels += b'\x00' * missing
            samples = list(pixels)
            effective_bpp = 8
        else:
            stats.terrain_height_json_failed += 1
            logging.error("Failed to normalize TerrainHeight for %s: %s", source, exc)
            return

    unique_values = len(set(samples))
    if unique_values <= 2:
        logging.warning(
            "TerrainHeight appears almost flat for %s (unique values: %d).",
            source,
            unique_values,
        )

    # For 24-bit BMPs the C++ applies: height = composite + g_fMinHeight (-500)
    # For 8-bit BMPs the C++ applies: height = byte * multiplier (1.5 or 3.0)
    # We store the raw values and let the Rust client apply the correct formula
    # using the metadata fields below.
    if effective_bpp == 24:
        height_multiplier = 1.0  # composite is already in world units
        height_offset = -500.0   # g_fMinHeight from ZzzLodTerrain.cpp:68
    else:
        height_offset = 0.0

    heights: List[List[float]] = []
    min_sample = float("inf")
    max_sample = float("-inf")
    for row_index in range(TERRAIN_SIZE):
        row = samples[row_index * TERRAIN_SIZE:(row_index + 1) * TERRAIN_SIZE]
        normalized_row = [float(sample) for sample in row]
        min_sample = min(min_sample, min(normalized_row))
        max_sample = max(max_sample, max(normalized_row))
        heights.append(normalized_row)

    payload_json = {
        "width": TERRAIN_SIZE,
        "height": TERRAIN_SIZE,
        "heights": heights,
        "metadata": {
            "source": source.name,
            "source_bits_per_pixel": effective_bpp,
            "row_order": "legacy_client",
            "height_multiplier": height_multiplier,
            "height_offset": height_offset,
            "legacy_terrain_scale": LEGACY_TERRAIN_SCALE,
            "source_unique_values": unique_values,
            "source_min": min(samples),
            "source_max": max(samples),
            "normalized_sample_min": min_sample,
            "normalized_sample_max": max_sample,
        },
    }

    if dry_run:
        logging.info("[dry-run][terrain] %s -> %s", source, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload_json, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.terrain_height_json_failed += 1
        logging.error("Failed to write terrain_height.json for %s: %s", source, exc)
        return

    stats.terrain_height_json_emitted += 1
    logging.debug("Emitted normalized terrain height %s -> %s", source, target)


def map_file_decrypt(data: bytes) -> bytes:
    out = bytearray(len(data))
    map_key = MAP_KEY_SEED
    key_len = len(MAP_XOR_KEY)

    for index, value in enumerate(data):
        out[index] = ((value ^ MAP_XOR_KEY[index % key_len]) - map_key) & 0xFF
        map_key = (value + 0x3D) & 0xFF

    return bytes(out)


def apply_bux_convert(data: bytes) -> bytes:
    out = bytearray(data)
    key_len = len(BUX_KEY)
    for index in range(len(out)):
        out[index] ^= BUX_KEY[index % key_len]
    return bytes(out)


def has_modulus_magic(data: bytes) -> bool:
    """Return True if data starts with Season16+ ATT or MAP magic header."""
    return (
        len(data) >= 4
        and (
            (data[0:3] == b"ATT" and data[3] == 1)
            or (data[0:3] == b"MAP" and data[3] == 1)
        )
    )


# Path to the compiled C++ ModulusDecrypt tool (built from mu_terrain_decrypt.cpp).
_MODULUS_DECRYPT_TOOL: Optional[Path] = None


def _find_modulus_decrypt_tool() -> Optional[Path]:
    """Locate the mu_terrain_decrypt binary next to this script."""
    global _MODULUS_DECRYPT_TOOL
    if _MODULUS_DECRYPT_TOOL is not None:
        return _MODULUS_DECRYPT_TOOL
    candidate = Path(__file__).resolve().parent / "mu_terrain_decrypt"
    if candidate.is_file():
        _MODULUS_DECRYPT_TOOL = candidate
        return candidate
    return None


def modulus_decrypt(data: bytes, is_att: bool) -> bytes:
    """Decrypt Season16+ ATT/MAP data using the external C++ tool.

    *data* must include the 4-byte magic header (``ATT\\x01`` or ``MAP\\x01``).
    Returns the fully decrypted payload (header stripped, ModulusDecrypt applied,
    Xor3Byte applied for ATT).
    """
    tool = _find_modulus_decrypt_tool()
    if tool is None:
        raise RuntimeError(
            "mu_terrain_decrypt tool not found. "
            "Build it with: g++ -O2 -o mu_terrain_decrypt mu_terrain_decrypt.cpp -lcryptopp"
        )

    import tempfile

    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp_in:
        tmp_in.write(data)
        tmp_in_path = tmp_in.name

    tmp_out_path = tmp_in_path + ".dec"

    try:
        result = subprocess.run(
            [str(tool), tmp_in_path, tmp_out_path],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if result.returncode != 0:
            raise RuntimeError(
                f"mu_terrain_decrypt failed (exit {result.returncode}): {result.stderr.strip()}"
            )
        return Path(tmp_out_path).read_bytes()
    finally:
        Path(tmp_in_path).unlink(missing_ok=True)
        Path(tmp_out_path).unlink(missing_ok=True)


def terrain_number_from_stem(stem: str) -> Optional[int]:
    match = ENC_TERRAIN_PATTERN.match(stem.lower())
    if not match:
        return None
    return int(match.group(1))


def is_terrain_stem(stem: str) -> bool:
    """Return True if stem matches any terrain file pattern (encrypted or not)."""
    return TERRAIN_STEM_PATTERN.match(stem) is not None


def is_encrypted_terrain_stem(stem: str) -> bool:
    """Return True if the stem indicates an encrypted terrain file (enc_terrain* or encterrain*)."""
    s = stem.lower()
    return s.startswith("enc_terrain") or s.startswith("encterrain")


def world_number_from_path(source: Path) -> Optional[int]:
    for part in reversed(source.parts):
        world_match = WORLD_DIR_PATTERN.match(part.lower())
        if world_match:
            return int(world_match.group(1))

    return terrain_number_from_stem(source.stem)


def as_terrain_rows(values: Sequence[int]) -> List[List[int]]:
    return [
        list(values[row * TERRAIN_SIZE:(row + 1) * TERRAIN_SIZE])
        for row in range(TERRAIN_SIZE)
    ]


def build_layer_stats(layer: Sequence[int]) -> Dict[str, object]:
    unique_values = len(set(layer))
    mean = round(sum(layer) / len(layer), 5) if layer else 0.0
    return {
        "min": min(layer) if layer else 0,
        "max": max(layer) if layer else 0,
        "mean": mean,
        "unique_values": unique_values,
    }


def emit_default_terrain_config(
    world_dir: Path,
    output_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    world_number = parse_world_number(world_dir)
    if world_number is None:
        return

    target = output_root / world_dir / "terrain_config.json"
    if target.exists() and not force:
        return

    payload = {
        "size": {
            "width": TERRAIN_SIZE,
            "depth": TERRAIN_SIZE,
            "scale": LEGACY_TERRAIN_SCALE,
        },
        "height_multiplier": DEFAULT_WORLD_HEIGHT_MULTIPLIER,
        "legacy_terrain_scale": LEGACY_TERRAIN_SCALE,
        "texture_layers": [
            {
                "id": "grass01",
                "path": f"data/world_{world_number}/tile_grass01.png",
                "scale": 1.0,
            },
            {
                "id": "ground01",
                "path": f"data/world_{world_number}/tile_ground01.png",
                "scale": 1.0,
            },
            {
                "id": "rock01",
                "path": f"data/world_{world_number}/tile_rock01.png",
                "scale": 1.0,
            },
        ],
        "alpha_map": f"data/world_{world_number}/alpha_tile01.png",
        "lightmap": f"data/world_{world_number}/terrain_light.png",
        "metadata": {
            "generated_placeholder": True,
            "reason": "terrain_config.json missing in legacy data",
            "world": world_number,
        },
    }

    if dry_run:
        logging.info("[dry-run][terrain-config] %s -> %s", world_dir, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        print(target)
    except Exception as exc:  # noqa: BLE001
        stats.terrain_config_json_failed += 1
        stats.failures.append(f"{world_dir} -> {target}: {exc}")
        logging.error("Failed to write terrain config json for %s: %s", world_dir, exc)
        return

    stats.terrain_config_json_emitted += 1
    logging.warning(
        "No terrain_config.json found for %s; emitted default terrain_config.json.",
        world_dir,
    )


def emit_default_camera_tour(
    world_dir: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    world_number = parse_world_number(world_dir)
    if world_number is None:
        return

    target = output_root / world_dir / "camera_tour.json"
    if target.exists() and not force:
        return

    center_x, center_z, ground_y = estimate_world_anchor(
        legacy_root=legacy_root,
        world_dir=world_dir,
        world_number=world_number,
    )

    radius = 1900.0
    height = 320.0
    waypoint_count = 8
    normalized_waypoints: List[Dict[str, object]] = []
    for index in range(waypoint_count):
        angle = (2.0 * math.pi * index) / waypoint_count
        position_x = center_x + math.cos(angle) * radius
        position_z = center_z + math.sin(angle) * radius
        look_x = center_x + math.cos(angle + (math.pi / 8.0)) * 320.0
        look_z = center_z + math.sin(angle + (math.pi / 8.0)) * 320.0

        normalized_waypoints.append(
            {
                "index": index,
                "position": [round(position_x, 3), round(ground_y + height, 3), round(position_z, 3)],
                "look_at": [round(look_x, 3), round(ground_y, 3), round(look_z, 3)],
                "move_acceleration": 16.0,
                "distance_level": 8.0,
                "delay": 0.0,
            }
        )

    payload = {
        "waypoints": normalized_waypoints,
        "loop": True,
        "blend_distance": 300.0,
        "interpolation": "smooth",
        "metadata": {
            "generated_placeholder": True,
            "reason": "cw_script*.cws not found",
            "world": world_number,
        },
    }

    if dry_run:
        logging.info("[dry-run][camera] %s -> %s", world_dir, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.camera_tour_json_failed += 1
        stats.failures.append(f"{world_dir} -> {target}: {exc}")
        logging.error("Failed to write default camera tour json for %s: %s", world_dir, exc)
        return

    stats.camera_tour_json_emitted += 1
    logging.warning(
        "No cw_script found for %s; emitted default camera_tour.json.",
        world_dir,
    )


def is_terrain_map_asset(source: Path) -> bool:
    return source.suffix.lower() == ".map" and is_terrain_stem(source.stem)


def is_terrain_attribute_asset(source: Path) -> bool:
    return source.suffix.lower() == ".att" and is_terrain_stem(source.stem)


def is_camera_script_asset(source: Path) -> bool:
    s = source.stem.lower()
    return source.suffix.lower() == ".cws" and (s.startswith("cw_script") or s.startswith("cwscript"))


def is_scene_objects_asset(source: Path) -> bool:
    return source.suffix.lower() == ".obj" and terrain_number_from_stem(source.stem) is not None


def emit_terrain_map_json(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    world_dir = canonical_world_dir_from_path(rel)
    target_parent = world_dir if world_dir is not None else rel.parent
    target = output_root / target_parent / f"{source.name}.json"

    if target.exists() and not force:
        return

    try:
        raw = source.read_bytes()
        if has_modulus_magic(raw):
            # Season16+ ModulusDecrypt format (MAP\x01 header)
            decoded = modulus_decrypt(raw, is_att=False)
        elif is_encrypted_terrain_stem(source.stem):
            decoded = map_file_decrypt(raw)
        else:
            decoded = raw
        expected = 2 + TERRAIN_TILE_COUNT * 3
        # Off-by-one: same SaveTerrainMapping() bug as .att files.
        if len(decoded) == expected - 1:
            decoded = decoded + b"\x00"
        if len(decoded) < expected:
            raise ValueError(
                f"decoded map payload too small ({len(decoded)} < {expected})"
            )

        version = decoded[0]
        map_number = decoded[1]
        layer1 = list(decoded[2:2 + TERRAIN_TILE_COUNT])
        layer2 = list(decoded[2 + TERRAIN_TILE_COUNT:2 + TERRAIN_TILE_COUNT * 2])
        alpha = list(decoded[2 + TERRAIN_TILE_COUNT * 2:2 + TERRAIN_TILE_COUNT * 3])
    except Exception as exc:  # noqa: BLE001
        stats.terrain_map_json_failed += 1
        stats.failures.append(f"{source} -> {source.name}.json: {exc}")
        logging.error("Failed to decode terrain map %s: %s", source, exc)
        return

    payload_json = {
        "header": {
            "version": int(version),
            "map_number": int(map_number),
        },
        "terrain_size": TERRAIN_SIZE,
        "layer_stats": {
            "layer1": build_layer_stats(layer1),
            "layer2": build_layer_stats(layer2),
            "alpha": build_layer_stats(alpha),
        },
        "layer1": as_terrain_rows(layer1),
        "layer2": as_terrain_rows(layer2),
        "alpha": as_terrain_rows(alpha),
    }

    if dry_run:
        logging.info("[dry-run][terrain-map] %s -> %s", source, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload_json, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.terrain_map_json_failed += 1
        stats.failures.append(f"{source} -> {target}: {exc}")
        logging.error("Failed to write terrain map json for %s: %s", source, exc)
        return

    stats.terrain_map_json_emitted += 1
    logging.debug("Emitted terrain map json %s -> %s", source, target)


def emit_terrain_attribute_json(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    world_dir = canonical_world_dir_from_path(rel)
    target_parent = world_dir if world_dir is not None else rel.parent
    target = output_root / target_parent / f"{source.stem}.json"

    if target.exists() and not force:
        return

    try:
        raw = source.read_bytes()
        if has_modulus_magic(raw):
            # Season16+ ModulusDecrypt format (ATT\x01 header)
            decoded = modulus_decrypt(raw, is_att=True)
        elif is_encrypted_terrain_stem(source.stem):
            decoded = apply_bux_convert(map_file_decrypt(raw))
        else:
            decoded = raw
        # Unencrypted files (and some _SERVER variants) are 1 byte short due
        # to an off-by-one in the original C++ SaveTerrainAttribute():
        # it writes MAX_TERRAIN_SIZE*MAX_TERRAIN_SIZE-1 data bytes.
        # Pad with a zero byte so the parsing logic stays uniform.
        if len(decoded) in (65539, 131075):
            decoded = decoded + b"\x00"
        if len(decoded) not in (65540, 131076):
            raise ValueError(f"unexpected terrain attribute payload size: {len(decoded)}")

        version = decoded[0]
        map_number = decoded[1]
        width = decoded[2]
        height = decoded[3]
        body = decoded[4:]
        is_extended = len(decoded) == 131076

        if is_extended:
            values = [
                struct.unpack_from("<H", body, offset)[0]
                for offset in range(0, len(body), 2)
            ]
        else:
            values = list(body)

        if len(values) != TERRAIN_TILE_COUNT:
            raise ValueError(
                f"invalid terrain attribute sample count: {len(values)} (expected {TERRAIN_TILE_COUNT})"
            )
    except Exception as exc:  # noqa: BLE001
        stats.terrain_att_json_failed += 1
        stats.failures.append(f"{source} -> {source.stem}.json: {exc}")
        logging.error("Failed to decode terrain attributes %s: %s", source, exc)
        return

    payload_json = {
        "header": {
            "version": int(version),
            "map_number": int(map_number),
            "width": int(width),
            "height": int(height),
        },
        "is_extended": is_extended,
        "terrain_size": TERRAIN_SIZE,
        "terrain_data": as_terrain_rows(values),
    }

    if dry_run:
        logging.info("[dry-run][terrain-att] %s -> %s", source, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload_json, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.terrain_att_json_failed += 1
        stats.failures.append(f"{source} -> {target}: {exc}")
        logging.error("Failed to write terrain attribute json for %s: %s", source, exc)
        return

    stats.terrain_att_json_emitted += 1
    logging.debug("Emitted terrain attribute json %s -> %s", source, target)


def parse_camera_script(payload: bytes) -> List[Dict[str, float]]:
    if len(payload) < 8:
        raise ValueError("camera script too small")

    magic = struct.unpack_from("<I", payload, 0)[0]
    if magic != 0x00535743:
        raise ValueError(f"invalid camera script signature: {magic}")

    waypoint_size = 28
    waypoint_count: Optional[int] = None
    count_width = 0
    for candidate_width in (8, 4):
        if len(payload) < 4 + candidate_width:
            continue
        candidate_count = int.from_bytes(payload[4:4 + candidate_width], "little")
        expected = 4 + candidate_width + candidate_count * waypoint_size
        if expected == len(payload):
            waypoint_count = candidate_count
            count_width = candidate_width
            break

    if waypoint_count is None:
        candidate_count = int.from_bytes(payload[4:8], "little")
        expected = 8 + candidate_count * waypoint_size
        if expected > len(payload):
            raise ValueError(
                "could not infer waypoint count width from camera script payload"
            )
        waypoint_count = candidate_count
        count_width = 4

    waypoints: List[Dict[str, float]] = []
    offset = 4 + count_width
    for _ in range(waypoint_count):
        if offset + waypoint_size > len(payload):
            break
        (
            index,
            camera_x,
            camera_y,
            camera_z,
            delay_frames,
            move_accel,
            distance_level,
        ) = struct.unpack_from("<i3fiff", payload, offset)
        offset += waypoint_size
        grid_x = int(index % TERRAIN_SIZE)
        grid_y = int(index // TERRAIN_SIZE)
        waypoints.append(
            {
                "index": int(index),
                "camera_x": float(camera_x),
                "camera_y": float(camera_y),
                "camera_z": float(camera_z),
                "delay": int(delay_frames),
                "move_accel": float(move_accel),
                "distance_level": float(distance_level),
                "grid_x": grid_x,
                "grid_y": grid_y,
            }
        )

    return waypoints


def emit_camera_tour_json(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    world_dir = canonical_world_dir_from_path(rel)
    target_parent = world_dir if world_dir is not None else rel.parent
    raw_target = output_root / target_parent / f"{source.name}.json"
    normalized_target = output_root / target_parent / "camera_tour.json"

    if (
        raw_target.exists()
        and normalized_target.exists()
        and not force
    ):
        return

    try:
        waypoints = parse_camera_script(source.read_bytes())
    except Exception as exc:  # noqa: BLE001
        stats.camera_tour_json_failed += 1
        stats.failures.append(f"{source} -> camera_tour.json: {exc}")
        logging.error("Failed to decode camera script %s: %s", source, exc)
        return

    if waypoints:
        grid_x = [int(wp["grid_x"]) for wp in waypoints]
        grid_y = [int(wp["grid_y"]) for wp in waypoints]
        grid_bounds = {
            "x": {"min": min(grid_x), "max": max(grid_x)},
            "y": {"min": min(grid_y), "max": max(grid_y)},
        }
    else:
        grid_bounds = {"x": {"min": 0, "max": 0}, "y": {"min": 0, "max": 0}}

    raw_payload = {
        "magic": 0x00535743,
        "waypoint_count": len(waypoints),
        "waypoints": waypoints,
        "grid_bounds": grid_bounds,
    }

    normalized_waypoints: List[Dict[str, object]] = []
    for index, waypoint in enumerate(waypoints):
        next_waypoint = waypoints[(index + 1) % len(waypoints)]
        ground_x = float(waypoint["camera_x"])
        ground_y = float(waypoint["camera_z"])
        ground_z = float(waypoint["camera_y"])
        next_x = float(next_waypoint["camera_x"])
        next_z = float(next_waypoint["camera_y"])

        direction_x = next_x - ground_x
        direction_z = next_z - ground_z
        direction_length = (direction_x * direction_x + direction_z * direction_z) ** 0.5
        if direction_length <= 0.0001:
            direction_x, direction_z = 1.0, 0.0
        else:
            direction_x /= direction_length
            direction_z /= direction_length

        distance_level = max(5.0, float(waypoint["distance_level"]))
        look_ahead = max(350.0, distance_level * 80.0)
        elevation = max(140.0, distance_level * 40.0)

        camera_position = [ground_x, ground_y + elevation, ground_z]
        look_at = [
            ground_x + direction_x * look_ahead,
            ground_y,
            ground_z + direction_z * look_ahead,
        ]

        normalized_waypoints.append(
            {
                "index": int(waypoint["index"]),
                "position": camera_position,
                "look_at": look_at,
                "move_acceleration": max(0.1, float(waypoint["move_accel"])),
                "distance_level": distance_level,
                "delay": max(0, float(waypoint["delay"])) / 60.0,
            }
        )

    normalized_payload = {
        "waypoints": normalized_waypoints,
        "loop": True,
        "blend_distance": 300.0,
        "interpolation": "smooth",
    }

    if dry_run:
        logging.info("[dry-run][camera] %s -> %s", source, normalized_target)
        return

    try:
        ensure_dir(raw_target.parent, dry_run=False)
        raw_target.write_text(json.dumps(raw_payload, indent=2), encoding="utf-8")
        normalized_target.write_text(
            json.dumps(normalized_payload, indent=2),
            encoding="utf-8",
        )
    except Exception as exc:  # noqa: BLE001
        stats.camera_tour_json_failed += 1
        stats.failures.append(f"{source} -> {normalized_target}: {exc}")
        logging.error("Failed to write camera json for %s: %s", source, exc)
        return

    stats.camera_tour_json_emitted += 1
    logging.debug("Emitted camera tour json %s -> %s", source, normalized_target)


def scene_object_entry_sizes_for_version(version: int) -> Tuple[int, ...]:
    size = SCENE_OBJECT_ENTRY_SIZE_BY_VERSION.get(version)
    if size is not None:
        return (size,)
    return SCENE_OBJECT_FALLBACK_ENTRY_SIZES


def scene_object_sanity_score(payload: bytes, offset: int, count: int, entry_size: int) -> float:
    if count <= 0:
        return 0.0

    if entry_size < SCENE_OBJECT_BASE_ENTRY_SIZE:
        return 0.0

    sample_count = min(count, 64)
    sane_entries = 0

    for index in range(sample_count):
        entry_offset = offset + index * entry_size
        try:
            obj_type, px, py, pz, ax, ay, az, scale = struct.unpack_from("<h7f", payload, entry_offset)
        except struct.error:
            return 0.0

        if not all(
            math.isfinite(value)
            for value in (px, py, pz, ax, ay, az, scale)
        ):
            continue

        if obj_type < -1 or obj_type > 4096:
            continue
        if abs(px) > 250_000.0 or abs(py) > 250_000.0 or abs(pz) > 250_000.0:
            continue
        if abs(ax) > 36_000.0 or abs(ay) > 36_000.0 or abs(az) > 36_000.0:
            continue
        if scale < 0.001 or scale > 128.0:
            continue

        sane_entries += 1

    return sane_entries / max(sample_count, 1)


def decode_scene_object_payload(raw_payload: bytes, world_hint: Optional[int]) -> Dict[str, object]:
    map_decrypted = map_file_decrypt(raw_payload)
    decode_candidates: List[Tuple[str, bytes]] = [
        ("map_file_decrypt", map_decrypted),
        ("raw", raw_payload),
        ("bux_after_map_file_decrypt", apply_bux_convert(map_decrypted)),
        ("bux_raw", apply_bux_convert(raw_payload)),
    ]

    best: Optional[Dict[str, object]] = None
    for decode_name, payload in decode_candidates:
        if len(payload) < 4:
            continue

        layout_candidates: List[Tuple[str, int, int, int, int, Optional[int]]] = []
        payload_version = int(payload[0])
        payload_map = int(payload[1])
        inferred_map = payload_map if world_hint is None else int(world_hint)

        count_vmc = struct.unpack_from("<h", payload, 2)[0]
        if count_vmc >= 0:
            layout_candidates.append(
                ("version_map_count", 4, payload_version, payload_map, count_vmc, None)
            )

        count_vc = struct.unpack_from("<h", payload, 1)[0]
        if count_vc >= 0:
            layout_candidates.append(
                ("version_count", 3, payload_version, inferred_map, count_vc, None)
            )

        payload_body_size = len(payload) - 4
        if payload_body_size >= 0:
            for entry_size in SCENE_OBJECT_FALLBACK_ENTRY_SIZES:
                if entry_size <= 0 or payload_body_size % entry_size != 0:
                    continue
                count_sz = payload_body_size // entry_size
                layout_candidates.append(
                    ("size_derived", 4, payload_version, inferred_map, count_sz, entry_size)
                )

        for (
            layout_name,
            entry_offset,
            version,
            map_number,
            object_count,
            entry_size_hint,
        ) in layout_candidates:
            if object_count < 0 or object_count > 200_000:
                continue

            if entry_size_hint is not None:
                entry_sizes = (entry_size_hint,)
            else:
                entry_sizes = scene_object_entry_sizes_for_version(version)

            for entry_size in entry_sizes:
                required = entry_offset + object_count * entry_size
                if required > len(payload):
                    continue

                score = scene_object_sanity_score(payload, entry_offset, object_count, entry_size)
                trailing = len(payload) - required
                if trailing == 0:
                    score += 0.05
                elif trailing > entry_size:
                    score -= 0.10

                if world_hint is not None and map_number == world_hint:
                    score += 0.20
                elif world_hint is not None:
                    score -= 0.10

                candidate = {
                    "decode_name": decode_name,
                    "layout_name": layout_name,
                    "payload": payload,
                    "entry_offset": entry_offset,
                    "entry_size": entry_size,
                    "version": version,
                    "map_number": map_number,
                    "object_count": object_count,
                    "trailing_bytes": trailing,
                    "score": score,
                }
                if best is None or candidate["score"] > best["score"]:
                    best = candidate

    if best is None or float(best["score"]) < 0.45:
        best_score = float(best["score"]) if best is not None else 0.0
        raise ValueError(
            "no valid scene object decoding candidate found "
            f"(best_score={best_score:.3f}, world_hint={world_hint})"
        )

    return best


def emit_scene_objects_json(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
    optional: bool = False,
) -> None:
    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    world_dir = canonical_world_dir_from_path(rel)
    world_dir_number = parse_world_number(world_dir) if world_dir is not None else None
    stem_world_number = terrain_number_from_stem(source.stem)
    if (
        world_dir_number is not None
        and stem_world_number is not None
        and stem_world_number != world_dir_number
    ):
        logging.debug(
            "Skipping scene object sidecar for %s (stem world=%d, dir world=%d)",
            source,
            stem_world_number,
            world_dir_number,
        )
        return

    target_parent = world_dir if world_dir is not None else rel.parent
    target = output_root / target_parent / "scene_objects.json"

    if target.exists() and not force:
        return

    try:
        world_number_hint = world_number_from_path(source)
        decode_result = decode_scene_object_payload(source.read_bytes(), world_number_hint)

        decoded = decode_result["payload"]
        if not isinstance(decoded, bytes):
            raise ValueError("decoded scene object payload is not bytes")

        version = int(decode_result["version"])
        map_number = int(decode_result["map_number"])
        object_count = int(decode_result["object_count"])
        entry_offset = int(decode_result["entry_offset"])
        entry_size = int(decode_result["entry_size"])
        decode_name = str(decode_result["decode_name"])
        layout_name = str(decode_result["layout_name"])
        decode_score = float(decode_result["score"])

        world_number = world_number_hint or map_number
        required = entry_offset + object_count * entry_size
        if len(decoded) < required:
            raise ValueError(
                f"payload shorter than required object data ({len(decoded)} < {required})"
            )

        objects: List[Dict[str, object]] = []
        world_uses_named_models = world_number == 1
        model_path_cache: Dict[Tuple[int, int], str] = {}
        model_quality_cache: Dict[str, Tuple[bool, str]] = {}
        offset = entry_offset
        for index in range(object_count):
            obj_type, px, py, pz, ax, ay, az, scale = struct.unpack_from("<h7f", decoded, offset)
            offset += entry_size

            model_index = max(1, int(obj_type) + 1)
            if world_uses_named_models:
                model_cache_key = (int(obj_type), 0)
            else:
                model_cache_key = (0, model_index)
            model_path = model_path_cache.get(model_cache_key)
            if model_path is None:
                model_path = resolve_scene_object_model_path(
                    output_root=output_root,
                    legacy_root=legacy_root,
                    world_number=world_number,
                    obj_type=int(obj_type),
                    model_index=model_index,
                )
                model_path_cache[model_cache_key] = model_path

            model_quality = model_quality_cache.get(model_path)
            if model_quality is None:
                model_disk_path = scene_model_disk_path(output_root, model_path)
                model_quality = inspect_converted_model(model_disk_path)
                model_quality_cache[model_path] = model_quality
            model_renderable, model_reason = model_quality

            properties: Dict[str, object] = {}
            if model_renderable:
                properties["model_renderable"] = True
            elif model_reason != "missing converted model":
                properties["model_renderable"] = False
                properties["model_validation_reason"] = model_reason

            objects.append(
                {
                    "id": f"obj_{index:05d}",
                    "type": int(obj_type),
                    "model": model_path,
                    "position": [float(px), float(pz), float(py)],
                    "rotation": [float(ax), float(ay), float(az)],
                    "scale": [float(scale), float(scale), float(scale)],
                    "properties": properties,
                }
            )
    except Exception as exc:  # noqa: BLE001
        if optional:
            logging.warning(
                "Optional scene object decode failed for %s: %s",
                source,
                exc,
            )
            return
        stats.scene_objects_json_failed += 1
        stats.failures.append(f"{source} -> scene_objects.json: {exc}")
        logging.error("Failed to decode scene objects %s: %s", source, exc)
        return

    payload = {
        "objects": objects,
        "metadata": {
            "source": source.name,
            "version": int(version),
            "map_number": int(map_number),
            "object_count": object_count,
            "entry_size": entry_size,
            "decode_name": decode_name,
            "layout_name": layout_name,
            "decode_score": round(decode_score, 4),
            "rotation_encoding": "mu_angles_degrees",
            "rotation_convention": "mu_anglematrix_zyx_degrees",
            "rotation_yaw_offset_degrees": 0.0,
        },
    }

    if dry_run:
        logging.info("[dry-run][scene-objects] %s -> %s", source, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.scene_objects_json_failed += 1
        stats.failures.append(f"{source} -> {target}: {exc}")
        logging.error("Failed to write scene_objects json for %s: %s", source, exc)
        return

    stats.scene_objects_json_emitted += 1
    logging.debug("Emitted scene objects json %s -> %s", source, target)


def parse_world_number(world_dir: Path) -> Optional[int]:
    match = WORLD_DIR_PATTERN.match(world_dir.name.lower())
    if not match:
        return None
    return int(match.group(1))


def _snake_case_name(stem: str) -> str:
    out: List[str] = []
    for index, ch in enumerate(stem):
        if ch.isupper():
            if index != 0:
                out.append("_")
            out.append(ch.lower())
        else:
            out.append(ch)
    return "".join(out)


def world1_named_model_map() -> Dict[int, str]:
    global _WORLD1_NAMED_MODEL_MAP_CACHE
    if _WORLD1_NAMED_MODEL_MAP_CACHE is not None:
        return _WORLD1_NAMED_MODEL_MAP_CACHE

    mapping: Dict[int, str] = {}
    for start_type, count, legacy_base_name, start_index in WORLD1_NAMED_MODEL_GROUPS:
        stem = _snake_case_name(legacy_base_name)
        for offset in range(count):
            obj_type = start_type + offset
            variant = start_index + offset
            mapping[obj_type] = f"data/object_1/{stem}_{variant:02d}.glb"

    _WORLD1_NAMED_MODEL_MAP_CACHE = mapping
    return mapping


def _terrain_map_candidate_score(file_name: str, world_number: int) -> int:
    name = file_name.lower()
    if name in {
        f"enc_terrain{world_number}.map.json",
        f"encterrain{world_number}.map.json",
    }:
        return 300
    if name == f"terrain{world_number}.map.json":
        return 250
    if name == "terrain.map.json":
        return 200
    if name.endswith(".map.json"):
        return 100
    return -1


def emit_canonical_terrain_map(
    world_dir: Path,
    output_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    world_number = parse_world_number(world_dir)
    if world_number is None:
        return

    world_output_dir = output_root / world_dir
    if not world_output_dir.exists():
        return

    target = world_output_dir / "terrain_map.json"
    if target.exists() and not force:
        return

    candidates: List[Tuple[int, str, Path]] = []
    try:
        for child in world_output_dir.iterdir():
            if not child.is_file():
                continue
            score = _terrain_map_candidate_score(child.name, world_number)
            if score < 0 or child.name.lower() == "terrain_map.json":
                continue
            candidates.append((score, child.name.lower(), child))
    except Exception as exc:  # noqa: BLE001
        stats.terrain_map_canonical_failed += 1
        stats.failures.append(f"{world_output_dir} -> terrain_map.json: {exc}")
        logging.error("Failed to scan terrain map candidates for %s: %s", world_dir, exc)
        return

    if not candidates:
        logging.warning("No terrain map source found for %s; terrain_map.json not emitted.", world_dir)
        return

    candidates.sort(key=lambda item: (-item[0], item[1]))
    source = candidates[0][2]

    if dry_run:
        logging.info("[dry-run][terrain-map-canonical] %s -> %s", source, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.terrain_map_canonical_failed += 1
        stats.failures.append(f"{source} -> {target}: {exc}")
        logging.error("Failed to emit canonical terrain map for %s: %s", world_dir, exc)
        return

    stats.terrain_map_canonical_emitted += 1
    logging.debug("Emitted canonical terrain map %s -> %s", source, target)


def _world_texture_candidates(slot_texture_name: str) -> List[str]:
    base = Path(slot_texture_name).stem
    ext = Path(slot_texture_name).suffix or ".png"
    names: List[str] = [base]
    names.extend(TERRAIN_SLOT_ALIASES.get(base.lower(), ()))

    candidates: List[str] = []
    seen: set[str] = set()
    for name in names:
        lower = name.lower()
        snake = _snake_case_name(name)
        # Match normalize.py pattern: insert underscore before digit sequences
        snake_digit_sep = re.sub(r'([a-zA-Z])(\d)', r'\1_\2', snake)
        for stem in (name, snake, lower, snake_digit_sep):
            entry = f"{stem}{ext}"
            key = entry.lower()
            if key in seen:
                continue
            seen.add(key)
            candidates.append(entry)

    return candidates


def _terrain_slot_texture_looks_valid(slot_file: str, texture_path: Path) -> bool:
    if texture_path.suffix.lower() != ".png":
        return True
    stem = Path(slot_file).stem.lower()
    if not _terrain_prefers_color_texture(stem):
        return True

    if Image is not None:
        try:
            with Image.open(texture_path) as image:
                image.load()
                rgba = image.convert("RGBA")
                alpha = rgba.getchannel("A")
                min_alpha, max_alpha = alpha.getextrema()
                if max_alpha <= 0:
                    return False
                if min_alpha == 0 and max_alpha == 0:
                    return False
        except Exception:  # noqa: BLE001
            return True
        return True

    # Pillow-free fallback: use ImageMagick to detect fully transparent alpha.
    try:
        alpha_mean_raw = subprocess.check_output(
            [
                "magick",
                "identify",
                "-format",
                "%[fx:mean.a]",
                str(texture_path),
            ],
            stderr=subprocess.DEVNULL,
            text=True,
        ).strip()
        if not alpha_mean_raw:
            return True
        alpha_mean = float(alpha_mean_raw)
        if alpha_mean <= 0.0:
            return False
    except Exception:  # noqa: BLE001
        return True
    return True


def emit_terrain_texture_slots_json(
    world_dir: Path,
    output_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    world_number = parse_world_number(world_dir)
    if world_number is None:
        return

    world_output_dir = output_root / world_dir
    if not world_output_dir.exists():
        return

    target = world_output_dir / "terrain_texture_slots.json"
    if target.exists() and not force:
        return

    slots: Dict[int, str] = {}

    for slot_index, slot_file in DEFAULT_TERRAIN_TEXTURE_SLOT_FILES.items():
        for candidate in _world_texture_candidates(slot_file):
            found = find_case_insensitive_file(world_output_dir, candidate)
            if found is None:
                continue
            if not _terrain_slot_texture_looks_valid(slot_file, found):
                logging.warning(
                    "Skipping invalid terrain slot texture for world %s slot %d: %s",
                    world_dir.name,
                    slot_index,
                    found,
                )
                continue
            slots[slot_index] = f"data/{world_dir.name}/{found.name}"
            break

    # Guarantee slot coverage for core ground slots used in terrain map base pass.
    first_ground = next((slots.get(slot) for slot in (2, 3, 4) if slot in slots), None)
    if first_ground is not None:
        for slot in (2, 3, 4):
            slots.setdefault(slot, first_ground)

    for ext_index in range(1, 17):
        slot_index = 13 + ext_index
        ext_candidates = [
            f"ExtTile{ext_index:02}.png",
            f"ext_tile{ext_index:02}.png",
            f"exttile{ext_index:02}.png",
        ]
        for candidate in ext_candidates:
            found = find_case_insensitive_file(world_output_dir, candidate)
            if found is None:
                continue
            slots[slot_index] = f"data/{world_dir.name}/{found.name}"
            break

    payload = {
        "world": world_number,
        "slots": slots,
        "metadata": {
            "generated": True,
            "generator": "assets_convert.py",
            "source": "muclient5.2_tile_slot_defaults",
        },
    }

    if dry_run:
        logging.info("[dry-run][terrain-slots] %s -> %s", world_dir, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.terrain_texture_slots_json_failed += 1
        stats.failures.append(f"{world_dir} -> {target}: {exc}")
        logging.error("Failed to emit terrain texture slots for %s: %s", world_dir, exc)
        return

    stats.terrain_texture_slots_json_emitted += 1
    logging.debug("Emitted terrain texture slots %s", target)


def object_model_index(model_path: str) -> Optional[int]:
    stem = Path(model_path).stem.lower()
    match = OBJECT_MODEL_PATTERN.match(stem)
    if not match:
        return None
    return int(match.group(1))


def _legacy_object_model_index(legacy_root: Path) -> Dict[int, Tuple[int, ...]]:
    cache_key = str(legacy_root.resolve())
    cached = _LEGACY_OBJECT_MODEL_INDEX_CACHE.get(cache_key)
    if cached is not None:
        return cached

    by_model: Dict[int, set[int]] = {}
    object_dirs: set[int] = set()

    if legacy_root.exists():
        for child in legacy_root.iterdir():
            if not child.is_dir():
                continue
            match = OBJECT_DIR_PATTERN.match(child.name)
            if not match:
                continue
            object_dir_number = int(match.group(1))
            object_dirs.add(object_dir_number)
            for model_file in child.iterdir():
                if not model_file.is_file() or model_file.suffix.lower() != ".bmd":
                    continue
                model_index = object_model_index(model_file.name)
                if model_index is None:
                    continue
                by_model.setdefault(model_index, set()).add(object_dir_number)

    normalized = {model_index: tuple(sorted(dirs)) for model_index, dirs in by_model.items()}
    _LEGACY_OBJECT_MODEL_INDEX_CACHE[cache_key] = normalized
    _LEGACY_OBJECT_DIR_INDEX_CACHE[cache_key] = tuple(sorted(object_dirs))
    return normalized


def scene_model_disk_path(output_root: Path, model_path: str) -> Path:
    normalized = model_path.strip().replace("\\", "/")
    parts = [part for part in normalized.split("/") if part]
    if parts and parts[0].lower() == "data":
        parts = parts[1:]
    if not parts:
        return output_root
    return output_root.joinpath(*parts)


def resolve_model_asset_path(
    output_root: Path,
    legacy_root: Path,
    world_number: int,
    model_index: int,
    allow_cross_world_fallback: bool = True,
) -> str:
    glb_name = f"object_{model_index:02d}.glb"
    bmd_name = f"object_{model_index:02d}.bmd"

    legacy_index = _legacy_object_model_index(legacy_root)
    candidate_dirs: List[int] = [world_number]
    if allow_cross_world_fallback:
        for object_dir_number in sorted(
            legacy_index.get(model_index, ()),
            key=lambda number: (
                0 if number == world_number else 1,
                abs(number - world_number),
                number,
            ),
        ):
            if object_dir_number not in candidate_dirs:
                candidate_dirs.append(object_dir_number)

    for object_dir_number in candidate_dirs:
        object_dir = output_root / f"object_{object_dir_number}"
        if (object_dir / glb_name).exists():
            return f"data/object_{object_dir_number}/{glb_name}"

    for object_dir_number in candidate_dirs:
        object_dir = legacy_root / f"object_{object_dir_number}"
        if (object_dir / bmd_name).exists():
            # Default to GLB because convert_all_assets.sh emits GLB models.
            return f"data/object_{object_dir_number}/{glb_name}"

    # No direct match found anywhere: keep deterministic fallback.
    fallback_dir = candidate_dirs[0] if candidate_dirs else world_number
    return f"data/object_{fallback_dir}/{glb_name}"


def resolve_scene_object_model_path(
    output_root: Path,
    legacy_root: Path,
    world_number: int,
    obj_type: int,
    model_index: int,
) -> str:
    if world_number == 1:
        named_model = world1_named_model_map().get(obj_type)
        if named_model is not None:
            return named_model
        return resolve_model_asset_path(
            output_root=output_root,
            legacy_root=legacy_root,
            world_number=world_number,
            model_index=model_index,
            allow_cross_world_fallback=False,
        )

    return resolve_model_asset_path(
        output_root=output_root,
        legacy_root=legacy_root,
        world_number=world_number,
        model_index=model_index,
        allow_cross_world_fallback=True,
    )


def inspect_converted_model(path: Path) -> Tuple[bool, str]:
    if not path.exists():
        return False, "missing converted model"

    suffix = path.suffix.lower()
    if suffix == ".gltf":
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
        except Exception as exc:  # noqa: BLE001
            return False, f"invalid gltf json: {exc}"

        meshes = payload.get("meshes")
        buffers = payload.get("buffers")
        accessors = payload.get("accessors")
        buffer_views = payload.get("bufferViews")
        if not isinstance(meshes, list):
            meshes = []
        if not isinstance(buffers, list):
            buffers = []
        if not isinstance(accessors, list):
            accessors = []
        if not isinstance(buffer_views, list):
            buffer_views = []

        primitive_count = 0
        position_primitive_count = 0
        for mesh in meshes:
            if not isinstance(mesh, dict):
                continue
            primitives = mesh.get("primitives")
            if not isinstance(primitives, list):
                continue
            primitive_count += len(primitives)
            for primitive in primitives:
                if not isinstance(primitive, dict):
                    continue
                attributes = primitive.get("attributes")
                if not isinstance(attributes, dict):
                    continue
                position_accessor = attributes.get("POSITION")
                if isinstance(position_accessor, int) and 0 <= position_accessor < len(accessors):
                    position_primitive_count += 1

        if len(meshes) == 0 or len(buffers) == 0 or primitive_count == 0:
            return (
                False,
                f"non-renderable gltf (meshes={len(meshes)}, buffers={len(buffers)}, primitives={primitive_count})",
            )
        if position_primitive_count == 0:
            return False, "non-renderable gltf (missing POSITION attribute in primitives)"
        if len(accessors) == 0 or len(buffer_views) == 0:
            return (
                False,
                f"non-renderable gltf (accessors={len(accessors)}, bufferViews={len(buffer_views)})",
            )

        for accessor in accessors:
            if not isinstance(accessor, dict):
                continue
            buffer_view_index = accessor.get("bufferView")
            if not isinstance(buffer_view_index, int):
                return False, "non-renderable gltf (accessor without bufferView)"
            if buffer_view_index < 0 or buffer_view_index >= len(buffer_views):
                return False, "non-renderable gltf (accessor references missing bufferView)"

        for buffer_view in buffer_views:
            if not isinstance(buffer_view, dict):
                continue
            buffer_index = buffer_view.get("buffer")
            if not isinstance(buffer_index, int):
                return False, "non-renderable gltf (bufferView without buffer)"
            if buffer_index < 0 or buffer_index >= len(buffers):
                return False, "non-renderable gltf (bufferView references missing buffer)"

        for buffer in buffers:
            if not isinstance(buffer, dict):
                continue
            uri = buffer.get("uri")
            if isinstance(uri, str) and not uri.startswith("data:"):
                binary_path = path.parent / uri
                if not binary_path.exists():
                    return False, f"non-renderable gltf (missing buffer payload: {uri})"
                if binary_path.stat().st_size == 0:
                    return False, f"non-renderable gltf (empty buffer payload: {uri})"

        return True, "ok"

    if suffix == ".glb":
        size = path.stat().st_size
        if size < 128:
            return False, f"non-renderable glb (size={size})"
        return True, "ok"

    return True, "ok"


def discover_object_models(
    legacy_root: Path,
    output_root: Path,
    world_number: int,
) -> List[Tuple[int, str, int, bool, str]]:
    object_dir = legacy_root / f"object_{world_number}"
    if not object_dir.exists() or not object_dir.is_dir():
        return []

    models: List[Tuple[int, str, int, bool, str]] = []
    for path in object_dir.iterdir():
        if not path.is_file() or path.suffix.lower() != ".bmd":
            continue
        match = OBJECT_MODEL_PATTERN.match(path.stem.lower())
        if not match:
            continue
        model_index = int(match.group(1))
        model_path = resolve_model_asset_path(
            output_root=output_root,
            legacy_root=legacy_root,
            world_number=world_number,
            model_index=model_index,
        )
        output_model = scene_model_disk_path(output_root, model_path)
        if output_model.exists():
            size_weight = output_model.stat().st_size
            renderable, render_reason = inspect_converted_model(output_model)
        else:
            # Keep a stable fallback heuristic when GLB conversion is unavailable.
            size_weight = path.stat().st_size * 4
            renderable = False
            render_reason = "missing converted model"
        models.append((model_index, model_path, size_weight, renderable, render_reason))

    models.sort(key=lambda item: item[0])
    return models


def estimate_world_anchor(
    legacy_root: Path,
    world_dir: Path,
    world_number: int,
) -> Tuple[float, float, float]:
    world_root = legacy_root / world_dir
    if not world_root.exists():
        fallback_root = find_world_dir_in_root(legacy_root, world_number)
        if fallback_root is not None:
            world_root = fallback_root
    camera_script = world_root / f"cw_script{world_number}.cws"
    if camera_script.exists():
        try:
            waypoints = parse_camera_script(camera_script.read_bytes())
            if waypoints:
                center_x = sum(float(wp["camera_x"]) for wp in waypoints) / len(waypoints)
                center_z = sum(float(wp["camera_y"]) for wp in waypoints) / len(waypoints)
                ground_y = sum(float(wp["camera_z"]) for wp in waypoints) / len(waypoints)
                return center_x, center_z, ground_y
        except Exception as exc:  # noqa: BLE001
            logging.warning("Failed to estimate world anchor from %s: %s", camera_script, exc)

    center = TERRAIN_SIZE * LEGACY_TERRAIN_SCALE * 0.5
    return center, center, 170.0


def load_world_height_context(
    output_root: Path,
    world_dir: Path,
) -> Optional[Tuple[float, float, List[List[float]]]]:
    terrain_config_path = output_root / world_dir / "terrain_config.json"
    terrain_height_path = output_root / world_dir / "terrain_height.json"
    if not terrain_config_path.exists() or not terrain_height_path.exists():
        return None

    try:
        config_payload = json.loads(terrain_config_path.read_text(encoding="utf-8"))
        height_payload = json.loads(terrain_height_path.read_text(encoding="utf-8"))
    except Exception as exc:  # noqa: BLE001
        logging.debug(
            "Could not load terrain context for %s (%s / %s): %s",
            world_dir,
            terrain_config_path,
            terrain_height_path,
            exc,
        )
        return None

    size_payload = config_payload.get("size", {})
    try:
        terrain_scale = float(size_payload.get("scale", LEGACY_TERRAIN_SCALE))
        legacy_scale = float(config_payload.get("legacy_terrain_scale", LEGACY_TERRAIN_SCALE))
        height_multiplier = float(
            config_payload.get("height_multiplier", DEFAULT_WORLD_HEIGHT_MULTIPLIER)
        )
    except (TypeError, ValueError):
        return None

    heights = height_payload.get("heights")
    if not isinstance(heights, list) or not heights:
        return None
    first_row = heights[0]
    if not isinstance(first_row, list) or not first_row:
        return None

    normalized_rows: List[List[float]] = []
    expected_width = len(first_row)
    for row in heights:
        if not isinstance(row, list) or len(row) != expected_width:
            return None
        normalized_rows.append([float(value) for value in row])

    vertical_scale = height_multiplier * (terrain_scale / max(legacy_scale, 1.0))
    if terrain_scale <= 0.0 or vertical_scale <= 0.0:
        return None

    return terrain_scale, vertical_scale, normalized_rows


def sample_world_height(
    height_context: Optional[Tuple[float, float, List[List[float]]]],
    x: float,
    z: float,
) -> Optional[float]:
    if height_context is None:
        return None

    terrain_scale, vertical_scale, heights = height_context
    if terrain_scale <= 0.0:
        return None

    depth = len(heights)
    width = len(heights[0]) if depth > 0 else 0
    if width == 0:
        return None

    grid_x = int(round(x / terrain_scale))
    grid_z = int(round(z / terrain_scale))
    grid_x = max(0, min(width - 1, grid_x))
    grid_z = max(0, min(depth - 1, grid_z))
    return heights[grid_z][grid_x] * vertical_scale


def build_placeholder_scene_objects(
    world_dir: Path,
    legacy_root: Path,
    output_root: Path,
    world_number: int,
    models: Optional[List[Tuple[int, str, int, bool, str]]] = None,
) -> List[Dict[str, object]]:
    if models is None:
        models = discover_object_models(
            legacy_root=legacy_root,
            output_root=output_root,
            world_number=world_number,
        )
    if not models:
        return []

    renderable_models = [model for model in models if model[3]]
    candidate_models = renderable_models if renderable_models else models
    if not renderable_models:
        logging.warning(
            "World%d has no renderable converted object models; placeholders will rely on runtime proxy fallback.",
            world_number,
        )

    preferred_model_indices = (149, 114, 124, 106, 104, 112, 63, 71)
    max_unique_models = 6
    target_placeholder_count = 12
    model_quality_by_path = {
        model: (renderable, reason)
        for (_index, model, _weight, renderable, reason) in candidate_models
    }
    by_index = {
        index: (model, weight)
        for (index, model, weight, _renderable, _reason) in candidate_models
    }

    selected_models: List[str] = []
    for preferred_index in preferred_model_indices:
        data = by_index.get(preferred_index)
        if data is None:
            continue
        model, _weight = data
        if model not in selected_models:
            selected_models.append(model)
        if len(selected_models) >= max_unique_models:
            break

    for _index, model, _weight, _renderable, _reason in sorted(
        candidate_models,
        key=lambda item: item[2],
        reverse=True,
    ):
        if len(selected_models) >= max_unique_models:
            break
        if model not in selected_models:
            selected_models.append(model)

    if not selected_models:
        return []

    center_x, center_z, ground_y = estimate_world_anchor(
        legacy_root=legacy_root,
        world_dir=world_dir,
        world_number=world_number,
    )
    height_context = load_world_height_context(output_root=output_root, world_dir=world_dir)
    radii = (450.0, 800.0, 1200.0, 1450.0, 1700.0, 1300.0, 900.0, 600.0, 1550.0, 1050.0)
    angle_step = 360.0 / max(target_placeholder_count, 1)

    objects: List[Dict[str, object]] = []
    for index in range(target_placeholder_count):
        model = selected_models[index % len(selected_models)]
        angle_deg = 35.0 + angle_step * index
        angle_rad = math.radians(angle_deg)
        radius = radii[index % len(radii)]

        pos_x = center_x + math.cos(angle_rad) * radius
        pos_z = center_z + math.sin(angle_rad) * radius
        sampled_ground_y = sample_world_height(height_context, pos_x, pos_z)
        object_ground_y = sampled_ground_y if sampled_ground_y is not None else ground_y
        rot_y = (angle_deg + 180.0) % 360.0
        model_index = object_model_index(model) or 1
        object_type = max(0, model_index - 1)
        model_renderable, model_reason = model_quality_by_path.get(
            model,
            (False, "unknown model quality"),
        )

        properties: Dict[str, object] = {
            "model_renderable": bool(model_renderable),
        }
        if not model_renderable:
            properties["model_validation_reason"] = model_reason
        if index == 0:
            properties.update(
                {
                    "light_color": [1.0, 0.65, 0.3],
                    "light_intensity": 300.0,
                    "light_range": 380.0,
                }
            )

        objects.append(
            {
                "id": f"placeholder_{index:03d}",
                "type": object_type,
                "model": model,
                "position": [round(pos_x, 3), round(object_ground_y + 3.0, 3), round(pos_z, 3)],
                "rotation": [0.0, 0.0, round(rot_y, 3)],
                "scale": [1.0, 1.0, 1.0],
                "properties": properties,
            }
        )

    eagle_data = by_index.get(63)
    if eagle_data:
        eagle_model, _weight = eagle_data
        eagle_renderable, eagle_reason = model_quality_by_path.get(
            eagle_model,
            (False, "unknown model quality"),
        )
        eagle_x = center_x - 250.0
        eagle_z = center_z + 650.0
        eagle_ground = sample_world_height(height_context, eagle_x, eagle_z)
        eagle_base_y = eagle_ground if eagle_ground is not None else ground_y
        objects.append(
            {
                "id": "placeholder_eagle_spawn",
                "type": 62,
                "model": eagle_model,
                "position": [round(eagle_x, 3), round(eagle_base_y + 140.0, 3), round(eagle_z, 3)],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
                "properties": {
                    "model_renderable": bool(eagle_renderable),
                    "model_validation_reason": eagle_reason if not eagle_renderable else None,
                    "flight_radius": 900.0,
                    "flight_height": 260.0,
                },
            }
        )

    return objects


def emit_empty_scene_objects(
    world_dir: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    target = output_root / world_dir / "scene_objects.json"
    if target.exists() and not force:
        return

    world_number = parse_world_number(world_dir)
    model_candidates: List[Tuple[int, str, int, bool, str]] = []
    renderable_model_count = 0
    non_renderable_model_count = 0
    non_renderable_reasons: Dict[str, int] = {}
    placeholder_objects: List[Dict[str, object]] = []
    if world_number is not None:
        model_candidates = discover_object_models(
            legacy_root=legacy_root,
            output_root=output_root,
            world_number=world_number,
        )
        renderable_model_count = sum(1 for item in model_candidates if item[3])
        non_renderable_model_count = len(model_candidates) - renderable_model_count
        for _index, _model_path, _size, renderable, reason in model_candidates:
            if renderable:
                continue
            non_renderable_reasons[reason] = non_renderable_reasons.get(reason, 0) + 1

        placeholder_objects = build_placeholder_scene_objects(
            world_dir=world_dir,
            legacy_root=legacy_root,
            output_root=output_root,
            world_number=world_number,
            models=model_candidates,
        )

    payload: Dict[str, object] = {
        "objects": placeholder_objects,
        "metadata": {
            "generated_placeholder": True,
            "reason": "enc_terrain*.obj not found",
            "world": int(world_number) if world_number is not None else None,
            "object_count": len(placeholder_objects),
            "model_candidates": len(model_candidates),
            "renderable_model_candidates": renderable_model_count,
            "non_renderable_model_candidates": non_renderable_model_count,
            "non_renderable_reasons": non_renderable_reasons,
            "rotation_encoding": "mu_angles_degrees",
            "rotation_convention": "mu_anglematrix_zyx_degrees",
            "rotation_yaw_offset_degrees": 0.0,
        },
    }

    if dry_run:
        logging.info("[dry-run][scene-objects] %s -> %s", world_dir, target)
        return

    try:
        ensure_dir(target.parent, dry_run=False)
        target.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    except Exception as exc:  # noqa: BLE001
        stats.scene_objects_json_failed += 1
        stats.failures.append(f"{world_dir} -> {target}: {exc}")
        logging.error("Failed to write placeholder scene_objects json for %s: %s", world_dir, exc)
        return

    stats.scene_objects_json_emitted += 1
    logging.warning(
        "No enc_terrain*.obj found for %s; emitted placeholder scene_objects.json with %d object(s).",
        world_dir,
        len(placeholder_objects),
    )


def emit_sidecar_assets(
    source: Path,
    output_root: Path,
    legacy_root: Path,
    dry_run: bool,
    force: bool,
    stats: ConversionStats,
) -> None:
    if is_terrain_height_asset(source):
        try:
            # Use a lenient loader for TerrainHeight: the BMP header may be all
            # zeros while pixel data further in the file is valid (e.g. World1).
            raw = source.read_bytes()
            strip = int(IMAGE_SPECS[".ozb"].get("strip", 0))
            payload = raw[strip:] if len(raw) > strip else raw
            if not payload:
                raise ValueError(f"TerrainHeight payload is empty after stripping {strip} bytes.")
        except Exception as exc:  # noqa: BLE001
            stats.terrain_height_json_failed += 1
            logging.error("Failed to read TerrainHeight payload %s: %s", source, exc)
            return

        emit_terrain_height_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            payload=payload,
            dry_run=dry_run,
            stats=stats,
        )
    elif is_terrain_map_asset(source):
        emit_terrain_map_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )
    elif is_terrain_attribute_asset(source):
        emit_terrain_attribute_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )
    elif is_camera_script_asset(source):
        emit_camera_tour_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )
    elif is_scene_objects_asset(source):
        emit_scene_objects_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )


def handle_texture(
    source: Path,
    legacy_root: Path,
    output_root: Path,
    force: bool,
    dry_run: bool,
    stats: ConversionStats,
    emit_terrain_json: bool,
) -> bool:
    ext = source.suffix.lower()
    spec = IMAGE_SPECS[ext]

    rel = canonicalize_rel_path(source.relative_to(legacy_root))
    canonical_rel = canonicalize_rel_path(rel)
    target_dir = output_root / canonical_rel.parent
    target = target_dir / f"{texture_target_stem(source)}.png"
    legacy_ozt_target = target_dir / f"{source.stem}.tga"

    if ext == ".ozt" and legacy_ozt_target.exists():
        try:
            if not dry_run:
                legacy_ozt_target.unlink()
            logging.debug("Removed legacy OZT TGA output: %s", legacy_ozt_target)
        except OSError as exc:
            logging.warning("Failed to remove legacy OZT TGA output %s: %s", legacy_ozt_target, exc)

    if target.exists() and not force:
        logging.debug("Texture already converted: %s", target)
        stats.textures_skipped += 1
        return True

    if dry_run:
        logging.info("[dry-run][texture] %s -> %s", source, target)
        return False

    try:
        payload = load_image_payload(source, spec)
        image = convert_image_to_png(
            payload,
            mode_hint=str(spec.get("inner", "")),
        )
    except Exception as exc:  # noqa: BLE001
        # TerrainHeight files may have corrupted BMP headers but valid pixel data.
        # Skip the PNG conversion silently and let the sidecar path handle them.
        if is_terrain_height_asset(source):
            logging.debug(
                "Skipping PNG for TerrainHeight %s (handled by sidecar path): %s",
                source, exc,
            )
            stats.textures_skipped += 1
            return False
        stats.textures_failed += 1
        stats.failures.append(f"{source}: {exc}")
        logging.error("Failed to convert %s: %s", source, exc)
        return False

    ensure_dir(target_dir, dry_run=False)
    try:
        image.save(target, format="PNG")
    except Exception as exc:  # noqa: BLE001
        stats.textures_failed += 1
        stats.failures.append(f"{source} -> {target}: {exc}")
        logging.error("Failed to write texture for %s: %s", source, exc)
        return False

    stats.textures_converted += 1
    logging.debug("Converted texture %s -> %s", source, target)

    if emit_terrain_json and is_terrain_height_asset(source):
        emit_terrain_height_json(
            source=source,
            output_root=output_root,
            legacy_root=legacy_root,
            payload=payload,
            dry_run=dry_run,
            stats=stats,
        )

    return True


def _texture_worker(
    source: Path,
    legacy_root: Path,
    output_root: Path,
    force: bool,
    dry_run: bool,
    emit_terrain_json: bool,
) -> Tuple[ConversionStats, Optional[Path]]:
    """Worker function for parallel texture processing. Returns (stats, resolved_path_or_None)."""
    stats = ConversionStats()
    try:
        success = handle_texture(
            source=source,
            legacy_root=legacy_root,
            output_root=output_root,
            force=force,
            dry_run=dry_run,
            stats=stats,
            emit_terrain_json=emit_terrain_json,
        )
        return stats, source.resolve() if success else None
    except Exception as exc:  # noqa: BLE001
        stats.textures_failed += 1
        stats.failures.append(f"{source}: unhandled worker error: {exc}")
        logging.error("Texture worker error for %s: %s", source, exc)
        return stats, None


def is_encrypted_candidate(path: Path) -> bool:
    name_lower = path.name.lower()
    if name_lower.startswith("enc"):
        return True
    return path.suffix.lower() in ENCRYPTED_EXT_HINTS


def find_case_insensitive_child_dir(root: Path, child_name: str) -> Optional[Path]:
    direct = root / child_name
    if direct.exists() and direct.is_dir():
        return direct

    needle = child_name.lower()
    try:
        for child in root.iterdir():
            if child.is_dir() and child.name.lower() == needle:
                return child
    except Exception:  # noqa: BLE001
        return None
    return None


def find_case_insensitive_file(directory: Path, file_name: str) -> Optional[Path]:
    direct = directory / file_name
    if direct.exists() and direct.is_file():
        return direct

    needle = file_name.lower()
    try:
        for child in directory.iterdir():
            if child.is_file() and child.name.lower() == needle:
                return child
    except Exception:  # noqa: BLE001
        return None
    return None


def sidecar_is_generated_placeholder(path: Path) -> bool:
    if not path.exists():
        return False
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception:  # noqa: BLE001
        return False
    metadata = payload.get("metadata")
    if not isinstance(metadata, dict):
        return False
    return bool(metadata.get("generated_placeholder"))


@dataclass
class OtherAssetResult:
    stats: ConversionStats
    world_dir: Optional[Path] = None
    has_scene_objects: bool = False
    has_camera_tour: bool = False


def _other_asset_worker(
    path: Path,
    legacy_root: Path,
    output_root: Path,
    force: bool,
    dry_run: bool,
    sidecar_only_extensions: set,
    allow_copy_extensions: set,
) -> OtherAssetResult:
    """Worker function for parallel other-asset processing."""
    stats = ConversionStats()
    result = OtherAssetResult(stats=stats)
    ext = path.suffix.lower()
    rel = canonicalize_rel_path(path.relative_to(legacy_root))
    canonical_world_dir = canonical_world_dir_from_path(rel)
    if canonical_world_dir is not None:
        result.world_dir = canonical_world_dir
    sidecar_parent = result.world_dir if result.world_dir is not None else rel.parent

    try:
        if ext in sidecar_only_extensions:
            emit_sidecar_assets(
                source=path,
                output_root=output_root,
                legacy_root=legacy_root,
                dry_run=dry_run,
                force=force,
                stats=stats,
            )
            if is_scene_objects_asset(path):
                scene_sidecar = output_root / sidecar_parent / "scene_objects.json"
                if scene_sidecar.exists() and not sidecar_is_generated_placeholder(scene_sidecar):
                    result.has_scene_objects = True
            if is_camera_script_asset(path):
                camera_sidecar = output_root / sidecar_parent / "camera_tour.json"
                if camera_sidecar.exists() and not sidecar_is_generated_placeholder(camera_sidecar):
                    result.has_camera_tour = True
            return result

        if ext not in allow_copy_extensions:
            return result

        target = output_root / canonicalize_rel_path(rel)

        if target.exists() and not force and not dry_run:
            stats.others_skipped += 1
        elif dry_run:
            logging.info("[dry-run][misc] %s -> %s", path, target)
        else:
            ensure_dir(target.parent, dry_run=False)
            shutil.copy2(path, target)
            stats.others_copied += 1
            if is_encrypted_candidate(path):
                stats.encrypted_candidates.append(str(path))

        emit_sidecar_assets(
            source=path,
            output_root=output_root,
            legacy_root=legacy_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )
        if is_scene_objects_asset(path):
            scene_sidecar = output_root / sidecar_parent / "scene_objects.json"
            if scene_sidecar.exists() and not sidecar_is_generated_placeholder(scene_sidecar):
                result.has_scene_objects = True
        if is_camera_script_asset(path):
            camera_sidecar = output_root / sidecar_parent / "camera_tour.json"
            if camera_sidecar.exists() and not sidecar_is_generated_placeholder(camera_sidecar):
                result.has_camera_tour = True
    except Exception as exc:  # noqa: BLE001
        stats.others_failed += 1
        stats.failures.append(f"Worker error for {path}: {exc}")
        logging.error("Other-asset worker error for %s: %s", path, exc)

    return result


def copy_other_assets(
    legacy_root: Path,
    output_root: Path,
    fallback_roots: Sequence[Path],
    processed: Iterable[Path],
    world_filter: Optional[set[int]],
    force: bool,
    dry_run: bool,
    stats: ConversionStats,
    workers: int = 1,
) -> None:
    processed_set = {p.resolve() for p in processed}
    world_dirs_seen: set[Path] = set()
    world_dirs_with_scene_objects: set[Path] = set()
    world_dirs_with_camera_tour: set[Path] = set()

    # Extensions that are converted to JSON sidecars (not copied, but still
    # need to pass through emit_sidecar_assets for conversion).
    SIDECAR_ONLY_EXTENSIONS = {".att", ".att1", ".map", ".obj", ".cws"}

    # Extensions that are completely irrelevant — skip entirely.
    SKIP_EXTENSIONS = {
        ".bmd",
        ".ozj", ".ozt", ".ozb", ".ozp", ".ozd", ".ozg",
        ".tga", ".bmp", ".jpg", ".psd",
        ".smd", ".fbx",
        ".db", ".lnk", ".rar", ".csr", ".dat",
        ".ein", ".mp3", ".wav", ".ogg", ".mpr", ".txt",
    }

    # Only copy files with extensions that the Rust client actually needs.
    ALLOW_COPY_EXTENSIONS = {".png", ".json", ".glb", ".bin", ".wgsl"}

    if world_filter:
        for world_number in sorted(world_filter):
            world_dir = find_world_dir_in_root(legacy_root, world_number)
            if world_dir is None:
                continue
            world_dirs_seen.add(Path(f"world_{world_number}"))

    # --- Phase 3a: Collect eligible files, then process in parallel ---
    eligible_files: List[Path] = []
    for path in legacy_root.rglob("*"):
        if not path.is_file():
            continue
        ext = path.suffix.lower()
        if ext in SKIP_EXTENSIONS:
            continue
        resolved = path.resolve()
        if resolved in processed_set:
            continue

        rel = canonicalize_rel_path(path.relative_to(legacy_root))
        if not path_matches_world_filter(rel, world_filter):
            continue

        world_dir = canonical_world_dir_from_path(rel)
        if world_dir is not None:
            world_dirs_seen.add(world_dir)

        if ext in SIDECAR_ONLY_EXTENSIONS or ext in ALLOW_COPY_EXTENSIONS:
            eligible_files.append(path)

    if workers <= 1:
        for path in eligible_files:
            result = _other_asset_worker(
                path=path,
                legacy_root=legacy_root,
                output_root=output_root,
                force=force,
                dry_run=dry_run,
                sidecar_only_extensions=SIDECAR_ONLY_EXTENSIONS,
                allow_copy_extensions=ALLOW_COPY_EXTENSIONS,
            )
            merge_stats(stats, result.stats)
            if result.world_dir is not None:
                world_dirs_seen.add(result.world_dir)
            if result.has_scene_objects and result.world_dir is not None:
                world_dirs_with_scene_objects.add(result.world_dir)
            if result.has_camera_tour and result.world_dir is not None:
                world_dirs_with_camera_tour.add(result.world_dir)
    else:
        worker_fn = partial(
            _other_asset_worker,
            legacy_root=legacy_root,
            output_root=output_root,
            force=force,
            dry_run=dry_run,
            sidecar_only_extensions=SIDECAR_ONLY_EXTENSIONS,
            allow_copy_extensions=ALLOW_COPY_EXTENSIONS,
        )
        chunksize = max(1, len(eligible_files) // (workers * 4))
        with ProcessPoolExecutor(max_workers=workers) as executor:
            for result in executor.map(worker_fn, eligible_files, chunksize=chunksize):
                merge_stats(stats, result.stats)
                if result.world_dir is not None:
                    world_dirs_seen.add(result.world_dir)
                if result.has_scene_objects and result.world_dir is not None:
                    world_dirs_with_scene_objects.add(result.world_dir)
                if result.has_camera_tour and result.world_dir is not None:
                    world_dirs_with_camera_tour.add(result.world_dir)

    # --- Phase 3b: Sequential fallback roots ---
    for world_dir in sorted(world_dirs_seen):
        world_number = parse_world_number(world_dir)
        if world_number is None:
            continue

        for fallback_root in fallback_roots:
            fallback_world_dir = find_world_dir_in_root(fallback_root, world_number)
            if fallback_world_dir is None:
                continue

            if world_dir not in world_dirs_with_camera_tour:
                fallback_camera = find_case_insensitive_file(
                    fallback_world_dir,
                    f"cw_script{world_number}.cws",
                )
                if fallback_camera is not None:
                    camera_sidecar = output_root / world_dir / "camera_tour.json"
                    allow_overwrite = force or sidecar_is_generated_placeholder(camera_sidecar)
                    emit_camera_tour_json(
                        source=fallback_camera,
                        output_root=output_root,
                        legacy_root=fallback_root,
                        dry_run=dry_run,
                        force=allow_overwrite,
                        stats=stats,
                    )
                    if camera_sidecar.exists() and not sidecar_is_generated_placeholder(camera_sidecar):
                        world_dirs_with_camera_tour.add(world_dir)
                        logging.info(
                            "Loaded camera tour for %s from fallback root %s",
                            world_dir,
                            fallback_root,
                        )

            if world_dir not in world_dirs_with_scene_objects:
                fallback_scene_objects = find_case_insensitive_file(
                    fallback_world_dir,
                    f"enc_terrain{world_number}.obj",
                )
                if fallback_scene_objects is not None:
                    scene_sidecar = output_root / world_dir / "scene_objects.json"
                    allow_overwrite = force or sidecar_is_generated_placeholder(scene_sidecar)
                    emit_scene_objects_json(
                        source=fallback_scene_objects,
                        output_root=output_root,
                        legacy_root=fallback_root,
                        dry_run=dry_run,
                        force=allow_overwrite,
                        stats=stats,
                        optional=True,
                    )
                    if scene_sidecar.exists() and not sidecar_is_generated_placeholder(scene_sidecar):
                        world_dirs_with_scene_objects.add(world_dir)
                        logging.info(
                            "Loaded scene objects for %s from fallback root %s",
                            world_dir,
                            fallback_root,
                        )

    # --- Phase 3c: Emit defaults for worlds missing sidecars ---
    for world_dir in sorted(world_dirs_seen):
        emit_default_terrain_config(
            world_dir=world_dir,
            output_root=output_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )

        if world_dir not in world_dirs_with_camera_tour:
            emit_default_camera_tour(
                world_dir=world_dir,
                output_root=output_root,
                legacy_root=legacy_root,
                dry_run=dry_run,
                force=force,
                stats=stats,
            )

        if world_dir not in world_dirs_with_scene_objects:
            emit_empty_scene_objects(
                world_dir=world_dir,
                output_root=output_root,
                legacy_root=legacy_root,
                dry_run=dry_run,
                force=force,
                stats=stats,
            )

        emit_canonical_terrain_map(
            world_dir=world_dir,
            output_root=output_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )
        emit_terrain_texture_slots_json(
            world_dir=world_dir,
            output_root=output_root,
            dry_run=dry_run,
            force=force,
            stats=stats,
        )


def compute_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main(argv: Sequence[str]) -> int:
    args = parse_args(argv)
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="[%(levelname)s] %(message)s",
    )

    try:
        world_filter = parse_world_filters(args.world)
    except ValueError as exc:
        logging.error("Invalid --world filter: %s", exc)
        return 1

    if world_filter:
        logging.info(
            "World filter active: %s",
            ", ".join(f"world_{number}" for number in sorted(world_filter)),
        )

    legacy_root = args.legacy_root.resolve()
    output_root = args.output_root.resolve()
    fallback_roots: List[Path] = []
    for fallback_root_arg in args.legacy_fallback_root:
        fallback_root = fallback_root_arg.resolve()
        if fallback_root == legacy_root:
            continue
        if not fallback_root.exists():
            logging.warning("Ignoring missing legacy fallback root: %s", fallback_root)
            continue
        fallback_roots.append(fallback_root)

    if not legacy_root.exists():
        logging.error("Legacy root does not exist: %s", legacy_root)
        return 1

    if args.skip_textures and args.skip_models and args.skip_others:
        logging.warning("Models, textures, and auxiliary assets are all skipped; nothing to do.")
        return 0

    if fallback_roots:
        logging.info(
            "Using %d legacy fallback root(s): %s",
            len(fallback_roots),
            ", ".join(str(root) for root in fallback_roots),
        )

    workers = max(1, args.workers)
    logging.info("Using %d worker process(es).", workers)
    stats = ConversionStats()
    processed_paths: set[Path] = set()

    if not args.skip_models:
        if args.bmd_converter is None and not args.copy_raw_bmd:
            logging.warning(
                "No BMD converter provided and --copy-raw-bmd not set; all .bmd files will be skipped."
            )
        logging.info("Scanning for BMD models under %s", legacy_root)
        jobs = list(
            prepare_model_jobs(
                legacy_root=legacy_root,
                output_root=output_root,
                world_filter=world_filter,
            )
        )
        logging.info("Found %d model(s) to process.", len(jobs))
        if workers <= 1:
            for job in jobs:
                worker_stats, resolved = _bmd_worker(
                    job=job,
                    converter=args.bmd_converter,
                    converter_args=args.bmd_arg,
                    copy_raw=args.copy_raw_bmd,
                    dry_run=args.dry_run,
                    target_extension=args.bmd_output_format,
                    force=args.force,
                )
                merge_stats(stats, worker_stats)
                if resolved is not None:
                    processed_paths.add(resolved)
        else:
            with ProcessPoolExecutor(max_workers=workers) as executor:
                futures = {
                    executor.submit(
                        _bmd_worker,
                        job=job,
                        converter=args.bmd_converter,
                        converter_args=args.bmd_arg,
                        copy_raw=args.copy_raw_bmd,
                        dry_run=args.dry_run,
                        target_extension=args.bmd_output_format,
                        force=args.force,
                    ): job
                    for job in jobs
                }
                for future in as_completed(futures):
                    worker_stats, resolved = future.result()
                    merge_stats(stats, worker_stats)
                    if resolved is not None:
                        processed_paths.add(resolved)

    if not args.skip_textures:
        if Image is None:
            logging.error("Pillow is required for texture conversion. Install with `pip install Pillow`.")
            return 1
        logging.info("Scanning for textures/images under %s", legacy_root)
        discovered_textures = list(discover_textures(legacy_root, world_filter=world_filter))
        textures = select_texture_sources(legacy_root, discovered_textures)
        logging.info(
            "Found %d texture file(s), selected %d source(s) after deduplication.",
            len(discovered_textures),
            len(textures),
        )
        if workers <= 1:
            for texture in textures:
                worker_stats, resolved = _texture_worker(
                    source=texture,
                    legacy_root=legacy_root,
                    output_root=output_root,
                    force=args.force,
                    dry_run=args.dry_run,
                    emit_terrain_json=not args.disable_terrain_height_json,
                )
                merge_stats(stats, worker_stats)
                if resolved is not None:
                    processed_paths.add(resolved)
        else:
            worker_fn = partial(
                _texture_worker,
                legacy_root=legacy_root,
                output_root=output_root,
                force=args.force,
                dry_run=args.dry_run,
                emit_terrain_json=not args.disable_terrain_height_json,
            )
            chunksize = max(1, len(textures) // (workers * 4))
            with ProcessPoolExecutor(max_workers=workers) as executor:
                for worker_stats, resolved in executor.map(worker_fn, textures, chunksize=chunksize):
                    merge_stats(stats, worker_stats)
                    if resolved is not None:
                        processed_paths.add(resolved)

    if not args.skip_others:
        logging.info("Copying remaining auxiliary assets...")
        copy_other_assets(
            legacy_root=legacy_root,
            output_root=output_root,
            fallback_roots=fallback_roots,
            processed=processed_paths,
            world_filter=world_filter,
            force=args.force,
            dry_run=args.dry_run,
            stats=stats,
            workers=workers,
        )

    if args.report:
        ensure_dir(args.report.parent, dry_run=args.dry_run)
        report_payload: Dict[str, object] = {
            "legacy_root": str(legacy_root),
            "legacy_fallback_roots": [str(root) for root in fallback_roots],
            "output_root": str(output_root),
            "world_filter": sorted(world_filter) if world_filter else None,
            "stats": {
                "textures_converted": stats.textures_converted,
                "textures_skipped": stats.textures_skipped,
                "textures_failed": stats.textures_failed,
                "terrain_height_json_emitted": stats.terrain_height_json_emitted,
                "terrain_height_json_failed": stats.terrain_height_json_failed,
                "terrain_map_json_emitted": stats.terrain_map_json_emitted,
                "terrain_map_json_failed": stats.terrain_map_json_failed,
                "terrain_map_canonical_emitted": stats.terrain_map_canonical_emitted,
                "terrain_map_canonical_failed": stats.terrain_map_canonical_failed,
                "terrain_att_json_emitted": stats.terrain_att_json_emitted,
                "terrain_att_json_failed": stats.terrain_att_json_failed,
                "terrain_config_json_emitted": stats.terrain_config_json_emitted,
                "terrain_config_json_failed": stats.terrain_config_json_failed,
                "terrain_texture_slots_json_emitted": stats.terrain_texture_slots_json_emitted,
                "terrain_texture_slots_json_failed": stats.terrain_texture_slots_json_failed,
                "camera_tour_json_emitted": stats.camera_tour_json_emitted,
                "camera_tour_json_failed": stats.camera_tour_json_failed,
                "scene_objects_json_emitted": stats.scene_objects_json_emitted,
                "scene_objects_json_failed": stats.scene_objects_json_failed,
                "models_converted": stats.models_converted,
                "models_skipped": stats.models_skipped,
                "models_failed": stats.models_failed,
                "others_copied": stats.others_copied,
                "others_skipped": stats.others_skipped,
                "others_failed": stats.others_failed,
            },
            "failures": stats.failures,
            "encrypted_candidates": stats.encrypted_candidates,
        }

        if args.hash and not args.dry_run:
            hash_entries: List[Tuple[str, str]] = []
            for path in output_root.rglob("*"):
                if path.is_file():
                    hash_entries.append((str(path.relative_to(output_root)), compute_sha256(path)))
            report_payload["hashes"] = hash_entries

        if args.dry_run:
            logging.info("[dry-run] report would be written to %s", args.report)
        else:
            args.report.write_text(json.dumps(report_payload, indent=2), encoding="utf-8")
            logging.info("Wrote conversion report to %s", args.report)

    logging.info(
        "Models: %d ok / %d skipped / %d failed | "
        "Textures: %d ok / %d skipped / %d failed | "
        "TerrainHeight JSON: %d emitted / %d failed | "
        "TerrainMap JSON: %d emitted / %d failed | "
        "TerrainMap Canonical: %d emitted / %d failed | "
        "TerrainAtt JSON: %d emitted / %d failed | "
        "TerrainConfig JSON: %d emitted / %d failed | "
        "TerrainSlots JSON: %d emitted / %d failed | "
        "CameraTour JSON: %d emitted / %d failed | "
        "SceneObjects JSON: %d emitted / %d failed | "
        "Others: %d copied / %d skipped / %d failed",
        stats.models_converted,
        stats.models_skipped,
        stats.models_failed,
        stats.textures_converted,
        stats.textures_skipped,
        stats.textures_failed,
        stats.terrain_height_json_emitted,
        stats.terrain_height_json_failed,
        stats.terrain_map_json_emitted,
        stats.terrain_map_json_failed,
        stats.terrain_map_canonical_emitted,
        stats.terrain_map_canonical_failed,
        stats.terrain_att_json_emitted,
        stats.terrain_att_json_failed,
        stats.terrain_config_json_emitted,
        stats.terrain_config_json_failed,
        stats.terrain_texture_slots_json_emitted,
        stats.terrain_texture_slots_json_failed,
        stats.camera_tour_json_emitted,
        stats.camera_tour_json_failed,
        stats.scene_objects_json_emitted,
        stats.scene_objects_json_failed,
        stats.others_copied,
        stats.others_skipped,
        stats.others_failed,
    )

    if stats.encrypted_candidates:
        logging.info(
            "Potential encrypted assets detected (first 5 shown): %s",
            ", ".join(stats.encrypted_candidates[:5]),
        )

    # Only return non-zero for critical failures (textures, models, file copies).
    # Sidecar-only failures (terrain decrypt, map/att parsing) are non-critical
    # and should not block the rest of the pipeline.
    critical_failures = (
        stats.textures_failed
        + stats.models_failed
        + stats.others_failed
    )
    if stats.failures:
        logging.warning(
            "%d failure(s) recorded (%d critical).",
            len(stats.failures),
            critical_failures,
        )
    return 0 if critical_failures == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
