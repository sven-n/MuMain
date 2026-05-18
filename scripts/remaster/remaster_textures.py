#!/usr/bin/env python3
"""
Definitive texture remaster pipeline using Gemini image generation.

Pipeline per texture:
1) Generate/refine a director prompt (designer stage).
2) Generate remastered image (director stage).
3) Normalize output (ratio/size), restore source alpha when needed, save ready texture.

Caching behavior:
- If ready texture exists -> skip.
- Else if raw model output exists -> convert raw -> ready.
- Else -> run full generation pipeline and save raw+ready artifacts.
"""

from __future__ import annotations

import argparse
import base64
import copy
import glob
import json
import logging
import os
import random
import sys
import time
import urllib.error
import urllib.request
from dataclasses import dataclass
from io import BytesIO
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Sequence, Set, Tuple

try:
    from PIL import Image, UnidentifiedImageError
except ImportError:  # pragma: no cover - runtime dependency check
    Image = None
    UnidentifiedImageError = Exception


DEFAULT_GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_RAW_ROOT = PROJECT_ROOT / "assets/remaster/textures/raw"
DEFAULT_READY_ROOT = PROJECT_ROOT / "assets/remaster/textures/ready"

IMAGE_SPECS: Dict[str, Dict[str, Any]] = {
    ".ozj": {"strip": 24, "inner": "jpeg"},
    ".ozj2": {"strip": 24, "inner": "jpeg"},
    ".ozp": {"strip": 4, "inner": "png"},
    ".ozt": {"strip": 4, "inner": "tga"},
    ".ozb": {"strip": 4, "inner": "bmp"},
    ".png": {"strip": 0, "inner": "png"},
    ".jpg": {"strip": 0, "inner": "jpeg"},
    ".jpeg": {"strip": 0, "inner": "jpeg"},
    ".tga": {"strip": 0, "inner": "tga"},
    ".bmp": {"strip": 0, "inner": "bmp"},
    ".webp": {"strip": 0, "inner": "webp"},
    ".gif": {"strip": 0, "inner": "gif"},
    ".tif": {"strip": 0, "inner": "tiff"},
    ".tiff": {"strip": 0, "inner": "tiff"},
}

MIME_TO_EXT = {
    "image/png": ".png",
    "image/jpeg": ".jpeg",
    "image/jpg": ".jpeg",
    "image/webp": ".webp",
    "image/bmp": ".bmp",
    "image/gif": ".gif",
    "image/x-tga": ".tga",
    "image/tga": ".tga",
    "image/tiff": ".tiff",
}

IMAGE_FILE_EXTS = {".png", ".jpg", ".jpeg", ".webp", ".bmp", ".gif", ".tga", ".tif", ".tiff"}
RETRYABLE_HTTP_STATUS = {408, 409, 429, 500, 502, 503, 504}


@dataclass
class ItemPaths:
    source_path: Path
    asset_rel: Path
    rel_no_ext: Path
    base_name: str
    raw_dir: Path
    ready_dir: Path
    mirror_strategy: str


@dataclass
class DecodedSource:
    image_rgba: Image.Image
    has_alpha: bool
    target_size: Tuple[int, int]


@dataclass
class PreparedInput:
    input_bytes: bytes
    input_mime: str
    input_ext: str
    mask_bytes: Optional[bytes]
    alpha_mask: Optional[Image.Image]
    has_alpha: bool
    target_size: Tuple[int, int]
    original_decoded: DecodedSource


@dataclass
class RunStats:
    total: int = 0
    skipped_ready: int = 0
    converted_raw_to_ready: int = 0
    generated: int = 0
    failed: int = 0


class ApiHttpError(RuntimeError):
    def __init__(self, status: int, raw_text: str):
        self.status = status
        self.raw_text = raw_text
        super().__init__(f"HTTP {status}")


