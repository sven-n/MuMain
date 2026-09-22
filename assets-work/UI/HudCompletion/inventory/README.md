# Remaining bottom-HUD inventory

Baseline: merged main `0f589224` (PR #9). `assets.json` records exact filenames,
dimensions, GPU allocation, format/alpha, hashes and original validator warnings.
`baseline-contact.png` shows the retained merged-baseline inputs; the separate
`original-candidates.png` shows the older stock art for context. Containers and
untouched JPEG payloads are retained in `../original/`.

This batch covers 14 connected textures. The empty gauge backings are embedded
in the panel atlases and the fill textures are opaque, so panels and fills must
be repainted together. The previously approved four right-side controls remain
unchanged. Skill-icon atlases, disabled skill icons, number atlases, item models,
chat, minimap and unrelated window art are outside this batch.

## Files and atlas regions

All paths below are under `src/bin/Data/Interface/`. All are OZJ-wrapped RGB JPEG,
fully opaque including black. Coordinates are top-left, rectangles half-open.

| File | Payload / allocation | Source → logical destination |
|---|---|---|
| `newui_menu01.OZJ` | 256×51 / 256×64 | (0,0,152,41) → (0,429,152,41), left; (152,0,104,41) → (152,429,104,41), center; (0,41,256,10) → (0,470,256,10), XP |
| `newui_menu02.OZJ` | 128×51 / 128×64 | (0,0,128,41) → (256,429,128,41), center; (0,41,128,10) → (256,470,128,10), XP |
| `partCharge1/newui_menu03.OZJ` | 256×51 / 256×64 | (0,0,104,41) → (384,429,104,41), center; (104,0,152,41) → (488,429,152,41), right; (0,41,256,10) → (384,470,256,10), XP |
| `newui_menu02-03.OZJ` | 160×42 / 256×64 | (0,0,160,40) → (222,429,160,40), center, alternate key bank. Last two source rows are not sampled by this draw. |
| `newui_menu_red.OZJ` | 45×39 / 64×64 | Life fill at (158,432), 45×39; source and destination are cropped vertically with remaining life. |
| `newui_menu_green.OZJ` | 45×39 / 64×64 | Same life geometry while poisoned. |
| `newui_menu_blue.OZJ` | 45×39 / 64×64 | Mana fill at (437,432), 45×39. |
| `newui_menu_SD.OZJ` | 16×39 / 16×64 | Shield fill at (204,431), 16×39. |
| `newui_menu_AG.OZJ` | 16×39 / 16×64 | Ability fill at (420,431), 16×39. |
| `newui_Exbar.OZJ` | 6×4 / 8×4 | Normal XP: (0,0,6,4) → (2,473,629×progress,4). |
| `Exbar_Master.OZJ` | 6×4 / 8×4 | Same geometry for master XP. |
| `partCharge1/newui_menu_Bt05.OZJ` | 30×164 / 32×256 | Four 30×41 cells at y=0,41,82,123 → (489,429,30,41), right transform. |
| `newui_skillbox.OZJ` | 32×38 / 32×64 | Full cell, normal skill selection/list background. |
| `newui_skillbox2.OZJ` | 32×38 / 32×64 | Full cell, selected skill background. |

The engine spells `newui_exbar.jpg`, `newui_menu_ag.jpg` and `newui_menu_sd.jpg`
in code; the actual Git paths above preserve their existing case. No rename is
needed. All loaders use GL_LINEAR and the default GL_CLAMP_TO_EDGE.

## UV and repeated sections

Panels, cash-shop states and skill slots use the inset source mapping
`u=(sx+0.5)/W, v=(sy+0.5)/H, du=(sw-1)/W, dv=(sh-1)/H`, where W/H are the padded
allocation. Cash uses u=0.015625, du=0.90625, dv=0.15625 and
v=0.001953125/0.162109375/0.322265625/0.482421875. Skill slots use
u=0.015625, v=0.0078125, du=0.96875, dv=0.578125.

For gauges, missing fraction m selects source y=m×39 and height=(1−m)×39. Their
RenderBitmap path has **no half-texel inset**: u=0, du=width/W, v=m×39/64,
dv=(1−m)×39/64. The destination shifts down by the same missing height. XP also
has no inset and always samples u=0, du=6/8, v=0, dv=1 while its destination
width changes. Neither bars nor panels are nine-slices.

Item slots repeat every 38 pixels across logical x=0…152. Skill slots repeat
every 32 pixels from x=222 through x=382. The alternate overlay contains the
same five positions for keys 6,7,8,9,0. The fixed Q/W/E/R and 1–0 legends remain
in the texture; item counts, gauge values, experience digits and assigned-skill
badges are still rendered dynamically. The slot overlays leave the existing
20×28 icon area at offset (6,6) clear.

The three panel payloads are cut from one 640×51 composition at x=256 and x=384.
XP rail dividers retain the 63-pixel rhythm at x=0,63,…630 and the final counter
well. Empty life/mana backings occupy logical strip rectangles (158,3,45,39)
and (437,3,45,39); SD/AG use (204,2,16,39) and (420,2,16,39). Filled and empty
art share the same component pixels outside the energy mask. Their bottom
edges intentionally follow the existing 39-pixel draw and overlap the XP
background's first row for life/mana, as in the shipped layout.

## States and shared uses

Cash-shop states are normal, hover, pressed/selected and selected-hover. An open
shop remaps rest/hover to rows 2/3. No separate disabled row exists. Gauge states
are continuous vertical crops, with green life for poison. XP has normal/master
textures; the experience-gain flash is a runtime white overlay, not another
atlas state.

Skill backgrounds are shared by the bottom hotbar, expanded skill list and pet
command list. Normal `newui_skillbox` is also drawn at 32×38 by MU Helper's
skill chooser and the pet-information window. Both backgrounds are loaded by
MU Helper, although its inspected skill-list draw uses the normal texture.
Their geometry and icon offsets are unchanged. Disabled skill artwork and
cooldown shading are independent runtime/atlas overlays and remain untouched.

## Read-only engine evidence

- `UI/NewUI/HUD/NewUIMainFrameWindow.cpp`: load 69–84; cash geometry 127–169;
  panel slices 275–307; gauge crops 311–475; normal/master XP and flash 487–708;
  fixed item bindings/rendering 1012–1380; skill loading 1410–1421; hotbar/list/
  pet slots 1989–2170; skill icons, key badges and cooldowns 2530–2645;
  cash state remapping 2683–2705.
- `UI/NewUI/NewUIMuHelper.cpp`: shared loads 1872–1873, 32×38 draw 1970–2000.
- `UI/NewUI/Character/NewUIPetInfoWindow.cpp`: four 32×38 slots 262–265,
  shared load 389.
- `UI/NewUI/NewUICommon.cpp:77–119`: inset UV normalization.
- `Render/Textures/ZzzTexture.h:15–17`: default clamp mode.
- `Render/Sprites/GlobalBitmap.cpp:775–889`: OZJ prefix, RGB decode and padded
  allocation; `:115–127` assigns alpha=255.
- `UI/Scaling/UITransform.cpp`: existing anchored/classic transforms. At
  1920×1080/contentScale=1, content scale is 2. Classic X offset is 320 for all
  bands; anchored X offsets are 0/320/640 and XP scale-X is 3. Y offset is 120.

No engine source is changed. The authoritative source rectangles are preserved;
offline sampling approximates GPU output and still needs client confirmation.
