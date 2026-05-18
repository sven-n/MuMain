#!/usr/bin/env python3
"""
Remaster GLB assets using a full-object generation workflow (V2).

Goal:
- Send full GLB context to the model.
- Expect an improved self-contained GLB in the response.
- Validate candidate output before accepting it.
- Track approximate polygon growth target (e.g., around 2x) with tolerance.

Behavior:
- Input can be a `.glb` file or directory.
- Outputs accepted assets to `assets/remaster_v2/...` by default.
- Stores raw request/response artifacts under `assets/remaster_v2_raw/...`.
- If generation/validation fails, falls back to original GLB copy.
"""

from __future__ import annotations

import argparse
import base64
import copy
import json
import logging
import os
import random
import re
import struct
import sys
import time
from dataclasses import dataclass, field
from io import BytesIO
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple
from urllib.parse import unquote, urlencode
import urllib.error
import urllib.request

try:
    import httpx
except ImportError:  # pragma: no cover - runtime dependency check
    httpx = None

try:
    from PIL import Image, UnidentifiedImageError
except ImportError:  # pragma: no cover - runtime dependency check
    Image = None
    UnidentifiedImageError = Exception

GLTF_MAGIC = 0x46546C67
JSON_CHUNK_TYPE = 0x4E4F534A
BIN_CHUNK_TYPE = 0x004E4942

DEFAULT_GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
DEFAULT_OPENAI_ENDPOINT = "https://api.openai.com/v1/responses"

EXT_BY_MIME = {
    "image/png": ".png",
    "image/jpeg": ".jpg",
    "image/jpg": ".jpg",
    "image/webp": ".webp",
    "image/bmp": ".bmp",
    "image/gif": ".gif",
    "image/x-tga": ".tga",
}
SAFE_NAME_RE = re.compile(r"[^a-zA-Z0-9._-]+")

DEFAULT_PROMPT = """You are a senior 3D technical artist and glTF engineer.

Task:
- Receive the source GLB and return an improved GLB (self-contained).
- You may improve mesh/materials/textures/UVs, but keep scale, orientation, and pivot stable.
- Preserve renderability and structural validity.
- If any input texture has black background (RGB 0,0,0), keep black background in the output.
- Never replace black background with white or gray.
- For regions intended to be transparent, render background as chroma key green RGB(0,255,0).
- Output must be a valid GLB 2.0 file with embedded textures (no external URI files).
- Return only JSON with this schema:
  {
    "glb_base64": "<base64 GLB binary>",
    "notes": "optional"
  }
- Do not wrap the JSON in markdown fences.
- Ensure `glb_base64` is complete and correctly padded with "=" when needed.
- Encode exact binary bytes of the GLB; do not alter or summarize the base64.
"""


@dataclass
class CandidateValidation:
    ok: bool
    reason: str
    triangle_count: int = 0


@dataclass
class FileReport:
    rel_glb: Path
    status: str = "pending"
    accepted: bool = False
    skipped_existing: bool = False
    fallback_used: bool = False
    fallback_reason: Optional[str] = None
    api_calls: int = 0
    original_triangles: int = 0
    candidate_triangles: Optional[int] = None
    triangle_ratio: Optional[float] = None
    notes: List[str] = field(default_factory=list)


@dataclass
class RunReport:
    files_total: int = 0
    files_ok: int = 0
    files_failed: int = 0
    files_skipped_existing: int = 0
    files_fallback: int = 0
    api_calls: int = 0
    accepted: int = 0


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Remaster GLB objects by sending full GLB context to a model and validating returned GLB.",
    )
    parser.add_argument(
        "input_paths",
        type=Path,
        nargs="+",
        help="One or more GLB files or directories containing GLB assets.",
    )
    parser.add_argument(
        "--raw-dir",
        type=Path,
        default=Path("assets/remaster_v2_raw"),
        help="Directory for raw request/response artifacts.",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("assets/remaster_v2"),
        help="Directory for accepted remastered GLBs.",
    )
    parser.add_argument(
        "--api-mode",
        choices=["gemini", "openai"],
        default="gemini",
        help="API request format (default: gemini).",
    )
    parser.add_argument(
        "--api-key",
        default=os.getenv("GEMINI_API_KEY") or os.getenv("NANO_BANANA_API_KEY"),
        help="API key. Defaults to GEMINI_API_KEY/NANO_BANANA_API_KEY env vars.",
    )
    parser.add_argument(
        "--model",
        default="nano-banana-pro-preview",
        help="Model name.",
    )
    parser.add_argument(
        "--endpoint-template",
        default=DEFAULT_GEMINI_ENDPOINT,
        help="Endpoint template, supports {model} placeholder.",
    )
    parser.add_argument(
        "--prompt",
        default=DEFAULT_PROMPT,
        help="Base prompt for full-object remaster request.",
    )
    parser.add_argument(
        "--poly-target",
        type=float,
        default=2.0,
        help="Target polygon ratio relative to source (default: 2.0).",
    )
    parser.add_argument(
        "--poly-tolerance",
        type=float,
        default=0.6,
        help="Allowed ratio tolerance around --poly-target (default: 0.6).",
    )
    parser.add_argument(
        "--min-poly-ratio",
        type=float,
        default=0.9,
        help="Hard minimum candidate/source triangle ratio to accept output (default: 0.9).",
    )
    parser.add_argument(
        "--strict-poly",
        action="store_true",
        help="Reject candidate if triangle ratio is outside target±tolerance.",
    )
    parser.add_argument(
        "--max-input-bytes",
        type=int,
        default=2_500_000,
        help="Maximum GLB bytes sent to model. Larger files fallback to original.",
    )
    parser.add_argument(
        "--max-retries",
        type=int,
        default=2,
        help="Retry attempts for transient API failures.",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=float,
        default=180.0,
        help="API request timeout in seconds.",
    )
    parser.add_argument(
        "--chroma-tolerance",
        type=int,
        default=70,
        help="Tolerance (0-255) for chroma-green to alpha conversion in candidate GLB textures.",
    )
    parser.add_argument(
        "--skip-existing",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="Skip processing when output GLB already exists (default: true).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Do not call API or write outputs.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable debug logging.",
    )

    args = parser.parse_args(list(argv))

    if args.poly_target <= 0:
        parser.error("--poly-target must be > 0")
    if args.poly_tolerance < 0:
        parser.error("--poly-tolerance must be >= 0")
    if args.min_poly_ratio <= 0:
        parser.error("--min-poly-ratio must be > 0")
    if args.max_input_bytes <= 0:
        parser.error("--max-input-bytes must be > 0")
    if args.max_retries < 0:
        parser.error("--max-retries must be >= 0")
    if args.timeout_seconds <= 0:
        parser.error("--timeout-seconds must be > 0")
    if args.chroma_tolerance < 0 or args.chroma_tolerance > 255:
        parser.error("--chroma-tolerance must be between 0 and 255")

    return args


