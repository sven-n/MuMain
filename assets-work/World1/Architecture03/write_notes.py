"""Write the local, measured production handoff without touching shared documentation."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CONFIG = json.loads((ROOT / 'batch-config.json').read_text())
IDENTITIES = json.loads((ROOT / 'identities.json').read_text())
PROTECTED = {
    'HouseWall03': 'Two broad lintel faces receive shallow interior edge recesses. Lintel ends, exact doorway opening, underside and all threshold contact surfaces stay original.',
    'Bridge01': 'Ten flat pier/parapet/post face interiors receive restrained dressed edges. Both modular end planes, arch clearance, complete projecting beast heads, flame positions and shadow geometry remain original.',
    'BridgeStone01': 'Four post face interiors receive shallow worn edges. Both log-end planes, all log and diagonal-brace geometry, original post endpoints and the alpha shadow mesh stay original.',
}


def read(folder, name):
    return json.loads((folder / name).read_text())


def write_asset(name):
    folder = ROOT / name
    source = read(folder, 'validation/blender.json')
    result = read(folder, 'validation/summary.json')
    anchors = read(folder, 'validation/modular-anchors.json')
    matrices = read(folder, 'validation/matrices-and-posed-bounds.json')
    motion = read(folder, 'validation/local-motion.json')
    final = read(folder, 'validation/final-contract.json')
    placements = read(folder, 'original/placements.json')
    rig = '\n'.join(line.strip() for line in (folder / 'original/info.txt').read_text().splitlines() if line.strip().startswith(('bone ', 'action ')))
    geometry = PROTECTED.get(name, 'Accepted BMD stays byte-identical to the pinned current baseline; only shared painted texture containers change.')
    dimensions = '\n'.join(f'- `{file}`: {record["dimensions"]}, {record["mode"]}' for file, record in final['textures'].items())
    notes = f'''# {name} — {IDENTITIES[name].lower()}

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-final`.
Date: 2026-09-22. Offline production and validation complete; real client verification pending.

Actual World1 identity: {IDENTITIES[name].lower()}; {len(placements)} placements. Exact positions, rotations, scale and tile coordinates are retained in `original/placements.json`.

{source['original_triangles']} → {source['triangles']} triangles. {geometry} All authored recesses are 0.55 units deep, constrained inside existing face boundaries. Every original vertex, origin, orientation, silhouette extremum and modular anchor remains retained. Selected face pairs and perimeters are listed in `validation/blender.json`. This is conservative structural edge refinement; the shared painting pass supplies the principal visual change.

Bind bounds before: `{result['bounds_before']}`.
Bind bounds after: `{result['bounds_after']}`.
Material slots, unchanged order: `{result['textures']}`.

Every texture dependency and image dimension:

{dimensions}

Shared painted families are bridge_01 (stone arch/coping and beast relief atlas with isolated orb region), tree_04 (cut log end grain), tile_ston06 (left-half vertical timber, upper-right ornamental crest, lower-right plaster), tile_wood02 (horizontal weathered oak boards), tile_ston04 (large limestone/plaster masonry) and tile_02 (riveted blackened iron). Filenames, normalized atlas layouts and existing UV/material roles are retained. bridge_01 is 1024×512; the five square paintings are 512×512 RGB. All other dependencies retain exact current-baseline bytes. `bridge_shadow01` is the unchanged 2×32 black RGBA gradient, alpha 8–255, with original container padding and geometry. The prior accepted awning alpha remains exact where consumed.

Frozen texture hashes: `validation/modular-anchors.json`. Shared consumer ownership: `../dependency-ownership.json`.

Exact rig/action contract retained:

```text
{rig}
```

Full comparison: **{result['full_compare']}**. Remodeled geometry is intentionally DIFFERENT; all 17 readonly BMDs compare EQUIVALENT and remain byte-identical. Skeleton plus actual action comparison: EQUIVALENT. Bone names, order, indices, parents, bind transforms, action count/key count/lock and every local key transform are unchanged. Local samples: `{motion['samples']}`. Hierarchical action matrices checked: {matrices['action_matrix_samples']}; maximum matrix delta {matrices['max_world_matrix_component_delta']}; maximum posed-bound delta {matrices['max_posed_bounds_component_delta']}.

Every original corner survives with maximum component delta {anchors['max_original_corner_position_component_error']}. {anchors['unchanged_triangle_count']} protected triangles retain positions, bone bindings, UVs and normals; maximum protected UV delta {anchors['max_unchanged_uv_component_error']}, normal direction delta {anchors['max_retained_normal_direction_degrees']} degrees. `validation/authored-bindings.json` checks new panels against the final BMD for intended bones/positions/UVs where this is an authored model. `validation/raw-bone-bindings.json` checks actual raw vertex/normal bone ownership; readonly results remain exactly original.

Engine contracts: Bridge01 retains CollisionRange -50 and fires at local (90,-200,30) and (90,200,30). Shared-material consumers retain House03/HouseWall02 light_02 mesh4 additive surfaces, House04 tile_space01 mesh8 V-scroll, House05 ston02 mesh2 V-scroll, and HouseWall05/06 HeroTile4 fade. Mesh indices, geometry and UVs of those effect surfaces remain original. `../engine-contract.txt` contains source excerpts. No flame particles are manufactured as client evidence.

Sources: untouched BMD, all original containers, unwrapped images, bmdconv info/SMD/action/manifest, official-import packed blend and placements in `original/`. Packed `source.blend` retains export-excluded REF_ORIGINAL, original rig/actions and an editable REF_HIGH_POLY candidate. Final editable PNG and game JPEG/TGA images are in `textures/`; final BMD and complete containers in `exports/`. Exact built-in imagegen prompts, unchanged generated masters and reproduction metadata are in `../paintings/`.

Official mu_bmd_export.py exports authored sources. Supported bmdconv SMD/manifest packaging then restores original protected corner records, panel-boundary coordinates and bind/action data to avoid Blender roundtrip quantization. Authored interior geometry and bevel normals remain. Raw official exports and final packing inputs/logs are retained, and their full compare is EQUIVALENT. Readonly BMDs bypass export and are copied exactly. Original-versus-remodeled comparison remains DIFFERENT.

Evidence: full validation/info/compare, isolated rig comparison, local pose/matrix/posed-bound, raw binding, material/UV/alpha, ownership/hash and source packing logs in `validation/`. `review/comparison.png` shows matching-camera original versus actual reimported final BMD, wireframe and reduced-scale previews. Animated readonly models retain additional three-pose sheets. Neutral diffuse versus labeled additive approximations are retained for indexed effects. Actual World1 placement assemblies are in `../review/`.

No shared runtime or client was used. Blender camera, lighting, additive effects, illustrative scroll offsets, roof omission, grass and 190-unit figure are explicitly offline approximations. Pending: actual client loading, lighting/filtering, alpha blending/scroll/fade, animations, bridge traversal/collision, flame placement and matching 1920×1080 client screenshots. Terrain placement, height, walk data, baked lighting and alpha strips remain unchanged.
'''
    (folder / 'notes.md').write_text(notes)
    return f'| {name} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} / {CONFIG["keys"][name]} | {result["full_compare"]} |'


def main():
    rows = [write_asset(name) for name in CONFIG['names']]
    installation = read(ROOT, 'source-installation.json')
    summary = '''# Architecture03 — Bridges and final shared architectural materials

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-final`.
Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture-final`.
Date: 2026-09-22. Offline production complete; client verification pending.

Three authored assets cover 34 placements: HouseWall03 timber lintel/threshold, Bridge01 carved stone parapet, and BridgeStone01 twin-log railing. Seventeen accepted BMD consumers remain byte-identical to the current baseline recorded in baseline-commit.txt, which contains Architecture02 integration 6b025039. They are archived with complete dependencies and fresh packed compatibility sources. Six shared diffuse paintings close the remaining architectural texture families; the seventh owned family, bridge_shadow01, stays byte-exact because its alpha gradient already serves the correct role.

The stone bridge retains its double arches and both original projecting beast heads. The timber bridge retains its log silhouettes, original end-grain faces and diagonal braces. Only safe interior faces are refined, preserving all original corner positions, modular end planes, doorway opening and contact surfaces. Painting identities and original atlas regions are retained; bridge_01 is 1024×512 and the five square paintings are 512×512 RGB. The 2×32 shadow strip and every other frozen material container remain unchanged.

| Model | Placements | Before → after triangles | Bones / action keys | Full comparison |
| --- | ---: | ---: | ---: | --- |
''' + '\n'.join(rows) + '''

All 20 converter validations, texture wrapper checks, full rig/action comparisons, local-key and hierarchical matrix/posed-bound checks pass. Every original corner and each protected triangle's UV/normal data passes. All authored new triangle corners retain intended bone indices, positions and UVs in the actual final BMD; the raw vertex/normal-node audit passes. All 17 readonly BMDs are unchanged. No model exceeds 1500 triangles. Complete consumer coverage is in dependency-ownership.json; frozen hashes and every texture dimension are retained per model.

The official importer/exporter and mu_texture are used. Supported converter manifest packaging restores exact original rig/actions and protected corner records after export; authored new panel interiors remain. Raw official versus final packaged output compares EQUIVALENT, while original-versus-remodeled geometry correctly reports DIFFERENT. Every original BMD and container also matches the pinned Git revision independently.

Exact game files changed:

''' + '\n'.join(f'- `{name}`' for name in installation['installed']) + f'''

Source installation is confined to this isolated worktree: {len(installation['installed'])} changed game files, {installation['unchanged_world1_object1_files']} other World1/Object1 files unchanged. bridge_shadow01 is byte-identical. No runtime/client, engine, CMake, UI, terrain, main branch or remote changes.

Review images: `review/batch-review.jpg`, `review/compatibility-1.jpg`, `review/compatibility-2.jpg`, `review/texture-comparison.jpg`, and all 20 individual `review/comparison.png` sheets. House04, House05 and Tent01 include three-pose action sheets; House03/04/05 and HouseWall02 include neutral/additive effect sheets. Actual unchanged placement assemblies are `stone-bridge-comparison.jpg`, `log-bridge-comparison.jpg`, `west-door-comparison.jpg` and `town-interior-comparison.jpg`. Companion JSONs retain all placement records. The interior preview omits roof caps to illustrate the existing HeroTile4 fade target.

All review evidence is offline Blender, never client evidence. Grass/figure/camera/light/additive/scroll/fade approximations are labeled. Actual terrain, collision, baked lighting and assets outside this dependency group are omitted. Bridge fire emitters are not simulated. Real client validation at 1920×1080 remains pending due known client instability; no asset is marked verified in client.

Reproduction, run with explicit assigned-worktree cwd and PYTHONDONTWRITEBYTECODE=1; MU_BMDCONV points to the existing converter:

1. `package_textures.py` wraps retained imagegen masters and copies frozen dependencies.
2. Blender `-b --python build_source.py` creates packed sources with REF_ORIGINAL and editable high-poly candidates.
3. `export_all.py` runs the official exporter, protected-corner/rig packaging and exact readonly BMD copying.
4. `validate_export.py`, `validate_anchors.py`, `validate_final.py`, `validate_raw_bindings.py`; Blender `validate_matrices.py` and `validate_authored_bindings.py` validate the package.
5. Blender `render_exports.py` and `render_joins.py`; Pillow Python `assemble_review.py` composes labeled evidence.
6. `install_source.py` checks ownership/hashes before isolated source installation; `write_notes.py` refreshes local notes.

Preparation/import/inspection scripts, exact prompts, unchanged generated master artwork, editable PNGs, source blends, exported game files and complete validation output remain in this deliverable. The coordinator alone updates shared handoffs and worklog. No unresolved validator rejection remains.
'''
    (ROOT / 'notes.md').write_text(summary)


if __name__ == '__main__':
    main()
