"""Write asset-local handoffs from final measured export evidence."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
IDENTITIES = {'House01': 'Flat-roof stone house with chimney, timber beams and carved doorway',
              'House03': 'Three-chimney smithy with cutout slatted awning and lit shutter',
              'House04': 'Animated round workshop with green shingle roof and blue porthole',
              'Tent01': 'Animated woven canopy on four timber posts',
              'HouseWall02': 'Timber and stone window-wall module with raised decorative shutter',
              'HouseEtc02': 'Accepted low-walled Masonry annex, readonly geometry compatibility consumer'}
KEYS = {'House01': 1, 'House03': 1, 'House04': 40, 'Tent01': 36, 'HouseWall02': 1, 'HouseEtc02': 1}
EFFECTS = {'House03': 'Mesh 4 light_02.jpg; engine random additive brightness 0.4–0.7.',
           'House04': 'Mesh 8 tile_space01.jpg; engine world-time V scroll.',
           'HouseWall02': 'Mesh 4 light_02.jpg; engine random additive brightness 0.4–0.7.'}


def read(folder, filename):
    return json.loads((folder / filename).read_text())


def write_asset(name, identity):
    folder = ROOT / name
    source = read(folder, 'validation/blender.json')
    validation = read(folder, 'validation/summary.json')
    anchors = read(folder, 'validation/modular-anchors.json')
    contract = read(folder, 'validation/final-contract.json')
    motion = read(folder, 'validation/local-motion.json')
    placements = read(folder, 'original/placements.json')
    textures = '\n'.join(f'- `{texture}`: {image["dimensions"]}, {image["mode"]}' for texture, image in contract['textures'].items())
    frozen = ', '.join(anchors['frozen_container_sha256']) or 'None'
    changes = 'BMD unchanged byte for byte from accepted Masonry commit f1fbb29c. Only the shared carved-door atlas repaint is evaluated here.' if name == 'HouseEtc02' else f'{len(source["panel_recesses"])} selected panel interiors gain narrow beveled recesses 0.25–0.65 units deep. All original perimeter corners, openings, roof edges and footings remain in place.'
    comparison = 'Full model EQUIVALENT and exact BMD bytes unchanged.' if name == 'HouseEtc02' else 'Full model DIFFERENT because geometry was remodeled; no full-equivalence claim.'
    notes = f'''# {name} — {identity}

Owner: ASTRA fences/timber/masonry/architecture worker. Branch `codex/lorencia-architecture`.
Date: 2026-09-22. Offline production and engine validation complete. Actual client verification pending.

Identity and placement: {identity}; {len(placements)} unchanged World1 placements. Actual coordinates, rotations, scales and tile coordinates are retained in `original/placements.json`.

Geometry: {source['original_triangles']} → {source['triangles']} triangles. {changes} Footprints, origins, orientation and modular boundaries are unchanged. `validation/blender.json` identifies every modified face pair and original perimeter. High-poly bevel candidates are retained for editing only, excluded from game export.

Material slot order: {', '.join(validation['textures'])}.
{EFFECTS.get(name, 'No indexed additive effect in this asset.')}
The indexed effect surfaces and original UVs are protected. Smithy awning geometry stays 12 triangles; Tent canopy stays 26 triangles with original per-vertex rigid bone assignments. No additive geometry was remodeled.

Texture dimensions and modes:

{textures}

Frozen dependency containers, copied unchanged for self-contained review: {frozen}.
The six owned paint files preserve material roles and exact filenames. tile_house01 keeps woven linen above and carved winged-scroll door panel below. tile_ston05 is muted rough tan limestone; tile_windows01 is aged oak; tile_ston07 keeps horizontal timber slats with the exact original alpha mask enlarged nearest4x. tile_space01 remains a restrained blue swirl; light_02 remains a narrow warm light-to-black additive gradient.

Bind bounds before: `{validation['bounds_before']}`.
Bind bounds after: `{validation['bounds_after']}`.
Every original vertex and bone assignment retained within {anchors['max_original_corner_position_component_error']:.9f} units. {anchors['unchanged_triangle_count']} protected triangles preserve positions and UVs. Maximum unchanged UV component delta: {anchors['max_unchanged_uv_component_error']}. Maximum protected normal component delta: {anchors['max_unchanged_normal_component_error']:.9f}; direction delta: {anchors['max_retained_normal_direction_degrees']:.8f} degrees.

Rig: `{source['bone_order']}`. Names, order, indices and parents exactly retained, together with one action and {KEYS[name]} keys plus original lock metadata. All local bind/action translations and Euler components are checked, not just key counts: `{motion['samples']}`. Skeleton plus actual action compare EQUIVALENT. {comparison}

Official Blender exporter provides remodeled mesh data. `preserve_contract.py` then restores untouched original triangle corners and original bind/actions using the documented bmdconv SMD/manifest workflow. This fixes Blender split-normal quantization and House04 quaternion/Euler drift without changing new panel topology. The raw official BMD and conversion files remain in `validation/official-export/`; the final packing inputs remain in `validation/locked-motion/`. Their comparison is EQUIVALENT. The readonly annex bypasses this conversion and is copied exactly.

UV range retained: U `{contract['uv_range'][0]}`, V `{contract['uv_range'][1]}`. New UVs interpolate inside existing panel islands. Export has finite vertices/UVs, one original full-weight non-dummy bone per corner, original material order and unchanged texture names. Opaque images remain RGB; awning TGA is bottom-left uncompressed 32-bit RGBA with original alpha retained exactly. All OZJ/OZT wrappers pass mu_texture checks.

Evidence: `validation/compare.txt`, `skeleton-compare.txt`, `local-motion.json`, `modular-anchors.json`, `smd-validation.txt`, `info-after.txt`, `texture-check.txt`, `source-audit.json`, `final-contract.json`, `summary.json`. Immutable original bytes are independently checked against the pinned baseline commit.

Review: `review/comparison.png` contains matching-camera original and actual re-imported final BMD renders, exported wireframe and reduced-scale grass/190-unit figure study. `effect-comparison.jpg` exists for indexed-effect models; its neutral diffuse and approximate additive images are clearly labeled. House04 action frames 0/19/39 and Tent frames 0/17/35 are compared in `action-comparison.jpg`. House04 scroll offsets 0/-1/3/-2/3 are illustrative; the actual engine uses world time. All images are Blender offline evidence, never client screenshots.

Sources: original BMD/containers/unwrapped images/bmdconv info/SMD/manifest/official-import blend under `original/`. Packed editable `source.blend` contains REF_ORIGINAL and REF_HIGH_POLY, hidden and excluded from export, plus the original rig and final authored mesh. Painted masters and exact imagegen prompts are retained in `../paintings/`; editable PNGs and final images are in `textures/`; wrapped game exports in `exports/`.

Pending: real client lighting, alpha sorting/additive appearance, motion readability, occlusion and traversal checks at 1920×1080. No client or shared runtime was operated. No World1 placement, terrain, walk/collision, TerrainLight or alpha-strip data was edited.
'''
    (folder / 'notes.md').write_text(notes)
    return f'| {name} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} / {KEYS[name]} |'


def main():
    rows = [write_asset(name, identity) for name, identity in IDENTITIES.items()]
    files = read(ROOT, 'source-installation.json')['installed']
    overview = '''# Architecture01 — Lorencia houses, workshop, canopy and window walls

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture`.
Date: 2026-09-22. Offline production complete; client verification pending.

Five remodeled models cover 21 World1 placements. The accepted Masonry annex is a sixth readonly consumer with one placement; its 250-triangle BMD remains byte-identical. Shared paints have been previewed across all six consumers. Conservative recessed stone, shutter and timber panel details preserve every original corner, roof edge, opening and connection. Repainted linen, carved doorway, roof limestone, shutter wood, slatted awning and restrained light effects preserve recognizable material roles and the dark medieval MU palette. Frozen wood and stone atlases remain unchanged.

| Model | Placements | Before → after triangles | Bones / action keys |
| --- | ---: | ---: | ---: |
''' + '\n'.join(rows) + '''

All five remodeled full comparisons report DIFFERENT; all six skeleton plus full-action comparisons report EQUIVALENT. Local bind/action poses, names/order/parents and lock metadata are checked explicitly. All local translation and Euler deltas are zero at the converter's retained precision, including House04's 360 action bone samples and Tent's 144 samples. All bind bounds match. Every original corner survives within 0.000016 units. Full triangle, UV, bone and normal checks protect unmodified roof silhouettes, openings, modular ends, cloth and special effect geometry.

The official Blender exporter is used for all authored models. A documented SMD/manifest packaging step restores original protected corner records and exact bind/action data to avoid the Blender exporter's normal quantization and House04 Euler conversion drift. It does not replace the authored geometry: the raw official and final packaged BMDs compare EQUIVALENT. Both stages and their logs are retained. The final original-versus-remodeled model comparison remains DIFFERENT.

Engine effects retained: House03 and HouseWall02 mesh 4 light_02 (brightness 0.4–0.7); House04 mesh 8 tile_space01 (world-time V scroll). Awning alpha is the exact original 128² mask expanded to 512² nearest-neighbor; top-left origin is not introduced. Existing material slot order and game filenames remain unchanged. Surface paints are 512²; blue effect 256²; light effect 128². No other texture container is modified.

`dependency-ownership.json` records all consumers. `original-sha256.json` proves untouched original game files against pinned baseline f1fbb29c. `source-installation.json` records exactly 11 changed game files and 313 unchanged other World1/Object1 files; accepted HouseEtc02 is among the unchanged files. No shared runtime/client, engine, CMake, UI, terrain, main branch or remote was changed.

Exact game files changed:

''' + '\n'.join(f'- `{file}`' for file in files) + '''

Review images:

- `review/batch-review.jpg` and `review/texture-comparison.jpg`.
- Each asset's `review/comparison.png`: original versus actual final exported BMD, wireframe, reduced-scale preview.
- House04 and Tent01 `review/action-comparison.jpg`: matched poses across retained clips.
- House03, House04 and HouseWall02 `review/effect-comparison.jpg`: neutral diffuse versus labeled additive approximation.
- `review/house-pair-comparison.jpg` and `review/town-wall-comparison.jpg`: actual unchanged World1 transforms, matched camera. Placement JSONs accompany them.

All images are offline Blender renders. Additive light/blue effect shading, flat grass, 190-unit figure and camera scale are approximations; actual terrain, baked lighting, unrelated buildings and collision are omitted. No image is presented as client evidence.

Reproduction: run all commands explicitly in this worktree, with `PYTHONDONTWRITEBYTECODE=1` and `MU_BMDCONV` set to the existing converter.

1. `package_textures.py`: retained imagegen masters to editable/final images, original alpha preservation, official mu_texture wrap.
2. Blender `-b --python build_source.py`: original official import to packed source including REF_ORIGINAL and optional high-poly candidate.
3. `export_all.py`: official mu_bmd_export.py, then preserve_contract.py and supported bmdconv manifest packaging; readonly annex copied exactly.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`: full and rig/action comparisons, local pose proof, corners/UV/normals, immutable originals, dependency/alpha contracts.
5. Blender `-b --python render_exports.py`, then `render_joins.py`; bundled Pillow Python `assemble_review.py`.
6. `install_source.py`: hash guards, owned source-files-only installation; `write_notes.py` refreshes these local handoffs.

The original preparation/import and inspection scripts, exact generation prompts and unedited master images are retained. Re-running preparation refuses to overwrite preserved originals. Shared helpers are imported read-only from approved StaticBatch01 and Masonry01 scripts.

Pending: coordinator visual/integration review, independent technical review, and actual 1920×1080 client observation when stability permits. No asset is marked verified in client. User-authorized offline production has no unresolved validator rejection.
'''
    (ROOT / 'notes.md').write_text(overview)


if __name__ == '__main__':
    main()
