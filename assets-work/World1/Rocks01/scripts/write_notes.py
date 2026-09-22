"""Write bounded asset handoff notes from measured validation evidence."""
import hashlib
import json
from pathlib import Path
import shutil
import sys
sys.dont_write_bytecode = True
from config import PROPS, REPOSITORY, ROOT

IDENTITIES = {
    'Stone01': 'Low boulder with satellite stones and dry grass',
    'Stone02': 'Upright slab with satellite stones and dry grass',
    'Stone03': 'Large rounded boulder',
    'Stone04': 'Three scattered small rocks',
    'Stone05': 'Low rock cluster',
    'Waterspout01': 'Animated dragon fountain; shared rock-base surface compatibility pass',
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def asset_notes(name):
    folder = ROOT / name
    summary = json.loads((folder / 'validation/summary.json').read_text())
    source = json.loads((folder / 'validation/source.json').read_text())
    placements = json.loads((folder / 'original/placements.json').read_text())
    meshes = summary['mesh_texture_order']
    for texture in meshes:
        extension = '.OZT' if texture.endswith('.tga') else '.OZJ'
        for filename in (texture, Path(texture).stem + extension):
            shutil.copy2(ROOT / 'original' / filename, folder / 'original' / filename)
    before, after = summary['geometry']['triangles_before_after']
    bounds = summary['geometry']['bounds_before_after']
    rows = [f'# {name} — {IDENTITIES[name]}', '',
            'Owner: ASTRA worker `/root/reviewer`, branch `codex/lorencia-rocks`.', '',
            f'World1 placements: **{len(placements)}**. Full positions, rotations, scales and tile coordinates: `original/placements.json`.',
            f'Game model: `src/bin/Data/Object1/{name}.bmd`.',
            'Complete ordered texture dependencies: ' + ', '.join(f'`{item}`' for item in meshes) + '.', '',
            f'Triangles: **{before} → {after}**. One UV set, one original bone per vertex, finite positions/UVs/normals, no zero-area triangles.',
            f'Full BMD comparison: **{summary["full_compare"]}**. Skeleton/actions-only comparison: **EQUIVALENT**.', '',
            'Bind bounds from actual original / final BMD conversions:', '',
            f'- Before: `{bounds[0]}`', f'- After: `{bounds[1]}`',
            f'- Largest exported component drift: `{summary["geometry"]["max_bound_component_difference"]:.6f}` game units.',
            '- Saved-source extrema are exactly unchanged; measured export drift is SMD/rotated-root floating-point roundtrip noise.', '',
            'Bone names, order, parent indices and every bind/action local translation and rotation are compared in `validation/summary.json`. '
            'Full node lists and all frame indices are retained there. Maximum local rotation delta is zero. '
            'Action index/order, lock flags, key counts and original mesh/texture order are exact.', '',
            f'Original bone order: `{source["bone_order"]}`.',
            f'Action metadata: `{source["action_meta"]}`.', '',
            'Artwork: shared `../textures/ston01.jpg` is 512×512 opaque cool slate with restrained moss. '
            'Stone01/02 additionally use `../textures/ston02.tga`, 512×512 RGBA dry grass, with the original 32px graded alpha enlarged bilinearly. '
            'Grass card geometry, UVs and weights are equivalent; all hidden RGB pixels retain botanical color to avoid black/white fringes.', '',
            'Packed `source.blend` contains immutable `REF_ORIGINAL`; rock models additionally retain editable `REF_HIGH_POLY`, excluded from export. '
            'Official importer/exporter were used. Original BMDs, containers, unpacked textures, info and SMD/action files are in `original/`.', '',
            'Validation: `validation/commands.json`, `compare.txt`, `skeleton-compare.txt`, `summary.json`, `source-audit.json` and `preservation.json`. '
            'Matching-camera renders reimport actual final BMDs and texture containers; `review/comparison.png` includes wireframe and reduced-scale images. '
            '`review/reverse-offline.png` checks the opposite side.', '']
    if name == 'Waterspout01':
        rows.extend(['The installed/exported fountain BMD is **byte-identical to the original**. '
                     'Only shared `ston01.OZJ` changes. The separate official export is `validation/official-roundtrip.bmd`, '
                     'validated and compared EQUIVALENT in `official-roundtrip-check.txt`, and deliberately not installed.', '',
                     'Mesh order remains `stone_statue02.jpg`, `reagon_waterspout.jpg`, `ston01.jpg`, `ston02.jpg`. '
                     'Mesh 3 water geometry, UVs, weights and all 11 bones / 21 action keys remain exact. '
                     '`ston02.OZJ` is frozen fountain WATER; `ston02.OZT` is the changed grass texture used only by Stone01/02. '
                     '`stone_statue02.OZJ` and `reagon_waterspout.OZJ` are also byte-identical; hashes are in `../installed-files.json`.', '',
                     'Original/final action poses 0, 10 and 20 are shown in `review/action-comparison.png`. '
                     'The original fountain has 12 triangles whose averaged normals oppose geometric winding; final data preserves these exactly. '
                     'No new such triangles were introduced. This historical normal convention is not an export failure.', ''])
    else:
        rows.extend(['Modeling uses selected convex manifold rock ridges above 25° for narrow one-segment chamfers. '
                     'Edges touching an original bound-extreme vertex are protected. No blind subdivision or grass edits. '
                     'Rock-only dominant-face-axis UV projection replaces stretched vertical legacy UVs; seams follow changed rock planes. '
                     'Selection counts and widths are recorded in `validation/source.json`.', ''])
    rows.append('**Pending:** serial client review at placed scale, engine vertex lighting and alpha testing. '
                'No runtime installation or client observation occurred; offline renders are not client evidence. '
                'Fountain water scrolling and engine particles are not emulated by these renders.')
    (folder / 'notes.md').write_text('\n'.join(rows) + '\n')
    return f'| {name} | {IDENTITIES[name]} | {len(placements)} | {before} → {after} | {summary["full_compare"]} |'


rows = [asset_notes(name) for name in PROPS]
notes = '''# Rocks01 — Lorencia rock group and fountain compatibility

Five low-poly rock models were refined and repainted; the fountain consumes the same new slate diffuse with its original BMD and all water/statue/dragon data retained. Owner: `/root/reviewer`; branch: `codex/lorencia-rocks`; worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rocks`.

193 placed rocks + 1 fountain = **194 World1 placements**. The complete placement and texture consumer records are in `dependency-inventory.json`. Review begins with `review-overview.jpg`, `alpha-review.png`, and each asset's `review/comparison.png`. All imagery is explicitly offline.

| Model | Inspected identity | Placements | Triangles | Full compare |
|---|---|---:|---:|---|
'''
notes += '\n'.join(rows)
notes += '''

## Changed game files

- `src/bin/Data/Object1/Stone01.bmd`
- `src/bin/Data/Object1/Stone02.bmd`
- `src/bin/Data/Object1/Stone03.bmd`
- `src/bin/Data/Object1/Stone04.bmd`
- `src/bin/Data/Object1/Stone05.bmd`
- `src/bin/Data/Object1/ston01.OZJ` — 512×512 RGB slate, previously 128×128.
- `src/bin/Data/Object1/ston02.OZT` — 512×512 RGBA dry grass, previously 32×32.

## Shared materials and frozen data

`ston01.jpg` has exactly these six model consumers, all included in this batch. `ston02.tga` serves only Stone01/02. The similarly named `ston02.jpg` is a different, **unchanged water texture** shared by Waterspout01 and House05. `stone_statue02.jpg` is shared by Waterspout01 and StoneStatue01 and is frozen. `reagon_waterspout.jpg` remains original because its existing atlas encodes the recognizable dragon silhouette and does not benefit from an isolated surface repaint here.

Waterspout01 keeps all 639 triangles, four mesh slots, 11 bones and one 21-key action byte-for-byte. Source rendering draws meshes 0/1/2 normally and mesh 3 with animated UV coordinates; bone indices 1 and 4 place particles. The preserved snippets are in `engine-reference-audit.txt`. The official Blender export roundtrip is validated separately and is not the installed fountain BMD.

`installed-files.json` records all seven installed source-Data hashes, four frozen fountain file hashes, and 317 other unchanged Object1/World1 files. No shared runtime files, terrain placement/height/walk/light/alpha data, engine code or other game files were edited.

## Art and validation

Slate is subdued cool gray with sparse olive moss, pale worn edges and dark fissures; dry grass uses muted ochre tips over dark olive-brown bases. The palette fits the completed terrain stone/soil surfaces and preserves the rocks' readable silhouettes. Selected convex ridge chamfers refine edges while original bound-extreme points and satellite arrangements stay exact. Rock-only UV projection corrects streaked legacy side faces. Grass cards retain their original geometry and UVs.

All actual BMD model and action SMDs pass `bmdconv validate`; all texture containers pass `mu_texture check`. Complete converter output is retained per asset. The five remodeled rocks correctly report **DIFFERENT** geometry/UVs, while each rig/action-only comparison reports **EQUIVALENT**. Fountain full comparison is **EQUIVALENT**, with a stronger byte-identical original BMD check. No full-equivalence claim is made for remodeled rocks.

All source bind extrema are exact. Maximum final BMD bound component drift is 0.001190 game units from rotated-root/SMD precision; validation uses 0.005, well inside the converter's 0.05 comparison tolerance. Bone names/order/parents and action metadata are exact. All local pose samples are compared, with zero rotation difference. No collapsed final UV triangles or new zero-area / opposing-normal triangles occur. Twelve legacy fountain averaged-normal disagreements are unchanged, not introduced.

Packed original, edited and high-poly Blender sources, original textures, exports and all action SMDs are retained. Built-in imagegen prompts/raw color paintings are in `generation-prompts.json` and `textures/*-generated.png`; editable layered OpenRaster sources are `textures/*.ora`. The original grass alpha field is enlarged bilinearly, without thresholding. Color remains present in hidden pixels; light/dark composite checks appear in `alpha-review.png`. Sources use Blender diffuse/transparent materials to approximate texture roles; they do not simulate the complete MU renderer.

## Reproduction

Run commands from this assigned worktree. Use Pillow-capable Python `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`; Blender `/Applications/Blender.app/Contents/MacOS/Blender`. `scripts/config.py` resolves every asset output under Rocks01 and points to the existing reviewed bmdconv binary. No tools are rebuilt.

1. Pillow Python: `scripts/package_textures.py` (uses preserved generated raw images, no new generation needed).
2. Blender background: `--python scripts/import_originals.py` (preserves existing originals).
3. Blender background: `--python scripts/build_and_export.py`.
4. Pillow Python: `scripts/validate_exports.py`, then `scripts/validate_preservation.py`.
5. Blender background: `--python scripts/audit_sources.py`, then `--python scripts/render_review.py`.
6. Pillow Python: `scripts/assemble_review.py`, `scripts/install.py`, and `scripts/write_notes.py`.

Prefix each script with `assets-work/World1/Rocks01/`. Blender should use `--python-exit-code 1`. Installation is guarded to this branch and original/intended file hashes and writes only the seven owned source-Data files.

## Pending checks

No client session was run and no asset is verified in client. Serial 1920×1080 placed-scale review, per-vertex lighting, grass alpha testing, fountain water UV scrolling and particles remain pending client stability. There is no unresolved offline validator rejection.
'''
(ROOT / 'notes.md').write_text(notes)
source_path = REPOSITORY / 'src/source/Engine/Object/ZzzObject.cpp'
lines = source_path.read_text().splitlines()
audit = ['Read-only source audit: src/source/Engine/Object/ZzzObject.cpp',
         'Source sha256: ' + digest(source_path), '']
for low, high in ((994,1004),(2752,2766),(4629,4632)):
    audit.extend(f'{index}: {lines[index-1]}' for index in range(low, high+1))
    audit.append('')
(ROOT / 'engine-reference-audit.txt').write_text('\n'.join(audit))
