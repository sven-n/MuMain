# ASTRA — modern right HUD revision

2026-09-22. **Offline validated; client review pending.** Branch `art/ui-modern-pilot`
in `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-ui-modern`, based on
current main `7a88d829`. This revises the visually insufficient first pass from PR #4.
The original inventory baseline remains `9a8b2027`; untouched originals are unchanged.

## Art direction and changed assets

The first pass retained too much of the old patterned framing and reduced the new
paintings to nearly the same old appearance. This revision repaints complete
button faces and borders with smooth blue-black metal, restrained steel/brass
edges, and bold pale symbols. It is judged at actual 1080p control size, not from
the large source paintings. The selected state has a clear gold underline.

The same five game filenames are replaced:

| File under Interface/partCharge1 | Payload size | Revision |
|---|---|---|
| newui_menu03.OZJ | 256×51 | Clean emerald skill well, thin metal framing, quieter XP trough and backing. |
| newui_menu_Bt01.OZJ | 30×164 | Crisp circular inspection symbol for Character. |
| newui_menu_Bt02.OZJ | 30×164 | Legible satchel symbol for Inventory. |
| newui_menu_Bt03.OZJ | 30×164 | Clear paired-profile symbol for Friends. |
| newui_menu_Bt04.OZJ | 30×164 | Familiar power/menu symbol. |

Inventory and Friends symbols are reinterpreted for immediate recognition of the
same functions. No control, hit rectangle, tooltip or interaction behavior is changed.
The four 30×41 atlas cells remain at Y=0,41,82,123: normal, hover, pressed/selected,
selected-hover. No separate disabled state is invented.

The panel changes 5,007 master pixels versus the original. Its 68×42 AG/mana
backing is retained because the unchanged gauge-fill images contain their own
old ornament; mixing those fills with a newly shaped empty backing created a
visible mismatch. The panel's first and last columns are also retained for joins.
The old border pixels on the four buttons are fully repainted inside the same
atlas rectangles. Preserving UV boundaries does not require preserving the old
surface artwork.

## What remains fixed

- Same filenames, OZJ wrappers, RGB JPEG encoding, payload dimensions and GPU
  padded allocations (256×64 panel, 32×256 buttons).
- Same panel slices: (0,0,104,41), (104,0,152,41), (0,41,256,10).
- Same logical control positions and half-texel UV insets; see
  [inventory/README.md](inventory/README.md) and [source/layout.json](source/layout.json).
- No dynamic text, counters, status values or skill icons are baked in.
- All final pixels remain opaque, including black. Working paintings with alpha
  are explicitly composited onto black before conversion to the original RGB
  contract, avoiding colored fringes from hidden RGB.
- Quality-100, 4:4:4 baseline JPEG, wrapped with `tools/mu_texture.py`.
- No engine, CMake, World1, Object1 or shared-runtime changes.

The cash-shop button and the rest of the HUD are unchanged. This remains a
five-file visual benchmark, not a completed whole-HUD skin. The 30-pixel source
width still limits fine detail; this revision improves shape and contrast within
that constraint and does not claim HD texture resolution.

## Review artifacts

Start with [first pass versus revision at 1080p control size](previews/offline-revision-comparison-1080p.png).
Those are actual 60×82 pixel draw sizes, reconstructed from decoded OZJ exports.

- `previews/offline-1920x1080-after.png`: current default separate-anchor mode.
- `previews/offline-1920x1080-after-classic.png`: current classic centered mode.
- Matching before images, native 640×480 logical-layout comparisons, all four
  states, and light/dark opacity checks are retained in `previews/`.
- **All previews are offline mockups, not client screenshots.** Runtime text,
  item models and skill icons are omitted; unchanged gauge fills are illustrated
  roughly half full.
- At 1920×1080/contentScale=1 the HUD scale is 2×. Default mode stretches XP
  horizontally 3×; classic mode centers the complete HUD at 2× including XP.
  This models the existing main-branch option without changing client code.

## Editable sources and reproduction

All 760 original images and 26 inventory contact sheets remain intact.
`previous/Interface/partCharge1/` retains the first-pass JPEGs for honest comparison.
Historical first-pass documentation is in [notes-v1.md](notes-v1.md).

Five new painted sources were made with the **built-in imagegen tool**.
[source/prompts.json](source/prompts.json) preserves their prompts;
`prompts-v1.json` preserves the earlier prompts. `source/editable/*.ora` contains
native-size original and masked painting layers. `source/masks/`, `masters/`,
`payloads/` and `exports/` retain masks, lossless PNGs, final JPEGs and wrapped OZJs.

With Python, Pillow and numpy installed, run from this worktree:

```sh
python3 assets-work/UI/scripts/assemble.py
python3 assets-work/UI/scripts/preview.py
python3 assets-work/UI/scripts/validate.py --install
```

The bundled interpreter on this Mac is
`/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`.
Scripts recreate deterministic master/export bytes from the retained paintings;
image generation itself is not deterministic. Installation is guarded to this
branch and this worktree's source Data paths. The shared runtime is never addressed.

## Validation and pending client checks

All five exports pass the loader-format checks with exit 0 and the same five
pre-existing non-power-of-two warnings as the originals. Exact dimensions,
alpha=255, wrapping, state ordering, protected panel pixels and 760 original
payload hashes are verified. Decoded JPEG error is at most 4/255 per channel.
The editable layer composites and repeated assembly are checked separately;
reports are in `validation/`.

Visual inspection covers all states at actual 1080p size, classic/default full
layouts and light/dark backgrounds. Actual client loading, input, selected/alert
feedback, localized text, overlaid skills/counters, HiDPI scaling and motion
readability remain pending. This revision does not attempt a stability fix.