class ApiNetworkError(RuntimeError):
    pass


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Remaster textures with Gemini, storing raw and ready artifacts with cache-aware behavior.",
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="Input file(s), directory path(s), or glob pattern(s) (e.g. assets/data/item/*wing*).",
    )
    parser.add_argument(
        "--raw-root",
        type=Path,
        default=DEFAULT_RAW_ROOT,
        help="Root directory for raw artifacts.",
    )
    parser.add_argument(
        "--ready-root",
        type=Path,
        default=DEFAULT_READY_ROOT,
        help="Root directory for final ready textures.",
    )
    parser.add_argument(
        "--api-key",
        default=os.getenv("GEMINI_API_KEY") or os.getenv("NANO_BANANA_API_KEY"),
        help="Gemini API key (defaults to GEMINI_API_KEY/NANO_BANANA_API_KEY).",
    )
    parser.add_argument(
        "--model",
        default="gemini-3-pro-image-preview",
        help="Gemini model (default: gemini-3-pro-image-preview).",
    )
    parser.add_argument(
        "--endpoint-template",
        default=DEFAULT_GEMINI_ENDPOINT,
        help="Gemini endpoint template supporting {model}.",
    )
    parser.add_argument(
        "--max-dim",
        type=int,
        default=1024,
        help="Largest dimension for model input and ready output (default: 1024).",
    )
    parser.add_argument(
        "--input-jpeg-quality",
        type=int,
        default=95,
        help="JPEG quality for non-alpha model input images (default: 95).",
    )
    parser.add_argument(
        "--ready-jpeg-quality",
        type=int,
        default=92,
        help="JPEG quality for final ready non-alpha textures (default: 92).",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=float,
        default=180.0,
        help="HTTP timeout for API requests in seconds.",
    )
    parser.add_argument(
        "--max-retries",
        type=int,
        default=2,
        help="Retry attempts for transient failures (default: 2).",
    )
    parser.add_argument(
        "--include-ext",
        nargs="*",
        default=None,
        help="Optional extension filter (e.g. --include-ext .ozj .png).",
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        action="append",
        default=None,
        help=(
            "Optional source root(s) for mirror path resolution when inputs are outside assets/. "
            "Can be repeated."
        ),
    )
    parser.add_argument(
        "--recursive",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="When input is a directory, recurse into subdirectories (default: true).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Do not call API or write files.",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Ignore ready/raw caches and force a fresh generation.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable debug logs.",
    )

    args = parser.parse_args(list(argv))
    if args.max_dim <= 0:
        parser.error("--max-dim must be > 0")
    if args.timeout_seconds <= 0:
        parser.error("--timeout-seconds must be > 0")
    if args.max_retries < 0:
        parser.error("--max-retries must be >= 0")
    if args.input_jpeg_quality < 1 or args.input_jpeg_quality > 100:
        parser.error("--input-jpeg-quality must be between 1 and 100")
    if args.ready_jpeg_quality < 1 or args.ready_jpeg_quality > 100:
        parser.error("--ready-jpeg-quality must be between 1 and 100")
    return args


def configure_logging(verbose: bool) -> None:
    logging.basicConfig(
        level=logging.DEBUG if verbose else logging.INFO,
        format="%(levelname)s: %(message)s",
    )
    logging.getLogger("PIL").setLevel(logging.WARNING)


def normalize_extensions(exts: Optional[Sequence[str]]) -> Set[str]:
    if not exts:
        return set(IMAGE_SPECS.keys())
    normalized: Set[str] = set()
    for value in exts:
        raw = value.strip().lower()
        if not raw:
            continue
        normalized.add(raw if raw.startswith(".") else f".{raw}")
    return normalized


def discover_input_files(inputs: Sequence[str], recursive: bool) -> List[Path]:
    discovered: Dict[str, Path] = {}
    for token in inputs:
        expanded = os.path.expanduser(token)
        has_glob = glob.has_magic(expanded)
        if has_glob:
            matches = glob.glob(expanded, recursive=True)
            if not matches:
                logging.warning("Pattern did not match any file: %s", token)
            for match in matches:
                path = Path(match)
                if path.is_file():
                    resolved = path.resolve()
                    discovered[str(resolved)] = resolved
            continue

        path = Path(expanded)
        if path.is_file():
            resolved = path.resolve()
            discovered[str(resolved)] = resolved
            continue
        if path.is_dir():
            iterator = path.rglob("*") if recursive else path.glob("*")
            for child in iterator:
                if child.is_file():
                    resolved = child.resolve()
                    discovered[str(resolved)] = resolved
            continue

        logging.warning("Input not found, skipping: %s", token)

    return sorted(discovered.values(), key=lambda p: p.as_posix().lower())


def resolve_asset_relative(path: Path) -> Optional[Path]:
    parts = path.resolve().parts
    assets_indices = [idx for idx, part in enumerate(parts) if part == "assets"]
    if not assets_indices:
        return None
    idx = assets_indices[-1]
    if idx + 1 >= len(parts):
        return None
    return Path(*parts[idx + 1 :])


def resolve_mirrored_relative(
    source_path: Path,
    source_roots: Sequence[Path],
) -> Tuple[Path, str]:
    resolved = source_path.resolve()
    parts = resolved.parts
    lower_parts = [part.lower() for part in parts]

    # Rule 1: if path contains DataNorm, DataNorm is the mirror root.
    datanorm_indices = [idx for idx, part in enumerate(lower_parts) if part == "datanorm"]
    if datanorm_indices:
        idx = datanorm_indices[-1]
        if idx + 1 < len(parts):
            return Path(*parts[idx + 1 :]), "anchor:datanorm"

    # Rule 2: inside assets/data, `data` is the mirror root.
    for idx in range(len(lower_parts) - 1):
        if lower_parts[idx] == "assets" and lower_parts[idx + 1] == "data":
            if idx + 2 < len(parts):
                return Path(*parts[idx + 2 :]), "anchor:assets-data"

    # Optional explicit roots provided by CLI.
    for root in source_roots:
        try:
            rel = resolved.relative_to(root)
        except ValueError:
            continue
        if rel.parts:
            return rel, "source-root"

    # Generic assets fallback (relative to assets/).
    asset_rel = resolve_asset_relative(source_path)
    if asset_rel is not None:
        return asset_rel, "assets"

    # Legacy fallback for external .../Data/... trees.
    data_indices = [idx for idx, part in enumerate(lower_parts) if part == "data"]
    if data_indices:
        idx = data_indices[-1]
        if idx + 1 < len(parts):
            return Path(*parts[idx + 1 :]), "anchor:data"

    # Last fallback: keep filename only instead of hard-failing.
    return Path(resolved.name), "basename-fallback"


