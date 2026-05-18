#!/usr/bin/env python3
"""PoC: Remaster game asset images via OpenAI or Gemini image APIs.

Pre/post-processing pipeline:
  1. If image has alpha channel -> extract alpha mask, flatten onto solid bg
  2. Send flattened image to API for remaster
  3. Restore original alpha mask onto the result (pixel-perfect shape)

Usage:
    python scripts/remaster_image_test.py [--provider openai|gemini] [--filter NAME]

Examples:
    python scripts/remaster_image_test.py                          # all tests, openai
    python scripts/remaster_image_test.py --provider gemini        # all tests, gemini
    python scripts/remaster_image_test.py --provider gemini --filter black-bg
"""

import argparse
import base64
import os
import sys
from datetime import datetime
from io import BytesIO

import numpy as np
from PIL import Image, ImageOps

SCRIPT_DIR = os.path.dirname(__file__)
INPUT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "assets", "prototype"))
OUTPUT_BASE = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "images-generated"))
MAX_DIM = 1024

# Pricing per 1M tokens (USD)
PRICING = {
    "gpt-image-1.5": {"input": 2.00, "output": 8.00},
    "gpt-image-1": {"input": 2.00, "output": 8.00},
    "gpt-image-1-mini": {"input": 1.00, "output": 4.00},
    "gemini-3-pro-image-preview": {"input": 2.00, "output": 12.00},
    "gemini-2.5-flash-image": {"input": 0.30, "output": 2.50},
}

PROVIDERS = {
    "openai": {
        "env_key": "OPENAI_API_KEY",
        "model": "gpt-image-1.5",
    },
    "gemini": {
        "env_key": "GEMINI_API_KEY",
        "model": "gemini-3-pro-image-preview",
    },
}

TESTS = [
    {
        "name": "image-test",
        "input": "image-test.png",
        "background": "transparent",
        "flatten_bg": (0, 0, 0),
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "This is a game texture of a wing on a black background. "
            "Create a high-resolution version of this exact same wing. "
            "Keep the exact same shape, position, proportions, feather arrangement and color palette. "
            "The wing is white/light gray — keep it white/light gray. "
            "Make each feather sharp and well-defined with clean, firm edges. "
            "Smooth gradients between feathers, no noise or artifacts. "
            "Black background, no other elements."
        ),
    },
    {
        "name": "imagem-test-black-bg",
        "input": "imagem-test-black-bg.png",
        "background": "auto",
        "flatten_bg": None,
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "This is a game VFX texture of a glowing energy effect on a black background. "
            "Create a high-resolution version of this exact same effect. "
            "Keep the exact same shape, position, flow direction and color palette. "
            "The effect is cyan/teal — keep it cyan/teal. "
            "Make each energy strand sharp and well-defined with clean, firm lines. "
            "Smooth glow falloff, no noise or artifacts. "
            "Pure black background RGB(0,0,0), no other elements."
        ),
    },
    {
        "name": "image-test-masked",
        "input": "image-test.png",
        "background": "transparent",
        "flatten_bg": None,  # no flatten — use mask mode instead
        "use_mask": True,    # send alpha as mask to OpenAI
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "Re-imagine the content inside the masked area as a high-resolution detailed version of the same subject. "
            "Fill ONLY the masked region — generate content strictly within the shape boundary. "
            "Keep the same color palette, sharp and well-defined details with clean firm edges. "
            "Smooth gradients, no noise or artifacts. "
            "Keep transparent background outside the mask."
        ),
    },
    {
        "name": "image-test-3",
        "input": "image-test-3.png",
        "background": "transparent",
        "flatten_bg": (0, 0, 0),
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "Create a high-resolution version of this exact same game asset. "
            "Keep the exact same shape, position, proportions and color palette. "
            "Sharp and well-defined details with clean, firm edges. "
            "Smooth gradients, no noise or artifacts. "
            "Black background, no other elements."
        ),
    },
    {
        "name": "image-test-4",
        "input": "image-test-4.png",
        "background": "transparent",
        "flatten_bg": (0, 0, 0),
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "Create a high-resolution version of this exact same game asset. "
            "Keep the exact same shape, position, proportions and color palette. "
            "Sharp and well-defined details with clean, firm edges. "
            "Smooth gradients, no noise or artifacts. "
            "Black background, no other elements."
        ),
    },
    {
        "name": "image-test-5",
        "input": "image-test-5.png",
        "background": "auto",
        "flatten_bg": None,
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "Create a high-resolution version of this exact same game asset. "
            "Keep the exact same shape, position, proportions and color palette. "
            "Sharp and well-defined details with clean, firm edges. "
            "Smooth gradients, no noise or artifacts."
        ),
    },
    {
        "name": "image-test-6",
        "input": "image-test-6.png",
        "background": "transparent",
        "flatten_bg": (0, 0, 0),
        "prompt": (
            "Polished 3D CG render style — smooth surfaces, studio lighting, no brush strokes or outlines. "
            "Create a high-resolution version of this exact same game asset. "
            "Keep the exact same shape, position, proportions and color palette. "
            "Sharp and well-defined details with clean, firm edges. "
            "Smooth gradients, no noise or artifacts. "
            "Black background, no other elements."
        ),
    },
]


