"""Read-only World1 inventory; run from the repository root with Python 3."""

import collections
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, "tools")
import mu_texture

SOURCE = Path("src/bin/Data/World1")
OUTPUT = Path("assets-work/World1")
MAP_KEY = bytes.fromhex("d17352f6d29acb273eaf593137b3e7a2")
MAP_CELL_COUNT = 256 * 256
OBJECT_RECORD_BYTES = 30
BEER_OBJECT_TYPE = 151
SLOTS = ["TileGrass01", "TileGrass02", "TileGround01", "TileGround02",
         "TileGround03", "TileWater01", "TileWood01"]
SLOTS += [f"TileRock{i:02}" for i in range(1, 8)]
SURFACES = ["olive grass/moss", "pale gritty earth", "irregular town cobbles",
            "worn rectangular slabs", "carved gothic paving", "blue-green water",
            "weathered wood decking", "pale rough rock/gravel", "mossy earth/rock",
            "cold grey rock blocks", "dark granular rock", "sandstone masonry",
            "black marble with gold corners", "dark rock with inverted legacy logo"]
OTHER_USES = {
    "TileGround01x.OZJ": "Legacy striated stone; not loaded by current terrain loader. Same bytes as TileMgrass.OZJ.",
    "TileMgrass.OZJ": "Legacy striated stone despite name; not loaded by current terrain loader.",
    "TileMstone.OZJ": "Legacy bright green moss/grass despite name; not loaded by current terrain loader.",
    "TerrainLight.OZJ": "Baked RGB lighting across the map. DATA: preserve byte-for-byte; never repaint.",
    "TerrainHeight.OZB": "8-bit terrain heights. DATA: preserve byte-for-byte.",
    "TileGrass01.OZT": "Alpha grass strip, BITMAP_MAPGRASS+0, terrain slot 0. Preserve 256×64 dimensions.",
    "TileGrass02.OZT": "Alpha grass strip, BITMAP_MAPGRASS+1, terrain slot 1. Preserve 256×64 dimensions.",
    "TileGrass03.OZT": "Alpha tall dry grass, BITMAP_MAPGRASS+2, terrain slot 2. Preserve 256×128 dimensions.",
    "TileGround01.OZT": "Legacy alpha strip; no current terrain-loader reference. Preserve dimensions.",
    "leaf01.OZJ": "Legacy opaque particle variant; Lorencia loads leaf01.tga.",
    "leaf01.OZT": "Lorencia leaf particle, BITMAP_LEAF1.",
    "leaf02.OZJ": "Particle texture, BITMAP_LEAF2.",
    "rain01.OZT": "Rain streak, BITMAP_RAIN; shared with other worlds via World1 path. Preserve.",
    "rain011.OZT": "Alternate rain streak loaded by Crywolf. Preserve.",
    "rain02.OZT": "Rain splash ring, BITMAP_RAIN_CIRCLE; shared via World1 path. Preserve.",
    "mini_map.OZT": "World minimap image used by minimap UI. Preserve.",
    "map1.OZJ": "Legacy map overview; no matching filename reference found in source. Preserve.",
    "angeflo_R.OZJ": "Legacy glow/particle; no matching filename reference found in source. _R is the bright flag. Preserve.",
}


def decode_map(data):
    """Mirror MapFileDecrypt for inspection only; never write map data."""
    result = bytearray()
    rolling = 0x5E
    for index, value in enumerate(data):
        result.append(((value ^ MAP_KEY[index % len(MAP_KEY)]) - rolling) & 0xFF)
        rolling = (value + 0x3D) & 0xFF
    return result


def terrain_uses():
    mapping = decode_map((SOURCE / "EncTerrain1.map").read_bytes())
    assert len(mapping) == 2 + MAP_CELL_COUNT * 3
    assert mapping[:2] == bytes([0, 1])
    layers = [collections.Counter(mapping[2 + i * MAP_CELL_COUNT:2 + (i + 1) * MAP_CELL_COUNT])
              for i in range(2)]
    uses = dict(OTHER_USES)
    for index, (name, surface) in enumerate(zip(SLOTS, SURFACES)):
        uses[name + ".OZJ"] = (
            f"Terrain slot {index}; {surface}. "
            f"Cells: layer 1={layers[0][index]}, layer 2={layers[1][index]}.")
    return uses


def image_dimensions(path):
    extension = mu_texture.PAYLOAD_EXTENSIONS[path.suffix.lower()]
    payload = mu_texture.unwrap_bytes(path.read_bytes(), path.suffix.lower())
    if extension == ".jpg":
        width, height, _ = mu_texture.read_jpeg_dimensions(payload)
    elif extension == ".tga":
        _, width, height, *_ = mu_texture.parse_tga(payload)
    else:
        width, height = struct.unpack_from("<ii", payload, 18)
    return width, height, extension