def build_item_paths(
    source_path: Path,
    raw_root: Path,
    ready_root: Path,
    source_roots: Sequence[Path],
) -> ItemPaths:
    asset_rel, mirror_strategy = resolve_mirrored_relative(source_path, source_roots)
    rel_no_ext = asset_rel.with_suffix("")
    base_name = rel_no_ext.name
    return ItemPaths(
        source_path=source_path,
        asset_rel=asset_rel,
        rel_no_ext=rel_no_ext,
        base_name=base_name,
        raw_dir=raw_root / rel_no_ext.parent,
        ready_dir=ready_root / rel_no_ext.parent,
        mirror_strategy=mirror_strategy,
    )


def find_existing_ready(paths: ItemPaths) -> Optional[Path]:
    for suffix in (".png", ".jpeg", ".jpg"):
        candidate = paths.ready_dir / f"{paths.base_name}{suffix}"
        if candidate.exists() and candidate.is_file():
            return candidate
    return None


def find_existing_raw_image(paths: ItemPaths) -> Optional[Path]:
    if not paths.raw_dir.exists() or not paths.raw_dir.is_dir():
        return None

    candidates: List[Path] = []
    for file_path in paths.raw_dir.glob(f"{paths.base_name}.*"):
        if not file_path.is_file():
            continue
        name = file_path.name.lower()
        if name.startswith(f"{paths.base_name}.input."):
            continue
        if name.startswith(f"{paths.base_name}.original."):
            continue
        if name in (
            f"{paths.base_name}.prompt-designer.json",
            f"{paths.base_name}.prompt-director.json",
        ):
            continue
        if file_path.suffix.lower() not in IMAGE_FILE_EXTS:
            continue
        candidates.append(file_path)

    if not candidates:
        return None

    candidates.sort(key=lambda p: (p.stat().st_mtime, p.name.lower()), reverse=True)
    return candidates[0]


def load_image_payload(source_path: Path, spec: Dict[str, Any]) -> bytes:
    data = source_path.read_bytes()
    strip = int(spec.get("strip", 0))
    if len(data) <= strip:
        raise ValueError(f"File smaller than expected header strip ({strip}): {source_path}")
    payload = data[strip:]
    if not payload:
        raise ValueError(f"No image payload after stripping header: {source_path}")
    return payload


def _has_opaque_alpha_only(image: Image.Image) -> bool:
    alpha = image.getchannel("A")
    min_alpha, _max_alpha = alpha.getextrema()
    return min_alpha == 255


def _key_out_black_border_background(image: Image.Image) -> Image.Image:
    width, height = image.size
    if width <= 0 or height <= 0:
        return image

    pixels = image.load()
    if pixels is None:
        return image

    visited = bytearray(width * height)
    queue: List[Tuple[int, int]] = []

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

    for x_coord in range(width):
        enqueue_if_black(x_coord, 0)
        enqueue_if_black(x_coord, height - 1)
    for y_coord in range(height):
        enqueue_if_black(0, y_coord)
        enqueue_if_black(width - 1, y_coord)

    head = 0
    while head < len(queue):
        x_coord, y_coord = queue[head]
        head += 1
        r_val, g_val, b_val, _a_val = pixels[x_coord, y_coord]
        if r_val == 0 and g_val == 0 and b_val == 0:
            pixels[x_coord, y_coord] = (r_val, g_val, b_val, 0)

        if x_coord > 0:
            enqueue_if_black(x_coord - 1, y_coord)
        if x_coord + 1 < width:
            enqueue_if_black(x_coord + 1, y_coord)
        if y_coord > 0:
            enqueue_if_black(x_coord, y_coord - 1)
        if y_coord + 1 < height:
            enqueue_if_black(x_coord, y_coord + 1)

    return image


