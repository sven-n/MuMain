"""Document measured well, pottery and shared cask results."""
import json
from pathlib import Path
HERE=Path(__file__).resolve().parent
IDENTITIES={'Well01':'Roofed fieldstone well, two casks and four ceramic vessels','Well02':'Roofed fieldstone well','Well03':'Four ceramic pitchers and storage vessels','Well04':'Eight ceramic vessels including one tilted pot','Carriage03':'Accepted cask cart; unchanged BMD compatibility'}
ROWS=[]
for name,identity in IDENTITIES.items():
    p=HERE/name;s=json.loads((p/'validation/summary.json').read_text());a=json.loads((p/'validation/source-export-audit.json').read_text());place=json.loads((p/'placements.json').read_text())
    before,after=a['original']['triangles'],a['replacement']['triangles'];ROWS.append(f'| {name} | {len(place)} | {before} → {after} | {s["full_compare"]} |')
    text=f'''# {name} — {identity}

2026-09-22. ASTRA coordinator, Wells01. Offline validation complete; client pending.

{len(place)} World1 placements; exact transforms retained in placements.json. {before} → {after} triangles.
Bind bounds before: `{s['bounds_before']}`. After: `{s['bounds_after']}`.
Full geometry comparison **{s['full_compare']}**; rig/actions **EQUIVALENT**. Original local bind/action records are restored after official mesh export through supported bmdconv SMD/manifest packaging. Bone names, order, parents and all one-key actions retained; explicit sample checks in validation/local-motion.json.

Original anchor maximum final-reimport drift: {a['max_anchor_component_delta']:.9f} units. Every authored vertex also appears at its intended named bone and position in the final reimport; maximum distance component {a['max_authored_vertex_delta']:.9f}. Source footprints and all original corners are unchanged. Pottery subdivision adds curved edge points constrained to each original bone AABB. Original constant-color UV cap faces remain intact: collapsed UV count {a['original']['zero_uv']} before and {a['replacement']['zero_uv']} after; geometric degeneracies remain zero. No newly collapsed UVs or geometry. Every vertex rigidly uses one original bone; mesh/material order and all original texture filenames retained.

well/jar_01 paintings are 512×512 RGB, tub is 512×256 RGB. Shared horse_drawn_01 is frozen. Complete original dependencies are archived and described in dependencies.json. Source.blend is packed with REF_ORIGINAL. Review comparison.jpg shows matching original/final cameras, wireframe and reduced previews; reverse views remain separate. These are offline Blender studies, not client evidence.

'''
    if name=='Carriage03':text+='This BMD is byte-identical to accepted CartHay01 commit93ca193e. It is archived and rendered to prove shared tub painting compatibility; no Carriage03 game file is installed by this batch.\n\n'
    text+='See ../notes.md for group ownership, exact changed paths and reproduction.\n';(p/'notes.md').write_text(text)
installed=json.loads((HERE/'installed-files.json').read_text())
text='''# Wells01 — water well, pottery and cask material group

2026-09-22. Owner ASTRA coordinator. Branch `codex/lorencia-wells`; worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-wells`.

Four new inventory models cover five actual placements. Well01 is the combined roofed well/casks/pottery assembly; Well02 the well alone; Well03 four ceramic vessels; Well04 eight vessels including a tilted pot. Well01 and compatibility Carriage03 have no current World1 placements but are part of the actual Object1 inventory.

Three paintings restore weathered timber/fieldstone, wheel-thrown cream and slate pottery, and oak casks with blackened iron bands. Pottery gains curved subdivided edge points and smooth shading, constrained to each original rigid part's exact bounds. Every original corner remains. The well structure, bucket, rope, casks, contact points, origin and orientation remain geometrically unchanged.

| Model | Placements | Triangles | Full comparison |
|---|---:|---:|---|
'''+ '\n'.join(ROWS)+'''

The five full model/action SMD pairs validate. Three intentional geometry remodels correctly report DIFFERENT; well-only and readonly cart compare EQUIVALENT. Every original rig, order, parent and one-key action remains equivalent. Original bind/action headers are copied through supported SMD/manifest packaging after official Blender export; raw intermediate files/logs remain. Printed bind bounds match. All original source corners are exact, and the actual export is checked against every authored vertex with named-bone ownership. Maximum original-corner reimport drift on the four new assets is 0.000099183 units. This avoids cross-bone vertex deduplication regressions without engine changes.

Legacy constant-color cap UVs are deliberately retained: Well01/02/03/04 have 24/6/18/36 collapsed UV triangles both before and after. Subdivision excludes those original faces and their edges, introducing no additional collapsed UVs. Zero geometric degeneracies; finite geometry/UVs, rigid weights, material order and wrapping checks pass. No new textures or material names.

Complete texture group: well.OZJ serves Well01/02; jar_01.OZJ serves Well01/03/04; tub.OZJ serves Well01 and accepted Carriage03. CartHay01 Carriage03 BMD at93ca193e is archived as readonly compatibility and remains byte-identical in its export here. Its new tub painting is checked on the actual accepted cart geometry. The shared horse_drawn_01.OZJ remains untouched for its separate coordinated final pass.

well and jar_01 are 512×512 RGB; tub is 512×256 RGB. Original atlas regions/material roles remain. All four dependency containers pass mu_texture loader checks. Raw imagegen PNG masters, prompts, original inputs and editable final PNG/JPEG layers are retained under textures; textures/paint-comparison.jpg compares original/generated/final. Packed sources preserve REF_ORIGINAL and original rigs; final BMDs were actually reimported for checks and review.

Exact changed game paths:

'''+''.join('- `'+x+'`\n' for x in installed['installed'])+f'''
Only these seven paths were installed into this isolated checkout. All other {installed['protected_count']} World1/Object1 files were hash checked against this worktree baseline. The newer Carriage03 compatibility BMD is retained under deliverables only, not written into this checkout's frozen game path. No runtime, client, engine/CMake, UI, other map, terrain, lighting, placement or collision writes.

Reproduce from this worktree: bundled Python prepare.py once; package_textures.py; Blender build.py; bundled Python export_validate.py; Blender audit_sources.py and render_review.py; bundled Python assemble_review.py, assemble_paintings.py, install.py, write_notes.py. For accepted cart compatibility run prepare_compatibility.py93ca193e, then build.py -- Carriage03, export_validate.py Carriage03, audit_sources.py -- Carriage03 and render_review.py -- Carriage03. Use the spaced arguments shown in the scripts/tool logs; all scripts derive paths from this folder. Original and production logs are retained.

Review each model's review/comparison.jpg and reverse images. A190-unit bar illustrates scale; no runtime lighting or client behavior is claimed. Pending independent acceptance/integration and real serial1920×1080 client observation once stable. No client verification occurred.
'''
(HERE/'notes.md').write_text(text)
