#!/usr/bin/env python3
"""
Remaster GLB textures with a text-to-image API (Nano Banana 3 Flash workflow).

Pipeline:
1. Read one `.glb` file or recursively scan an input directory.
2. Load each GLB in memory, extract embedded/external image bytes.
3. Select textures (all images by default, optional baseColor-only mode).
4. For non-square textures, pad to square (black) before API request.
5. Save original texture bytes to `assets/remaster_raw/{same_glb_relative_path}/...`.
6. Perform one API request per eligible texture.
7. If padded, crop response back to original aspect ratio.
8. Convert chroma-green to alpha for transparent textures and inject into the GLB.
9. Save remastered GLBs to `assets/remaster/{same_relative_glb_path}`.
"""

from __future__ import annotations

import argparse
import base64
import json
import logging
import os
import random
import re
import struct
import sys
import time
from dataclasses import dataclass
from io import BytesIO
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Set, Tuple
from urllib.parse import unquote

try:
    import httpx
except ImportError:  # pragma: no cover - runtime dependency check
    httpx = None

try:
    from PIL import Image, UnidentifiedImageError
except ImportError:  # pragma: no cover - runtime dependency check
    Image = None
    UnidentifiedImageError = Exception

JSON_CHUNK_TYPE = 0x4E4F534A
BIN_CHUNK_TYPE = 0x004E4942
GLTF_MAGIC = 0x46546C67

MIME_BY_EXT = {
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".webp": "image/webp",
    ".bmp": "image/bmp",
    ".gif": "image/gif",
    ".tga": "image/x-tga",
}

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

DEFAULT_PROMPT = (
"""# You are a Game Asset Artist specialized in remaster game assets.

Remaster this game texture in high quality, preserving composition, UV layout,
And semantic details. Keep it tile-friendly, artifact-free, and production-ready.
Make it more realist. Remove Noise when possible. Keep the same shape! Dark Fantasy Game!
Must keep UV layout and preserving composition!!!!
If the source has black background (RGB 0,0,0), keep black background in output.
Never replace black background with white/gray.
Must keep the same visual style
"""
)

DEFAULT_NORMAL_MAP_PROMPT = (
"""Generate a tangent-space normal map from this texture.
Output only the normal map image (no text), with clean details and no transparency.
Keep UV alignment faithful to the source texture and avoid artifacts."""
)

DEFAULT_GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/{model}:generateContent"
DEFAULT_OPENAI_ENDPOINT = "https://api.openai.com/v1/responses"


@dataclass
class TextureSource:
    image_index: int
    source_kind: str  # bufferview | data_uri | external_uri
    raw_bytes: bytes
    mime_type: Optional[str]
    logical_name: str


@dataclass
class TexturePrepared:
    has_transparency: bool
    upload_png_bytes: bytes
    width: int
    height: int
    padded_to_square: bool


@dataclass
class FileReport:
    rel_glb: Path
    images_total: int = 0
    targeted_images: int = 0
    attempted_requests: int = 0
    remastered: int = 0
    normalmap_attempted: int = 0
    normalmap_generated: int = 0
    normalmap_failed: int = 0
    transparent_handled: int = 0
    skipped_transparent: int = 0
    skipped_not_target: int = 0
    extraction_failed: int = 0
    api_failed: int = 0


@dataclass
class RunReport:
    files_total: int = 0
    files_ok: int = 0
    files_failed: int = 0
    images_total: int = 0
    targeted_images: int = 0
    attempted_requests: int = 0
    remastered: int = 0
    normalmap_attempted: int = 0
    normalmap_generated: int = 0
    normalmap_failed: int = 0
    transparent_handled: int = 0
    skipped_transparent: int = 0
    skipped_not_target: int = 0
    extraction_failed: int = 0
    api_failed: int = 0