def configure_logging(verbose: bool) -> None:
    logging.basicConfig(
        level=logging.DEBUG if verbose else logging.INFO,
        format="%(levelname)s: %(message)s",
    )
    logging.getLogger("httpx").setLevel(logging.WARNING)
    logging.getLogger("httpcore").setLevel(logging.WARNING)


def _try_relative(path: Path, base: Path) -> Optional[Path]:
    try:
        return path.relative_to(base)
    except ValueError:
        return None


def resolve_rel_glb_path(glb_path: Path, input_path: Path) -> Path:
    # Preserve hierarchy under nearest `assets/` directory.
    assets_indices = [idx for idx, part in enumerate(glb_path.parts) if part == "assets"]
    if assets_indices:
        idx = assets_indices[-1]
        if idx + 1 < len(glb_path.parts):
            return Path(*glb_path.parts[idx + 1 :])

    if input_path.is_dir():
        rel = _try_relative(glb_path, input_path)
        if rel is not None:
            return rel
    else:
        rel = _try_relative(glb_path, input_path.parent)
        if rel is not None:
            return rel

    return Path(glb_path.name)


def safe_image_name(name: str) -> str:
    cleaned = SAFE_NAME_RE.sub("_", name).strip("._-")
    return cleaned or "image"


def extension_from_mime(mime: Optional[str]) -> str:
    if not mime:
        return ".bin"
    return EXT_BY_MIME.get(mime.lower(), ".bin")


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


def looks_like_glb(data: bytes) -> bool:
    if len(data) < 20:
        return False
    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != GLTF_MAGIC or version != 2:
        return False
    return 0 < total_length <= len(data)


def align4(value: int) -> int:
    return (value + 3) & ~3