def decode_source_image(source_path: Path, max_dim: int) -> DecodedSource:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    ext = source_path.suffix.lower()
    spec = IMAGE_SPECS.get(ext)
    if spec is None:
        raise ValueError(f"Unsupported texture extension: {ext}")

    payload = load_image_payload(source_path, spec)
    mode_hint = str(spec.get("inner", "")).lower()

    try:
        with Image.open(BytesIO(payload)) as img:
            img.load()
            rgba = img.convert("RGBA")
    except UnidentifiedImageError as exc:
        raise ValueError(f"Unsupported/corrupted image payload for {source_path}") from exc

    # For legacy TGA-like assets with fake opaque alpha, key out border-connected black.
    if mode_hint == "tga" and _has_opaque_alpha_only(rgba):
        rgba = _key_out_black_border_background(rgba)

    alpha = rgba.getchannel("A")
    min_alpha, _max_alpha = alpha.getextrema()
    has_alpha = min_alpha < 255

    width, height = rgba.size
    if width <= 0 or height <= 0:
        raise ValueError(f"Invalid source size {rgba.size} for {source_path}")

    scale = max_dim / float(max(width, height))
    target_size = (
        max(1, int(round(width * scale))),
        max(1, int(round(height * scale))),
    )
    return DecodedSource(image_rgba=rgba, has_alpha=has_alpha, target_size=target_size)


def image_to_bytes(image: Image.Image, fmt: str, **save_kwargs: Any) -> bytes:
    out = BytesIO()
    image.save(out, format=fmt, **save_kwargs)
    return out.getvalue()


def prepare_model_input(decoded: DecodedSource, input_jpeg_quality: int) -> PreparedInput:
    source_image = decoded.image_rgba
    if source_image.size != decoded.target_size:
        source_image = source_image.resize(decoded.target_size, Image.LANCZOS)

    alpha_mask = None
    if decoded.has_alpha:
        # For alpha assets, send a black-composited input image and the alpha mask separately.
        black_bg = Image.new("RGBA", source_image.size, (0, 0, 0, 255))
        flattened_rgb = Image.alpha_composite(black_bg, source_image).convert("RGB")
        input_bytes = image_to_bytes(flattened_rgb, "PNG")
        alpha_mask = source_image.getchannel("A")
        mask_bytes = image_to_bytes(alpha_mask, "PNG")
        return PreparedInput(
            input_bytes=input_bytes,
            input_mime="image/png",
            input_ext=".png",
            mask_bytes=mask_bytes,
            alpha_mask=alpha_mask,
            has_alpha=True,
            target_size=decoded.target_size,
            original_decoded=decoded
        )

    rgb = source_image.convert("RGB")
    input_bytes = image_to_bytes(rgb, "JPEG", quality=input_jpeg_quality, optimize=True)
    return PreparedInput(
        input_bytes=input_bytes,
        input_mime="image/jpeg",
        input_ext=".jpeg",
        mask_bytes=None,
        alpha_mask=None,
        has_alpha=False,
        target_size=decoded.target_size,
        original_decoded=decoded
    )


def build_original_decoded_input_artifact(
    prepared: PreparedInput,
    input_jpeg_quality: int,
) -> Tuple[bytes, str]:
    decoded = prepared.original_decoded
    original_image = decoded.image_rgba
    if decoded.has_alpha:
        return image_to_bytes(original_image, "PNG"), ".png"

    rgb = original_image.convert("RGB")
    return (
        image_to_bytes(rgb, "JPEG", quality=input_jpeg_quality, optimize=True),
        ".jpeg",
    )


def build_default_director_prompt(asset_rel: Path, prepared: PreparedInput) -> str:
    base = (
        "This is a REMASTER only, not a redesign and not a rethinking. "
        "Primary goal: realistic material response and believable shading while preserving the original asset identity. "
        "Preserve exact object format: same silhouette, same contour flow, same proportions, same volume read, and same negative spaces. "
        "Do not change object structure; do not invent/remove parts; do not reshape the subject. "
        "Preserve original background identity exactly: keep the same background tone family, gradients/noise pattern, and overall lighting mood. "
        "Do not replace or redesign the background. "
        "Preserve composition, UV readability, and color palette identity. "
        "Lock hue families per region/object: if source is rosy/pink, output must remain rosy/pink. "
        "Do not creative-recolor and do not introduce new dominant colors. "
        "You may refine local value/contrast/saturation subtly to increase realism without changing palette identity. "
        "Render in polished realistic 3D CG texture style with natural gradients, material depth, and clean detail. "
        "Keep edges clean, avoid artifacts, avoid text/logos, and avoid new decorative elements. "
        "If human faces appear, keep them as young adults around 30 years old. "
        "Respect a practical detail limit: avoid excessive micro-detail; keep detail one level above microscopic fidelity. "
        "If source background is black (RGB 0,0,0), keep it black."
    )
    if prepared.has_alpha:
        base += (
            " The second image is an alpha mask (white=opaque, black=transparent). "
            "Respect that silhouette exactly and keep outside-mask regions transparent."
        )
    return f"{base}\nSource asset: {asset_rel.as_posix()}\nOutput ratio target: {prepared.target_size[0]}:{prepared.target_size[1]}"


