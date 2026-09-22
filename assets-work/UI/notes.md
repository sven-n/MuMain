# ASTRA — right HUD art pilot

2026-09-22. **Offline validated; client verification pending.** Worktree:
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-ui-pilot`, branch
`art/ui-pilot`, based on `main` at `9a8b2027`. The World1 checkout and shared
runtime have not been written to. No engine, CMake, World1 or Object1 changes.

## What changed

| Source Data file, under `Interface/partCharge1/` | Size | Treatment |
|---|---|---|
| `newui_menu03.OZJ` | 256×51 | Subdued green worn-stone skill-well interior and aged exposed trim. Conservative partial repaint: 1,083 of 13,056 master pixels change. |
| `newui_menu_Bt01.OZJ` | 30×164 | Character icon retained, worn iron frame and quiet dark inset. |
| `newui_menu_Bt02.OZJ` | 30×164 | Inventory icon retained, same material family. |
| `newui_menu_Bt03.OZJ` | 30×164 | Friends icon retained, same material family. |
| `newui_menu_Bt04.OZJ` | 30×164 | Menu/power symbol retained, same material family. |

The controls have cleaner light/dark separation, restrained brass accents and
reduced background noise. This is a benchmark for the material language, not a
full-HUD repaint. The cash-shop button, left/center panels, colorful gauge fills,
skill icons, counters and XP textures remain original. The active assets live in
`partCharge1`; the similarly named root-level Interface files are untouched.

## Layout, alpha and edge handling

See [inventory/README.md](inventory/README.md) for engine references, normalized
UVs, dependencies and 1920×1080 transforms, and [source/layout.json](source/layout.json)
for exact source/destination rectangles and protected areas.

- Every button retains four 30×41 cells at Y=0,41,82,123. They mean normal,
  hover, pressed/selected and selected-hover. No separate disabled texture is
  registered by this HUD. Quest/mail alerts reuse hover states.
- The same normal painting produces all four states through fixed light/color
  adjustments, so there is no generated position drift between states.
- Original two-pixel side guards, top rail, cell boundaries and bottom edges
  remain byte-identical in the lossless button masters. Only x=2…27,y=4…38
  within each cell is repainted. Icon meanings and control hit rectangles stay
  the same; no click offset or geometry is introduced.
- Panel slices remain x=0…103 / x=104…255, y=0…40; its XP slice is y=41…50.
  Mana/AG rectangles, green well border, black backing, XP strip, outside borders
  and guards around x=104 are protected. The empty green well contains no icon,
  label or number; runtime skill content still has its original space.
- All five payloads are RGB JPEG in OZJ, with a repeated 24-byte JPEG prefix.
  Alpha is 255 everywhere, including black. No transparency, premultiplication,
  color key or new fringe has been introduced. Light/dark background previews
  explicitly demonstrate the opaque contract.
- The original non-power-of-two payload dimensions are intentional here:
  256×51 allocates 256×64, and 30×164 allocates 32×256. Existing half-texel UV
  insets and clamp-to-edge linear sampling are unchanged. No file is enlarged.
- JPEG exports use quality 100, 4:4:4, baseline encoding. Protected pixels are
  exact in PNG/ORA masters; decoded JPEG channel differences from those masters
  are at most **4/255**. This is measured, not an assertion of lossless JPEG.

## Deliverables

- `inventory/interface.csv` and `.json`: 760 UI images, paths, dimensions,
  alpha statistics, original hashes and literal source references. The 738
  wrapped images were unwrapped with `mu_texture.py`; 22 raw shop TGA tiles were
  copied unchanged. `Thumbs.db` is not an image and was excluded.
- `original/Interface/`: all untouched image payloads. `original/containers/`:
  byte-for-byte backups of the five selected original OZJ containers.
- `inventory/contact-01.png` … `contact-26.png`: labeled original contact sheets;
  `pilot-original-atlases.png`: enlarged atlas/state inspection reference.
- `source/*-generated.png`: five retained high-resolution painted inputs, made
  with the **built-in imagegen tool**. They are working paintings, not runtime
  textures. [source/prompts.json](source/prompts.json) preserves every final prompt
  and input reference. Prompt theme: aged charcoal iron, tarnished silver,
  restrained brass, worn stone/leather; fixed layout and familiar symbols; no text.
- `source/editable/*.ora`: native-size OpenRaster projects, with an untouched
  reference layer and a masked painted layer. Open with Krita, GIMP or another
  OpenRaster editor. `source/masks/` holds the exact protection masks.
- `masters/`: lossless native-size PNGs. `payloads/`: final JPEGs. `exports/`:
  wrapped OZJ game files, installed identically in this worktree's `src/bin/Data`.
- `previews/`: decoded-export native before/after, all four state comparisons,
  light/dark opacity inspection and separate 1920×1080 before/after mockups.
  **Every preview is an offline reconstruction, not a client screenshot.**
- `validation/`: machine report, original/export `mu_texture.py check` output,
  environment versions and visual review notes.

## Reproduce the assembly and checks

Run from this worktree, with Python, Pillow and numpy available (versions used
are in `validation/environment.txt`). On the current Mac the bundled interpreter
is `/Users/webproduktion3/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3`.

```sh
python3 assets-work/UI/scripts/assemble.py
python3 assets-work/UI/scripts/preview.py
python3 assets-work/UI/scripts/validate.py
python3 assets-work/UI/scripts/validate.py --install
```

Assembly uses the retained generated inputs and creates the same lossless pixels
and JPEG/OZJ bytes. Generating new paintings from the prompts is not deterministic.
The scripts write under `assets-work/UI`; only the explicit `--install` step
writes the five selected source Data files, after successful checks and a branch/
path guard. They never address the shared runtime. `inventory.py` can regenerate
contact sheets from the retained original inventory without re-indexing installed
replacements. Do not unwrap installed replacements into `original/`.

## Validation and client review

All five exports passed size, mode, alpha, protected-pixel, state-order, prefix,
wrap/unwrap and decoder checks. All 760 original payload hashes match the retained
inventory. `mu_texture.py check` returned exit 0 for every original and export,
with the **same five baseline NPOT warnings**, and no rejection. These are not
warning-free power-of-two textures; changing them would violate the requested
pixel contract. Installation is restricted to this worktree's source Data.

The mockup uses inspected engine draw rectangles and half-texel bilinear sampling
at contentScale=1. Its 1920×1080 HUD is 2× with separate left/center/right anchors;
the XP strip is 3× horizontally. It is not a GPU render. Neutral backgrounds and
roughly half-full unchanged gauges are illustrative. Runtime item/skill content,
numeric text, tooltips, animations and scene lighting are omitted. Existing hotkey
letters and numerals visible in unchanged original panels are not new painted text.

Pending client checks when a stable capture is available: loading without errors;
native and HiDPI scaling; hover/click/selected feedback; quest/mail blinking;
tooltips and localized labels; skill icons and counters over the reserved areas;
empty/full/poison life, mana and AG; normal/master XP; and seams during window
resizing. Compare the right controls against the unchanged cash-shop button and
other HUD regions before extending this benchmark. No stability fix was attempted.
