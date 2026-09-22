# FireProps01 — braziers, bonfire and cobra entrance

2026-09-22. Owner: ASTRA coordinator. Branch `codex/lorencia-fire-props`; worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-fire-props`.

Four static models / 54 placements completed offline. The standing brazier's square support becomes an eight-sided forged shaft transitioning to all eight original end corners. Its basket/coal geometry stays identical. Other models retain their geometry to protect wall attachments, the bonfire additive shell and the exact cobra entrance and flame-plinth dimensions. Five texture paintings give them aged iron, charred oak and sculpted weathered stone while retaining original identity/material roles.

| Model | Placements | Triangles | Full geometry compare |
| --- | ---: | ---: | --- |
| FireLight01 | 26 | 48 → 80 | DIFFERENT |
| FireLight02 | 18 | 34 → 34 | EQUIVALENT |
| Bonfire01 | 9 | 110 → 110 | EQUIVALENT |
| DoungeonGate01 | 1 | 333 → 333 | EQUIVALENT |

All original bones, order, parents and one-key actions pass isolated skeleton/action comparison. Local rotations are checked as matrices because the bonfire has an equivalent Euler representation differing by pi. All exported model/action SMD validations and texture loader checks pass. Mesh/material order and rigid skinning are unchanged. Full model comparisons are intentionally DIFFERENT for FireLight01 and EQUIVALENT for the other three.

Printed engine bind bounds match for all four. All original anchor vertices survive the official export/reimport; maximum Blender-space drift is 0.004524231 units on legacy-v10 FireLight02, 0.001060993 on the gateway, below 0.000001 on the bonfire and zero on the standing brazier. This is reported numerical reconstruction drift, not exact byte equivalence. Source footprints are exact. The gateway retains one pre-existing collapsed UV triangle; no collapsed UV/geometry triangle is introduced. Complete audit values remain per asset.

Engine controls are retained: FireLight01 particle offset (0,0,200), FireLight02 (0,-30,60), Bonfire01 mesh1 additive plus (0,0,60), gateway flame offsets (±150,-150,140), every bone/attachment and opening. Protected material-component comparisons are separately EQUIVALENT. The bonfire's original 38-triangle effect shell and fire_02 container remain unchanged.

Original scalar alpha masks for light/fire_light_01 are enlarged bilinearly to 256 square, with original holes retained; color comes from the imagegen iron master. light2, copra_gate and fire_01 are 512-square RGB. light3.OZJ, fire_02.OZJ and shared tile_02.OZJ are byte-identical. Every consumer of the five repainted containers is inside this batch. Retained PNG masters, alpha layers, raw generations and generation-prompts.json support editing/reproduction.

Exact isolated source Data changes:

- `src/bin/Data/Object1/FireLight01.bmd`
- `src/bin/Data/Object1/FireLight02.bmd`
- `src/bin/Data/Object1/Bonfire01.bmd`
- `src/bin/Data/Object1/DoungeonGate01.bmd`
- `src/bin/Data/Object1/light.OZT`
- `src/bin/Data/Object1/light2.OZJ`
- `src/bin/Data/Object1/fire_light_01.OZT`
- `src/bin/Data/Object1/copra_gate.OZJ`
- `src/bin/Data/Object1/fire_01.OZJ`

All other 315 World1/Object1 files were hash checked unchanged in this worktree. No runtime, client, engine, CMake, UI, other map, terrain or placement writes occurred.

Review each model's `review/comparison.jpg`. Matching cameras show actual reimported exported BMDs. Wireframes and reduced-scale images are retained. Bonfire material preview approximates the documented additive blend on both original and replacement; procedural runtime fire is absent. These are offline studies, never client screenshots.

Reproduce from this exact worktree: run `prepare.py` once (immutable originals); bundled Pillow Python `package_textures.py`; Blender `build.py`; bundled Python `export_validate.py`; Blender `audit_sources.py` and `render_review.py`; bundled Python `assemble_review.py`, `install.py`, `write_notes.py`. Use Blender at `/Applications/Blender.app/Contents/MacOS/Blender` and Python at `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`. All scripts derive paths from their own deliverable directory. Official importer/exporter and the existing converter are used.

Pending: independent coordinator review, integration and actual serial 1920×1080 client observation once stable. No asset is verified in client.
