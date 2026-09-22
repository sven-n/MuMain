# TavernDetails01 — remaining tavern furniture and still lifes

Owner: ASTRA environment artist (`/root/groundcover`), branch `codex/lorencia-tavern-details`.
Six assets, 32 World1 placements. Modeled, painted and validated offline. Client review pending.

| Model | Actual identity | Placements | Triangles before → after | Original bones |
|---|---|---:|---:|---:|
| Beer02 | Two apples with leaf cards, bottle, two ceramic bowls, pewter tankard | 2 | 232 → 952 | 6 |
| Beer03 | Two bottles and pewter tankard | 4 | 108 → 516 | 3 |
| Furniture01 | Pottery shelf with blue/terracotta jug, ochre pot and three bottles | 2 | 272 → 732 | 6 |
| Furniture02 | Twelve-bottle shelf | 3 | 464 → 1288 | 13 |
| Furniture06 | Carved high-back chair with fretwork panel | 18 | 54 → 132 | 1 |
| Furniture07 | Faceted round three-legged stool | 3 | 42 → 174 | 1 |

Rounded vessel profiles, bowl cavities, rolled tankard rims, curved handles, ceramic lips,
apple bodies, shelf bevels/rear rails, chair aprons/stretchers and shaped stool legs improve
the original economical forms. All models stay below the 1500-triangle prop budget. Shared
warm gray-brown carved oak matches accepted TavernProps; muted indigo/terracotta and ochre
pottery, ivory/green ceramics, crimson apples, pewter and brown glass retain original roles.
Accepted Beer01, desk_big and earlier tables/counter were read as references and remain unchanged.

The complete owned dependency group has nine textures, all painted through built-in imagegen
and packaged at 512x512. Exact changed game files under `src/bin/Data/Object1/` are:

`Beer02.bmd`, `Beer03.bmd`, `Furniture01.bmd`, `Furniture02.bmd`, `Furniture06.bmd`,
`Furniture07.bmd`, `pot3.ozt`, `apple.OZJ`, `bottle.ozt`, `plate.OZJ`, `winecup.OZJ`,
`pot.OZJ`, `pot2.OZJ`, `bookshelf.OZJ`, `chair2.OZT`.

| Texture dependency | All consumers |
|---|---|
| pot3.ozt, apple.OZJ, plate.OZJ | Beer02 |
| bottle.ozt | Beer02, Beer03, Furniture01, Furniture02 |
| winecup.OZJ | Beer02, Beer03 |
| pot.OZJ, pot2.OZJ | Furniture01 |
| bookshelf.OZJ | Furniture01, Furniture02, Furniture06, Furniture07 |
| chair2.OZT | Furniture06 |

Model mesh order remains exact. All per-material/per-bone component bind bounds match within
0.001 units after export. Origins, orientation, individual item arrangements and shelf contacts
are preserved. Original alpha-card quads and UVs are retained. For the chair, both original seat
triangles at z49.784698 remain with zero coordinate deviation. The stool's six original seat
triangles at z51.411301 also have zero deviation, retaining the original faceted boundary.
Every original chair/stool foot corner is retained with zero deviation. No terrain, placement,
collision, interaction or engine data changed.

Source inspection: MapInfra/MapManager.cpp:1091–1097 loads the existing Furniture/Beer series
and Object1 textures. ZzzObject.cpp:4668–4670 registers Furniture06/07 with CreateOperate.
ZzzInterface.cpp:1697–1698 uses both as seats and aligns the character angle for Furniture06.
This supports preserving the exact chair orientation and both original seat/foot surfaces.
No model-specific mesh/bone renderer override was found in the inspected object code; original
mesh order is preserved regardless.

All original BMDs and dependencies are archived untouched, with official converter info,
packed original imports and exact placement records. Source originals match starting revision
8b853434. All source.blend projects are packed and retain numeric-original geometry, UVs,
skinning and transforms in REF_ORIGINAL, proved by saved-file fingerprint audits. Excluded
REF_HIGH_POLY contains editable authored components with fine-bevel modifiers. Raw generated
paintings, rejected alpha iterations, editable PNGs, final textures, prompts and reproduction
scripts remain in this directory. Generation is not deterministic; packaging reuses retained art.

All six full BMD comparisons correctly say DIFFERENT because geometry was remodeled.
Skeleton/action comparisons are EQUIVALENT. Bone names, order, parents, action/frame order,
one key, lock=0 and original rigid bindings are preserved. Every bind/action local pose was
compared as translation plus rotation matrix; maximum translation deviation is 0.000002 units,
maximum matrix-component deviation 0.0000003072. Actual exported meshes have zero collapsed
UV/geometry triangles and zero winding/normal disagreements. Final texture wrappers pass.

Alpha coverage at cutoff 64: apple sprigs 37.1% versus original 36.5%; chair fretwork 86.6%
versus original 84.0%. Generated alpha is retained while RGB under discarded pixels is padded
from visible neighbors. Bottle opacity preserves the original 181..255 field exactly through
nearest resampling. OZT files are uncompressed 32-bit bottom-left, with exact legacy casing.
Opaque materials remain RGB JPEG/OZJ. There are no new material flags or rendering features.

Review each model's `review/comparison.jpg`: matching original/exported-BMD camera, terrain
scale proxy and reduced readability. `reverse-before.png`/`reverse-after.png` and wireframes
are separate. Source renders are from reimported official BMD exports. Ground staging uses
accepted terrain with 100-unit repeats and a 190-unit scale figure, at invented positions.
These are offline Blender previews, not client or actual-placement evidence.

`installed-source-files.json` records the 15 changed source-game hashes. `ownership-check.json`
proves the other 309 tracked Object1/World1 files unchanged. The shared runtime and client were
never touched. Actual client loading, lighting, transparency/filtering, tavern occlusion and
sitting contacts remain pending; no asset is marked client-verified.

Reproduce from this worktree: Python prepare.py, import_originals.py, package_textures.py;
Blender inspect_originals.py and build_sources.py; Python export_validate.py; Blender
audit_sources.py; Python audit_exports.py; Blender render_review.py; Python write_notes.py
and finalize.py. Use Blender `--python-exit-code 1` and bundled Pillow/numpy Python. Every
subprocess resolves against this worktree; the final installer permits only original or
previously recorded owned-file hashes and never targets shared runtime Data.

The 100-unit terrain repeats are a staging proxy. Actual engine terrain UV density depends
on texture width and has not been reproduced by these renders.