def log_usage(model, input_tokens, output_tokens):
    pricing = PRICING.get(model)
    if not pricing:
        print(f"  Tokens:   input={input_tokens}, output={output_tokens}")
        print(f"  Cost:     (pricing unknown for {model})")
        return

    input_cost = (input_tokens / 1_000_000) * pricing["input"]
    output_cost = (output_tokens / 1_000_000) * pricing["output"]
    total_cost = input_cost + output_cost

    print(f"  Tokens:   input={input_tokens:,}, output={output_tokens:,}, total={input_tokens + output_tokens:,}")
    print(f"  Cost:     input=${input_cost:.6f}, output=${output_cost:.6f}, total=${total_cost:.6f}")


# ---------------------------------------------------------------------------
# Alpha pre/post processing
# ---------------------------------------------------------------------------
def flatten_image(img, bg_color):
    """Composite RGBA image onto solid bg_color, return (flattened_RGB, alpha_mask)."""
    img = img.convert("RGBA")
    alpha = img.split()[3]  # extract alpha channel

    bg = Image.new("RGBA", img.size, bg_color + (255,))
    flattened = Image.alpha_composite(bg, img).convert("RGB")

    print(f"  Alpha:    Flattened onto bg={bg_color}, mask extracted ({img.size[0]}x{img.size[1]})")
    return flattened, alpha


def restore_alpha(result_img, alpha_mask):
    """Apply original alpha mask and crop to content bounding box."""
    target_size = result_img.size

    # Upscale alpha mask to match the result resolution
    if alpha_mask.size != target_size:
        alpha_mask = alpha_mask.resize(target_size, Image.LANCZOS)

    # Apply alpha
    result_rgba = result_img.convert("RGBA")
    r, g, b, _ = result_rgba.split()
    restored = Image.merge("RGBA", (r, g, b, alpha_mask))

    # Crop to bounding box of non-transparent pixels
    bbox = alpha_mask.getbbox()
    if bbox:
        restored = restored.crop(bbox)
        print(f"  Alpha:    Restored mask ({target_size[0]}x{target_size[1]}) -> cropped to {restored.size[0]}x{restored.size[1]} (bbox={bbox})")
    else:
        print(f"  Alpha:    Restored mask ({target_size[0]}x{target_size[1]}), no crop (empty alpha)")

    return restored


