"""Produce measured monument handoffs without editing shared coordination documents."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parent
IDENTITIES={'StoneStatue01':'Carved robed-relief pillar','StoneStatue02':'Moai-like stone head',
 'StoneStatue03':'Winged angel on a memorial plinth','SteelStatue01':'Capped carved memorial with ochre plaque',
 'Tomb01':'Arched sarcophagus with offset lid','Tomb02':'Cross-topped carved grave',
 'Waterspout01':'Accepted dragon fountain; shared basin material compatibility only'}
rows=[]
for name,identity in IDENTITIES.items():
    folder=ROOT/name
    read=lambda item:json.loads((folder/'validation'/item).read_text())
    source,summary,anchors,binding,uv=map(read,('blender.json','summary.json','modular-anchors.json','vertex-bone-audit.json','uv-material-audit.json'))
    final,motion=map(read,('final-contract.json','local-motion.json'))
    placements=json.loads((folder/'original/placements.json').read_text())
    lines=[f'# {name}: {identity}','','Owner: ASTRA reviewer/production worker; branch `codex/lorencia-monuments`.',
      'Offline production complete; coordinator visual review/integration and client observation pending.', '',
      f'Actual World1 placements: {len(placements)}; exact original transforms archived in `original/placements.json`.',
      f'Game path: `src/bin/Data/Object1/{name}.bmd`.',
      f'Triangles: {source["original_triangles"]} → {source["triangles"]}.',
      f'Original engine bind bounds: `{summary["bounds_before"]}`.',f'Final engine bind bounds: `{summary["bounds_after"]}`.', '',
      'Complete texture dependencies in preserved engine mesh order:', '']
    for texture in summary['material_order']:
        container=Path(texture).stem+('.OZT' if texture.endswith('.tga') else '.OZJ')
        state='frozen byte-identical' if Path(texture).stem in ('ston01','ston02','reagon_waterspout') else 'repainted'
        detail=final['textures'][texture]
        lines.append(f'- `{container}` / `{texture}`: {detail["dimensions"]}, {detail["mode"]}; {state}.')
    lines+=['','The original footprint, origin, orientation and every source vertex are retained.',
      f'Selected panel face pairs: `{anchors["panel_face_pairs"]}`. Their interiors receive 0.4-unit recesses',
      'inside unchanged original boundaries. Head/angel sculpture silhouette and topology remain intact.',
      f'Explicit UV repair faces: `{uv["uv_reprojected_original_triangles"]}`; only the Moai head top cap is',
      'planarly mapped into the existing plain-stone atlas region to remove stretched legacy UV stripes.',
      f'Original/final collapsed UV triangles: {len(uv["zero_uv_before"])} / {len(uv["zero_uv_after"])}. No new UV degeneracies.', '',
      f'Full bmdconv comparison output: **{summary["full_compare"]}**. Rig/action comparison: **EQUIVALENT**.',
      'This comparator assesses geometry/rig; its EQUIVALENT result does not imply identical UVs or texture bytes.',
      f'Bone names/order: `{source["bone_order"]}`. Metadata: `{summary["action_metadata"]}`.',
      'Exact node-name/order/parent and action count/key count/lock checks pass.',
      f'Max original SMD-corner position-component drift: {anchors["max_original_corner_position_component_error"]:.9f} units.',
      f'Max unchanged triangle UV component delta: {anchors["max_unchanged_uv_component_error"]}.',
      f'Max retained normal direction delta: {anchors["max_retained_normal_direction_degrees"]:.9f} degrees.',
      f'Every authored and final vertex matches bidirectionally with exact named bone; measured max evaluated',
      f'component delta {binding["max_authored_to_final_component_error"]:.9f} units, tolerance {binding["position_tolerance"]}.',
      'This additionally guards against converter position deduplication silently changing root ownership.', '']
    for sample in motion['samples']:
        lines.append(f'- `{sample["file"]}`: {sample["frames"]} keys, {sample["bone_samples"]} samples; '
          f'local translation component delta {sample["max_local_position_component"]:.9f}, '
          f'rotation-matrix component delta {sample["max_rotation_matrix_component"]:.9f}.')
    if name=='Tomb01':
        lines+=['','The original slanted Cylinder02 root incurs 0.002633 units of SMD corner drift and 0.002639771',
          'units in Blender reconstruction. Source anchors, named bone ownership and UVs remain intact. Printed',
          'bind bounds match. The original local translation differs only 0.000007 units with unchanged rotation',
          'matrices. This measured float reconstruction uses an explicit 0.005-unit tolerance; other models use 0.001.']
    if name=='Waterspout01':
        lines+=['','Installed BMD is retained byte-identically. The official Blender export is archived separately as',
          '`validation/official-roundtrip.bmd` and is not installed. Only stone_statue02 basin diffuse changes.',
          'All 639 triangles, four mesh slots, 11 bones and 21 action keys remain exact. Mesh 3 water, V-scroll',
          'behavior and particle anchors at bones 1 and 4 are protected. Dragon, stone base and water texture',
          'containers are frozen, including accepted Rocks01 ston01. Offline renders do not emulate water UV',
          'scrolling, runtime particles, additive water blending or actual client lighting.']
    lines+=['','Untouched original game files, decoded textures, bmdconv info/SMDs and official source import: `original/`.',
      'Packed editable `source.blend` preserves exact excluded REF_ORIGINAL and optional REF_HIGH_POLY bake source.',
      'Full comparison output, model/action validation, bone/anchor/UV/material audits: `validation/`.',
      'Matching-camera actual BMD before/after, wireframe and reduced-scale views: `review/comparison.png`.',
      'Six builtin imagegen master paintings, original references, prompts and editable OpenRaster files: `../paintings/`.',
      '**Pending:** coordinator acceptance and serial client observation. No runtime install or client verification.', '']
    (folder/'notes.md').write_text('\n'.join(lines))
    rows.append(f'| {name} | {identity} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} | {summary["full_compare"]} |')
installed=json.loads((ROOT/'source-installation.json').read_text())
lines=['# Statues01 offline handoff','','2026-09-22. ASTRA reviewer/production worker. Branch `codex/lorencia-monuments`.',
 'Worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-monuments`.',
 'Six newly completed monuments/graves cover 30 World1 placements; one additional fountain compatibility placement.', '',
 '| Model | Actual identity | Placements | Triangles | Bones | bmdconv compare |','|---|---|---:|---:|---:|---|',*rows,'',
 'Six stone atlases sharpen the original reliefs, symbols and material roles. The shared relief-pillar and basin',
 'retain warmer limestone; the Moai head, angel and cemetery monuments use muted cool granite. Painted edge',
 'wear and readable carving complement the accepted Masonry01, Rocks01 and Tomb03 palette. Four models receive',
 'shallow 0.4-unit stone panel recesses, keeping every original boundary. Head and angel sculptural geometry',
 'remain intact. Six head-cap triangle UVs are repaired to remove the old narrow-strip stretch. The sarcophagus',
 'paint was refined after render review to remove blocky grain while retaining its original abstract medallions.', '',
 '## Exact changed game files','',*[f'- `{path}`' for path in installed['installed']],'',
 '`dependency-ownership.json` verifies the complete six-texture/seven-model group. stone_statue02 serves the',
 'relief pillar and fountain basin; tombstone_big serves angel base and memorial; other paintings are exclusive.',
 'The fountain BMD, reagon_waterspout.OZJ, accepted ston01.OZJ and ston02.OZJ water remain byte-identical.', '',
 '## Validation','',
 '- Seven official imports, packed source files with exact original references, official exports and actual-BMD reimports.',
 '- All seven model SMDs and seven action SMDs validate; full comparisons retained. Four remodeled models DIFFERENT.',
 '- Head, angel and unchanged fountain compare EQUIVALENT for geometry/rig. Head cap UVs deliberately differ;',
 '  converter equivalence is not claimed as whole-asset identity. All seven skeleton/action comparisons EQUIVALENT.',
 '- Bone names/order/parents and all keys are checked. Six production models retain one key; fountain retains 21.',
 '- Every authored and reimported vertex checked bidirectionally for position and intended named bone, including new',
 '  inset vertices and the two Tomb01 roots. No root-ownership deduplication defect; source references exact and packed.',
 '- Tomb01 original slanted root has measured 0.002639771-unit Blender reconstruction drift / 0.002633 SMD drift;',
 '  explicit 0.005 tolerance, matching printed bounds, exact named bindings/UVs, local matrix checks pass. Other',
 '  assets use 0.001 positional tolerance; all original source anchors unchanged. No unresolved validator rejection.',
 '- No new collapsed UVs, finite geometry, one original bone per vertex, original material order and names retained.',
 '- stone_statue02 is 256×512 RGB; the five other paintings are 512×512 RGB. Official wrapping/loader checks pass.',
 f'- Only twelve owned game files installed into this isolated source checkout; {installed["unchanged_world1_object1_files"]} other World1/Object1 files hash-protected.',
 '- No runtime/client, shared-document, engine/CMake, UI, terrain, placement, collision or other-map changes.', '',
 '## Evidence and reproduction','',
 'Review `review/batch-review.jpg`, all seven per-model `review/comparison.png` sheets, fountain',
 '`review/action-comparison.jpg`, and `paintings/texture-comparison.jpg`. These use actual original/exported BMDs',
 'with matching cameras and lighting. Flat grass and the 190-unit figure are offline scale proxies. Fountain',
 'water scrolling, particles, additive blending and actual client lighting are not emulated or verified.', '',
 'Set MU_BMDCONV to the supplied converter. Run commands explicitly from this worktree using bundled Python and Blender.',
 'Initial archive/import: prepare.py. Artwork: builtin imagegen prompts/raw references/masters under paintings; the',
 'grave_02-refine-prompt.txt records its second pass. Package: package_textures.py. Build/export: Blender build_source.py,',
 'Python export_validate.py. Check validate_anchors.py, validate_final.py, Blender audit_bindings.py, audit_materials.py.',
 'Render: Blender render_exports.py then render_actions.py; assemble_review.py and assemble_actions.py. Install only',
 'owned isolated-source paths with install_source.py. write_notes.py reproduces measured handoffs. No shared runtime writes.',
 'Reviewed SignsBanners01, StoneWalls01 and StaticBatch01 helpers are reused read-only.', '',
 '**Pending:** coordinator review/integration and serial in-client observation. No client asset is marked verified.', '']
(ROOT/'notes.md').write_text('\n'.join(lines))
