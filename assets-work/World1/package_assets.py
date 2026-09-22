"""Package the pilot; use --install to copy its allowlisted files into source/runtime Data."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.dont_write_bytecode = True
REPOSITORY = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY / "tools"))
import mu_texture

ROOT = Path(__file__).resolve().parent
GROUND = ROOT / "GroundTiles"
PROP = ROOT / "Beer01"
EXPECTED_TILES = {
    "TileGrass01", "TileGrass02", "TileGround01", "TileGround01x", "TileGround02",
    "TileGround03", "TileMgrass", "TileMstone", "TileWater01", "TileWood01",
} | {f"TileRock{index:02}" for index in range(1, 8)}
CONVERTER = REPOSITORY / "out/build/macos-arm64/tools/bmdconv/Release/bmdconv"
RUNTIME_DATA = REPOSITORY / "out/build/macos-arm64/src/Release/Main.app/Contents/MacOS/Data"
TEXTURE_SIZE = 512
JPEG_QUALITY = 100
MAX_SEAM_MEAN = 1.0
MAX_SEAM_DELTA = 8


def seam_record(path):
    with Image.open(path) as image:
        assert image.mode == "RGB" and image.size == (TEXTURE_SIZE, TEXTURE_SIZE)
        pixels = np.asarray(image, dtype=float)
    horizontal = np.abs(pixels[:, 0] - pixels[:, -1])
    vertical = np.abs(pixels[0] - pixels[-1])
    mean = max(float(horizontal.mean()), float(vertical.mean()))
    maximum = float(max(horizontal.max(), vertical.max()))
    assert mean <= MAX_SEAM_MEAN and maximum <= MAX_SEAM_DELTA, path
    return dict(file=path.name, size=[TEXTURE_SIZE, TEXTURE_SIZE],
                horizontal_mean=float(horizontal.mean()), vertical_mean=float(vertical.mean()),
                max_delta=maximum)


def package_textures():
    paths = sorted((GROUND / "textures").glob("Tile*.png"))
    assert {path.stem for path in paths} == EXPECTED_TILES
    destination = GROUND / "exports"
    destination.mkdir(exist_ok=True)
    seams, files = [], []
    for path in paths:
        jpeg = path.with_suffix(".jpg")
        with Image.open(path) as image:
            image.convert("RGB").save(jpeg, quality=JPEG_QUALITY, subsampling=0)
        seams.append(seam_record(jpeg))
        container = destination / (path.stem + ".OZJ")
        mu_texture.wrap_file(jpeg, container)
        assert not mu_texture.check_file(container)
        assert mu_texture.unwrap_bytes(container.read_bytes(), ".ozj") == jpeg.read_bytes()
        files.append((container, Path("World1") / container.name))
    (GROUND / "seam-report.json").write_text(json.dumps(seams, indent=2) + "\n")
    return files


def package_prop():
    texture = PROP / "textures/plate2.jpg"
    container = PROP / "exports/plate2.OZJ"
    mu_texture.wrap_file(texture, container)
    assert not mu_texture.check_file(container)
    return [(PROP / "exports/Beer01.bmd", Path("Object1/Beer01.bmd")),
            (container, Path("Object1/plate2.OZJ"))]


def validate_engine(files):
    textures = [str(source) for source, _ in files if source.suffix == ".OZJ"]
    command = [sys.executable, str(REPOSITORY / "tools/mu_texture.py"), "check", *textures]
    result = subprocess.run(command, capture_output=True, text=True, check=True)
    (ROOT / "validation/exported-textures.txt").write_text(result.stdout)
    for smd in sorted((PROP / "validation/new").glob("Beer01*.smd")):
        arguments = [str(CONVERTER), "validate", str(smd)]
        if "_a00" in smd.name:
            arguments.append("--animation")
        subprocess.run(arguments, check=True)
    print(f"PASS: {len(textures)} wrapped textures and the exported model's SMD/action.")


def assert_protected_world_files():
    baseline = json.loads((ROOT / "validation/original-world1-sha256.json").read_text())
    protected = 0
    for name, checksum in baseline.items():
        if Path(name).stem in EXPECTED_TILES and name.endswith(".OZJ"):
            continue
        actual = hashlib.sha256((REPOSITORY / "src/bin/Data/World1" / name).read_bytes()).hexdigest()
        assert actual == checksum, f"Protected World1 file changed: {name}"
        protected += 1
    return protected


def install(files):
    protected = assert_protected_world_files()
    manifest = []
    for source, relative in files:
        digest = hashlib.sha256(source.read_bytes()).hexdigest()
        for data in (REPOSITORY / "src/bin/Data", RUNTIME_DATA):
            destination = data / relative
            assert destination.is_file(), f"Refusing to create a new game filename: {destination}"
            shutil.copy2(source, destination)
            assert hashlib.sha256(destination.read_bytes()).hexdigest() == digest
        manifest.append(dict(file=str(relative), sha256=digest))
    assert_protected_world_files()
    result = dict(files=manifest, protected_world1_files=protected,
                  client_verified=False, reason="Original client crashes; offline continuation authorized.")
    (ROOT / "validation/installed-files.json").write_text(json.dumps(result, indent=2) + "\n")
    print(f"Installed {len(files)} allowlisted files; {protected} other World1 files unchanged.")


def repeat_contact_sheet():
    paths = sorted((GROUND / "textures").glob("Tile*.jpg"))
    cell_width, cell_height, columns = 400, 425, 4
    canvas = Image.new("RGB", (columns * cell_width, ((len(paths) + columns - 1) // columns) * cell_height), (26, 29, 25))
    draw = ImageDraw.Draw(canvas)
    for index, path in enumerate(paths):
        x, y = index % columns * cell_width, index // columns * cell_height
        with Image.open(path) as image:
            tile = image.resize((128, 128), Image.Resampling.LANCZOS)
        for row in range(3):
            for column in range(3):
                canvas.paste(tile, (x + column * 128, y + row * 128))
        draw.text((x, y + 390), path.stem + " / 3x3 at 128px", fill="white")
    canvas.save(GROUND / "repeat-review.jpg", quality=95)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    files = package_textures() + package_prop()
    validate_engine(files)
    repeat_contact_sheet()
    if args.install:
        install(files)


if __name__ == "__main__":
    main()
