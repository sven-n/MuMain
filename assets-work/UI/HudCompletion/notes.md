# ASTRA — remaining bottom HUD

2026-09-22. Branch `codex/ui-hud-completion`, isolated worktree
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-ui-hud`, from merged
main `0f589224`. **Offline validated; client verification pending.**

The user requested the other HUD elements after merging PR #9. This extends
the selected clean, restrained dark-fantasy direction to the complete bottom
frame: quiet blue-black metal, thin steel/brass trim, readable faceted life/mana
reservoirs, distinct resource colors, consistent slots and a matching coin-purse
shop control. The four approved right-side controls are unchanged.

## Changed files

Fourteen source Data files under `Interface/` are replaced:

- `newui_menu01.OZJ`, `newui_menu02.OZJ`, `partCharge1/newui_menu03.OZJ`:
  item/skill slots, connected rails, matching empty gauge backings and XP trough.
- `newui_menu02-03.OZJ`: alternate 6–0 key-bank background.
- `newui_menu_red.OZJ`, `newui_menu_green.OZJ`, `newui_menu_blue.OZJ`:
  life, poison-life and mana versions of one aligned painted reservoir.
- `newui_menu_SD.OZJ`, `newui_menu_AG.OZJ`: gold shield and violet ability columns.
- `newui_Exbar.OZJ`, `Exbar_Master.OZJ`: green normal and cyan master XP fills.
- `partCharge1/newui_menu_Bt05.OZJ`: four-state coin-purse shop button.
- `newui_skillbox.OZJ`, `newui_skillbox2.OZJ`: normal/selected skill backgrounds.

The skill backgrounds are also shared with MU Helper and the pet window; see
[inventory and exact atlas mappings](inventory/README.md). New dynamic numbers,
labels or skill images are not baked in. Existing fixed key legends are retained
as compact labels; all live values and overlays still come from the engine.

The earlier panel's preserved old gauge backing can now be replaced because
the corresponding fill textures are included in this batch. Empty and full
versions are assembled from the same painted component and energy masks; their
frame pixels match exactly in the lossless masters.

## Contract and validation

Every filename, container format, dimension, GPU padding allocation, atlas cell,
source slice and control rectangle is unchanged. All 14 are fully opaque RGB
JPEG inside OZJ; black is opaque. Working imagegen alpha is composited on black.
Exports use quality-100 4:4:4 baseline JPEG and `tools/mu_texture.py wrap`.

`validation/report.json` records all loader checks, dimensions, alpha, hashes,
wrapper round trips, state contrast, shared backing alignment and editable-layer
composites. All checks pass with only each original's existing NPOT warning.
JPEG error is at most 4/255 per channel. All 760 untouched UI payload hashes and
14 merged-baseline container/payload pairs are verified. Reassembly reproduces
42 master/payload/export hashes. The 10% life/mana/SD/AG previews must contain
visible colored energy, not just a changed border.

No engine, CMake, World1, Object1, main-checkout or shared-runtime files are
changed. The installer checks branch, resolved Data paths, original/current
target hashes and the exact 14-file source scope before writing this worktree's
source Data. No client launch or build is performed.

## Review

Start with [merged pilot versus completed bottom frame at 1080p](previews/offline-hud-comparison-1080p.png).
All previews are explicitly labeled **offline mockups, not client screenshots**.
They sample decoded OZJ payloads with the inspected padded allocations and UVs.

Nine previews include full 1920×1080 anchored/classic before/after pairs, native
geometry, light/dark edge inspection, all cash states, normal/selected skill
slots, all 14 atlases and 0/10/25/50/75/100% fill levels. Poison life and master XP
are included. Example skills are unchanged atlas cells at the existing icon
coordinates; they illustrate overlap and are not a captured character loadout.
Item models and dynamic counters are omitted.

The source sizes remain small (45×39 reservoirs, 16×39 resource columns, 32×38
skill slots, 30×41 button cells); this improves artwork inside that constraint.
The 1080p mockups use the engine's existing 2× HUD scale, not larger game textures.

## Sources and reproduction

Four new raster paintings were produced with the **built-in imagegen tool**:
an empty metal slot, coin-purse control, ruby reservoir and gold energy column.
[source/prompts.json](source/prompts.json) retains every prompt and output path;
the PNG paintings are copied into this worktree. Color variants and exact atlas
assembly are deterministic Python operations. `source/layout.json` retains the
crop/placement contract and `source/editable/` contains 14 two-layer OpenRaster
files with untouched baseline and painting layers. Lossless masters, JPEGs,
wrapped exports, originals, inventory and contact sheets are retained locally.

With Python 3.12, Pillow and numpy, run from this worktree:

```sh
python3 assets-work/UI/HudCompletion/scripts/inventory.py
python3 assets-work/UI/HudCompletion/scripts/assemble_hud.py
python3 assets-work/UI/HudCompletion/scripts/preview_hud.py
python3 assets-work/UI/HudCompletion/scripts/validate_hud.py --rebuild --install
```

On this Mac the interpreter is
`/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`.
The first inventory command refuses to overwrite differing retained originals.
The scripts reuse the pilot's OpenRaster writer, state treatment and CPU sampler;
they do not regenerate or overwrite the earlier pilot's deliverables.

## Pending client checks

Verify real loading and GPU filtering, gauge cut lines at low/partial/full values,
poison changes, XP/master XP and gain flashes, all shop states, normal/alternate
hotkeys, selected and disabled skills, cooldown overlays, item models/counts,
localized tooltips and resource/XP digits. Check the shared skill backgrounds in
MU Helper and the pet window. Capture both HUD layouts and HiDPI scaling on a
real gameplay background. Client review remains pending under the existing
offline-production authorization; no stability work is included.