def build_designer_payload(asset_rel: Path, prepared: PreparedInput) -> Dict[str, Any]:
    text = (
        "You are a senior prompt designer for game texture remastering.\n"
        "Generate a single final production prompt for an image model.\n"
        "The final prompt must be concise and objective (around 80-140 words).\n"
        "Rules for the final prompt:\n"
        "- This is a remaster only. No rethinking, no redesign, no reinterpretation.\n"
        "- Prioritize realism: believable material response, natural gradients, and depth.\n"
        "- Preserve composition, silhouette, proportions, UV readability, and palette identity.\n"
        "- Keep hue families per object region (example: rosy/pink stays rosy/pink).\n"
        "- Do not perform creative recoloring and do not add new dominant colors.\n"
        "- Allow subtle local value/contrast/saturation adjustments for realism, while preserving palette identity.\n"
        "- Keep black backgrounds black when originally black.\n"
        "- Keep output clean and artifact-free.\n"
        "- If any human face appears, render as a young adult around 30 years old.\n"
        "- Respect detail budget: do not force microscopic details (example: feather can stay at a higher-level readable detail, not strand-by-strand micro detail).\n"
        "- No text, no watermark, no extra objects.\n"
        f"- Source asset path: {asset_rel.as_posix()}.\n"
        f"- Desired output ratio target: {prepared.target_size[0]}:{prepared.target_size[1]}.\n"
    )
    if prepared.has_alpha:
        text += (
            "- Input includes an image plus an alpha mask.\n"
            "- Output must respect the mask silhouette.\n"
        )
    text += "Return only the final prompt text. No markdown, no JSON."

    return {
        "contents": [
            {
                "role": "user",
                "parts": [
                    {"text": text},
                ],
            }
        ],
        "generationConfig": {
            "responseModalities": ["TEXT"],
            "temperature": 0.0,
        },
    }


def build_director_guardrail_prompt(asset_rel: Path, prepared: PreparedInput) -> str:
    text = (
        "REMASTER GUARDRAILS:\n"
        "1) This is a remaster, not a rethinking, redesign, or reinterpretation.\n"
        "2) Keep original object format unchanged: no reshape, no new parts, no removed parts.\n"
        "3) Keep silhouette/contour/proportions/negative-space exactly aligned to source.\n"
        "4) Prioritize realistic shading/material depth while preserving source identity.\n"
        "5) Preserve tone/hue families and palette identity from source.\n"
        "6) Keep per-object tones consistent: if source object is rosy/pink, output object must remain rosy/pink.\n"
        "7) Forbidden: hue-family swap, creative recoloring, structural redesign, new dominant colors.\n"
        "8) Allowed: subtle local value/contrast/saturation refinement for realism.\n"
        "9) Preserve background exactly: same tone family, gradients/noise, and lighting mood.\n"
        "10) Keep black background black when source is black.\n"
    )
    if prepared.has_alpha:
        text += "11) Respect alpha mask silhouette exactly; outside mask remains transparent.\n"
    text += f"Source asset: {asset_rel.as_posix()}\nOutput ratio target: {prepared.target_size[0]}:{prepared.target_size[1]}"
    return text


def compose_director_prompt(
    asset_rel: Path,
    prepared: PreparedInput,
) -> str:
    guardrail = build_director_guardrail_prompt(asset_rel, prepared)
    generation_prompt = build_default_director_prompt(asset_rel, prepared)
    return f"{guardrail}\n\nGeneration brief:\n{generation_prompt}"


def build_director_payload(director_prompt: str, prepared: PreparedInput) -> Dict[str, Any]:
    parts: List[Dict[str, Any]] = [
        {"text": director_prompt},
        {
            "inlineData": {
                "mimeType": prepared.input_mime,
                "data": base64.b64encode(prepared.input_bytes).decode("ascii"),
            }
        },
    ]
    if prepared.has_alpha and prepared.mask_bytes is not None:
        parts.append(
            {
                "inlineData": {
                    "mimeType": "image/png",
                    "data": base64.b64encode(prepared.mask_bytes).decode("ascii"),
                }
            }
        )

    return {
        "contents": [
            {
                "role": "user",
                "parts": parts,
            }
        ],
        "generationConfig": {
            "responseModalities": ["IMAGE"],
            "temperature": 0.1,
        },
    }


def sanitize_payload_without_images(payload: Dict[str, Any]) -> Dict[str, Any]:
    clone = copy.deepcopy(payload)

    def sanitize(value: Any) -> Any:
        if isinstance(value, dict):
            out: Dict[str, Any] = {}
            for key, item in value.items():
                if key in ("inlineData", "inline_data") and isinstance(item, dict):
                    sanitized_inline = dict(item)
                    if "data" in sanitized_inline:
                        sanitized_inline["data"] = "<omitted_image_base64>"
                    out[key] = sanitized_inline
                    continue
                out[key] = sanitize(item)
            return out
        if isinstance(value, list):
            return [sanitize(item) for item in value]
        return value

    return sanitize(clone)


