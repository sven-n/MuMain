"""Record measured model contracts and batch handoff without editing shared documents."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parent
IDENTITIES={'Sign01':'Hanging notice board, animated wood bracket and iron ornament',
 'Sign02':'Standing notice board with two posts', 'Curtain01':'Blue laurel heraldic standard',
 'StoneWall04':'Yellow and purple mounted-rider heraldic standard',
 'StoneWall06':'Accepted masonry pier with shared mounted-rider banner; compatibility only'}
rows=[]
for name,identity in IDENTITIES.items():
    folder=ROOT/name
    source=json.loads((folder/'validation/blender.json').read_text())
    summary=json.loads((folder/'validation/summary.json').read_text())
    anchors=json.loads((folder/'validation/modular-anchors.json').read_text())
    final=json.loads((folder/'validation/final-contract.json').read_text())
    motion=json.loads((folder/'validation/local-motion.json').read_text())
    placements=json.loads((folder/'original/placements.json').read_text())
    count=source['triangles']; before=source['original_triangles']
    lines=[f'# {name}: {identity}', '', 'Owner: ASTRA reviewer/production worker. Branch: `codex/lorencia-signs-banners`.',
      'Status: offline production complete; coordinator visual review and integration pending.', '',
      f'World1 placements: {len(placements)}; complete untouched transforms in `original/placements.json`.',
      f'Game path: `src/bin/Data/Object1/{name}.bmd`.', f'Triangles: {before} → {count}.',
      f'Before bind bounds: `{summary["bounds_before"]}`.',f'After bind bounds: `{summary["bounds_after"]}`.', '',
      'Complete texture dependencies in preserved engine mesh order:', '']
    for texture in summary['material_order']:
        detail=final['textures'][texture]
        container=Path(texture).stem+('.OZT' if texture.endswith('.tga') else '.OZJ')
        disposition='frozen byte-identical' if container=='tile_01.OZJ' else 'shared painted texture'
        lines.append(f'- `{container}` / `{texture}`: {detail["dimensions"]}, {detail["mode"]}; {disposition}.')
    lines+=['', 'Original origins, orientation, footprint and every original vertex are retained.',
      'Signs have 0.25-unit panel recesses inside unchanged original wood boundaries; the two panel interiors',
      'gain explicit edge profiles. Iron ornament, suspension rings, poles, cloth, stone and attachment triangles',
      'retain position, bone and UV corners. No placement, collision or terrain edits.', '',
      f'Full BMD comparison: **{summary["full_compare"]}**. Rig/actions: **{summary["rig_actions"]}**.',
      f'Bone names/order: `{source["bone_order"]}`.',f'Action metadata: `{summary["action_metadata"]}`.',
      'Exact node-name/order/parent and action-count/key-count/lock checks pass.',
      f'Max exported original-corner component drift: {anchors["max_original_corner_position_component_error"]:.9f} units.',
      f'Retained triangle count: {anchors["unchanged_triangle_count"]}; max retained UV error: {anchors["max_unchanged_uv_component_error"]}.',
      f'Max retained normal direction change: {anchors["max_retained_normal_direction_degrees"]:.9f} degrees.',
      'Official SMD export normalizes/requantizes normals; no byte-equivalence of recomputed normals is claimed.', '']
    for item in motion['samples']:
        lines.append(f'- `{item["file"]}`: {item["frames"]} frames / {item["bone_samples"]} bone samples; '
          f'max local position drift {item["max_local_position_component"]:.9f}, '
          f'max local rotation-matrix component drift {item["max_rotation_matrix_component"]:.9f}.')
    if name=='StoneWall06':
        lines+=['', 'Compatibility export is the accepted BMD from baseline 0800af23 (accepted StoneWalls01 revision 149780be),',
          'retained byte-identically together with tile_01.OZJ. Its official Blender roundtrip is retained separately',
          'as `validation/official-roundtrip.bmd`; it is evidence only and is not installed. Only shared banner/pole',
          'textures change in this compatibility model. Source and matching render reflect the accepted 88-triangle pier.']
    lines+=['', '`source.blend` is packed with excluded `REF_ORIGINAL` and `REF_HIGH_POLY` references.',
      'Untouched originals, official original import, textures, info and SMDs are in `original/`.',
      'Full comparisons, validated model/action SMDs, skin/UV/material checks, local motion and source audits',
      'are in `validation/`. The exact original scalar alpha fields are bilinearly enlarged; four RGBA masks',
      'are pixel-checked and keep padded hidden RGB. Editable artwork, prompts and raw generations are in `../paintings/`.',
      '', '`review/comparison.png` contains matching-camera actual BMD before/after renders, wireframe and',
      'reduced-scale views. Animated models also include `review/action-comparison.jpg` at beginning/middle/end.',
      'Both original and final review materials approximate the engine alpha >0.25 test plus scalar blend.',
      'This is offline Blender evidence, not proof of client shading, placement or animation playback.', '',
      '**Pending:** serial client observation. No runtime installation or client verification performed.', '']
    (folder/'notes.md').write_text('\n'.join(lines))
    rows.append(f'| {name} | {len(placements)} | {before} → {count} | {len(source["bone_order"])} | {summary["action_metadata"][0]} | {summary["full_compare"]} |')
install=json.loads((ROOT/'source-installation.json').read_text())
lines=['# SignsBanners01 offline handoff', '', 'Owner: ASTRA reviewer/production worker; branch `codex/lorencia-signs-banners`.',
 'Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-signs-banners`.',
 'Four newly completed models cover 26 World1 placements. StoneWall06 is a read-only compatibility model',
 'covering two additional placements of shared heraldry. Coordinator integration and client verification pending.', '',
 '| Model | Placements | Triangles before → after | Bones | Original action | Full comparison |',
 '|---|---:|---:|---:|---|---|', *rows, '',
 'The two notice boards keep their ochre plaque, original abstract pseudo-lettering, fasteners and oak utility',
 'atlas. A shallow 0.25-unit panel recess adds a clear edge without moving any original boundary. Muted forged',
 'iron supports and tarnished pole hardware match the accepted ironwork/tavern palette. Blue laurel and yellow',
 'mounted-rider symbols retain their identities, layout and cloth topology. No new insignia or readable text.', '',
 '## Changed game files', '', *[f'- `{path}`' for path in install['installed']], '',
 'The complete dependency map assigns notice to Sign01/02, signboard and doorknob to Sign01, badge_01 to',
 'Curtain01/StoneWall04/StoneWall06, badge_02 to Curtain01, and badge_03 to StoneWall04/StoneWall06.',
 '`dependency-ownership.json` verifies these are all consumers. StoneWall06.bmd and tile_01.OZJ remain',
 'byte-identical to the accepted baseline, both in Data and compatibility exports.', '',
 '## Validation and limits', '',
 '- Official importer/exporter used for all five sources; original imports and packed editable sources retained.',
 '- All five model SMDs and five action SMDs validate; actual BMD info and full comparison logs retained.',
 '- Sign01/02 full comparison is DIFFERENT for explicit geometry remodeling. Curtain01, StoneWall04 and the',
 '  accepted StoneWall06 compatibility export compare EQUIVALENT. All five rig/action comparisons EQUIVALENT.',
 '- Every local transform for all original keys passes translation <0.0001 and matrix component <0.00001.',
 '- Every original vertex/bone and all unchanged triangle UVs retained. Maximum original-corner export drift',
 '  is 0.000130 units in the animated StoneWall04 hierarchy; source anchors are exactly unchanged.',
 '- Material order, one rigid original bone per vertex, finite geometry/UVs, alpha convention and dimensions pass.',
 '- notice512²; signboard512×256; doorknob128²; badge_01 64×512; badge_02 256×512; badge_03 512².',
 '- Four final alpha textures are exact bilinear resamples of the original scalar masks. Filtering RGB is padded',
 '  from nearest alpha>=128 texels; masks themselves are never thresholded or repainted. Editable OpenRaster',
 '  files retain generated diffuse, mask and composited final layers; full generated PNG masters and prompts remain.',
 '- Legacy signboard has a detached ghost ring at alpha<=10/255. Raw Cycles blending initially exaggerated this',
 '  low-alpha debris after painting. Both review sides now approximate the documented RGBA >0.25 alpha test',
 '  with scalar blending above threshold. Game alpha is unchanged. See engine-contract-audit.txt and alpha-audit.json.',
 f'- Source installation changed only ten owned game paths; {install["unchanged_world1_object1_files"]} other World1/Object1 files were hash-protected.',
 '- No runtime/client, engine/CMake, UI, terrain, collision, placement or shared-document edits.', '',
 '## Evidence and reproduction', '',
 'Start with `review/batch-review.jpg`, all five `review/comparison.png` sheets, four `review/action-comparison.jpg`',
 'sheets and `paintings/alpha-review.jpg`. Cameras and lighting match before/after; previews use actual exported',
 'BMD imports. Flat grass and 190-unit figure are scale proxies, not actual World1 terrain or client observations.',
 '', 'Set `MU_BMDCONV` to the supplied absolute bmdconv binary. Use the supplied Blender and Pillow Python.',
 '`prepare.py` is initial-only; `paintings/*-prompt.txt` records builtin imagegen edits of the inspected originals.',
 'Run package_textures.py; Blender build_source.py and export_validate.py; validate_anchors.py, validate_final.py,',
 'audit_alpha.py; Blender render_exports.py followed sequentially by render_actions.py; assemble_review.py,',
 'assemble_actions.py; install_source.py; write_notes.py. All commands must use this worktree explicitly.',
 'Reviewed scripts from StoneWalls01 and StaticBatch01 are reused read-only; own scripts and outputs stay here.',
 '', '**Pending:** coordinator visual acceptance/integration and actual serial client review. No known blocking defect.', '']
(ROOT/'notes.md').write_text('\n'.join(lines))