def build_glb(payload: Dict[str, Any], binary_blob: bytes) -> bytes:
    json_bytes = json.dumps(payload, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    json_pad = align4(len(json_bytes)) - len(json_bytes)
    if json_pad:
        json_bytes += b" " * json_pad

    bin_pad = align4(len(binary_blob)) - len(binary_blob)
    if bin_pad:
        binary_blob += b"\x00" * bin_pad

    total_length = 12 + 8 + len(json_bytes) + 8 + len(binary_blob)

    out = bytearray()
    out += struct.pack("<III", GLTF_MAGIC, 2, total_length)
    out += struct.pack("<II", len(json_bytes), JSON_CHUNK_TYPE)
    out += json_bytes
    out += struct.pack("<II", len(binary_blob), BIN_CHUNK_TYPE)
    out += binary_blob
    return bytes(out)


def ensure_buffer_structures(payload: Dict[str, Any], initial_bin_len: int) -> List[Dict[str, Any]]:
    buffers = payload.get("buffers")
    if not isinstance(buffers, list) or not buffers:
        payload["buffers"] = [{"byteLength": initial_bin_len}]
        buffers = payload["buffers"]

    first_buffer = buffers[0]
    if not isinstance(first_buffer, dict):
        first_buffer = {"byteLength": initial_bin_len}
        buffers[0] = first_buffer

    first_buffer["byteLength"] = initial_bin_len
    first_buffer.pop("uri", None)

    buffer_views = payload.get("bufferViews")
    if not isinstance(buffer_views, list):
        payload["bufferViews"] = []
        buffer_views = payload["bufferViews"]

    return buffer_views


def append_buffer_view(
    buffer_views: List[Dict[str, Any]],
    binary_blob: bytearray,
    data: bytes,
) -> int:
    aligned_offset = align4(len(binary_blob))
    if aligned_offset > len(binary_blob):
        binary_blob.extend(b"\x00" * (aligned_offset - len(binary_blob)))

    byte_offset = len(binary_blob)
    binary_blob.extend(data)

    view = {
        "buffer": 0,
        "byteOffset": byte_offset,
        "byteLength": len(data),
    }
    buffer_views.append(view)
    return len(buffer_views) - 1


def load_glb_payload_from_bytes(data: bytes) -> Tuple[Dict[str, Any], bytes]:
    if len(data) < 20:
        raise ValueError("GLB too small")

    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != GLTF_MAGIC:
        raise ValueError("Invalid GLB magic")
    if version != 2:
        raise ValueError(f"Unsupported GLB version: {version}")
    if total_length > len(data):
        raise ValueError("GLB truncated")

    offset = 12
    json_chunk: Optional[bytes] = None
    bin_chunk = b""

    while offset + 8 <= len(data):
        chunk_len, chunk_type = struct.unpack_from("<II", data, offset)
        offset += 8
        chunk_end = offset + chunk_len
        if chunk_end > len(data):
            raise ValueError("GLB chunk exceeds file size")

        chunk_data = data[offset:chunk_end]
        offset = chunk_end

        if chunk_type == JSON_CHUNK_TYPE and json_chunk is None:
            json_chunk = chunk_data
        elif chunk_type == BIN_CHUNK_TYPE and not bin_chunk:
            bin_chunk = chunk_data

    if json_chunk is None:
        raise ValueError("GLB missing JSON chunk")

    payload = json.loads(json_chunk.decode("utf-8").rstrip(" \t\r\n\x00"))
    if not isinstance(payload, dict):
        raise ValueError("GLB JSON root is not an object")

    return payload, bin_chunk


def preserve_source_material_render_metadata(
    source_payload: Dict[str, Any],
    candidate_payload: Dict[str, Any],
) -> bool:
    """Preserve source material semantics (blend/extras) on remastered candidates."""
    source_materials = source_payload.get("materials")
    candidate_materials = candidate_payload.get("materials")
    if not isinstance(source_materials, list) or not isinstance(candidate_materials, list):
        return False

    changed = False
    for index, source_material in enumerate(source_materials):
        if index >= len(candidate_materials):
            break
        if not isinstance(source_material, dict):
            continue

        candidate_material = candidate_materials[index]
        if not isinstance(candidate_material, dict):
            continue

        source_extras = source_material.get("extras")
        candidate_extras = candidate_material.get("extras")
        if isinstance(source_extras, dict):
            merged_extras: Dict[str, Any] = {}
            if isinstance(candidate_extras, dict):
                merged_extras.update(candidate_extras)
            merged_extras.update(source_extras)
            if candidate_extras != merged_extras:
                candidate_material["extras"] = merged_extras
                changed = True
        elif "extras" in source_material and candidate_extras != source_extras:
            candidate_material["extras"] = copy.deepcopy(source_extras)
            changed = True

        for key in ("alphaMode", "alphaCutoff", "doubleSided"):
            if key not in source_material:
                continue
            source_value = source_material[key]
            if candidate_material.get(key) != source_value:
                candidate_material[key] = copy.deepcopy(source_value)
                changed = True

        for key in ("emissiveFactor", "emissiveTexture"):
            if key not in source_material or key in candidate_material:
                continue
            candidate_material[key] = copy.deepcopy(source_material[key])
            changed = True

    return changed


def decode_data_uri(uri: str) -> Tuple[bytes, Optional[str]]:
    if not uri.startswith("data:"):
        raise ValueError("Not a data URI")
    comma = uri.find(",")
    if comma == -1:
        raise ValueError("Invalid data URI")

    header = uri[5:comma]
    payload = uri[comma + 1 :]
    is_base64 = False
    mime: Optional[str] = None
    if header:
        parts = header.split(";")
        if parts and "/" in parts[0]:
            mime = parts[0]
        if any(part.lower() == "base64" for part in parts[1:] if part):
            is_base64 = True
    if is_base64:
        return base64.b64decode(payload), mime
    return unquote(payload).encode("utf-8"), mime


def extract_request_images(payload: Dict[str, Any], bin_chunk: bytes) -> List[Tuple[int, str, bytes, Optional[str]]]:
    images = payload.get("images")
    if not isinstance(images, list):
        return []

    buffer_views = payload.get("bufferViews")
    if not isinstance(buffer_views, list):
        buffer_views = []

    out: List[Tuple[int, str, bytes, Optional[str]]] = []

    for index, image in enumerate(images):
        if not isinstance(image, dict):
            continue

        image_name_raw = image.get("name")
        if isinstance(image_name_raw, str) and image_name_raw.strip():
            image_name = safe_image_name(Path(image_name_raw).stem)
        else:
            image_name = f"image_{index:03d}"

        mime_raw = image.get("mimeType")
        mime = mime_raw if isinstance(mime_raw, str) and mime_raw else None
        blob: Optional[bytes] = None

        buffer_view_index = image.get("bufferView")
        if isinstance(buffer_view_index, int):
            if 0 <= buffer_view_index < len(buffer_views):
                view = buffer_views[buffer_view_index]
                if isinstance(view, dict):
                    try:
                        byte_offset = int(view.get("byteOffset", 0))
                        byte_length = int(view.get("byteLength", 0))
                    except Exception:  # noqa: BLE001
                        byte_offset = -1
                        byte_length = -1
                    if byte_offset >= 0 and byte_length > 0 and byte_offset + byte_length <= len(bin_chunk):
                        blob = bin_chunk[byte_offset : byte_offset + byte_length]
        else:
            uri = image.get("uri")
            if isinstance(uri, str) and uri.startswith("data:"):
                try:
                    data, uri_mime = decode_data_uri(uri)
                    blob = data
                    if mime is None:
                        mime = uri_mime
                except Exception:  # noqa: BLE001
                    blob = None

        if not blob:
            continue

        if mime is None:
            mime = detect_mime_from_image_bytes(blob)

        out.append((index, image_name, blob, mime))

    return out


def write_request_images(raw_root: Path, payload: Dict[str, Any], bin_chunk: bytes) -> List[Dict[str, Any]]:
    request_images = extract_request_images(payload, bin_chunk)
    if not request_images:
        return []

    image_dir = raw_root / "request_images"
    image_dir.mkdir(parents=True, exist_ok=True)

    metadata: List[Dict[str, Any]] = []
    for index, image_name, blob, mime in request_images:
        ext = extension_from_mime(mime)
        filename = f"{index:03d}_{image_name}{ext}"
        path = image_dir / filename
        path.write_bytes(blob)
        metadata.append(
            {
                "image_index": index,
                "name": image_name,
                "mime": mime,
                "bytes": len(blob),
                "path": f"request_images/{filename}",
            }
        )

    return metadata


def analyze_source_image_aspects(payload: Dict[str, Any], bin_chunk: bytes) -> Dict[int, float]:
    if Image is None:
        return {}

    aspects: Dict[int, float] = {}
    for index, _name, blob, _mime in extract_request_images(payload, bin_chunk):
        try:
            with Image.open(BytesIO(blob)) as img:
                width, height = img.size
                if width > 0 and height > 0:
                    aspects[index] = width / height
        except Exception:  # noqa: BLE001
            continue
    return aspects


def pad_image_to_square_black(blob: bytes) -> Tuple[bytes, bool]:
    if Image is None:
        return blob, False

    with Image.open(BytesIO(blob)) as img:
        rgb = img.convert("RGB")
        width, height = rgb.size
        if width <= 0 or height <= 0 or width == height:
            return blob, False

        side = max(width, height)
        square = Image.new("RGB", (side, side), (0, 0, 0))
        offset_x = (side - width) // 2
        offset_y = (side - height) // 2
        square.paste(rgb, (offset_x, offset_y))

        out = BytesIO()
        square.save(out, format="PNG")
        return out.getvalue(), True


def prepare_request_glb_for_model(
    source_glb: bytes,
) -> Tuple[bytes, Dict[str, Any]]:
    if Image is None:
        return source_glb, {"enabled": False, "reason": "pillow_missing", "padded_images": 0}

    payload, bin_chunk = load_glb_payload_from_bytes(source_glb)
    images = payload.get("images")
    if not isinstance(images, list) or not images:
        return source_glb, {"enabled": True, "reason": "no_images", "padded_images": 0}

    extracted = extract_request_images(payload, bin_chunk)
    by_index: Dict[int, bytes] = {index: blob for index, _name, blob, _mime in extracted}

    buffer_views = ensure_buffer_structures(payload, len(bin_chunk))
    new_binary_blob = bytearray(bin_chunk)

    padded_images = 0
    for image_index, image_obj in enumerate(images):
        if not isinstance(image_obj, dict):
            continue
        blob = by_index.get(image_index)
        if blob is None:
            continue
        padded_blob, changed = pad_image_to_square_black(blob)
        if not changed:
            continue

        new_view_index = append_buffer_view(buffer_views, new_binary_blob, padded_blob)
        image_obj["bufferView"] = new_view_index
        image_obj["mimeType"] = "image/png"
        image_obj.pop("uri", None)
        padded_images += 1

    if padded_images == 0:
        return source_glb, {"enabled": True, "padded_images": 0}

    buffers = payload.get("buffers")
    if isinstance(buffers, list) and buffers and isinstance(buffers[0], dict):
        buffers[0]["byteLength"] = len(new_binary_blob)
        buffers[0].pop("uri", None)

    out_glb = build_glb(payload, bytes(new_binary_blob))
    return out_glb, {"enabled": True, "padded_images": padded_images}


def _is_chroma_green(r: int, g: int, b: int, tolerance: int) -> bool:
    dist_sq = (r * r) + ((255 - g) * (255 - g)) + (b * b)
    if dist_sq <= tolerance * tolerance:
        return True
    dominance = g - max(r, b)
    return g >= 180 and dominance >= max(24, tolerance // 2) and r <= tolerance + 20 and b <= tolerance + 20


def _center_crop_to_ratio(img: Any, target_ratio: float) -> Tuple[Any, bool]:
    width, height = img.size
    if width <= 0 or height <= 0 or target_ratio <= 0:
        return img, False

    current_ratio = width / height
    if abs(current_ratio - target_ratio) <= 1e-6:
        return img, False

    if current_ratio > target_ratio:
        new_width = max(1, int(round(height * target_ratio)))
        left = max(0, (width - new_width) // 2)
        return img.crop((left, 0, left + new_width, height)), True

    new_height = max(1, int(round(width / target_ratio)))
    top = max(0, (height - new_height) // 2)
    return img.crop((0, top, width, top + new_height)), True


def process_candidate_image_blob(
    blob: bytes,
    target_ratio: Optional[float],
    chroma_tolerance: int,
) -> Tuple[bytes, str, bool, bool]:
    if Image is None:
        return blob, detect_mime_from_image_bytes(blob) or "application/octet-stream", False, False

    with Image.open(BytesIO(blob)) as img:
        rgba = img.convert("RGBA")
        cropped = False
        if target_ratio is not None and target_ratio > 0:
            rgba, cropped = _center_crop_to_ratio(rgba, target_ratio)

        rgb = rgba.convert("RGB")
        r_band, g_band, b_band = rgb.split()
        alpha_bytes = bytearray(rgba.getchannel("A").tobytes())

        chroma_removed = False
        r_bytes = r_band.tobytes()
        g_bytes = g_band.tobytes()
        b_bytes = b_band.tobytes()
        for idx, (r, g, b) in enumerate(zip(r_bytes, g_bytes, b_bytes)):
            if _is_chroma_green(r, g, b, chroma_tolerance):
                alpha_bytes[idx] = 0
                chroma_removed = True

        if chroma_removed:
            alpha = Image.frombytes("L", rgba.size, bytes(alpha_bytes))
            rgba.putalpha(alpha)

        changed = cropped or chroma_removed
        if not changed:
            return blob, detect_mime_from_image_bytes(blob) or "application/octet-stream", False, False

        out = BytesIO()
        rgba.save(out, format="PNG")
        return out.getvalue(), "image/png", cropped, chroma_removed


def postprocess_candidate_glb_images(
    candidate_glb: bytes,
    source_aspects: Dict[int, float],
    chroma_tolerance: int,
) -> Tuple[bytes, Dict[str, Any]]:
    if Image is None:
        return candidate_glb, {"enabled": False, "reason": "pillow_missing", "updated_images": 0}

    payload, bin_chunk = load_glb_payload_from_bytes(candidate_glb)
    images = payload.get("images")
    if not isinstance(images, list) or not images:
        return candidate_glb, {"enabled": True, "reason": "no_images", "updated_images": 0}

    extracted = extract_request_images(payload, bin_chunk)
    by_index: Dict[int, Tuple[bytes, Optional[str]]] = {index: (blob, mime) for index, _name, blob, mime in extracted}

    buffer_views = ensure_buffer_structures(payload, len(bin_chunk))
    new_binary_blob = bytearray(bin_chunk)

    updated_images = 0
    cropped_images = 0
    chroma_images = 0

    for image_index, image_obj in enumerate(images):
        if not isinstance(image_obj, dict):
            continue
        source_blob = by_index.get(image_index)
        if source_blob is None:
            continue

        blob, _mime = source_blob
        target_ratio = source_aspects.get(image_index)
        new_blob, new_mime, cropped, chroma_removed = process_candidate_image_blob(
            blob=blob,
            target_ratio=target_ratio,
            chroma_tolerance=chroma_tolerance,
        )
        if not (cropped or chroma_removed):
            continue

        new_view_index = append_buffer_view(buffer_views, new_binary_blob, new_blob)
        image_obj["bufferView"] = new_view_index
        image_obj["mimeType"] = new_mime
        image_obj.pop("uri", None)
        updated_images += 1
        if cropped:
            cropped_images += 1
        if chroma_removed:
            chroma_images += 1

    if updated_images == 0:
        return candidate_glb, {
            "enabled": True,
            "updated_images": 0,
            "cropped_images": 0,
            "chroma_removed_images": 0,
            "source_aspect_count": len(source_aspects),
        }

    buffers = payload.get("buffers")
    if isinstance(buffers, list) and buffers and isinstance(buffers[0], dict):
        buffers[0]["byteLength"] = len(new_binary_blob)
        buffers[0].pop("uri", None)

    out_glb = build_glb(payload, bytes(new_binary_blob))
    return out_glb, {
        "enabled": True,
        "updated_images": updated_images,
        "cropped_images": cropped_images,
        "chroma_removed_images": chroma_images,
        "source_aspect_count": len(source_aspects),
    }


def compute_triangle_count(payload: Dict[str, Any]) -> int:
    meshes = payload.get("meshes")
    accessors = payload.get("accessors")

    if not isinstance(meshes, list) or not isinstance(accessors, list):
        return 0

    def accessor_count(index: int) -> int:
        if index < 0 or index >= len(accessors):
            return 0
        accessor = accessors[index]
        if not isinstance(accessor, dict):
            return 0
        count = accessor.get("count")
        return int(count) if isinstance(count, int) and count >= 0 else 0

    triangles = 0
    for mesh in meshes:
        if not isinstance(mesh, dict):
            continue
        primitives = mesh.get("primitives")
        if not isinstance(primitives, list):
            continue

        for primitive in primitives:
            if not isinstance(primitive, dict):
                continue

            mode = primitive.get("mode", 4)
            if not isinstance(mode, int):
                mode = 4

            index_count = 0
            indices = primitive.get("indices")
            if isinstance(indices, int):
                index_count = accessor_count(indices)
            else:
                attrs = primitive.get("attributes")
                if isinstance(attrs, dict):
                    position = attrs.get("POSITION")
                    if isinstance(position, int):
                        index_count = accessor_count(position)

            if index_count <= 0:
                continue

            if mode == 4:  # TRIANGLES
                triangles += index_count // 3
            elif mode in (5, 6):  # TRIANGLE_STRIP / TRIANGLE_FAN
                triangles += max(0, index_count - 2)
            else:
                # Ignore non-triangle modes for this metric.
                continue

    return triangles


def validate_images_self_contained(payload: Dict[str, Any], bin_chunk: bytes) -> Tuple[bool, str]:
    images = payload.get("images")
    if not isinstance(images, list):
        return True, "no images"

    buffer_views = payload.get("bufferViews")
    if not isinstance(buffer_views, list):
        buffer_views = []

    for idx, image in enumerate(images):
        if not isinstance(image, dict):
            return False, f"image[{idx}] is not an object"

        buffer_view_index = image.get("bufferView")
        uri = image.get("uri")

        if isinstance(buffer_view_index, int):
            if buffer_view_index < 0 or buffer_view_index >= len(buffer_views):
                return False, f"image[{idx}] invalid bufferView index"
            view = buffer_views[buffer_view_index]
            if not isinstance(view, dict):
                return False, f"image[{idx}] bufferView is not object"
            byte_offset = int(view.get("byteOffset", 0))
            byte_length = int(view.get("byteLength", 0))
            if byte_offset < 0 or byte_length <= 0:
                return False, f"image[{idx}] invalid byte range"
            if byte_offset + byte_length > len(bin_chunk):
                return False, f"image[{idx}] bufferView outside BIN chunk"
            continue

        if isinstance(uri, str) and uri:
            if uri.startswith("data:"):
                try:
                    _decoded, _mime = decode_data_uri(uri)
                except Exception as exc:  # noqa: BLE001
                    return False, f"image[{idx}] invalid data URI: {exc}"
                continue
            return False, f"image[{idx}] external URI not allowed ({uri})"

        return False, f"image[{idx}] missing bufferView/uri"

    return True, "ok"


def validate_candidate_glb(glb_bytes: bytes) -> CandidateValidation:
    if not looks_like_glb(glb_bytes):
        return CandidateValidation(ok=False, reason="candidate bytes are not a valid GLB header")

    try:
        payload, bin_chunk = load_glb_payload_from_bytes(glb_bytes)
    except Exception as exc:  # noqa: BLE001
        return CandidateValidation(ok=False, reason=f"invalid GLB structure: {exc}")

    images_ok, reason = validate_images_self_contained(payload, bin_chunk)
    if not images_ok:
        return CandidateValidation(ok=False, reason=f"invalid image references: {reason}")

    triangles = compute_triangle_count(payload)
    if triangles <= 0:
        return CandidateValidation(ok=False, reason="candidate has zero triangle primitives")

    return CandidateValidation(ok=True, reason="ok", triangle_count=triangles)


def build_object_prompt(
    base_prompt: str,
    rel_glb: Path,
    original_triangles: int,
    poly_target: float,
    poly_tolerance: float,
    min_poly_ratio: float,
    padded_image_count: int = 0,
) -> str:
    min_ratio = max(0.1, poly_target - poly_tolerance)
    max_ratio = poly_target + poly_tolerance
    prompt = (
        f"{base_prompt}\n\n"
        f"Source file: {rel_glb.as_posix()}\n"
        f"Original triangles (estimated): {original_triangles}\n"
        f"Target triangle ratio (approx): {poly_target:.3f}\n"
        f"Allowed triangle ratio window: {min_ratio:.3f} - {max_ratio:.3f}\n"
        f"Hard minimum triangle ratio: {min_poly_ratio:.3f}\n"
        "If source textures contain black background RGB(0,0,0), keep it black in output.\n"
        "Never convert black background to white/gray.\n"
        "Transparent regions should use chroma key green RGB(0,255,0).\n"
        f"Return strict JSON only."
    )
    if padded_image_count > 0:
        prompt += (
            f"\nNon-square source textures were padded to square with black borders "
            f"({padded_image_count} images) before this request. Keep main content centered "
            "so center-cropping back to original aspect is preserved."
        )
    return prompt


def build_gemini_payload(prompt: str, glb_bytes: bytes) -> Dict[str, Any]:
    text_prompt = (
        f"{prompt}\n\n"
        "Source GLB (base64):\n"
        f"{base64.b64encode(glb_bytes).decode('ascii')}"
    )
    return {
        "contents": [
            {
                "parts": [
                    {"text": text_prompt},
                ]
            }
        ],
        "generationConfig": {
            "responseModalities": ["TEXT"],
        },
    }


def _extract_json_objects_from_text(text: str) -> List[Dict[str, Any]]:
    results: List[Dict[str, Any]] = []
    candidates: List[str] = [text]

    # Prefer fenced blocks when model wraps JSON in markdown.
    for match in re.finditer(r"```(?:json)?\s*([\s\S]*?)```", text, flags=re.IGNORECASE):
        block = match.group(1).strip()
        if block:
            candidates.append(block)

    seen: set[str] = set()
    for candidate in candidates:
        candidate = candidate.strip()
        if not candidate or candidate in seen:
            continue
        seen.add(candidate)

        # Direct parse first.
        try:
            parsed = json.loads(candidate)
            if isinstance(parsed, dict):
                results.append(parsed)
                continue
        except Exception:  # noqa: BLE001
            pass

        # Fallback: extract inline object snippets.
        for match in re.finditer(r"\{[\s\S]*?\}", candidate):
            chunk = match.group(0)
            try:
                parsed = json.loads(chunk)
                if isinstance(parsed, dict):
                    results.append(parsed)
            except Exception:  # noqa: BLE001
                continue

    return results


def _decode_possible_glb_base64(value: str) -> Optional[bytes]:
    try:
        raw = value.strip()
        if raw.startswith("data:") and "," in raw:
            raw = raw.split(",", 1)[1]
        # Model outputs frequently include newlines/spaces or omit trailing "=" padding.
        raw = re.sub(r"\s+", "", raw)
        raw = re.sub(r"[^A-Za-z0-9+/=]", "", raw)
        if not raw:
            return None
        raw = raw + ("=" * ((4 - (len(raw) % 4)) % 4))
        data = base64.b64decode(raw)
    except Exception:  # noqa: BLE001
        return None
    return data if data else None


def extract_candidate_from_response(payload: Dict[str, Any]) -> Tuple[Optional[bytes], List[bytes], str]:
    """Return (candidate_glb, response_images, notes)."""
    images: List[bytes] = []
    text_parts: List[str] = []

    # Gemini-style candidates.
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
                if isinstance(inline_data, dict):
                    data_b64 = inline_data.get("data")
                    mime = str(inline_data.get("mimeType") or inline_data.get("mime_type") or "")
                    if isinstance(data_b64, str) and data_b64:
                        try:
                            blob = base64.b64decode(data_b64)
                        except Exception:  # noqa: BLE001
                            blob = b""
                        if blob and looks_like_glb(blob):
                            return blob, images, "inlineData GLB"
                        if blob and mime.startswith("image/"):
                            images.append(blob)

                text = part.get("text")
                if isinstance(text, str) and text.strip():
                    text_parts.append(text)

    # OpenAI-style response objects.
    output = payload.get("output")
    if isinstance(output, list):
        for item in output:
            if not isinstance(item, dict):
                continue
            content = item.get("content")
            if not isinstance(content, list):
                continue
            for block in content:
                if not isinstance(block, dict):
                    continue
                text = block.get("text")
                if isinstance(text, str) and text.strip():
                    text_parts.append(text)
                image_b64 = block.get("image_base64") or block.get("b64_json")
                if isinstance(image_b64, str) and image_b64:
                    try:
                        blob = base64.b64decode(image_b64)
                    except Exception:  # noqa: BLE001
                        blob = b""
                    if blob and looks_like_glb(blob):
                        return blob, images, "output content GLB"
                    if blob:
                        images.append(blob)

    # Try top-level direct keys.
    for key in ("glb_base64", "model_glb_base64", "glb"):
        value = payload.get(key)
        if isinstance(value, str) and value.strip():
            data = _decode_possible_glb_base64(value)
            if data is not None:
                return data, images, f"top-level {key}"

    # Try parsing text parts as JSON.
    for text in text_parts:
        for parsed in _extract_json_objects_from_text(text):
            for key in ("glb_base64", "model_glb_base64", "glb"):
                value = parsed.get(key)
                if isinstance(value, str) and value.strip():
                    data = _decode_possible_glb_base64(value)
                    if data is not None:
                        notes = str(parsed.get("notes") or parsed.get("note") or "json text response")
                        return data, images, notes

    notes = text_parts[0][:400] if text_parts else "no candidate GLB found"
    return None, images, notes


class ApiHttpError(RuntimeError):
    def __init__(self, status: int, raw_text: str):
        self.status = status
        self.raw_text = raw_text
        super().__init__(f"HTTP {status}")


class ApiNetworkError(RuntimeError):
    pass


def post_json_with_urllib(
    endpoint: str,
    params: Dict[str, str],
    headers: Dict[str, str],
    payload: Dict[str, Any],
    timeout_seconds: float,
) -> Dict[str, Any]:
    url = endpoint
    if params:
        url = f"{endpoint}?{urlencode(params)}"

    body = json.dumps(payload).encode("utf-8")
    request = urllib.request.Request(url=url, data=body, headers=headers, method="POST")

    try:
        with urllib.request.urlopen(request, timeout=timeout_seconds) as response:
            raw_body = response.read()
            status = response.getcode()
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


def call_object_remaster_api(
    client: Any,
    api_mode: str,
    endpoint_template: str,
    model: str,
    api_key: str,
    prompt: str,
    glb_bytes: bytes,
    timeout_seconds: float,
    max_retries: int,
) -> Dict[str, Any]:
    endpoint = endpoint_template.format(model=model)
    headers = {"Content-Type": "application/json"}
    params: Dict[str, str] = {}
    payload_variants: List[Tuple[str, Dict[str, Any]]] = []

    if api_mode == "gemini":
        headers["x-goog-api-key"] = api_key
        payload_variants.append(("gemini_text_base64", build_gemini_payload(prompt, glb_bytes)))
    elif api_mode == "openai":
        if endpoint_template == DEFAULT_GEMINI_ENDPOINT:
            endpoint = DEFAULT_OPENAI_ENDPOINT
        headers["Authorization"] = f"Bearer {api_key}"
        text_input = (
            f"{prompt}\n\n"
            "Source GLB (base64, may be large):\n"
            f"{base64.b64encode(glb_bytes).decode('ascii')}"
        )
        payload = {
            "model": model,
            "input": [
                {
                    "role": "user",
                    "content": [
                        {"type": "input_text", "text": text_input},
                    ],
                }
            ],
            "modalities": ["text"],
        }
        payload_variants.append(("openai_text_base64", payload))
    else:
        raise ValueError(f"Unsupported api_mode: {api_mode}")

    timeout = httpx.Timeout(timeout_seconds) if httpx is not None else timeout_seconds

    last_error: Optional[Exception] = None

    for variant_name, payload in payload_variants:
        for attempt in range(max_retries + 1):
            try:
                if httpx is not None:
                    if client is None:
                        raise RuntimeError("HTTP client unavailable")
                    try:
                        response = client.post(
                            endpoint,
                            params=params,
                            headers=headers,
                            json=payload,
                            timeout=timeout,
                        )
                    except Exception as exc:  # noqa: BLE001
                        if isinstance(exc, (httpx.TimeoutException, httpx.NetworkError)):
                            raise ApiNetworkError(str(exc)) from exc
                        raise

                    if response.status_code in {429, 500, 502, 503, 504} and attempt < max_retries:
                        backoff = (2**attempt) + random.uniform(0.0, 0.75)
                        logging.warning(
                            "API transient HTTP %s, retrying in %.2fs (attempt %d/%d) [%s]",
                            response.status_code,
                            backoff,
                            attempt + 1,
                            max_retries,
                            variant_name,
                        )
                        time.sleep(backoff)
                        continue

                    if response.status_code < 200 or response.status_code >= 300:
                        raise ApiHttpError(status=response.status_code, raw_text=response.text)

                    try:
                        parsed = response.json()
                    except Exception as exc:  # noqa: BLE001
                        raise RuntimeError("API returned non-JSON response") from exc
                    if not isinstance(parsed, dict):
                        raise RuntimeError("API returned JSON that is not an object")
                    return parsed

                parsed = post_json_with_urllib(
                    endpoint=endpoint,
                    params=params,
                    headers=headers,
                    payload=payload,
                    timeout_seconds=timeout_seconds,
                )
                return parsed
            except ApiHttpError as exc:
                status = exc.status
                raw_text = (exc.raw_text or "").strip()
                detail = f" | {raw_text[:260]}" if raw_text else ""

                last_error = RuntimeError(f"API returned HTTP {status}{detail}")
                if attempt < max_retries and status in {408, 409}:
                    backoff = (2**attempt) + random.uniform(0.0, 0.75)
                    logging.warning(
                        "API HTTP %s, retrying in %.2fs (attempt %d/%d) [%s]",
                        status,
                        backoff,
                        attempt + 1,
                        max_retries,
                        variant_name,
                    )
                    time.sleep(backoff)
                    continue
                break
            except ApiNetworkError as exc:
                if attempt < max_retries:
                    backoff = (2**attempt) + random.uniform(0.0, 0.75)
                    logging.warning(
                        "API network/timeout error (%s), retrying in %.2fs (attempt %d/%d) [%s]",
                        exc,
                        backoff,
                        attempt + 1,
                        max_retries,
                        variant_name,
                    )
                    time.sleep(backoff)
                    continue
                last_error = exc
                break

    if last_error is not None:
        raise last_error
    raise RuntimeError("Unexpected retry loop exit")


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")


def build_decision_payload(report: FileReport, args: argparse.Namespace) -> Dict[str, Any]:
    return {
        "status": report.status,
        "accepted": report.accepted,
        "skipped_existing": report.skipped_existing,
        "fallback_used": report.fallback_used,
        "fallback_reason": report.fallback_reason,
        "original_triangles": report.original_triangles,
        "candidate_triangles": report.candidate_triangles,
        "triangle_ratio": report.triangle_ratio,
        "poly_target": args.poly_target,
        "poly_tolerance": args.poly_tolerance,
        "min_poly_ratio": args.min_poly_ratio,
        "chroma_tolerance": args.chroma_tolerance,
        "strict_poly": args.strict_poly,
        "api_calls": report.api_calls,
        "notes": report.notes,
    }


def process_glb(
    glb_path: Path,
    rel_glb: Path,
    args: argparse.Namespace,
    client: Optional[Any],
) -> FileReport:
    report = FileReport(rel_glb=rel_glb)

    out_path = args.out_dir / rel_glb
    raw_root = args.raw_dir / rel_glb.with_suffix("")

    if args.skip_existing and out_path.exists() and out_path.is_file():
        report.status = "skipped_existing"
        report.skipped_existing = True
        report.accepted = True
        return report

    original_bytes = glb_path.read_bytes()
    raw_root.mkdir(parents=True, exist_ok=True)
    (raw_root / "input.glb").write_bytes(original_bytes)

    try:
        original_payload, original_bin = load_glb_payload_from_bytes(original_bytes)
    except Exception as exc:  # noqa: BLE001
        report.status = "failed"
        report.notes.append(f"Invalid input GLB: {exc}")
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    request_images_meta = write_request_images(raw_root, original_payload, original_bin)
    source_image_aspects = analyze_source_image_aspects(original_payload, original_bin)
    request_glb_bytes = original_bytes
    request_preprocess_meta: Dict[str, Any] = {"enabled": False, "padded_images": 0}
    try:
        request_glb_bytes, request_preprocess_meta = prepare_request_glb_for_model(original_bytes)
    except Exception as exc:  # noqa: BLE001
        report.notes.append(f"request_preprocess_failed: {exc}")
        request_glb_bytes = original_bytes
        request_preprocess_meta = {"enabled": False, "padded_images": 0, "error": str(exc)}

    if request_glb_bytes != original_bytes:
        (raw_root / "input_model.glb").write_bytes(request_glb_bytes)
    write_json(raw_root / "request_preprocess.json", request_preprocess_meta)

    write_json(
        raw_root / "request_images.json",
        {
            "count": len(request_images_meta),
            "images": request_images_meta,
        },
    )

    report.original_triangles = compute_triangle_count(original_payload)

    if report.original_triangles <= 0:
        report.status = "failed"
        report.notes.append("Input GLB has zero triangle primitives")
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    if args.max_input_bytes and len(request_glb_bytes) > args.max_input_bytes:
        report.status = "fallback"
        report.fallback_used = True
        report.fallback_reason = (
            f"input_too_large ({len(request_glb_bytes)} bytes > {args.max_input_bytes})"
        )
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(original_bytes)
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    prompt = build_object_prompt(
        base_prompt=args.prompt,
        rel_glb=rel_glb,
        original_triangles=report.original_triangles,
        poly_target=args.poly_target,
        poly_tolerance=args.poly_tolerance,
        min_poly_ratio=args.min_poly_ratio,
        padded_image_count=int(request_preprocess_meta.get("padded_images", 0) or 0),
    )
    write_json(
        raw_root / "request_meta.json",
        {
            "model": args.model,
            "api_mode": args.api_mode,
            "poly_target": args.poly_target,
            "poly_tolerance": args.poly_tolerance,
            "min_poly_ratio": args.min_poly_ratio,
            "strict_poly": args.strict_poly,
            "input_bytes": len(original_bytes),
            "model_input_bytes": len(request_glb_bytes),
            "original_triangles": report.original_triangles,
            "request_images_saved": len(request_images_meta),
            "source_image_aspects": len(source_image_aspects),
            "chroma_tolerance": args.chroma_tolerance,
            "request_preprocess": request_preprocess_meta,
            "prompt": prompt,
        },
    )

    if args.dry_run:
        report.status = "dry_run"
        report.accepted = True
        return report

    if client is None and httpx is not None:
        report.status = "failed"
        report.notes.append("HTTP client unavailable")
        return report

    try:
        report.api_calls += 1
        response_payload = call_object_remaster_api(
            client=client,
            api_mode=args.api_mode,
            endpoint_template=args.endpoint_template,
            model=args.model,
            api_key=args.api_key,
            prompt=prompt,
            glb_bytes=request_glb_bytes,
            timeout_seconds=args.timeout_seconds,
            max_retries=args.max_retries,
        )
    except Exception as exc:  # noqa: BLE001
        report.status = "fallback"
        report.fallback_used = True
        report.fallback_reason = f"api_failed: {exc}"
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(original_bytes)
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    write_json(raw_root / "response.json", response_payload)

    candidate_glb, response_images, notes = extract_candidate_from_response(response_payload)
    if notes:
        report.notes.append(notes)

    if response_images:
        image_dir = raw_root / "response_images"
        image_dir.mkdir(parents=True, exist_ok=True)
        for index, data in enumerate(response_images):
            image_path = image_dir / f"{index:03d}.bin"
            image_path.write_bytes(data)

    if candidate_glb is None:
        report.status = "fallback"
        report.fallback_used = True
        report.fallback_reason = "no_candidate_glb_in_response"
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(original_bytes)
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    (raw_root / "candidate.glb").write_bytes(candidate_glb)

    processed_candidate_glb = candidate_glb
    postprocess_meta: Dict[str, Any] = {
        "enabled": False,
        "updated_images": 0,
    }
    try:
        processed_candidate_glb, postprocess_meta = postprocess_candidate_glb_images(
            candidate_glb=candidate_glb,
            source_aspects=source_image_aspects,
            chroma_tolerance=args.chroma_tolerance,
        )
    except Exception as exc:  # noqa: BLE001
        report.notes.append(f"postprocess_failed: {exc}")

    write_json(raw_root / "postprocess.json", postprocess_meta)
    if processed_candidate_glb != candidate_glb:
        (raw_root / "candidate_post.glb").write_bytes(processed_candidate_glb)
        report.notes.append(
            f"postprocess updated images: {postprocess_meta.get('updated_images', 0)} "
            f"(cropped={postprocess_meta.get('cropped_images', 0)}, "
            f"chroma={postprocess_meta.get('chroma_removed_images', 0)})"
        )

    validation = validate_candidate_glb(processed_candidate_glb)
    if not validation.ok:
        if validation.triangle_count > 0:
            report.candidate_triangles = validation.triangle_count
            if report.original_triangles > 0:
                report.triangle_ratio = report.candidate_triangles / report.original_triangles
        report.status = "fallback"
        report.fallback_used = True
        report.fallback_reason = f"candidate_invalid: {validation.reason}"
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(original_bytes)
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    report.candidate_triangles = validation.triangle_count
    report.triangle_ratio = (
        report.candidate_triangles / report.original_triangles
        if report.original_triangles > 0
        else None
    )

    min_ratio = max(0.1, args.poly_target - args.poly_tolerance)
    max_ratio = args.poly_target + args.poly_tolerance

    if report.triangle_ratio is not None and report.triangle_ratio < args.min_poly_ratio:
        report.status = "fallback"
        report.fallback_used = True
        report.fallback_reason = (
            f"triangle_ratio_below_minimum: {report.triangle_ratio:.3f} "
            f"(minimum {args.min_poly_ratio:.3f})"
        )
        report.notes.append(report.fallback_reason)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_bytes(original_bytes)
        write_json(raw_root / "decision.json", build_decision_payload(report, args))
        return report

    ratio_outside = report.triangle_ratio is not None and not (min_ratio <= report.triangle_ratio <= max_ratio)
    if ratio_outside:
        ratio_msg = (
            f"triangle_ratio_out_of_range: {report.triangle_ratio:.3f} "
            f"(expected {min_ratio:.3f}-{max_ratio:.3f})"
        )
        report.notes.append(ratio_msg)
        if args.strict_poly:
            report.status = "fallback"
            report.fallback_used = True
            report.fallback_reason = ratio_msg
            out_path.parent.mkdir(parents=True, exist_ok=True)
            out_path.write_bytes(original_bytes)
            write_json(raw_root / "decision.json", build_decision_payload(report, args))
            return report

    output_glb = processed_candidate_glb
    try:
        candidate_payload, candidate_bin = load_glb_payload_from_bytes(processed_candidate_glb)
        if preserve_source_material_render_metadata(original_payload, candidate_payload):
            output_glb = build_glb(candidate_payload, candidate_bin)
            report.notes.append("source material render metadata preserved")
    except Exception as exc:  # noqa: BLE001
        report.notes.append(f"material metadata preservation failed: {exc}")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(output_glb)

    report.status = "accepted"
    report.accepted = True

    write_json(raw_root / "decision.json", build_decision_payload(report, args))

    return report


def aggregate(run_report: RunReport, file_report: FileReport) -> None:
    run_report.files_total += 1
    run_report.api_calls += file_report.api_calls

    if file_report.skipped_existing:
        run_report.files_skipped_existing += 1

    if file_report.fallback_used:
        run_report.files_fallback += 1

    if file_report.accepted or file_report.fallback_used or file_report.skipped_existing:
        run_report.files_ok += 1
    else:
        run_report.files_failed += 1

    if file_report.accepted:
        run_report.accepted += 1


def main(argv: Iterable[str]) -> int:
    args = parse_args(argv)
    configure_logging(args.verbose)

    if args.api_mode == "openai" and args.endpoint_template == DEFAULT_GEMINI_ENDPOINT:
        args.endpoint_template = DEFAULT_OPENAI_ENDPOINT

    if httpx is None and not args.dry_run:
        logging.warning("httpx not installed, using urllib fallback HTTP client.")
    if Image is None and not args.dry_run:
        logging.warning("Pillow not installed, texture pad/crop/chroma postprocess is disabled.")

    input_paths = [path.resolve() for path in args.input_paths]
    args.raw_dir = args.raw_dir.resolve()
    args.out_dir = args.out_dir.resolve()
    entries: List[Tuple[Path, Path]] = []
    seen_glb_paths: set[Path] = set()

    for input_path in input_paths:
        if not input_path.exists():
            logging.error("Input path does not exist: %s", input_path)
            return 2

        if input_path.is_file():
            if input_path.suffix.lower() != ".glb":
                logging.error("Input file must be a .glb: %s", input_path)
                return 2
            if input_path not in seen_glb_paths:
                seen_glb_paths.add(input_path)
                entries.append((input_path, resolve_rel_glb_path(input_path, input_path)))
            continue

        if input_path.is_dir():
            glb_files = sorted(input_path.rglob("*.glb"), key=lambda p: p.as_posix().lower())
            for glb_path in glb_files:
                if glb_path in seen_glb_paths:
                    continue
                seen_glb_paths.add(glb_path)
                entries.append((glb_path, resolve_rel_glb_path(glb_path, input_path)))
            continue

        logging.error("Input path must be a file or directory: %s", input_path)
        return 2

    if not entries:
        logging.warning("No .glb files found under provided input paths")
        return 0

    if not args.dry_run and not args.api_key:
        logging.error("API key is required. Use --api-key or env GEMINI_API_KEY/NANO_BANANA_API_KEY.")
        return 2

    if not args.dry_run:
        args.raw_dir.mkdir(parents=True, exist_ok=True)
        args.out_dir.mkdir(parents=True, exist_ok=True)

    if len(input_paths) == 1 and input_paths[0].is_file():
        logging.info("Input file: %s", input_paths[0])
    elif len(input_paths) == 1 and input_paths[0].is_dir():
        logging.info("Input dir: %s", input_paths[0])
    else:
        logging.info("Input paths: %d", len(input_paths))
        for input_path in input_paths:
            logging.info(" - %s", input_path)
    logging.info("Raw dir: %s", args.raw_dir)
    logging.info("Out dir: %s", args.out_dir)
    logging.info("Found %d GLB file(s)", len(entries))
    logging.info(
        "Triangle target: %.3fx ± %.3f (min=%.3f, strict=%s, chroma_tol=%d)",
        args.poly_target,
        args.poly_tolerance,
        args.min_poly_ratio,
        args.strict_poly,
        args.chroma_tolerance,
    )

    run_report = RunReport()

    if args.dry_run:
        for glb_path, rel_glb in entries:
            logging.info("[DRY-RUN] Would process %s", rel_glb.as_posix())
            report = FileReport(rel_glb=rel_glb, status="dry_run", accepted=True)
            try:
                payload, _ = load_glb_payload_from_bytes(glb_path.read_bytes())
                report.original_triangles = compute_triangle_count(payload)
            except Exception as exc:  # noqa: BLE001
                report.status = "failed"
                report.accepted = False
                report.notes.append(f"invalid input: {exc}")
            aggregate(run_report, report)
    else:
        def run_processing_loop(client_obj: Optional[Any]) -> None:
            for glb_path, rel_glb in entries:
                logging.info("Processing %s", rel_glb.as_posix())
                report = process_glb(glb_path, rel_glb, args, client_obj)
                aggregate(run_report, report)
                if report.status == "accepted":
                    ratio = f"{report.triangle_ratio:.3f}" if report.triangle_ratio is not None else "n/a"
                    logging.info(
                        "%s: accepted (tri %d -> %d, ratio=%s)",
                        rel_glb.as_posix(),
                        report.original_triangles,
                        report.candidate_triangles,
                        ratio,
                    )
                elif report.status == "skipped_existing":
                    logging.info("%s: skipped (existing output)", rel_glb.as_posix())
                elif report.fallback_used:
                    candidate_tri = report.candidate_triangles if report.candidate_triangles is not None else "n/a"
                    ratio = f"{report.triangle_ratio:.3f}" if report.triangle_ratio is not None else "n/a"
                    logging.warning(
                        "%s: fallback to original (%s) (tri %d -> %s, ratio=%s)",
                        rel_glb.as_posix(),
                        report.fallback_reason or "unknown",
                        report.original_triangles,
                        candidate_tri,
                        ratio,
                    )
                else:
                    candidate_tri = report.candidate_triangles if report.candidate_triangles is not None else "n/a"
                    ratio = f"{report.triangle_ratio:.3f}" if report.triangle_ratio is not None else "n/a"
                    logging.error(
                        "%s: failed (%s) (tri %d -> %s, ratio=%s)",
                        rel_glb.as_posix(),
                        "; ".join(report.notes) or "unknown",
                        report.original_triangles,
                        candidate_tri,
                        ratio,
                    )

        if httpx is not None:
            with httpx.Client() as client:
                run_processing_loop(client)
        else:
            run_processing_loop(None)

    logging.info("---- V2 Summary ----")
    logging.info("Files: %d total | %d ok | %d failed", run_report.files_total, run_report.files_ok, run_report.files_failed)
    logging.info("Accepted: %d", run_report.accepted)
    logging.info("Skipped existing: %d", run_report.files_skipped_existing)
    logging.info("Fallback copies: %d", run_report.files_fallback)
    logging.info("API calls: %d", run_report.api_calls)

    return 1 if run_report.files_failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