def write_inventory():
    lines = ["# World1 inventory — 2026-09-22", "",
             "Baseline: `main` at `9a8b2027`; branch `art/world1-pilot`. No game assets changed during inventory.", "",
             "Unwrapped with `python3 tools/mu_texture.py unwrap-dir src/bin/Data/World1 assets-work/World1/original`: **32 textures, 0 failures**.", "",
             "The brief says 14 base tiles but explicitly names **17**. The client loads 14 ground slots; the additional three named legacy files are included below. All 17 names remain in the requested repaint scope.", "",
             "Usage: `MapManager.cpp` loader, `ZzzLodTerrain.cpp` rendering, visual inspection and read-only decoding of `EncTerrain1.map`. Stored layer counts are not visible coverage: alpha, height and occlusion also matter.", "",
             "| Original file | Dimensions | Payload | Use / constraint |",
             "|---|---:|---|---|"]
    uses = terrain_uses()
    for path in sorted(SOURCE.iterdir()):
        if path.suffix.lower() not in mu_texture.PREFIX_SIZES:
            continue
        width, height, extension = image_dimensions(path)
        lines.append(f"| `{path.name}` | {width}×{height} | `{extension}` | {uses[path.name]} |")
    lines += ["", "## Loose image duplicates", "",
              "`TerrainHeight.bmp` (256×256), `TileGrass01.tga` (256×64), `rain01.tga` (4×32), and `rain02.tga` (32×16) also exist in the source folder. Height and rain images are byte-identical to their unwrapped payloads; the loose TileGrass01.tga differs from the OZT payload. The client reads the OZT container. Preserve all four loose images. Remaining files are terrain mapping/placement/walk data, BMD minimap models or legacy metadata.", "",
              "The grass-strip renderer derives world height from image pixel height; keeping dimensions is necessary for this pilot.", "",
              "## Reference status", "",
              "`reference/original-texture-contact-sheet.jpg` shows all 32 unwrapped textures. It is an inventory sheet, not a client screenshot.", "",
              "The three required 1920×1080 client reference captures remain pending because the unchanged client repeatedly crashed during setup. See `validation/client-baseline-crashes.md`.", "",
              "## Beer01 identity", "",
              "Import and render confirm a tavern still life (bottle, mug, bowls and food), not a barrel. The user confirmed rebuilding this still life. See `Beer01/original/`: 5 meshes, 216 triangles, 5 bones, one action with one keyframe."]
    (OUTPUT / "inventory.md").write_text("\n".join(lines) + "\n")


def record_baseline():
    hashes = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(SOURCE.iterdir()) if path.is_file()}
    target = OUTPUT / "validation/original-world1-sha256.json"
    if not target.exists():
        target.write_text(json.dumps(hashes, indent=2) + "\n")
    for path in SOURCE.iterdir():
        if path.suffix.lower() in (".jpg", ".tga", ".bmp") and path.name != "TileGrass01.tga":
            assert path.read_bytes() == (OUTPUT / "original" / path.name).read_bytes()
    command = ["out/build/macos-arm64/tools/bmdconv/Release/bmdconv", "info",
               str(OUTPUT / "Beer01/original/Beer01.bmd")]
    result = subprocess.run(command, text=True, capture_output=True, check=True)
    (OUTPUT / "Beer01/original/info.txt").write_text(result.stdout)


def record_prop_placements():
    objects = decode_map((SOURCE / "EncTerrain1.obj").read_bytes())
    count = struct.unpack_from("<h", objects, 2)[0]
    assert len(objects) == 4 + count * OBJECT_RECORD_BYTES
    placements = []
    for index in range(count):
        kind, *values = struct.unpack_from("<h7f", objects, 4 + index * OBJECT_RECORD_BYTES)
        if kind != BEER_OBJECT_TYPE:
            continue
        placements.append(dict(position=values[:3], rotation=values[3:6], scale=values[6],
                               tile=[values[0] / 100, values[1] / 100]))
    target = OUTPUT / "Beer01/original/placements.json"
    target.write_text(json.dumps(placements, indent=2) + "\n")
    print(f"Inventoried 32 textures; found {len(placements)} Beer01 placements.")


if __name__ == "__main__":
    manifest = OUTPUT / "validation/original-world1-sha256.json"
    if manifest.exists():
        for name, digest in json.loads(manifest.read_text()).items():
            actual = hashlib.sha256((SOURCE / name).read_bytes()).hexdigest()
            assert actual == digest, "Inventory requires original World1 files; stored inventory is already complete."
    write_inventory()
    record_baseline()
    record_prop_placements()
