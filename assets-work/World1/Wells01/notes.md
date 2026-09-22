# Wells01 — water well, pottery and cask material group

2026-09-22. Owner ASTRA coordinator. Branch `codex/lorencia-wells`; worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-wells`.

Four new inventory models cover five actual placements. Well01 is the combined roofed well/casks/pottery assembly; Well02 the well alone; Well03 four ceramic vessels; Well04 eight vessels including a tilted pot. Well01 and compatibility Carriage03 have no current World1 placements but are part of the actual Object1 inventory.

Three paintings restore weathered timber/fieldstone, wheel-thrown cream and slate pottery, and oak casks with blackened iron bands. Pottery gains curved subdivided edge points and smooth shading, constrained to each original rigid part's exact bounds. Every original corner remains. The well structure, bucket, rope, casks, contact points, origin and orientation remain geometrically unchanged.

| Model | Placements | Triangles | Full comparison |
|---|---:|---:|---|
| Well01 | 0 | 405 → 915 | DIFFERENT |
| Well02 | 1 | 123 → 123 | EQUIVALENT |
| Well03 | 2 | 194 → 704 | DIFFERENT |
| Well04 | 2 | 388 → 1408 | DIFFERENT |
| Carriage03 | 0 | 538 → 538 | EQUIVALENT |

The five full model/action SMD pairs validate. Three intentional geometry remodels correctly report DIFFERENT; well-only and readonly cart compare EQUIVALENT. Every original rig, order, parent and one-key action remains equivalent. Original bind/action headers are copied through supported SMD/manifest packaging after official Blender export; raw intermediate files/logs remain. Printed bind bounds match. All original source corners are exact, and the actual export is checked against every authored vertex with named-bone ownership. Maximum original-corner reimport drift on the four new assets is 0.000099183 units. This avoids cross-bone vertex deduplication regressions without engine changes.

Legacy constant-color cap UVs are deliberately retained: Well01/02/03/04 have 24/6/18/36 collapsed UV triangles both before and after. Subdivision excludes those original faces and their edges, introducing no additional collapsed UVs. Zero geometric degeneracies; finite geometry/UVs, rigid weights, material order and wrapping checks pass. No new textures or material names.

Complete texture group: well.OZJ serves Well01/02; jar_01.OZJ serves Well01/03/04; tub.OZJ serves Well01 and accepted Carriage03. CartHay01 Carriage03 BMD at93ca193e is archived as readonly compatibility and remains byte-identical in its export here. Its new tub painting is checked on the actual accepted cart geometry. The shared horse_drawn_01.OZJ remains untouched for its separate coordinated final pass.

well and jar_01 are 512×512 RGB; tub is 512×256 RGB. Original atlas regions/material roles remain. All four dependency containers pass mu_texture loader checks. Raw imagegen PNG masters, prompts, original inputs and editable final PNG/JPEG layers are retained under textures; textures/paint-comparison.jpg compares original/generated/final. Packed sources preserve REF_ORIGINAL and original rigs; final BMDs were actually reimported for checks and review.

Exact changed game paths:

- `src/bin/Data/Object1/Well01.bmd`
- `src/bin/Data/Object1/Well02.bmd`
- `src/bin/Data/Object1/Well03.bmd`
- `src/bin/Data/Object1/Well04.bmd`
- `src/bin/Data/Object1/well.OZJ`
- `src/bin/Data/Object1/jar_01.OZJ`
- `src/bin/Data/Object1/tub.OZJ`

Only these seven paths were installed into this isolated checkout. All other 317 World1/Object1 files were hash checked against this worktree baseline. The newer Carriage03 compatibility BMD is retained under deliverables only, not written into this checkout's frozen game path. No runtime, client, engine/CMake, UI, other map, terrain, lighting, placement or collision writes.

Reproduce from this worktree: bundled Python prepare.py once; package_textures.py; Blender build.py; bundled Python export_validate.py; Blender audit_sources.py and render_review.py; bundled Python assemble_review.py, assemble_paintings.py, install.py, write_notes.py. For accepted cart compatibility run prepare_compatibility.py93ca193e, then build.py -- Carriage03, export_validate.py Carriage03, audit_sources.py -- Carriage03 and render_review.py -- Carriage03. Use the spaced arguments shown in the scripts/tool logs; all scripts derive paths from this folder. Original and production logs are retained.

Review each model's review/comparison.jpg and reverse images. A190-unit bar illustrates scale; no runtime lighting or client behavior is claimed. Pending independent acceptance/integration and real serial1920×1080 client observation once stable. No client verification occurred.
