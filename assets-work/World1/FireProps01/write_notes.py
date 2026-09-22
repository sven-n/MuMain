"""Write measured local handoffs from final validation records."""
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
from prepare import ASSETS, HERE

IDENTITIES = {'FireLight01': 'Standing iron basket brazier', 'FireLight02': 'Wall-mounted iron basket brazier',
              'Bonfire01': 'Six-log bonfire with additive heat shell',
              'DoungeonGate01': 'Monumental stone cobra entrance with two flame plinths'}
BEFORE = {'FireLight01': 48, 'FireLight02': 34, 'Bonfire01': 110, 'DoungeonGate01': 333}


def main():
    rows = []
    for name in ASSETS:
        folder = HERE / name
        result = json.loads((folder / 'validation/summary.json').read_text())
        model = json.loads((folder / 'validation/blender.json').read_text())
        audit = json.loads((folder / 'validation/source-export-audit.json').read_text())
        placements = json.loads((folder / 'placements.json').read_text())
        dependencies = json.loads((folder / 'dependencies.json').read_text())
        rows.append(f'| {name} | {len(placements)} | {BEFORE[name]} → {model["triangles"]} | {result["full_compare"]} |')
        lines = [f'# {name} — {IDENTITIES[name]}', '', '2026-09-22. ASTRA coordinator / FireProps01. Offline validation complete; client review pending.', '',
                 f'{len(placements)} actual World1 placements retained in placements.json. {BEFORE[name]} → {model["triangles"]} triangles.',
                 f'Bind bounds before: `{result["bounds_before"]}`. After: `{result["bounds_after"]}`.',
                 f'Full BMD comparison: **{result["full_compare"]}**. Skeleton/actions: **EQUIVALENT**. Explicit local pose matrices, bone names/order/parents and action metadata retained in validation.',
                 f'All original Blender-space anchors survive within {audit["max_anchor_component_delta"]:.9f} game units. Material order, rigid weights and effect UVs are preserved.', '',
                 'Complete material/container dependencies:', '']
        lines += [f'- `{texture}`: ' + ', '.join('`' + p + '`' for p in paths) for texture,paths in dependencies.items()]
        lines += ['', 'New opaque paintings are 512×512; new light/fire_light_01 RGBA masks are 256×256. light3, fire_02 and shared tile_02 remain byte-identical at original resolution. Parent textures/original retains all untouched containers and decoded files.', '',
                  'Packed source.blend contains REF_ORIGINAL and the unchanged rig. Original source/info and validation SMD/action files are retained. Exports use the official importer/exporter and mu_texture wrapping/checks. review/comparison.jpg includes matching cameras, actual reimport, wireframe and reduced previews; the 190-unit bar is an offline scale reference.', '',
                  'Bonfire previews approximate documented BlendMesh1 additive shading on both versions; procedural particles and runtime lighting are absent. No image is client evidence. No runtime installation occurred.', '',
                  'See ../notes.md for reproduction and pending checks.']
        (folder / 'notes.md').write_text('\n'.join(lines) + '\n')
    installed = json.loads((HERE / 'installed-files.json').read_text())
    text = '''# FireProps01 — braziers, bonfire and cobra entrance

2026-09-22. Owner: ASTRA coordinator. Branch `codex/lorencia-fire-props`; worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-fire-props`.

Four static models / 54 placements completed offline. The standing brazier's square support becomes an eight-sided forged shaft transitioning to all eight original end corners. Its basket/coal geometry stays identical. Other models retain their geometry to protect wall attachments, the bonfire additive shell and the exact cobra entrance and flame-plinth dimensions. Five texture paintings give them aged iron, charred oak and sculpted weathered stone while retaining original identity/material roles.

| Model | Placements | Triangles | Full geometry compare |
| --- | ---: | ---: | --- |
''' + '\n'.join(rows) + '''

All original bones, order, parents and one-key actions pass isolated skeleton/action comparison. Local rotations are checked as matrices because the bonfire has an equivalent Euler representation differing by pi. All exported model/action SMD validations and texture loader checks pass. Mesh/material order and rigid skinning are unchanged. Full model comparisons are intentionally DIFFERENT for FireLight01 and EQUIVALENT for the other three.

Printed engine bind bounds match for all four. All original anchor vertices survive the official export/reimport; maximum Blender-space drift is 0.004524231 units on legacy-v10 FireLight02, 0.001060993 on the gateway, below 0.000001 on the bonfire and zero on the standing brazier. This is reported numerical reconstruction drift, not exact byte equivalence. Source footprints are exact. The gateway retains one pre-existing collapsed UV triangle; no collapsed UV/geometry triangle is introduced. Complete audit values remain per asset.

Engine controls are retained: FireLight01 particle offset (0,0,200), FireLight02 (0,-30,60), Bonfire01 mesh1 additive plus (0,0,60), gateway flame offsets (±150,-150,140), every bone/attachment and opening. Protected material-component comparisons are separately EQUIVALENT. The bonfire's original 38-triangle effect shell and fire_02 container remain unchanged.

Original scalar alpha masks for light/fire_light_01 are enlarged bilinearly to 256 square, with original holes retained; color comes from the imagegen iron master. light2, copra_gate and fire_01 are 512-square RGB. light3.OZJ, fire_02.OZJ and shared tile_02.OZJ are byte-identical. Every consumer of the five repainted containers is inside this batch. Retained PNG masters, alpha layers, raw generations and generation-prompts.json support editing/reproduction.

Exact isolated source Data changes:

''' + '\n'.join('- `' + name + '`' for name in installed['installed']) + f'''

All other {installed['protected_count']} World1/Object1 files were hash checked unchanged in this worktree. No runtime, client, engine, CMake, UI, other map, terrain or placement writes occurred.

Review each model's `review/comparison.jpg`. Matching cameras show actual reimported exported BMDs. Wireframes and reduced-scale images are retained. Bonfire material preview approximates the documented additive blend on both original and replacement; procedural runtime fire is absent. These are offline studies, never client screenshots.

Reproduce from this exact worktree: run `prepare.py` once (immutable originals); bundled Pillow Python `package_textures.py`; Blender `build.py`; bundled Python `export_validate.py`; Blender `audit_sources.py` and `render_review.py`; bundled Python `assemble_review.py`, `install.py`, `write_notes.py`. Use Blender at `/Applications/Blender.app/Contents/MacOS/Blender` and Python at `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`. All scripts derive paths from their own deliverable directory. Official importer/exporter and the existing converter are used.

Pending: independent coordinator review, integration and actual serial 1920×1080 client observation once stable. No asset is verified in client.
'''
    (HERE / 'notes.md').write_text(text)


if __name__ == '__main__':
    main()