def create_edit_mask(img):
    """Create OpenAI edit mask from RGBA image.

    OpenAI mask convention: transparent pixels = areas to edit/generate.
    So we invert the alpha: content area becomes transparent (editable),
    background stays opaque (untouched).
    """
    img = img.convert("RGBA")
    r, g, b, a = img.split()

    # Invert alpha: content (opaque) -> transparent (editable)
    inverted_a = ImageOps.invert(a)

    # Mask is fully black with inverted alpha
    mask = Image.merge("RGBA", (
        Image.new("L", img.size, 0),
        Image.new("L", img.size, 0),
        Image.new("L", img.size, 0),
        inverted_a,
    ))
    return mask


def image_to_bytes(img, fmt="PNG"):
    """Convert PIL Image to bytes for API upload."""
    buf = BytesIO()
    img.save(buf, format=fmt)
    buf.seek(0)
    buf.name = f"image.{fmt.lower()}"
    return buf


# ---------------------------------------------------------------------------
# Non-square pad/unpad helpers
# ---------------------------------------------------------------------------
def pad_to_square(img, fill_color=None):
    """Pad a non-square image to square.

    Positioning: wider than tall -> image at bottom; taller than wide -> image at left.
    Returns (padded_img, crop_box) where crop_box can reverse the padding.
    """
    w, h = img.size
    if w == h:
        return img, None

    side = max(w, h)
    if fill_color is None:
        fill_color = (0, 0, 0, 0)
    elif len(fill_color) == 3:
        fill_color = fill_color + (255,) if img.mode == "RGBA" else fill_color

    padded = Image.new(img.mode, (side, side), fill_color)

    if w > h:
        # Wider than tall: place at bottom
        offset_x = 0
        offset_y = side - h
    else:
        # Taller than wide: place at left
        offset_x = 0
        offset_y = 0
    padded.paste(img, (offset_x, offset_y))

    # crop_box maps back to original content region (in padded coordinates)
    crop_box = (offset_x, offset_y, offset_x + w, offset_y + h)
    print(f"  Pad:      {w}x{h} -> {side}x{side} (offset=({offset_x},{offset_y}), fill={fill_color})")
    return padded, crop_box


def unpad_from_square(img, crop_box, target_size):
    """Crop a square image back to original aspect ratio, then resize to target.

    crop_box is in the coordinate space of the original padded square.
    We scale it proportionally to the current (API result) image size.
    """
    if crop_box is None:
        return img

    sq = img.size[0]  # current square size (e.g. 1024)
    ox, oy, ox2, oy2 = crop_box

    # Recover the original padded square side from the crop_box
    padded_side = max(ox2, oy2)
    scale = sq / padded_side

    scaled_box = (
        int(ox * scale),
        int(oy * scale),
        int(ox2 * scale),
        int(oy2 * scale),
    )
    cropped = img.crop(scaled_box)
    print(f"  Unpad:    {sq}x{sq} -> crop {scaled_box} -> {cropped.size[0]}x{cropped.size[1]}")

    if cropped.size != target_size:
        cropped = cropped.resize(target_size, Image.LANCZOS)
        print(f"  Resize:   -> {target_size[0]}x{target_size[1]}")

    return cropped


# ---------------------------------------------------------------------------
# Provider: OpenAI
# ---------------------------------------------------------------------------
def call_openai(test, provider_cfg, send_img, mask_img=None):
    from openai import OpenAI

    client = OpenAI()
    model = provider_cfg["model"]

    img_buf = image_to_bytes(send_img)

    # Build kwargs — some params are only supported on certain models
    kwargs = dict(
        model=model,
        image=img_buf,
        prompt=test["prompt"],
        size="1024x1024",
    )

    if mask_img is not None:
        kwargs["mask"] = image_to_bytes(mask_img)

    # Try with all optional params, progressively strip unsupported ones
    optional_params = [
        {"quality": "high", "background": test.get("background", "auto")},
        {"quality": "high"},
        {},
    ]

    print(f"  Calling OpenAI images.edit (model={model}, mask={'yes' if mask_img else 'no'})...")
    for extra in optional_params:
        try:
            result = client.images.edit(**kwargs, **extra)
            break
        except Exception as e:
            if "Unknown parameter" in str(e) and extra:
                continue
            raise

    usage = getattr(result, "usage", None)
    if usage:
        input_tokens = getattr(usage, "input_tokens", 0) or getattr(usage, "prompt_tokens", 0) or 0
        output_tokens = getattr(usage, "output_tokens", 0) or getattr(usage, "completion_tokens", 0) or 0
        log_usage(model, input_tokens, output_tokens)
    else:
        print("  Tokens:   (usage not available in OpenAI images.edit response)")

    image_bytes = base64.b64decode(result.data[0].b64_json)
    return Image.open(BytesIO(image_bytes)).convert("RGBA")


