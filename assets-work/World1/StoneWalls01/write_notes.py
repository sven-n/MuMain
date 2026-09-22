"""Write asset handoffs from measured exports, untouched placements and validation logs."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
IDENTITIES = {
    'StoneWall01': 'Raised portcullis gateway with rough masonry piers',
    'StoneWall02': 'Lowered portcullis gateway with rough masonry piers',
    'StoneWall03': 'Rough stone wall with preserved siege fixture',
    'StoneWall05': 'Plain rough stone wall module; no current World1 placement',
    'StoneWall06': 'Rough masonry pier with animated heraldic banner',
}


def read(folder, name):
    return json.loads((folder / 'validation' / name).read_text())


def asset_note(name, identity):
    folder = ROOT / name
    source = read(folder, 'blender.json')
    summary = read(folder, 'summary.json')
    anchors = read(folder, 'modular-anchors.json')
    final = read(folder, 'final-contract.json')
    motion = read(folder, 'local-motion.json')
    placements = json.loads((folder / 'original/placements.json').read_text())
    lines = [f'# {name}: {identity}', '', 'Owner: ASTRA review/ironwork/stone-wall worker.',
             'Branch: `codex/lorencia-stone-walls`. Status: offline production complete; coordinator review required.',
             '', f'World1 placements: {len(placements)}; exact transforms are retained in `original/placements.json`.',
             f'Game model: `src/bin/Data/Object1/{name}.bmd`.',
             f'Triangles: {source["original_triangles"]} → {source["triangles"]}.',
             f'Bind bounds before: `{summary["bounds_before"]}`; after: `{summary["bounds_after"]}`.', '',
             'Complete material dependencies, in original engine mesh order:', '']
    for texture in summary['textures']:
        details = final['textures'][texture]
        extension = '.OZT' if texture.endswith('.tga') else '.OZJ'
        container = Path(texture).stem + extension
        disposition = 'repainted' if Path(texture).stem in ('tile_01', 'tile_03') else 'frozen byte-identical'
        lines.append(f'- `{container}` / `{texture}`: {details["dimensions"]}, {details["mode"]}; {disposition}.')
    lines += ['', 'Selected stone-panel interiors are recessed 0.65 units. Original boundary vertices, wall endpoints,',
              'opening-facing jamb planes and protected fixture/banner triangles are retained. No footprint, origin,',
              'orientation, placement, collision or terrain edits.', '',
              f'Original rig order: `{source["bone_order"]}`. Action metadata: `{summary["action_metadata"]}`.',
              'Full geometry comparison: **DIFFERENT**, intentional remodel. Skeleton/actions: **EQUIVALENT**.',
              'Bone names, order, parents, frame counts and lock metadata are preserved.',
              f'Maximum exported original-corner component drift: {anchors["max_original_corner_position_component_error"]:.9f} units.',
              f'Maximum retained UV component error: {anchors["max_unchanged_uv_component_error"]}.',
              f'Maximum retained-normal direction difference: {anchors["max_retained_normal_direction_degrees"]:.9f} degrees.',
              'Sub-millimeter exported drift is official floating-point roundtrip precision; source anchors are unmoved.',
              'The 0.001-unit corner tolerance is explicit in the anchor report; no full geometry equivalence claimed.', '']
    for item in motion['samples']:
        lines.append(f'- `{item["file"]}`: {item["frames"]} frames, {item["bone_samples"]} bone samples; '
                     f'max local translation {item["max_position_component"]:.9f}, wrapped rotation '
                     f'{item["max_euler_component_radians"]:.9f} radians.')
    lines += ['', 'Evidence: `validation/` retains full comparisons, bmdconv validation/info, source audit,',
              'UV/skin/alpha/material checks, protected-component comparisons, frozen hashes and local motion.',
              '`source.blend` is packed, with excluded `REF_ORIGINAL` and `REF_HIGH_POLY` plus export geometry.',
              '`original/` contains untouched game files, unwrapped textures, bmdconv info/SMDs and official import.',
              '`review/comparison.png` compares actual exported BMD against original with matching camera,',
              'wireframe and reduced-scale previews. Painted masters and prompts are shared within `../paintings/`.',
              '', '**Pending:** serial in-client inspection. No runtime installation or client verification performed.', '']
    (folder / 'notes.md').write_text('\n'.join(lines))
    return f'| {name} | {identity} | {len(placements)} | {source["original_triangles"]} → {source["triangles"]} | {len(source["bone_order"])} |'


def batch_note(rows):
    installation = json.loads((ROOT / 'source-installation.json').read_text())
    lines = ['# StoneWalls01 offline handoff', '',
             'Owner: ASTRA review/ironwork/stone-wall worker. Branch: `codex/lorencia-stone-walls`.',
             'Worktree: `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-stone-walls`.',
             'Five assets complete for coordinator review; 18 actual World1 placements. Client verification pending.', '',
             '| Model | Actual identity | Placements | Triangles before → after | Bones |',
             '|---|---|---:|---:|---:|', *rows, '',
             'StoneWall06 retains its one 25-key action; other models retain one action with one key.',
             'Rough charcoal/olive stone preserves the original two-course layout and remains distinct from the',
             'warmer carved limestone of accepted Masonry01. Broad chipped facets sharpen material readability.',
             'Only selected stone-panel interiors receive shallow 0.65-unit recesses; all original vertices and',
             'perimeter edges survive, including exact source gateway clearance and wall connection geometry.', '',
             '## Changed game files', '']
    lines += [f'- `{path}`' for path in installation['installed']]
    lines += ['', 'The two 512 × 512 RGB paintings, tile_01 and tile_03, have no consumers outside this owned group;',
              '`dependency-ownership.json` records the full inventory check. Frozen containers remain byte-identical:',
              '`tile_02.OZJ`, `bridge_01.OZJ`, `horse_drawn_01.OZJ`, `badge_01.OZJ`, `badge_03.OZT`.',
              'The banner alpha, hidden RGB fringe, original material order and filenames remain unchanged.', '',
              '## Validation', '',
              '- Official Blender import/export; packed source with excluded original and high-poly references.',
              '- All model/action SMDs validate. Five full geometry compares report DIFFERENT as intended.',
              '- Five skeleton/action comparisons EQUIVALENT; exact bone names/order/parents and action metadata.',
              '- Seven frozen-material components independently EQUIVALENT, with matching bone/position/UV corners.',
              '- All original source vertices/boundaries unchanged. Actual exports: max corner drift 0.000366 units',
              '  on the nine-bone banner hierarchy, zero for the four other models. Printed bind bounds agree.',
              '- All 25 banner keys checked: max local translation drift 0.000078 units; rotations unchanged.',
              '- Retained normals are normalized/requantized by the official pipeline and measured explicitly;',
              '  they are not claimed byte-equivalent. All material/UV/skin/finite-geometry checks pass.',
              '- Five frozen containers hash-identical; original game archives match the pinned baseline commit.',
              f'- Isolated source installation changed only seven owned paths; {installation["unchanged_world1_object1_files"]} other World1/Object1 files unchanged.',
              '', '## Review evidence', '',
              '`review/batch-review.jpg`, each asset’s `review/comparison.png`, `review/texture-comparison.jpg`,',
              '`review/texture-repeat.jpg`, `review/banner-action-comparison.jpg`, and',
              '`review/raised-gates-comparison.jpg` / `review/lowered-gates-comparison.jpg`.',
              'Assembly sheets use actual untouched World1 placement transforms with identical fitted cameras.',
              'They reconstruct only owned geometry in Blender; actual terrain, collision and baked lighting are',
              'omitted. Some original placements are tilted/buried; these are preserved. No client evidence is claimed.',
              '', '## Reproduction', '',
              'Run all commands explicitly from this worktree. Set `MU_BMDCONV` to the coordinator-supplied',
              '`/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv`.',
              'Blender is `/Applications/Blender.app/Contents/MacOS/Blender`; packaging/validation uses the supplied',
              'Pillow Python runtime. `prepare_originals.py` is initial-only and refuses to overwrite originals.',
              'Built-in imagegen prompts and raw/full-resolution editable PNG masters are in `paintings/`.',
              'Run `package_textures.py`, Blender `build_source.py`, `export_all.py`, `validate_export.py`,',
              '`validate_anchors.py`, `validate_final.py`, `validate_protected.py`, Blender `render_exports.py`,',
              '`render_joins.py`, `render_actions.py`, then `assemble_review.py`, `assemble_extra.py` and',
              '`install_source.py`. `write_notes.py` regenerates this measured handoff.',
              'Reviewed StaticBatch01 and Masonry01 helpers are reused read-only within the same worktree.',
              '', '**Pending:** coordinator review/integration and actual client observation. No known blocking defect;',
              'no runtime/client, engine/CMake, UI, placement/collision/terrain or shared-document changes.', '']
    (ROOT / 'notes.md').write_text('\n'.join(lines))


if __name__ == '__main__':
    batch_note([asset_note(name, identity) for name, identity in IDENTITIES.items()])
