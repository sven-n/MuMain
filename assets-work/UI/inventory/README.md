# UI inventory — 2026-09-22

Baseline: `main` at `9a8b2027`. `interface.json` and `interface.csv` enumerate all
760 images in `src/bin/Data/Interface`: 738 wrapped textures and 22 raw TGA shop
panel tiles. `Thumbs.db` is excluded. Every image has an untouched payload in
`../original/Interface/`, dimensions, format, alpha statistics and SHA-256 hashes.
The JSON also records matching literal filename references in the C++ source.
Those search results are discovery aids, not a claim that every shipped image is
currently loaded; some names are built dynamically and some shipped UI is legacy.
`contact-01.png` through `contact-26.png` are labeled, scaled reference thumbnails.

## Selected batch

Five files from `Interface/partCharge1/`: `newui_menu03.OZJ` and
`newui_menu_Bt01.OZJ` through `newui_menu_Bt04.OZJ`. This is the prominent right HUD
panel with Character, Inventory, Friends and Menu controls. It exercises connected
metal framing, icon readability, four aligned states and the center/right/XP
layout split without touching the rest of the UI. The cash-shop button (`Bt05`)
is an unchanged adjacent dependency, outside this five-file pilot.

These are the files actually loaded at `NewUIMainFrameWindow.cpp:69–84`; similarly
named root-level `Interface/newui_menu03` and `newui_menu_Bt*` are NOT the active
batch. The active cash-shop layout macro is in `Core/Globals/Defined_Global.h:15`.

## Atlas and draw contract

All coordinates below use the top-left decoded image origin; rectangles are
half-open. Exact mappings are also machine-readable in `../source/layout.json`.

| File | Payload / allocation | Region | Logical destination |
|---|---|---|---|
| `partCharge1/newui_menu03.OZJ` | RGB JPEG 256×51 / 256×64 | x=0…104, y=0…41 | (384,429), 104×41, center transform |
| same | same | x=104…256, y=0…41 | (488,429), 152×41, right transform |
| same | same | x=0…256, y=41…51 | (384,470), 256×10, XP transform |
| `newui_menu_Bt01.OZJ` | RGB JPEG 30×164 / 32×256 | x=0…30, y=s×41…(s+1)×41 | (519,429), 30×41 |
| `newui_menu_Bt02.OZJ` | same | same | (549,429), 30×41 |
| `newui_menu_Bt03.OZJ` | same | same | (579,429), 30×41 |
| `newui_menu_Bt04.OZJ` | same | same | (609,429), 30×41 |

For a region `(sx,sy,sw,sh)` in allocation `(W,H)`, the engine uses
`u=(sx+0.5)/W`, `v=(sy+0.5)/H`, `du=(sw-1)/W`, `dv=(sh-1)/H`.
Buttons therefore use `u=0.015625`, `du=0.90625`, `dv=0.15625`,
and `v=[0.001953125,0.162109375,0.322265625,0.482421875]`.
Panel center uses `(u,v,du,dv)=(0.001953125,0.0078125,0.40234375,0.625)`;
right `(0.408203125,0.0078125,0.58984375,0.625)`;
XP `(0.001953125,0.6484375,0.99609375,0.140625)`.

`NewUIMainFrameWindow.cpp:275–307` draws these panel slices. `NewUICommon.cpp:77–119`
performs the half-texel inset. `NewUIButton.cpp:283–315,449–464` uses `state*height`
as the source Y. Each atlas repeats its border geometry in every 41-pixel cell;
the revision repaints those borders without moving their cells. The original
panel XP trough repeats short divisions within y=41…51;
it is stretched as a whole, not a nine-slice or a tiled texture. There are no
new regions or gutters. Loader padding is not part of the JPEG payload.

## States and shared dependencies

`SetButtonInfo` registers rows 0/1/2 as normal/hover/down. `SetBtnState`
(`NewUIMainFrameWindow.cpp:2683–2794`) remaps an open window to row 2 at rest and
row 3 on hover. Thus row 2 is both pressed and selected. Row 1 also supplies the
Character quest blink and Friends unread alert; Friends uses row 3 for an alert
while the window is open (`:731–799`). There is no distinct disabled image in
this configuration; no fifth frame is invented. These image IDs have no other
draw consumers beyond this HUD in the inspected baseline.

Unchanged dependencies used for the offline full-HUD context:

- `Interface/newui_menu01.OZJ`, `newui_menu02.OZJ`: left and middle frame;
  `newui_menu02-03.OZJ`: expanded skill-list overlay.
- `Interface/partCharge1/newui_menu_Bt05.OZJ`: cash-shop control at (489,429), 30×41.
- `newui_menu_red/green/blue.OZJ`: 45×39 life, poison-life and mana fills;
  `newui_menu_SD/AG.OZJ`: 16×39 shield/ability fills. All are padded by the loader.
- `newui_exbar.OZJ`, `Exbar_Master.OZJ`: normal/master experience fill;
  render-number atlas and engine text renderer provide counters/tooltips.
- Runtime skill icons and 3D item hotkeys are independently drawn over the frame.

Panel-local AG destination is (36,2), 16×39; mana is (53,3), 45×39. The revision
retains the complete backing rectangle x=36…103, y=0…41 and the first/last panel
columns in the master. The unchanged opaque gauge fills contain ornament that
must match this backing. The emerald skill well, remaining metal frame, button
backing and XP trough are repainted within the same source rectangles. The skill
well is still an empty background for the runtime skill icon. All four button
faces are repainted across their complete 30×41 cells; control geometry is fixed.
No values, numbers, labels or tooltip text are baked into replacement textures.

## Alpha and loading

`Render/Sprites/GlobalBitmap.cpp:775–889` skips the 24-byte OZJ prefix, decodes RGB,
pads to powers of two, then explicitly sets alpha=255 (`:115–127`). Black is
opaque, not transparent. All five originals and replacements are fully opaque.
Filters are linear, address modes clamp to edge. The same source rectangles, half-texel
insets and padding sizes remain in use. The expected NPOT warning from
`mu_texture.py check` is a pre-existing condition, not a rejection or an export
change. Do not resize these files to satisfy the generic power-of-two guideline.

For the wider inventory, OZT is a four-byte prefix plus a TGA payload; alpha is
reported per image. The loader reads uncompressed BGRA and flips bottom-origin
rows (`GlobalBitmap.cpp:892–978`). This pilot changes no OZT or raw TGA files.

## Offline display geometry

`UI/Scaling/UITransform.cpp:127–159`: scale is
`clamp(min(width/640,height/480),contentScale,2*contentScale)`.
At 1920×1080 and contentScale=1, HUD scale=2, Y offset=120; center X offset=320,
right X offset=640. The panel's center slice begins (1088,978), its right slice
(1616,978), and controls begin (1678,978), (1738,978), (1798,978), (1858,978).
The XP strip is x-scaled by 3 and y-scaled by 2 at y=1060. The gaps between the
left/center/right HUD bands are real consequences of these transforms; previews
must not stretch the 640-pixel frame across them. HiDPI contentScale=2 would
produce different geometry and requires separate client review.

The revision also reconstructs main `7a88d829`'s optional classic layout. That
mode uses centered X offset=320 and scale=2 for all bands, including XP, at
1920×1080/contentScale=1. The right panel begins (1296,978), and controls begin
(1358,978), (1418,978), (1478,978), (1538,978). The continuous HUD spans x=320…1600.
Both existing layout modes have labeled offline before/after previews; no engine
setting or transform is changed by the art revision.