def save_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")


def post_json_with_urllib(
    endpoint: str,
    headers: Dict[str, str],
    payload: Dict[str, Any],
    timeout_seconds: float,
) -> Dict[str, Any]:
    body = json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(
        url=endpoint,
        data=body,
        headers=headers,
        method="POST",
    )

    try:
        with urllib.request.urlopen(request, timeout=timeout_seconds) as response:
            status = response.getcode()
            raw_body = response.read()
            text = raw_body.decode("utf-8", errors="replace")
            if status < 200 or status >= 300:
                raise ApiHttpError(status=status, raw_text=text)
            try:
                parsed = json.loads(text)
            except Exception as exc:  # noqa: BLE001
                raise RuntimeError("API returned non-JSON response") from exc
            if not isinstance(parsed, dict):
                raise RuntimeError("API returned JSON that is not an object")
            return parsed
    except urllib.error.HTTPError as exc:
        raw_text = ""
        try:
            raw_text = exc.read().decode("utf-8", errors="replace")
        except Exception:  # noqa: BLE001
            raw_text = str(exc)
        raise ApiHttpError(status=exc.code, raw_text=raw_text) from exc
    except urllib.error.URLError as exc:
        raise ApiNetworkError(str(getattr(exc, "reason", exc))) from exc
    except TimeoutError as exc:
        raise ApiNetworkError(str(exc)) from exc
    except OSError as exc:
        raise ApiNetworkError(str(exc)) from exc


def call_gemini_json(
    endpoint_template: str,
    model: str,
    api_key: str,
    payload: Dict[str, Any],
    timeout_seconds: float,
    max_retries: int,
    request_name: str,
) -> Dict[str, Any]:
    endpoint = endpoint_template.format(model=model)
    headers = {
        "Content-Type": "application/json",
        "x-goog-api-key": api_key,
    }

    last_error: Optional[Exception] = None
    for attempt in range(max_retries + 1):
        try:
            return post_json_with_urllib(
                endpoint=endpoint,
                headers=headers,
                payload=payload,
                timeout_seconds=timeout_seconds,
            )
        except ApiHttpError as exc:
            detail = (exc.raw_text or "").strip()
            if attempt < max_retries and exc.status in RETRYABLE_HTTP_STATUS:
                backoff = (2**attempt) + random.uniform(0.0, 0.75)
                logging.warning(
                    "%s: API HTTP %s, retrying in %.2fs (attempt %d/%d)",
                    request_name,
                    exc.status,
                    backoff,
                    attempt + 1,
                    max_retries,
                )
                time.sleep(backoff)
                continue
            message = f"{request_name}: API HTTP {exc.status}"
            if detail:
                message += f" | {detail[:260]}"
            last_error = RuntimeError(message)
            break
        except ApiNetworkError as exc:
            if attempt < max_retries:
                backoff = (2**attempt) + random.uniform(0.0, 0.75)
                logging.warning(
                    "%s: API network/timeout error (%s), retrying in %.2fs (attempt %d/%d)",
                    request_name,
                    exc,
                    backoff,
                    attempt + 1,
                    max_retries,
                )
                time.sleep(backoff)
                continue
            last_error = RuntimeError(f"{request_name}: API network/timeout error ({exc})")
            break

    if last_error is not None:
        raise last_error
    raise RuntimeError(f"{request_name}: unexpected retry loop exit")


def extract_text_from_response(payload: Dict[str, Any]) -> Optional[str]:
    candidates = payload.get("candidates")
    if isinstance(candidates, list):
        for candidate in candidates:
            if not isinstance(candidate, dict):
                continue
            content = candidate.get("content")
            if not isinstance(content, dict):
                continue
            parts = content.get("parts")
            if not isinstance(parts, list):
                continue
            for part in parts:
                if not isinstance(part, dict):
                    continue
                text = part.get("text")
                if isinstance(text, str) and text.strip():
                    return text.strip()
    return None


def detect_mime_from_image_bytes(data: bytes) -> Optional[str]:
    if data.startswith(b"\x89PNG\r\n\x1a\n"):
        return "image/png"
    if data.startswith(b"\xff\xd8\xff"):
        return "image/jpeg"
    if data.startswith(b"RIFF") and data[8:12] == b"WEBP":
        return "image/webp"
    if data.startswith(b"GIF87a") or data.startswith(b"GIF89a"):
        return "image/gif"
    if data.startswith(b"BM"):
        return "image/bmp"
    return None


def extension_from_mime(mime: Optional[str]) -> str:
    if not mime:
        return ".png"
    return MIME_TO_EXT.get(mime.lower(), ".png")