def parse_args(argv: Iterable[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Remaster textures from GLB files and write new GLBs preserving relative paths.",
    )
    parser.add_argument(
        "input_paths",
        type=Path,
        nargs="+",
        help="One or more GLB files or directories containing GLB assets (directories are searched recursively).",
    )
    parser.add_argument(
        "--raw-dir",
        type=Path,
        default=Path("assets/remaster_raw"),
        help="Directory for original texture backups (default: assets/remaster_raw).",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("assets/remaster"),
        help="Directory for remastered GLBs (default: assets/remaster).",
    )
    parser.add_argument(
        "--api-key",
        default=os.getenv("NANO_BANANA_API_KEY") or os.getenv("GEMINI_API_KEY"),
        help="API key for Nano Banana/Gemini endpoint. Defaults from env NANO_BANANA_API_KEY or GEMINI_API_KEY.",
    )
    parser.add_argument(
        "--model",
        default="nano-banana-3-flash",
        help="Model name (default: nano-banana-3-flash).",
    )
    parser.add_argument(
        "--endpoint-template",
        default=DEFAULT_GEMINI_ENDPOINT,
        help=(
            "HTTP endpoint template. Supports {model}. "
            "Default is Gemini-style generateContent endpoint."
        ),
    )
    parser.add_argument(
        "--api-mode",
        choices=["gemini", "openai"],
        default="gemini",
        help="Request format for the image remaster API (default: gemini).",
    )
    parser.add_argument(
        "--prompt",
        default=DEFAULT_PROMPT,
        help="Prompt sent with each texture image.",
    )
    parser.add_argument(
        "--normal-prompt",
        default=DEFAULT_NORMAL_MAP_PROMPT,
        help="Prompt used to generate normal maps from remastered textures.",
    )
    parser.add_argument(
        "--generate-normal",
        action="store_true",
        help="Enable normal map generation and binding into materials (default: disabled).",
    )
    parser.add_argument(
        "--jpeg-quality",
        type=int,
        default=90,
        help="JPEG quality used for remastered textures (1-100, default: 90).",
    )
    parser.add_argument(
        "--max-retries",
        type=int,
        default=3,
        help="Max retry attempts for API calls on transient errors (default: 3).",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=float,
        default=120.0,
        help="API request timeout in seconds (default: 120).",
    )
    parser.add_argument(
        "--chroma-tolerance",
        type=int,
        default=70,
        help="Tolerance (0-255) for chroma-green to alpha conversion (default: 70).",
    )
    parser.add_argument(
        "--only-base-color",
        action="store_true",
        help="Restrict remastering to baseColor textures only (default: process all textures).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Inspect files and show intended actions without writing outputs or calling API.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable debug logging.",
    )
    args = parser.parse_args(list(argv))

    if args.jpeg_quality < 1 or args.jpeg_quality > 100:
        parser.error("--jpeg-quality must be between 1 and 100")
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
    logging.getLogger("PIL").setLevel(logging.INFO)
    # Avoid leaking API query params in verbose transport logs.
    logging.getLogger("httpx").setLevel(logging.WARNING)
    logging.getLogger("httpcore").setLevel(logging.WARNING)


def align4(value: int) -> int:
    return (value + 3) & ~3


def _try_relative(path: Path, base: Path) -> Optional[Path]:
    try:
        return path.relative_to(base)
    except ValueError:
        return None


def resolve_rel_glb_path(glb_path: Path, input_path: Path) -> Path:
    # Preferred: preserve hierarchy below the nearest `assets/` directory.
    # Example:
    #   /repo/assets/data4/object_25/object_03.glb -> data4/object_25/object_03.glb
    assets_indices = [idx for idx, part in enumerate(glb_path.parts) if part == "assets"]
    if assets_indices:
        idx = assets_indices[-1]
        if idx + 1 < len(glb_path.parts):
            return Path(*glb_path.parts[idx + 1 :])

    # Fallbacks for paths outside an assets tree.
    if input_path.is_dir():
        rel = _try_relative(glb_path, input_path)
        if rel is not None:
            return rel
    else:
        rel = _try_relative(glb_path, input_path.parent)
        if rel is not None:
            return rel

    return Path(glb_path.name)


def safe_texture_name(name: str) -> str:
    cleaned = SAFE_NAME_RE.sub("_", name).strip("._-")
    return cleaned or "texture"


def guess_mime_from_name(name: str) -> Optional[str]:
    suffix = Path(name).suffix.lower()
    return MIME_BY_EXT.get(suffix)


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


def load_glb_payload(path: Path) -> Tuple[Dict[str, Any], bytes]:
    data = path.read_bytes()
    if len(data) < 20:
        raise ValueError("GLB too small")

    magic, version, total_length = struct.unpack_from("<III", data, 0)
    if magic != GLTF_MAGIC:
        raise ValueError("Invalid GLB magic")
    if version != 2:
        raise ValueError(f"Unsupported GLB version: {version}")
    if total_length > len(data):
        raise ValueError("GLB is truncated")

    offset = 12
    json_chunk: Optional[bytes] = None
    bin_chunk: bytes = b""

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


def decode_data_uri(uri: str) -> Tuple[bytes, Optional[str]]:
    # Format: data:[<mime>][;base64],<data>
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
        data = base64.b64decode(payload)
    else:
        data = unquote(payload).encode("utf-8")

    return data, mime


def extract_image_source(
    image_index: int,
    image_obj: Dict[str, Any],
    payload: Dict[str, Any],
    binary_blob: bytes,
    glb_path: Path,
) -> TextureSource:
    logical_name = image_obj.get("name") or f"image_{image_index:03d}"
    mime_type = image_obj.get("mimeType")

    if isinstance(image_obj.get("bufferView"), int):
        buffer_views = payload.get("bufferViews")
        if not isinstance(buffer_views, list):
            raise ValueError("GLB has image.bufferView but no bufferViews list")

        view_index = image_obj["bufferView"]
        if view_index < 0 or view_index >= len(buffer_views):
            raise ValueError(f"image[{image_index}] references invalid bufferView {view_index}")

        view = buffer_views[view_index]
        if not isinstance(view, dict):
            raise ValueError(f"bufferView[{view_index}] is not an object")

        byte_offset = int(view.get("byteOffset", 0))
        byte_length = int(view.get("byteLength", 0))
        if byte_offset < 0 or byte_length <= 0:
            raise ValueError(f"image[{image_index}] has invalid byte range")
        end = byte_offset + byte_length
        if end > len(binary_blob):
            raise ValueError(f"image[{image_index}] bufferView exceeds BIN chunk")

        raw = binary_blob[byte_offset:end]
        return TextureSource(
            image_index=image_index,
            source_kind="bufferview",
            raw_bytes=raw,
            mime_type=mime_type,
            logical_name=str(logical_name),
        )

    uri = image_obj.get("uri")
    if not isinstance(uri, str) or not uri:
        raise ValueError(f"image[{image_index}] has neither bufferView nor uri")

    if uri.startswith("data:"):
        raw, uri_mime = decode_data_uri(uri)
        return TextureSource(
            image_index=image_index,
            source_kind="data_uri",
            raw_bytes=raw,
            mime_type=mime_type or uri_mime,
            logical_name=str(logical_name),
        )

    rel_uri = unquote(uri.replace("\\", "/"))
    texture_path = (glb_path.parent / rel_uri).resolve()
    if not texture_path.exists() or not texture_path.is_file():
        raise ValueError(f"External texture not found: {uri}")

    raw = texture_path.read_bytes()
    return TextureSource(
        image_index=image_index,
        source_kind="external_uri",
        raw_bytes=raw,
        mime_type=mime_type or guess_mime_from_name(rel_uri),
        logical_name=Path(rel_uri).name or str(logical_name),
    )


def prepare_texture(raw_bytes: bytes) -> TexturePrepared:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    try:
        with Image.open(BytesIO(raw_bytes)) as img:
            rgba = img.convert("RGBA")
            alpha = rgba.getchannel("A")
            min_alpha, _max_alpha = alpha.getextrema()
            has_transparency = min_alpha < 255

            width, height = img.size
            padded_to_square = width != height

            rgb = img.convert("RGB")
            if padded_to_square:
                side = max(width, height)
                square = Image.new("RGB", (side, side), (0, 0, 0))
                offset_x = (side - width) // 2
                offset_y = (side - height) // 2
                square.paste(rgb, (offset_x, offset_y))
                rgb = square

            out = BytesIO()
            rgb.save(out, format="PNG")
            upload_png = out.getvalue()

        return TexturePrepared(
            has_transparency=has_transparency,
            upload_png_bytes=upload_png,
            width=width,
            height=height,
            padded_to_square=padded_to_square,
        )
    except UnidentifiedImageError as exc:
        raise ValueError(f"Unsupported/unknown texture format: {exc}") from exc


def encode_jpeg(image_bytes: bytes, quality: int) -> bytes:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    with Image.open(BytesIO(image_bytes)) as img:
        rgb = img.convert("RGB")
        out = BytesIO()
        rgb.save(out, format="JPEG", quality=quality, optimize=True)
        return out.getvalue()


def encode_png(image_bytes: bytes) -> bytes:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    with Image.open(BytesIO(image_bytes)) as img:
        rgb = img.convert("RGB")
        out = BytesIO()
        rgb.save(out, format="PNG")
        return out.getvalue()


def build_texture_prompt(
    base_prompt: str,
    rel_glb: Path,
    texture: TextureSource,
    has_transparency: bool = False,
    source_width: Optional[int] = None,
    source_height: Optional[int] = None,
    padded_to_square: bool = False,
) -> str:
    glb_file = rel_glb.as_posix()
    texture_name = texture.logical_name
    prompt = (
        f"{base_prompt}\n\n"
        f"Source file: {glb_file}\n"
        f"Texture file: {texture_name}\n"
        f"Texture index: {texture.image_index}\n"
        "Rule: if input background is black RGB(0,0,0), output background must stay black RGB(0,0,0).\n"
        "Do not convert black background to white, gray, or transparent."
    )
    if has_transparency:
        prompt += (
            "\nThe source texture contains transparency. "
            "Render only transparent/background regions as chroma key green RGB(0,255,0), "
            "with no text overlay."
        )
    if padded_to_square and source_width and source_height:
        prompt += (
            f"\nThe source texture aspect ratio is {source_width}:{source_height}. "
            "Input image was padded to a square with black borders before this request. "
            "Keep those padded black borders black (RGB 0,0,0). "
            "Keep the main content centered and proportionally consistent so a center-crop "
            "back to the original ratio preserves the object."
        )
    return prompt


def crop_image_to_original_ratio(image_bytes: bytes, target_width: int, target_height: int) -> bytes:
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")
    if target_width <= 0 or target_height <= 0:
        return image_bytes

    target_ratio = target_width / target_height
    with Image.open(BytesIO(image_bytes)) as img:
        rgba = img.convert("RGBA")
        width, height = rgba.size
        if width <= 0 or height <= 0:
            return image_bytes

        current_ratio = width / height
        crop_box = (0, 0, width, height)
        if abs(current_ratio - target_ratio) > 1e-6:
            if current_ratio > target_ratio:
                new_width = max(1, int(round(height * target_ratio)))
                left = max(0, (width - new_width) // 2)
                crop_box = (left, 0, left + new_width, height)
            else:
                new_height = max(1, int(round(width / target_ratio)))
                top = max(0, (height - new_height) // 2)
                crop_box = (0, top, width, top + new_height)
            rgba = rgba.crop(crop_box)

        out = BytesIO()
        rgba.save(out, format="PNG")
        return out.getvalue()


def _is_chroma_green(r: int, g: int, b: int, tolerance: int) -> bool:
    # Base distance check around pure green key.
    dist_sq = (r * r) + ((255 - g) * (255 - g)) + (b * b)
    if dist_sq <= tolerance * tolerance:
        return True

    # Relaxed rule to handle common green spill/compression deviations.
    dominance = g - max(r, b)
    return g >= 180 and dominance >= max(24, tolerance // 2) and r <= tolerance + 20 and b <= tolerance + 20


def restore_transparency_from_chroma_green(image_bytes: bytes, tolerance: int) -> bytes:
    """Treat chroma-green pixels as fully transparent and return PNG RGBA bytes."""
    if Image is None:
        raise RuntimeError("Pillow is required. Install with `pip install Pillow`.")

    with Image.open(BytesIO(image_bytes)) as img:
        rgb = img.convert("RGB")
        r_band, g_band, b_band = rgb.split()
        alpha_bytes = bytes(
            0 if _is_chroma_green(r, g, b, tolerance) else 255
            for r, g, b in zip(r_band.tobytes(), g_band.tobytes(), b_band.tobytes())
        )
        alpha = Image.frombytes("L", rgb.size, alpha_bytes)
        rgba = rgb.copy()
        rgba.putalpha(alpha)
        out = BytesIO()
        rgba.save(out, format="PNG")
        return out.getvalue()


def build_normal_map_prompt(base_prompt: str, rel_glb: Path, texture: TextureSource) -> str:
    glb_file = rel_glb.as_posix()
    texture_name = texture.logical_name
    return (
        f"{base_prompt}\n\n"
        f"Source file: {glb_file}\n"
        f"Texture file: {texture_name}\n"
        f"Texture index: {texture.image_index}\n"
        "Return only a tangent-space normal map image."
    )


def collect_materials_by_base_color_image(payload: Dict[str, Any]) -> Dict[int, List[int]]:
    textures = payload.get("textures")
    materials = payload.get("materials")
    images = payload.get("images")

    if not isinstance(textures, list) or not isinstance(materials, list) or not isinstance(images, list):
        return {}

    by_image: Dict[int, List[int]] = {}
    for material_index, material in enumerate(materials):
        if not isinstance(material, dict):
            continue
        pbr = material.get("pbrMetallicRoughness")
        if not isinstance(pbr, dict):
            continue
        base_color = pbr.get("baseColorTexture")
        if not isinstance(base_color, dict):
            continue
        texture_index = base_color.get("index")
        if not isinstance(texture_index, int):
            continue
        if texture_index < 0 or texture_index >= len(textures):
            continue

        texture_obj = textures[texture_index]
        if not isinstance(texture_obj, dict):
            continue
        image_index = texture_obj.get("source")
        if not isinstance(image_index, int):
            continue
        if image_index < 0 or image_index >= len(images):
            continue
        by_image.setdefault(image_index, []).append(material_index)

    return by_image


def extract_image_bytes_from_response(payload: Dict[str, Any]) -> Tuple[bytes, Optional[str]]:
    # Gemini-style response: candidates[*].content.parts[*].inlineData{mimeType,data}
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
                    mime = inline_data.get("mimeType") or inline_data.get("mime_type")
                    if isinstance(data_b64, str) and data_b64:
                        return base64.b64decode(data_b64), mime

    # OpenAI-style image payload (best-effort)
    data = payload.get("data")
    if isinstance(data, list):
        for item in data:
            if isinstance(item, dict):
                b64 = item.get("b64_json") or item.get("image_base64")
                if isinstance(b64, str) and b64:
                    return base64.b64decode(b64), "image/png"

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
                image_b64 = block.get("image_base64") or block.get("b64_json")
                if isinstance(image_b64, str) and image_b64:
                    return base64.b64decode(image_b64), "image/png"

    top_level = payload.get("image_base64")
    if isinstance(top_level, str) and top_level:
        return base64.b64decode(top_level), "image/png"

    raise ValueError("API response did not include any image payload")


def call_remaster_api(
    client: Any,
    api_mode: str,
    endpoint_template: str,
    model: str,
    api_key: str,
    prompt: str,
    png_bytes: bytes,
    timeout_seconds: float,
    max_retries: int,
) -> Tuple[bytes, Optional[str]]:
    endpoint = endpoint_template.format(model=model)
    headers = {"Content-Type": "application/json"}
    params: Dict[str, str] = {}

    if api_mode == "gemini":
        # Gemini-style authentication via header (avoid key in URL logs).
        headers["x-goog-api-key"] = api_key
        payload = {
            "contents": [
                {
                    "parts": [
                        {"text": prompt},
                        {
                            "inlineData": {
                                "mimeType": "image/png",
                                "data": base64.b64encode(png_bytes).decode("ascii"),
                            }
                        },
                    ]
                }
            ],
            "generationConfig": {
                "responseModalities": ["IMAGE"],
            },
        }
    elif api_mode == "openai":
        headers["Authorization"] = f"Bearer {api_key}"
        payload = {
            "model": model,
            "input": [
                {
                    "role": "user",
                    "content": [
                        {"type": "input_text", "text": prompt},
                        {
                            "type": "input_image",
                            "image_url": f"data:image/png;base64,{base64.b64encode(png_bytes).decode('ascii')}",
                        },
                    ],
                }
            ],
            "modalities": ["image"],
        }
    else:
        raise ValueError(f"Unsupported api_mode: {api_mode}")

    # Ensure per-call timeout can override client defaults.
    timeout = httpx.Timeout(timeout_seconds) if httpx is not None else None

    for attempt in range(max_retries + 1):
        try:

            response = client.post(
                endpoint,
                params=params,
                headers=headers,
                json=payload,
                timeout=timeout,
            )

            if response.status_code in {429, 500, 502, 503, 504}:
                if attempt < max_retries:
                    backoff = (2**attempt) + random.uniform(0.0, 0.75)
                    logging.warning(
                        "API transient error %s, retrying in %.2fs (attempt %d/%d)",
                        response.status_code,
                        backoff,
                        attempt + 1,
                        max_retries,
                    )
                    time.sleep(backoff)
                    continue

            response.raise_for_status()

            content_type = response.headers.get("content-type", "").lower()
            if content_type.startswith("image/"):
                mime = content_type.split(";", 1)[0].strip()
                return response.content, mime

            body = response.json()
            image_bytes, mime = extract_image_bytes_from_response(body)
            return image_bytes, mime
        except httpx.HTTPStatusError as exc:
            status = exc.response.status_code
            transient = status in {429, 500, 502, 503, 504}
            if transient and attempt < max_retries:
                backoff = (2**attempt) + random.uniform(0.0, 0.75)
                logging.warning(
                    "API transient HTTP %s, retrying in %.2fs (attempt %d/%d)",
                    status,
                    backoff,
                    attempt + 1,
                    max_retries,
                )
                time.sleep(backoff)
                continue

            detail = ""
            try:
                text = exc.response.text.strip()
                if text:
                    detail = f" | {text[:240]}"
            except Exception:  # noqa: BLE001
                detail = ""
            raise RuntimeError(f"API returned HTTP {status}{detail}") from exc
        except (httpx.TimeoutException, httpx.NetworkError) as exc:
            if attempt < max_retries:
                backoff = (2**attempt) + random.uniform(0.0, 0.75)
                logging.warning(
                    "API network/timeout error (%s), retrying in %.2fs (attempt %d/%d)",
                    exc,
                    backoff,
                    attempt + 1,
                    max_retries,
                )
                time.sleep(backoff)
                continue
            raise

    raise RuntimeError("Unexpected retry loop exit")


def collect_target_image_indices(payload: Dict[str, Any], only_base_color: bool) -> Set[int]:
    images = payload.get("images")
    if not isinstance(images, list) or not images:
        return set()

    if not only_base_color:
        return set(range(len(images)))

    textures = payload.get("textures")
    materials = payload.get("materials")
    if not isinstance(textures, list) or not isinstance(materials, list):
        return set(range(len(images)))

    texture_indices: Set[int] = set()
    for material in materials:
        if not isinstance(material, dict):
            continue
        pbr = material.get("pbrMetallicRoughness")
        if not isinstance(pbr, dict):
            continue
        base_color = pbr.get("baseColorTexture")
        if not isinstance(base_color, dict):
            continue
        texture_index = base_color.get("index")
        if not isinstance(texture_index, int):
            continue
        if texture_index < 0 or texture_index >= len(textures):
            continue
        texture = textures[texture_index]
        if not isinstance(texture, dict):
            continue
        image_index = texture.get("source")
        if isinstance(image_index, int) and 0 <= image_index < len(images):
            texture_indices.add(image_index)

    # If we cannot resolve any baseColor images, fallback to all textures.
    if not texture_indices:
        return set(range(len(images)))
    return texture_indices


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


def raw_texture_stem(texture: TextureSource, suffix: str = "") -> str:
    base_name = safe_texture_name(Path(texture.logical_name).stem)
    suffix_part = f"_{suffix}" if suffix else ""
    return f"{texture.image_index:03d}_{base_name}{suffix_part}"


def raw_texture_dir(raw_root: Path, rel_glb: Path) -> Path:
    return raw_root / rel_glb.with_suffix("")


def find_existing_raw_texture(
    raw_root: Path,
    rel_glb: Path,
    texture: TextureSource,
    suffix: str = "",
) -> Optional[Path]:
    target_dir = raw_texture_dir(raw_root, rel_glb)
    if not target_dir.exists() or not target_dir.is_dir():
        return None

    stem = raw_texture_stem(texture, suffix)
    matches = sorted(target_dir.glob(f"{stem}.*"))
    if not matches:
        return None

    ext_priority = {".png": 0, ".jpg": 1, ".jpeg": 2, ".webp": 3, ".bmp": 4, ".gif": 5, ".tga": 6, ".bin": 7}
    matches.sort(key=lambda path: (ext_priority.get(path.suffix.lower(), 99), path.name.lower()))
    return matches[0]


def load_existing_raw_texture(
    raw_root: Path,
    rel_glb: Path,
    texture: TextureSource,
    suffix: str = "",
) -> Optional[Tuple[bytes, Optional[str], Path]]:
    existing = find_existing_raw_texture(raw_root, rel_glb, texture, suffix=suffix)
    if existing is None:
        return None
    try:
        payload = existing.read_bytes()
    except OSError:
        return None
    mime = guess_mime_from_name(existing.name) or detect_mime_from_image_bytes(payload)
    return payload, mime, existing


def save_raw_texture(
    raw_root: Path,
    rel_glb: Path,
    texture: TextureSource,
    *,
    suffix: str = "",
    bytes_override: Optional[bytes] = None,
    mime_override: Optional[str] = None,
) -> Path:
    target_dir = raw_texture_dir(raw_root, rel_glb)
    target_dir.mkdir(parents=True, exist_ok=True)

    payload = texture.raw_bytes if bytes_override is None else bytes_override
    mime = mime_override or texture.mime_type or detect_mime_from_image_bytes(payload)
    ext = extension_from_mime(mime)
    stem = raw_texture_stem(texture, suffix=suffix)
    filename = f"{stem}{ext}"
    out_path = target_dir / filename
    out_path.write_bytes(payload)
    return out_path


def ensure_original_model_output_saved(
    raw_root: Path,
    rel_glb: Path,
    texture: TextureSource,
    model_bytes: bytes,
    model_mime: Optional[str],
) -> Optional[Path]:
    existing = find_existing_raw_texture(raw_root, rel_glb, texture, suffix="model_output_original")
    if existing is not None:
        return existing
    return save_raw_texture(
        raw_root,
        rel_glb,
        texture,
        suffix="model_output_original",
        bytes_override=model_bytes,
        mime_override=model_mime,
    )


def process_glb(
    glb_path: Path,
    rel_glb: Path,
    raw_dir: Path,
    out_dir: Path,
    args: argparse.Namespace,
    client: Optional[Any],
) -> FileReport:
    report = FileReport(rel_glb=rel_glb)

    payload, binary_blob = load_glb_payload(glb_path)
    images = payload.get("images")

    out_path = out_dir / rel_glb
    out_path.parent.mkdir(parents=True, exist_ok=True)

    if not isinstance(images, list) or not images:
        logging.info("%s: no images found, copying GLB", rel_glb.as_posix())
        if not args.dry_run:
            out_path.write_bytes(glb_path.read_bytes())
        return report

    raw_target_dir = raw_texture_dir(raw_dir, rel_glb)
    if not args.dry_run:
        raw_target_dir.mkdir(parents=True, exist_ok=True)

    report.images_total = len(images)
    target_indices = collect_target_image_indices(payload, args.only_base_color)
    material_map_by_image = collect_materials_by_base_color_image(payload)

    buffer_views = ensure_buffer_structures(payload, len(binary_blob))
    new_binary_blob = bytearray(binary_blob)

    replacements: Dict[int, Tuple[bytes, str]] = {}
    normal_replacements: Dict[int, Tuple[bytes, str, str]] = {}
    sources: Dict[int, TextureSource] = {}

    for image_index, image_obj in enumerate(images):
        if not isinstance(image_obj, dict):
            report.extraction_failed += 1
            logging.warning("%s: image[%d] is not an object", rel_glb.as_posix(), image_index)
            continue

        try:
            texture_source = extract_image_source(
                image_index=image_index,
                image_obj=image_obj,
                payload=payload,
                binary_blob=binary_blob,
                glb_path=glb_path,
            )
            sources[image_index] = texture_source
        except Exception as exc:  # noqa: BLE001
            report.extraction_failed += 1
            logging.warning(
                "%s: failed to extract image[%d]: %s",
                rel_glb.as_posix(),
                image_index,
                exc,
            )
            continue

    for image_index in sorted(sources):
        source = sources[image_index]

        if image_index not in target_indices:
            report.skipped_not_target += 1
            continue

        report.targeted_images += 1

        try:
            prepared = prepare_texture(source.raw_bytes)
        except Exception as exc:  # noqa: BLE001
            report.extraction_failed += 1
            logging.warning(
                "%s: image[%d] cannot be prepared for remaster: %s",
                rel_glb.as_posix(),
                image_index,
                exc,
            )
            continue

        if args.dry_run:
            if prepared.has_transparency:
                logging.info(
                    "[DRY-RUN] %s: image[%d] would remaster (%dx%d)%s with chroma-green prompt and green->alpha restore",
                    rel_glb.as_posix(),
                    image_index,
                    prepared.width,
                    prepared.height,
                    " [padded-to-square]" if prepared.padded_to_square else "",
                )
            else:
                logging.info(
                    "[DRY-RUN] %s: image[%d] would remaster (%dx%d)%s",
                    rel_glb.as_posix(),
                    image_index,
                    prepared.width,
                    prepared.height,
                    " [padded-to-square]" if prepared.padded_to_square else "",
                )
            continue

        if find_existing_raw_texture(raw_dir, rel_glb, source, suffix="") is None:
            save_raw_texture(raw_dir, rel_glb, source)

        if find_existing_raw_texture(raw_dir, rel_glb, source, suffix="model_input") is None:
            save_raw_texture(
                raw_dir,
                rel_glb,
                source,
                suffix="model_input",
                bytes_override=prepared.upload_png_bytes,
                mime_override="image/png",
            )

        cached_model_raw = load_existing_raw_texture(raw_dir, rel_glb, source, suffix="model_raw")
        if cached_model_raw is not None:
            remastered_bytes, remastered_mime, remastered_path = cached_model_raw
            _ = ensure_original_model_output_saved(
                raw_root=raw_dir,
                rel_glb=rel_glb,
                texture=source,
                model_bytes=remastered_bytes,
                model_mime=remastered_mime,
            )
            logging.info(
                "%s: image[%d] reusing existing remaster %s",
                rel_glb.as_posix(),
                image_index,
                remastered_path.name,
            )
        else:
            report.attempted_requests += 1
            try:
                prompt = build_texture_prompt(
                    args.prompt,
                    rel_glb,
                    source,
                    has_transparency=prepared.has_transparency,
                    source_width=prepared.width,
                    source_height=prepared.height,
                    padded_to_square=prepared.padded_to_square,
                )
                remastered_bytes, remastered_mime = call_remaster_api(
                    client=client,
                    api_mode=args.api_mode,
                    endpoint_template=args.endpoint_template,
                    model=args.model,
                    api_key=args.api_key,
                    prompt=prompt,
                    png_bytes=prepared.upload_png_bytes,
                    timeout_seconds=args.timeout_seconds,
                    max_retries=args.max_retries,
                )
                save_raw_texture(
                    raw_dir,
                    rel_glb,
                    source,
                    suffix="model_raw",
                    bytes_override=remastered_bytes,
                    mime_override=remastered_mime,
                )
                _ = ensure_original_model_output_saved(
                    raw_root=raw_dir,
                    rel_glb=rel_glb,
                    texture=source,
                    model_bytes=remastered_bytes,
                    model_mime=remastered_mime,
                )
            except Exception as exc:  # noqa: BLE001
                report.api_failed += 1
                logging.warning(
                    "%s: image[%d] remaster failed: %s",
                    rel_glb.as_posix(),
                    image_index,
                    exc,
                )
                continue

        processed_texture_bytes = remastered_bytes
        if prepared.padded_to_square:
            cached_model_crop = load_existing_raw_texture(raw_dir, rel_glb, source, suffix="model_crop")
            if cached_model_crop is not None:
                processed_texture_bytes, _crop_mime, crop_path = cached_model_crop
                logging.info(
                    "%s: image[%d] reusing existing cropped texture %s",
                    rel_glb.as_posix(),
                    image_index,
                    crop_path.name,
                )
            else:
                processed_texture_bytes = crop_image_to_original_ratio(
                    remastered_bytes,
                    target_width=prepared.width,
                    target_height=prepared.height,
                )
                save_raw_texture(
                    raw_dir,
                    rel_glb,
                    source,
                    suffix="model_crop",
                    bytes_override=processed_texture_bytes,
                    mime_override="image/png",
                )

        final_texture_bytes: bytes
        final_texture_mime: str
        if prepared.has_transparency:
            cached_model_alpha = load_existing_raw_texture(raw_dir, rel_glb, source, suffix="model_alpha_chroma")
            if cached_model_alpha is not None:
                final_texture_bytes, cached_alpha_mime, alpha_path = cached_model_alpha
                final_texture_mime = cached_alpha_mime or "image/png"
                logging.info(
                    "%s: image[%d] reusing existing alpha texture %s",
                    rel_glb.as_posix(),
                    image_index,
                    alpha_path.name,
                )
            else:
                final_texture_bytes = restore_transparency_from_chroma_green(
                    processed_texture_bytes,
                    tolerance=args.chroma_tolerance,
                )
                final_texture_mime = "image/png"
                save_raw_texture(
                    raw_dir,
                    rel_glb,
                    source,
                    suffix="model_alpha_chroma",
                    bytes_override=final_texture_bytes,
                    mime_override=final_texture_mime,
                )
            report.transparent_handled += 1
        else:
            final_texture_bytes = encode_jpeg(processed_texture_bytes, quality=args.jpeg_quality)
            final_texture_mime = "image/jpeg"

        replacements[image_index] = (final_texture_bytes, final_texture_mime)
        report.remastered += 1

        if args.generate_normal:
            normal_png_input = encode_png(final_texture_bytes)
            if find_existing_raw_texture(raw_dir, rel_glb, source, suffix="normal_input") is None:
                save_raw_texture(
                    raw_dir,
                    rel_glb,
                    source,
                    suffix="normal_input",
                    bytes_override=normal_png_input,
                    mime_override="image/png",
                )

            cached_normal_raw = load_existing_raw_texture(raw_dir, rel_glb, source, suffix="normal_raw")
            if cached_normal_raw is not None:
                normal_bytes, normal_mime, normal_path = cached_normal_raw
                logging.info(
                    "%s: image[%d] reusing existing normal map %s",
                    rel_glb.as_posix(),
                    image_index,
                    normal_path.name,
                )
            else:
                report.normalmap_attempted += 1
                try:
                    normal_prompt = build_normal_map_prompt(args.normal_prompt, rel_glb, source)
                    normal_bytes, normal_mime = call_remaster_api(
                        client=client,
                        api_mode=args.api_mode,
                        endpoint_template=args.endpoint_template,
                        model=args.model,
                        api_key=args.api_key,
                        prompt=normal_prompt,
                        png_bytes=normal_png_input,
                        timeout_seconds=args.timeout_seconds,
                        max_retries=args.max_retries,
                    )
                    save_raw_texture(
                        raw_dir,
                        rel_glb,
                        source,
                        suffix="normal_raw",
                        bytes_override=normal_bytes,
                        mime_override=normal_mime,
                    )
                except Exception as exc:  # noqa: BLE001
                    report.normalmap_failed += 1
                    report.api_failed += 1
                    logging.warning(
                        "%s: image[%d] normal map generation failed: %s",
                        rel_glb.as_posix(),
                        image_index,
                        exc,
                    )
                    continue

            normal_jpeg = encode_jpeg(normal_bytes, quality=args.jpeg_quality)
            normal_name = f"{Path(source.logical_name).stem}_normal"
            normal_replacements[image_index] = (normal_jpeg, "image/jpeg", normal_name)
            report.normalmap_generated += 1

    # Rebind changed images and embed any URI-based images so output GLB is self-contained.
    changed_any = False
    for image_index, source in sources.items():
        image_obj = images[image_index]

        replacement = replacements.get(image_index)
        should_embed_uri = source.source_kind in {"external_uri", "data_uri"}

        if replacement is None and not should_embed_uri:
            continue

        if replacement is not None:
            data_bytes, mime_type = replacement
        else:
            data_bytes = source.raw_bytes
            mime_type = source.mime_type or guess_mime_from_name(source.logical_name)

        new_view_index = append_buffer_view(buffer_views, new_binary_blob, data_bytes)
        image_obj["bufferView"] = new_view_index
        image_obj.pop("uri", None)
        if mime_type:
            image_obj["mimeType"] = mime_type

        changed_any = True

    textures = payload.get("textures")
    if not isinstance(textures, list):
        payload["textures"] = []
        textures = payload["textures"]

    materials = payload.get("materials")
    if not isinstance(materials, list):
        payload["materials"] = []
        materials = payload["materials"]

    for image_index, (normal_bytes, normal_mime, normal_name) in normal_replacements.items():
        material_indices = material_map_by_image.get(image_index)
        if not material_indices:
            logging.debug(
                "%s: no baseColor material uses image[%d], skipping normal map binding",
                rel_glb.as_posix(),
                image_index,
            )
            continue

        normal_view_index = append_buffer_view(buffer_views, new_binary_blob, normal_bytes)
        new_image_index = len(images)
        images.append(
            {
                "name": normal_name,
                "bufferView": normal_view_index,
                "mimeType": normal_mime,
            }
        )

        new_texture_index = len(textures)
        textures.append({"source": new_image_index})

        for material_index in material_indices:
            material = materials[material_index]
            if not isinstance(material, dict):
                continue
            material["normalTexture"] = {"index": new_texture_index}

        changed_any = True

    if args.dry_run:
        return report

    if changed_any:
        buffers = payload["buffers"]
        first = buffers[0]
        first["byteLength"] = len(new_binary_blob)

        out_data = build_glb(payload, bytes(new_binary_blob))
        out_path.write_bytes(out_data)
    else:
        # Nothing changed in payload/binary: still mirror to remaster output tree.
        out_path.write_bytes(glb_path.read_bytes())

    return report


def aggregate(global_report: RunReport, file_report: FileReport, success: bool) -> None:
    global_report.files_total += 1
    if success:
        global_report.files_ok += 1
    else:
        global_report.files_failed += 1

    global_report.images_total += file_report.images_total
    global_report.targeted_images += file_report.targeted_images
    global_report.attempted_requests += file_report.attempted_requests
    global_report.remastered += file_report.remastered
    global_report.normalmap_attempted += file_report.normalmap_attempted
    global_report.normalmap_generated += file_report.normalmap_generated
    global_report.normalmap_failed += file_report.normalmap_failed
    global_report.transparent_handled += file_report.transparent_handled
    global_report.skipped_transparent += file_report.skipped_transparent
    global_report.skipped_not_target += file_report.skipped_not_target
    global_report.extraction_failed += file_report.extraction_failed
    global_report.api_failed += file_report.api_failed


def main(argv: Iterable[str]) -> int:
    args = parse_args(argv)
    configure_logging(args.verbose)

    if args.api_mode == "openai" and args.endpoint_template == DEFAULT_GEMINI_ENDPOINT:
        args.endpoint_template = DEFAULT_OPENAI_ENDPOINT

    if Image is None:
        logging.error("Missing dependency: Pillow. Install with `pip install Pillow`.")
        return 2

    if httpx is None and not args.dry_run:
        logging.error("Missing dependency: httpx. Install with `pip install httpx`.")
        return 2

    input_paths = [path.resolve() for path in args.input_paths]
    raw_dir = args.raw_dir.resolve()
    out_dir = args.out_dir.resolve()
    glb_entries: List[Tuple[Path, Path]] = []
    seen_glb_paths: Set[Path] = set()

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
                glb_entries.append((input_path, resolve_rel_glb_path(input_path, input_path)))
            continue

        if input_path.is_dir():
            glb_files = sorted(input_path.rglob("*.glb"), key=lambda p: p.as_posix().lower())
            for glb_path in glb_files:
                if glb_path in seen_glb_paths:
                    continue
                seen_glb_paths.add(glb_path)
                glb_entries.append((glb_path, resolve_rel_glb_path(glb_path, input_path)))
            continue

        logging.error("Input path must be a file or directory: %s", input_path)
        return 2

    if not glb_entries:
        logging.warning("No .glb files found under provided input paths")
        return 0

    if not args.dry_run and not args.api_key:
        logging.error(
            "API key is required. Use --api-key or env NANO_BANANA_API_KEY/GEMINI_API_KEY.",
        )
        return 2

    if not args.dry_run:
        raw_dir.mkdir(parents=True, exist_ok=True)
        out_dir.mkdir(parents=True, exist_ok=True)

    if len(input_paths) == 1 and input_paths[0].is_file():
        logging.info("Input file: %s", input_paths[0])
    elif len(input_paths) == 1 and input_paths[0].is_dir():
        logging.info("Input dir: %s", input_paths[0])
    else:
        logging.info("Input paths: %d", len(input_paths))
        for input_path in input_paths:
            logging.info(" - %s", input_path)
    logging.info("Raw texture backup dir: %s", raw_dir)
    logging.info("Output GLB dir: %s", out_dir)
    logging.info("Found %d GLB file(s)", len(glb_entries))
    if args.dry_run:
        logging.info("Running in dry-run mode (no writes, no API calls)")

    run_report = RunReport()

    if args.dry_run:
        client = None
        for glb_path, rel in glb_entries:
            logging.info("Processing %s", rel.as_posix())
            try:
                file_report = process_glb(
                    glb_path=glb_path,
                    rel_glb=rel,
                    raw_dir=raw_dir,
                    out_dir=out_dir,
                    args=args,
                    client=client,
                )
                aggregate(run_report, file_report, success=True)
            except Exception as exc:  # noqa: BLE001
                logging.exception("Failed processing %s: %s", rel.as_posix(), exc)
                aggregate(run_report, FileReport(rel_glb=rel), success=False)
    else:
        with httpx.Client() as client:
            for glb_path, rel in glb_entries:
                logging.info("Processing %s", rel.as_posix())
                try:
                    file_report = process_glb(
                        glb_path=glb_path,
                        rel_glb=rel,
                        raw_dir=raw_dir,
                        out_dir=out_dir,
                        args=args,
                        client=client,
                    )
                    aggregate(run_report, file_report, success=True)
                except Exception as exc:  # noqa: BLE001
                    logging.exception("Failed processing %s: %s", rel.as_posix(), exc)
                    aggregate(run_report, FileReport(rel_glb=rel), success=False)

    logging.info("---- Remaster Summary ----")
    logging.info("Files: %d total | %d ok | %d failed", run_report.files_total, run_report.files_ok, run_report.files_failed)
    logging.info("Images discovered: %d", run_report.images_total)
    logging.info("Images targeted: %d", run_report.targeted_images)
    logging.info("Requests attempted: %d", run_report.attempted_requests)
    logging.info("Images remastered: %d", run_report.remastered)
    logging.info(
        "Normal maps: %d attempted | %d generated | %d failed",
        run_report.normalmap_attempted,
        run_report.normalmap_generated,
        run_report.normalmap_failed,
    )
    logging.info("Transparent textures handled (chroma-green->alpha): %d", run_report.transparent_handled)
    logging.info("Skipped (transparent): %d", run_report.skipped_transparent)
    logging.info("Skipped (not target set): %d", run_report.skipped_not_target)
    logging.info("Extraction/prep failures: %d", run_report.extraction_failed)
    logging.info("API failures: %d", run_report.api_failed)

    return 1 if run_report.files_failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