# ---------------------------------------------------------------------------
# Provider: Gemini
# ---------------------------------------------------------------------------
def call_gemini(test, provider_cfg, send_img, mask_img=None):
    from google import genai
    from google.genai.types import GenerateContentConfig, Modality

    api_key = os.environ.get("GEMINI_API_KEY")
    client = genai.Client(api_key=api_key)
    model = provider_cfg["model"]

    print(f"  Calling Gemini generate_content (model={model})...")
    response = client.models.generate_content(
        model=model,
        contents=[test["prompt"], send_img],
        config=GenerateContentConfig(
            response_modalities=[Modality.IMAGE, Modality.TEXT],
        ),
    )

    usage = getattr(response, "usage_metadata", None)
    if usage:
        input_tokens = getattr(usage, "prompt_token_count", 0) or 0
        output_tokens = getattr(usage, "candidates_token_count", 0) or 0
        log_usage(model, input_tokens, output_tokens)
    else:
        print("  Tokens:   (usage_metadata not available)")

    for part in response.candidates[0].content.parts:
        if part.inline_data is not None:
            return Image.open(BytesIO(part.inline_data.data)).convert("RGBA")

    raise RuntimeError("Gemini response did not contain an image.")


PROVIDER_CALLERS = {
    "openai": call_openai,
    "gemini": call_gemini,
}


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def remaster_image(test, provider_name, provider_cfg, run_dir):
    name = test["name"]
    model_tag = provider_cfg["model"]
    input_path = os.path.normpath(os.path.join(INPUT_DIR, test["input"]))
    flatten_bg = test.get("flatten_bg")

    # Output dir: images-generated/{timestamp}/{model}/{test-name}/
    out_dir = os.path.join(run_dir, model_tag, name)
    os.makedirs(out_dir, exist_ok=True)

    if not os.path.isfile(input_path):
        print(f"  SKIP: Input image not found: {input_path}", file=sys.stderr)
        return

    print(f"\n{'='*60}")
    print(f"  Test:     {name}")
    print(f"  Input:    {input_path}")
    print(f"  Provider: {provider_name}")
    print(f"  Model:    {model_tag}")
    print(f"  Output:   {out_dir}")
    print(f"{'='*60}")

    original = Image.open(input_path)
    original_size = original.size
    alpha_mask = None

    # --- Compute output size: scale largest dim to MAX_DIM, proportional ---
    orig_w, orig_h = original_size
    scale_factor = MAX_DIM / max(orig_w, orig_h)
    output_size = (int(orig_w * scale_factor), int(orig_h * scale_factor))
    is_square = (orig_w == orig_h)
    print(f"  Original: {orig_w}x{orig_h}, output target: {output_size[0]}x{output_size[1]}")

    # --- Save original for reference ---
    orig_path = os.path.join(out_dir, "original.png")
    original.save(orig_path, "PNG")
    print(f"  Saved original: {orig_path} ({original_size[0]}x{original_size[1]})")

    # --- Save prompt ---
    prompt_path = os.path.join(out_dir, "prompt.txt")
    with open(prompt_path, "w") as f:
        f.write(test["prompt"])
    print(f"  Saved prompt: {prompt_path}")

    # --- Pad to square if non-square ---
    crop_box = None
    if not is_square:
        pad_fill = flatten_bg if flatten_bg is not None else None
        original, crop_box = pad_to_square(original, pad_fill)

    # --- Resize to API input size (1024x1024) ---
    api_size = (1024, 1024)
    if original.size != api_size:
        pre_size = original.size
        original = original.resize(api_size, Image.LANCZOS)
        print(f"  Resize:   {pre_size[0]}x{pre_size[1]} -> {api_size[0]}x{api_size[1]}")

    # --- Pre-processing ---
    use_mask = test.get("use_mask", False)
    mask_img = None

    if use_mask and original.mode == "RGBA":
        # Mask mode: send original RGBA + mask derived from alpha
        mask_img = create_edit_mask(original)
        send_img = original
        mask_path = os.path.join(out_dir, "mask.png")
        mask_img.save(mask_path, "PNG")
        print(f"  Mask:     Generated edit mask from alpha ({mask_img.size[0]}x{mask_img.size[1]})")
    elif flatten_bg is not None and original.mode == "RGBA":
        # Flatten mode: composite onto solid bg, restore alpha later
        send_img, alpha_mask = flatten_image(original, flatten_bg)
    else:
        send_img = original

    # --- Save input sent to API ---
    input_sent_path = os.path.join(out_dir, "input-sent.png")
    send_img.save(input_sent_path, "PNG")
    print(f"  Saved input sent: {input_sent_path} ({send_img.size[0]}x{send_img.size[1]})")

    # --- Call API ---
    caller = PROVIDER_CALLERS[provider_name]
    result_img = caller(test, provider_cfg, send_img, mask_img=mask_img)

    # --- Save raw API result ---
    raw_path = os.path.join(out_dir, "raw.png")
    result_img.save(raw_path, "PNG")
    print(f"  Saved raw:  {raw_path} ({result_img.size[0]}x{result_img.size[1]})")

    # --- Unpad if was non-square (before alpha restore) ---
    if crop_box is not None:
        result_img = unpad_from_square(result_img, crop_box, output_size)
        if alpha_mask is not None:
            alpha_mask = unpad_from_square(
                alpha_mask.convert("RGBA"), crop_box, output_size
            ).split()[3]

    # --- Post-processing: restore alpha ---
    if alpha_mask is not None:
        result_img = restore_alpha(result_img, alpha_mask)

    # --- Save single output at final proportional size ---
    if result_img.size != output_size:
        result_img = result_img.resize(output_size, Image.LANCZOS)

    out_path = os.path.join(out_dir, "remaster.png")
    result_img.save(out_path, "PNG")
    print(f"  Saved: {out_path} ({result_img.size[0]}x{result_img.size[1]})")


def main():
    parser = argparse.ArgumentParser(description="Remaster game asset images.")
    parser.add_argument(
        "--provider", choices=list(PROVIDERS.keys()), default="openai",
        help="Which AI provider to use (default: openai)",
    )
    parser.add_argument(
        "--filter", dest="filter_name", default=None,
        help="Only run tests whose name contains this string",
    )
    args = parser.parse_args()

    provider_cfg = PROVIDERS[args.provider]
    env_key = provider_cfg["env_key"]

    if not os.environ.get(env_key):
        print(f"Error: {env_key} environment variable is not set.", file=sys.stderr)
        sys.exit(1)

    # Create timestamped run directory
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    run_dir = os.path.join(OUTPUT_BASE, timestamp)
    os.makedirs(run_dir, exist_ok=True)
    print(f"Run directory: {run_dir}")

    for test in TESTS:
        if args.filter_name and args.filter_name not in test["name"]:
            continue
        remaster_image(test, args.provider, provider_cfg, run_dir)

    print("\nDone.")


if __name__ == "__main__":
    main()