def extract_image_bytes_from_response(payload: Dict[str, Any]) -> Tuple[bytes, Optional[str]]:
    candidates = payload.get("candidates")
    if isinstance(candidates, list):
        for candidate in candidates:
            if not isinstance(candidate, dict):
                continue
            content = candidate.get("content")
            if not isinstance(content, dict):
                continue
            parts = content.get("parts")
            if not isinstance(parts, list):
                continue
            for part in parts:
                if not isinstance(part, dict):
                    continue
                inline_data = part.get("inlineData") or part.get("inline_data")
                if not isinstance(inline_data, dict):
                    continue
                data_b64 = inline_data.get("data")
                mime = inline_data.get("mimeType") or inline_data.get("mime_type")
                if isinstance(data_b64, str) and data_b64:
                    return base64.b64decode(data_b64), str(mime) if isinstance(mime, str) else None

    # Best effort compatibility with other response shapes.
    image_b64 = payload.get("image_base64")
    if isinstance(image_b64, str) and image_b64:
        return base64.b64decode(image_b64), "image/png"

    raise ValueError("No image found in Gemini response")


def crop_to_target_ratio(image: Image.Image, target_size: Tuple[int, int]) -> Image.Image:
    target_w, target_h = target_size
    if target_w <= 0 or target_h <= 0:
        return image

    width, height = image.size
    if width <= 0 or height <= 0:
        return image

    target_ratio = target_w / target_h
    current_ratio = width / height
    if abs(current_ratio - target_ratio) < 1e-6:
        return image

    if current_ratio > target_ratio:
        new_width = max(1, int(round(height * target_ratio)))
        left = max(0, (width - new_width) // 2)
        box = (left, 0, left + new_width, height)
    else:
        new_height = max(1, int(round(width / target_ratio)))
        top = max(0, (height - new_height) // 2)
        box = (0, top, width, top + new_height)
    return image.crop(box)


def convert_raw_bytes_to_ready(
    raw_bytes: bytes,
    prepared: PreparedInput,
    ready_jpeg_quality: int,
    ready_target_path: Path,
) -> Path:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    with Image.open(BytesIO(raw_bytes)) as img:
        working = img.convert("RGBA")
    working = crop_to_target_ratio(working, prepared.target_size)
    if working.size != prepared.target_size:
        working = working.resize(prepared.target_size, Image.LANCZOS)

    ready_target_path.parent.mkdir(parents=True, exist_ok=True)

    if prepared.has_alpha and prepared.alpha_mask is not None:
        alpha = prepared.alpha_mask
        if alpha.size != prepared.target_size:
            alpha = alpha.resize(prepared.target_size, Image.LANCZOS)
        r_chan, g_chan, b_chan, _ = working.split()
        merged = Image.merge("RGBA", (r_chan, g_chan, b_chan, alpha))
        merged.save(ready_target_path, format="PNG")
        return ready_target_path

    rgb = working.convert("RGB")
    rgb.save(ready_target_path, format="JPEG", quality=ready_jpeg_quality, optimize=True)
    return ready_target_path


def process_item(args: argparse.Namespace, paths: ItemPaths, stats: RunStats) -> None:
    stats.total += 1
    source_label = paths.asset_rel.as_posix()
    logging.debug(
        "mirror[%s]: strategy=%s raw_dir=%s ready_dir=%s",
        paths.source_path.as_posix(),
        paths.mirror_strategy,
        paths.raw_dir.as_posix(),
        paths.ready_dir.as_posix(),
    )

    ready_existing = find_existing_ready(paths)
    if ready_existing is not None and not args.force:
        stats.skipped_ready += 1
        logging.info("%s: ready already exists (%s), skipping", source_label, ready_existing.name)
        return
    if ready_existing is not None and args.force:
        logging.info("%s: forcing regeneration (ignoring existing ready %s)", source_label, ready_existing.name)

    try:
        decoded = decode_source_image(paths.source_path, args.max_dim)
        prepared = prepare_model_input(decoded, args.input_jpeg_quality)
    except Exception as exc:  # noqa: BLE001
        stats.failed += 1
        logging.error("%s: failed to decode source image: %s", source_label, exc)
        return

    raw_existing = None if args.force else find_existing_raw_image(paths)
    ready_ext = ".png" if prepared.has_alpha else ".jpeg"
    ready_path = paths.ready_dir / f"{paths.base_name}{ready_ext}"

    if raw_existing is not None:
        if args.dry_run:
            logging.info(
                "[DRY-RUN] %s: raw exists (%s), would convert raw -> %s",
                source_label,
                raw_existing.name,
                ready_path.name,
            )
            stats.converted_raw_to_ready += 1
            return
        try:
            raw_bytes = raw_existing.read_bytes()
            out_path = convert_raw_bytes_to_ready(
                raw_bytes=raw_bytes,
                prepared=prepared,
                ready_jpeg_quality=args.ready_jpeg_quality,
                ready_target_path=ready_path,
            )
            stats.converted_raw_to_ready += 1
            logging.info("%s: converted raw -> ready (%s)", source_label, out_path.as_posix())
            return
        except Exception as exc:  # noqa: BLE001
            stats.failed += 1
            logging.error("%s: failed converting existing raw -> ready: %s", source_label, exc)
            return

    # Full generation path.
    if args.dry_run:
        logging.info(
            "[DRY-RUN] %s: would generate director image from local custom prompt and save ready (%s)",
            source_label,
            ready_path.name,
        )
        stats.generated += 1
        return

    try:
        paths.raw_dir.mkdir(parents=True, exist_ok=True)

        original_input_bytes, original_input_ext = build_original_decoded_input_artifact(
            prepared,
            args.input_jpeg_quality,
        )
        original_path = paths.raw_dir / f"{paths.base_name}.original{original_input_ext}"
        original_path.write_bytes(original_input_bytes)
        input_path = paths.raw_dir / f"{paths.base_name}.input{prepared.input_ext}"
        input_path.write_bytes(prepared.input_bytes)

        save_json(
            paths.raw_dir / f"{paths.base_name}.prompt-designer.json",
            {
                "disabled": True,
                "stage": "designer",
                "reason": "disabled_by_configuration",
                "note": "Director prompt is now fully generated locally.",
            },
        )
        director_prompt = compose_director_prompt(
            asset_rel=paths.asset_rel,
            prepared=prepared,
        )

        director_payload = build_director_payload(director_prompt, prepared)
        save_json(
            paths.raw_dir / f"{paths.base_name}.prompt-director.json",
            sanitize_payload_without_images(director_payload),
        )

        director_response = call_gemini_json(
            endpoint_template=args.endpoint_template,
            model=args.model,
            api_key=args.api_key,
            payload=director_payload,
            timeout_seconds=args.timeout_seconds,
            max_retries=args.max_retries,
            request_name=f"{source_label} director",
        )
        raw_bytes, raw_mime = extract_image_bytes_from_response(director_response)
        if raw_mime is None:
            raw_mime = detect_mime_from_image_bytes(raw_bytes)
        raw_ext = extension_from_mime(raw_mime)

        raw_output_path = paths.raw_dir / f"{paths.base_name}{raw_ext}"
        raw_output_path.write_bytes(raw_bytes)

        out_path = convert_raw_bytes_to_ready(
            raw_bytes=raw_bytes,
            prepared=prepared,
            ready_jpeg_quality=args.ready_jpeg_quality,
            ready_target_path=ready_path,
        )
        stats.generated += 1
        logging.info(
            "%s: generated raw (%s) and ready (%s)",
            source_label,
            raw_output_path.name,
            out_path.as_posix(),
        )
    except Exception as exc:  # noqa: BLE001
        stats.failed += 1
        logging.error("%s: generation pipeline failed: %s", source_label, exc)


def main(argv: Iterable[str]) -> int:
    args = parse_args(argv)
    configure_logging(args.verbose)

    if Image is None:
        logging.error("Pillow is required. Install with `pip install Pillow`.")
        return 1

    if not args.api_key and not args.dry_run:
        logging.error("API key is required. Use --api-key or set GEMINI_API_KEY/NANO_BANANA_API_KEY.")
        return 1

    args.raw_root = args.raw_root.resolve()
    args.ready_root = args.ready_root.resolve()
    source_roots = [root.expanduser().resolve() for root in (args.source_root or [])]

    files = discover_input_files(args.inputs, recursive=args.recursive)
    if not files:
        logging.error("No input files found.")
        return 1

    allowed_exts = normalize_extensions(args.include_ext)
    candidates = [path for path in files if path.suffix.lower() in allowed_exts]
    skipped_unsupported = len(files) - len(candidates)
    if skipped_unsupported:
        logging.info("Skipping %d file(s) due to unsupported extension/filter.", skipped_unsupported)

    if not candidates:
        logging.error("No supported files to process after extension filtering.")
        return 1

    stats = RunStats()
    for source_path in candidates:
        item_paths = build_item_paths(source_path, args.raw_root, args.ready_root, source_roots)
        if item_paths.mirror_strategy == "basename-fallback":
            logging.warning(
                "%s: could not infer mirror root; using basename-only mirrored path (%s)",
                source_path.as_posix(),
                item_paths.asset_rel.as_posix(),
            )
        process_item(args, item_paths, stats)

    logging.info("---- Remaster Texture Summary ----")
    logging.info("Processed candidates: %d", stats.total)
    logging.info("Skipped (ready exists): %d", stats.skipped_ready)
    logging.info("Converted (raw -> ready): %d", stats.converted_raw_to_ready)
    logging.info("Generated (new raw + ready): %d", stats.generated)
    logging.info("Failed: %d", stats.failed)

    return 0 if stats.failed == 0 else 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
