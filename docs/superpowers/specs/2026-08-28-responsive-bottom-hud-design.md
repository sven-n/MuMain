# Responsive Bottom HUD Design

## Goal

Keep the legacy bottom HUD readable without stretching it across large or wide
windows. Preserve the familiar small-resolution layout while reorganizing the
HUD into three responsive zones:

- left: Q/W/E/R item shortcuts;
- center: HP, SD, skill slots, current skill, AG, and mana;
- right: character, inventory, friend, and menu buttons.

The experience rail remains full-width. No new textures or dependencies are
required.

## Root Cause

`INTERFACE_MAINFRAME` currently uses the same full-window transform as screen
overlays:

```text
scaleX = windowWidth / 640
scaleY = windowHeight / 480
```

This is uncapped and non-uniform. A 1920x1200 window renders the HUD at 3x
horizontal scale and 2.5x vertical scale. Controls become oversized and their
aspect ratio changes.

The HUD artwork is already split across three 256/128/256-pixel textures, but
the code renders them as one continuous 640-pixel canvas. The combat gauges
share those textures with the side controls, so moving whole textures cannot
put the gauges in the center. Existing source regions must be rendered as
separate UV slices.

## Selected Layout

Retain the original 640x480 logical coordinates. Define three named horizontal
bands from the current control positions:

| Region | Logical X band | Contents |
|---|---:|---|
| Left utility | `[0, 152)` | Q/W/E/R backgrounds, items, counts |
| Center combat | `[152, 488)` | HP, SD, skill slots, current skill, AG, mana |
| Right menu | `[488, 640]` | menu background and four buttons |

These bands exactly cover the original 640-pixel HUD without changing control
coordinates. At 640x480 they join into the original layout. At wider aspect
ratios, the utility band stays left, the combat band stays centered, and the
menu band stays right.

The center order is:

```text
[HP + SD] [hot skills + current skill] [AG + Mana]
```

## Scale and Anchoring

Use one uniform scale for all three fixed-content regions:

```text
hudScale = clamp(min(windowWidth / 640, windowHeight / 480), 1.0, 2.0)
```

All regions map logical `y=480` to the physical window bottom. Their horizontal
anchors are:

- left transform: logical `x=0` maps to physical `x=0`;
- center transform: logical `x=320` maps to the window center;
- right transform: logical `x=640` maps to the physical right edge.

At 1024x768 the scale is 1.6 and the regions reconstruct the original full
width. At 1280x720 the scale remains 1.5, preserving the current vertical size.
At 1920x1200 the scale stops at 2.0 instead of growing to 2.5-3.0.

The experience rail uses the same vertical scale and bottom anchor, but maps
logical `x=0..640` across the complete physical window width. Only this flat,
repeatable rail may stretch horizontally.

## Texture Rendering

Use the existing `RenderImageStretch` source-region API. Render named slices
from `newui_menu01`, `newui_menu02`, and `newui_menu03` into their owning
region. Do not create derivative image files.

Source boundaries become constants beside the HUD renderer. Half-texel handling
remains owned by `RenderImageStretch`; callers provide source pixels, not raw UV
fractions.

At 640x480, the sliced render must be pixel-equivalent to the current three
whole-image draws. At larger widths, only the gaps between the three regions
change.

## Rendering Ownership

Add bottom-HUD transforms to `UI::Scaling` and expose them through layout modes
for left, center, and right regions. Existing general HUD overlays keep the
full-window transform.

`CNewUIMainFrameWindow` renders each concern under its owning transform:

- left: frame slice, `CNewUIItemHotKey` items and counts;
- center: frame slices, life/mana, SD/AG, current/hot skills;
- right: frame slice and menu buttons;
- stretch: experience background and progress rail.

`CNewUISkillList` uses the center transform for rendering, tooltips, expanded
skill lists, and mouse input. `CNewUIHotKey` remains a keyboard-command owner;
its unrelated world interactions retain screen-overlay coordinates.

Transform changes must restore the prior active transform on every exit path.
Use one small scoped helper rather than repeating manual save/restore blocks.

## Mouse Input and Tooltips

Every interactive region uses the inverse of the same transform used to render
it. Derive regional logical mouse coordinates from `g_fWindowMouseX` and
`g_fWindowMouseY`; do not reuse coordinates transformed for another region.

- Q/W/E/R right-click checks use the left transform.
- Skill selection and expanded skill-list checks use the center transform.
- Character/inventory/friend/menu buttons use the right transform.
- Gauge and experience tooltips use their rendering transform.

Clicks in the gaps between regions fall through to the world. The three visible
region bounds block world input.

## World Viewport and Docked Panels

The physical HUD top is derived from the fixed frame height:

```text
hudTop = windowHeight - 51 * hudScale
```

The main world viewport, terrain culling, default/orbital camera frusta, world
mouse boundary, and left/right dock bottom alignment must use this same value.
Projection and every culling path continue sharing one computed aspect ratio.

Rounding happens once in the shared viewport helper. Consumers use the returned
integer physical dimensions so projection and culling cannot disagree by a
pixel.

## Tests

Extend `tests/ui/test_ui_scaling.cpp` with literal expectations for:

- 640x480: three regions reconstruct the original layout;
- 1024x768: 1.6x regions meet without gaps;
- 1280x720: 1.5x height is preserved and horizontal gaps are symmetric;
- 1920x1200: scale caps at 2.0, center midpoint is screen center, side regions
  touch their respective edges;
- regional position/inverse-position round trips;
- experience transform spans the complete window width;
- HUD top matches the world viewport and dock bottom;
- general HUD/world-overlay transforms remain unchanged.

Add focused source-level or unit coverage for the selected interface-to-region
policy. Run the complete CTest suite, native executable link, and `git diff
--check`.

Native screenshots must cover at least one 4:3 or 5:4 resolution and one wide
resolution with inventory plus character panels open. Verify texture seams,
gauge centering, button hitboxes, tooltips, world clicks in gaps, dock/HUD
alignment, and absence of black terrain gaps before claiming visual completion.

## Non-Goals

- No user-configurable UI scale yet.
- No new HUD artwork.
- No changes to inventory/dialog scaling.
- No broad cleanup of the legacy main-frame implementation.
- No redesign of chat, minimap, event timers, or other independent HUD widgets.
