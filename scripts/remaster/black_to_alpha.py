#!/usr/bin/env python3
"""Convert dark pixels into opacity and keep the result as PNG in ready textures."""

from __future__ import annotations

from pathlib import Path
from typing import Iterable

from PIL import Image, ImageChops

def detect_project_root() -> Path:
    cwd = Path.cwd().resolve()
    for parent in (cwd, *cwd.parents):
        if (parent / "Cargo.toml").exists() and (parent / "assets" / "remaster").exists():
            return parent
    # Fallback to repository layout relative to this script.
    return Path(__file__).resolve().parents[2]


PROJECT_ROOT = detect_project_root()
RAW_ROOT = PROJECT_ROOT / "assets/remaster/textures/raw"
READY_ROOT = PROJECT_ROOT / "assets/remaster/textures/ready"


def resolve_input_path(value: str, fallback_root: Path) -> Path:
    if not value:
        return Path(value)
    path = Path(value)
    if path.is_absolute():
        return path
    absolute_candidate = (Path.cwd() / path).resolve()
    if absolute_candidate.exists():
        return absolute_candidate
    project_candidate = (fallback_root / path).resolve()
    if project_candidate.exists():
        return project_candidate
    return absolute_candidate


def output_path_for_input(input_path: Path, ready_root: Path = READY_ROOT, raw_root: Path = RAW_ROOT) -> Path:
    input_path = input_path.resolve()
    try:
        relative = input_path.relative_to(raw_root.resolve())
    except ValueError:
        relative = input_path.name
    return (ready_root.resolve() / relative).with_suffix(".png")


def convert_black_to_alpha(image_path: Path, output_path: Path) -> None:
    with Image.open(image_path) as img:
        image_rgba = img.convert("RGBA")
        luminance = img.convert("L")

        # Existing alpha is kept and modulated by brightness (black -> transparent, white -> opaque).
        output_alpha = ImageChops.multiply(image_rgba.getchannel("A"), luminance)
        image_rgba.putalpha(output_alpha)

        output_path.parent.mkdir(parents=True, exist_ok=True)
        image_rgba.save(output_path, format="PNG")


def parse_inputs(argv: Iterable[str]) -> tuple[Path, Path]:
    import argparse

    parser = argparse.ArgumentParser(
        description="Converte tons de preto em opacidade proporcional e salva PNG no ready."
    )
    parser.add_argument(
        "input",
        nargs="?",
        default=str(RAW_ROOT / "item/elfin_wing.jpeg"),
        help="Imagem de entrada. Default: assets/remaster/textures/raw/item/elfin_wing.jpeg",
    )
    parser.add_argument(
        "--ready-root",
        type=Path,
        default=READY_ROOT,
        help="Raiz de saída para texturas prontas (padrão: assets/remaster/textures/ready).",
    )
    parser.add_argument(
        "--raw-root",
        type=Path,
        default=RAW_ROOT,
        help="Raiz usada para manter estrutura relativa (padrão: assets/remaster/textures/raw).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Caminho de saída explícito. Quando vazio, usa ready/<subpasta relativa>/nome.png.",
    )
    args = parser.parse_args(list(argv) if argv is not None else None)

    input_path = resolve_input_path(args.input, RAW_ROOT)
    output_path = args.output if args.output else output_path_for_input(input_path, args.ready_root, args.raw_root)
    return input_path, output_path


def main(argv: Iterable[str] | None = None) -> None:
    input_path, out_path = parse_inputs(argv)
    convert_black_to_alpha(input_path, out_path)


if __name__ == "__main__":
    main()
