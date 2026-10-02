# Responsive Bottom HUD Design

## Goal

Keep the legacy bottom HUD readable without stretching it across large or wide
windows, and without pulling it apart. The bar stays one continuous 640-wide
piece:

- left: Q/W/E/R item shortcuts;
- center: HP, SD, skill slots, current skill, AG, and mana;
- right: character, inventory, friend, and menu buttons.

The experience rail is the bottom strip of that same bar. No new textures or
dependencies are required.

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
coordinates. They always share one transform, so the artwork meets at every
resolution. At 640x480 and at other 4:3 sizes the bar fills the window width.
On wider windows it stays centered and the 3D view shows in the side margins.

The center order is:

```text
[HP + SD] [hot skills + current skill] [AG + Mana]
```

## Scale and Anchoring

Use one uniform scale for all three fixed-content regions:

```text
hudScale = clamp(min(windowWidth / 640, windowHeight / 480), 1.0, 2.0)
```

All regions, including the experience strip, share one transform. Logical
`y=480` maps to the physical window bottom. Logical `x=0..640` maps to a bar
of width `640 * hudScale`, centered in the window:

```text
offsetX = (windowWidth - 640 * hudScale) / 2
```

Pinning the bands to opposite screen edges opened a hole between the hotkeys,
the gauges, and the menu. That reads as a split HUD on 16:9 and ultrawide
(about 320px per side at 1920x1080, about 640px per side at 2560x1440). The
side margins are world space. Map haze — Tarkan and Karutan sand, and the
smoke on Swamp of Quiet, Crywolf, Raklion, Empire Guardian, and Battle Castle —
covers the full window, including those margins. Stopping it at the HUD top
left a darker rectangle beside the bar, because the haze brightens everything
above the HUD and the gutters kept the raw ground. The HUD is drawn afterwards
and stays opaque, so the extra haze under the bar is hidden. Dialog dimming
still stops above the HUD. The scale cap stays at 2.0 so the bar does not
grow to 3x–4x just to touch both edges.

At 1024x768 the scale is 1.6 and the bar fills the window. At 1280x720 the
scale remains 1.5 and the bar is centered. At 1920x1080 and 2560x1440 the
scale stops at 2.0; the gauge midpoint stays on the screen center.

Chat, the chat input, the window menu, and the bottom event timers use this
same frame so they stay on the bar instead of stretching to the screen edges.

Docked panels keep their own scale cap (2.25). The left dock's logical `x=0`
sits on the bar's left edge. The right dock's logical `x=640` sits on the
bar's right edge, so inventory and character windows stay on the bar the way
they do at 4:3.

## Texture Rendering

Use the existing `RenderImageStretch` source-region API. Render named slices
from `newui_menu01`, `newui_menu02`, and `newui_menu03` into their owning
region. Do not create derivative image files.

Source boundaries become constants beside the HUD renderer. Half-texel handling
remains owned by `RenderImageStretch`; callers provide source pixels, not raw UV
fractions.

At 640x480, the sliced render must be pixel-equivalent to the current three
whole-image draws. At larger widths the slices stay joined; only the margins
outside the bar change.

## Rendering Ownership

Add bottom-HUD transforms to `UI::Scaling` and expose them through layout modes
for left, center, and right regions. Existing general HUD overlays keep the
full-window transform.

`CNewUIMainFrameWindow` renders each concern under its owning transform:

- left: frame slice, `CNewUIItemHotKey` items and counts;
- center: frame slices, life/mana, SD/AG, current/hot skills;
- right: frame slice and menu buttons;
- experience: background and progress rail, on the same transform as the bar.

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

Clicks in the side margins fall through to the world. The continuous bar
blocks world input.

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

- 640x480 and 1024x768: the bar fills the width and the regions meet;
- 1280x720: 1.5x height is preserved and the bar is centered with no internal gaps;
- 1920x1200, 1920x1080, and 2560x1440: scale caps at 2.0, the midpoint is the
  screen center, and the right dock's logical `x=640` meets the bar's right edge;
- regional position/inverse-position round trips;
- the experience strip uses the bar's uniform scale;
- HUD top matches the dock bottom;
- general screen-overlay and world-overlay transforms remain unchanged.

Add focused source-level or unit coverage for the selected interface-to-region
policy. Run the complete CTest suite, native executable link, and `git diff
--check`.

Native screenshots must cover at least one 4:3 or 5:4 resolution and one wide
resolution (1920x1080 and 2560x1440) with inventory plus character panels open.
Verify the bar is one piece, gauge centering, button hitboxes, tooltips, world
clicks in the side margins, dock/HUD alignment, and absence of black terrain
gaps before claiming visual completion.

## Non-Goals

- No user-configurable UI scale yet.
- No new HUD artwork.
- No changes to inventory/dialog scaling.
- No broad cleanup of the legacy main-frame implementation.
- No redesign of chat, minimap, or event-timer contents. Chat and the bottom
  timers only change which frame they follow.
