# Text Labels, Monster Health Bars, and UI Scaling

## Problem

The current text and UI paths combine several incompatible assumptions:

- NPC name assets are stored as ISO-8859 text while `OpenMonsterScript()`
  decodes them as UTF-8. Spanish and Portuguese names containing accented
  characters therefore become replacement glyphs or empty strings.
- Runtime monster names can exceed the 31-character capacity of
  `UI::Chat::CHAT::ID`. Unbounded copies overwrite adjacent chat fields.
- `AddChat()` clears 256 bytes instead of the complete `wchar_t[256]` buffer.
- The selected-monster health bar rejects `HealthStatus == -1`, although the
  drawing code defines that value as the supported "HP unknown" sentinel.
- UI panels, world-attached overlays, input, and fonts share one resolution
  model. Panels stretch to the full window, world labels need full-window
  projection, and SDL_ttf fonts remain fixed-size. Small resolutions crowd
  text; large resolutions produce oversized panels with relatively tiny text.

## Goals

- Render every shipped English, Spanish, and Portuguese NPC name as valid
  Unicode without memory corruption.
- Keep selected-monster health feedback visible before the first HP update.
- Preserve F8 as the opt-in toggle for overhead monster health bars.
- Keep world-attached labels aligned with the 3D scene across aspect ratios.
- Use the available viewport for HUD and docked panels without stretching every
  interface through one centered canvas.
- Size text from each interface's rendered space, with readable lower and upper
  bounds.
- Preserve the supported 640x480 minimum resolution and existing 3D camera
  behavior.

## Non-Goals

- No user-configurable UI scale setting.
- No redesign of individual window contents.
- No font-family changes.
- No health values invented client-side when the server reports unknown HP.
- No changes to 3D FOV, terrain culling, or world rendering scale.

## NPC Name Data and Storage

Convert the three checked-in `NpcName_*.txt` assets to UTF-8
without changing their visible text or record ordering. UTF-8 becomes the only
runtime encoding; no locale-dependent fallback decoder is added.

Increase `MAX_MONSTER_NAME` from 40 to 64. This covers the longest shipped name
(42 characters) with termination room and avoids translation-specific limits.
Change `MONSTER_SCRIPT::Name` to `MAX_MONSTER_NAME + 1`, keep
`CHARACTER::ID` at `MAX_MONSTER_NAME + 1`, and make every affected copy
bounded and explicitly terminated.

Change `CHAT::ID` to `MAX_MONSTER_NAME + 1`. Replace its three unbounded name
copies with one shared bounded helper. Clear wide chat buffers by object size,
not a byte literal.

An asset regression test will decode every shipped NPC-name file as strict
UTF-8, verify every parsed name fits `MAX_MONSTER_NAME`, and cover accented and
long translated names. A storage test will verify truncation and termination at
the destination boundary.

## Monster Health Bars

The selected-monster bar renders whenever HP is nonzero. Positive values use
the received fraction. `-1` uses the existing unknown/full presentation. Zero
continues to mean empty/dead and does not render.

Overhead bars remain disabled by default and remain controlled by F8. This
avoids adding permanent combat clutter while fixing the always-available
selected target feedback.

A focused test will cover positive, unknown, and zero health eligibility.

## Responsive Layout Modes

Remove the single panel transform around the complete managed UI. Every managed
object has one layout mode, selected at registration and applied independently
for rendering and mouse input:

1. **HUD** uses the full-window mapping:
   `scaleX = windowWidth / 640`, `scaleY = windowHeight / 480`, offsets `0,0`.
   Bottom bars, chat, minimap, event timers, status widgets, and other edge HUD
   elements therefore reach the real viewport edges.
2. **Dock right** keeps the existing aspect-preserving panel scale, clamped to
   `[1.0, 1.5]`, but anchors the logical 640-wide canvas to the right viewport
   edge. Existing one-, two-, and three-column panel positions remain valid.
3. **Dock left** uses the same capped scale, anchored to the left edge.
4. **Dialog** uses the capped scale and centers its logical 640x480 canvas.
   Message boxes and modal event dialogs remain centered and do not become giant
   at high resolution.
5. **World overlay** uses the full-window mapping. NPC/player names, selected
   monster labels and bars, ground-item labels, and other projected overlays
   stay aligned with the 3D scene.

At 1920x1080, a dialog keeps the capped 960x720 canvas at offset `480,180`.
A right-docked panel uses the same scale at offset `960,180`, putting logical
`x=450..640` exactly against the physical right edge. The HUD and world-overlay
spaces cover all `1920x1080` pixels.

The manager activates an object's layout only around that object's render,
update, and mouse-event calls. Mouse coordinates use the matching inverse
transform and are restored afterwards. UI 3D previews and queued 2D effects
retain their owner's active transform. Legacy title, server-selection, and
character-selection sprites already store native-pixel positions, while their
text and portable input fields convert those positions back to 640x480 logical
coordinates. Their render pass therefore uses full-window coordinate mapping
with an independent 1.0 typography scale. This preserves sprite, text, caret,
and click-target alignment without enlarging legacy fonts.

Login and character-selection 3D scenes use the complete 640x480 reference
viewport. The former 25-pixel top offset and 430-pixel height are removed, so
the scene fills the window vertically. Camera FOV remains unchanged; frustum
dimensions follow the same full-height viewport.

When right-docked panels reserve world space, the scene viewport maps the
legacy logical width through the dock transform and then back through the
full-window transform. The 3D viewport therefore ends at the panel's rendered
left edge instead of leaving an unused strip between the world and the UI.

## Font Scaling and Text Fit

Stop reloading one globally resolution-scaled font set. SDL_ttf keeps one
preloaded maximum-size atlas per role and scales glyph geometry down for the
active layout:

- normal and bold: readable range 11-13 points
- big bold: readable range 22-26 points
- fixed: readable range 13-15 points

The active point size comes from the rendered layout height, not the complete
window alone. Growth is gradual between 1x and the capped 1.5x panel height and
stops at the role maximum. A 1.5x dialog or docked panel therefore uses
13-point normal text; larger HUD transforms remain capped at 13 points.

Before alignment, text calculates its physical box from the active transform.
When a bounded string exceeds the box width or height, glyph scale reduces to
fit, never below the role's readable minimum. Measurement and rendering use the
same final scale. Unbounded world labels use the layout-derived readable size,
remaining screen-space without growing indefinitely at 4K.

Fonts are opened only during startup or font-family reload. Resize changes the
layout-derived glyph scale; it performs no font file I/O and does not invalidate
the text atlas.

## Error Handling

- Invalid UTF-8 in checked-in NPC assets fails the asset test and is not
  silently replaced at runtime.
- Bounded name copies always terminate the destination.
- A zero-sized destination is rejected by the shared copy helper.
- Existing font reload failure behavior remains unchanged: log the failure and
  retain the previous usable maximum-size font set.

## Verification

- Observe each regression test fail before its production change.
- Run focused NPC asset/storage, health-bar, UI-transform, and font-size tests.
- Run the complete test suite.
- Build the client in Release configuration.
- On Windows, Linux, and macOS verify English, Spanish, and Portuguese NPCs,
  including names longer than 31 characters and names containing accents.
- At 640x480, 800x600, 1280x720, 1920x1080, and 3840x2160 verify HUD edge
  anchors, docked panels, centered dialogs, text fit, matching mouse hit testing,
  world-label alignment, and selected-monster HP.
