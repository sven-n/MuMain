# Rocks01 — Lorencia rock group and fountain compatibility

Five low-poly rock models were refined and repainted; the fountain consumes the same new slate diffuse with its original BMD and all water/statue/dragon data retained. Owner: `/root/reviewer`; branch: `codex/lorencia-rocks`; worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rocks`.

193 placed rocks + 1 fountain = **194 World1 placements**. The complete placement and texture consumer records are in `dependency-inventory.json`. Review begins with `review-overview.jpg`, `alpha-review.png`, and each asset's `review/comparison.png`. All imagery is explicitly offline.

| Model | Inspected identity | Placements | Triangles | Full compare |
|---|---|---:|---:|---|
| Stone01 | Low boulder with satellite stones and dry grass | 28 | 166 → 518 | DIFFERENT |
| Stone02 | Upright slab with satellite stones and dry grass | 41 | 159 → 503 | DIFFERENT |
| Stone03 | Large rounded boulder | 49 | 80 → 186 | DIFFERENT |
| Stone04 | Three scattered small rocks | 67 | 45 → 137 | DIFFERENT |
| Stone05 | Low rock cluster | 8 | 120 → 417 | DIFFERENT |
| Waterspout01 | Animated dragon fountain; shared rock-base surface compatibility pass | 1 | 639 → 639 | EQUIVALENT |

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
