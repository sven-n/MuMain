# UI Architecture Principles

**Read this before touching any RmlUi UI code on this branch — before opening `README.md` or any
other doc in this directory.** Everything else in `docs/rmlui-ui-system/` is either an
implementation of these principles on this specific codebase (`layout-and-scaling.md`,
`theming-and-modding.md`, `newui-tier-adapter.md`), an amendment scoped to one recurring question
(`legacy-theme-modernization.md`), or a status report against all of the above (`STATUS.md`). None
of them repeat the reasoning here — if something below and something elsewhere in this directory
ever seem to disagree, this file wins; go fix the other file.

This is the governing policy for the RmlUi UI migration, issued by the user. It reframes the
migration's success criterion: not "the old UI now renders through RmlUi" but "a unified,
maintainable, resolution-independent UI framework." It is written to stand on its own —
it does not assume familiarity with this repository's specific classes or history, only with
RmlUi/RCSS in general. **This file changes rarely and should stay stable; project-specific status
(what's done, what's still a gap, known open conflicts) belongs in `STATUS.md`, not here.**

---

As part of the ongoing migration of the legacy MU Online UI to **RmlUI**, treat this as a **UI
architecture modernization**, not simply a replacement of the existing rendering implementation.

The goal is to establish a single, maintainable UI system that is resolution-independent,
aspect-ratio aware, responsively laid out, user-scalable, themeable, moddable, reusable,
maintainable, and independent of historical MU Online UI/version boundaries. The new architecture
should make it possible to substantially modify the UI's appearance, layout, assets, theme, and
user preferences without recompiling the client, wherever reasonably possible.

## 1. Core Architectural Principle

Establish a clear separation between game/UI behavior, UI structure, layout, visual
presentation/theme, and user customization:

```text
Game / C++  →  Unified UI Components / API  →  RML (structure/composition)
  →  RCSS (layout, sizing, positioning, responsive behavior, styling)
  →  Theme / Assets (textures, sprites, icons, fonts)
  →  User Preferences (UI scale, drag positions, layout preferences)
  →  Final Runtime UI
```

**C++ defines what the UI does. RML/RCSS defines how the UI is structured, positioned, sized, and
presented. Themes define visual presentation. User preferences define permitted runtime
customization.** Avoid implementing presentation-specific behavior in C++ when it can reasonably
be expressed through RML/RCSS. E.g. HUD position/width, window dimensions, button layout,
alignment, margins, spacing, font size, colors, textures, default anchoring, and responsive
behavior belong in RML/RCSS/theme — not C++ (which owns HP/inventory data, button actions,
open/close, game events, component behavior, drag interaction, preference persistence).

## 2. Reverse-Engineer Original Layout Intent Before Migration

Before converting any legacy UI component to RmlUI, **first analyze and document the layout
intent of the existing implementation**. Do not perform a mechanical translation of legacy
coordinates, dimensions, or rendering calls into RML/RCSS.

Determine, per component: what it is, its intended size, what it's anchored to, what it's aligned
to, whether it stretches/scales/is fixed-size, whether it's positioned relative to the screen or
another component, and whether any resolution/aspect-ratio-specific behavior is intentional or
merely a legacy implementation artifact. Establish a concise layout-intent description (anchor,
size, stretch, responsive behavior, UI scale, draggable, default position) before implementing
the RmlUI version. Values must come from analysis of the existing implementation and intended
UX, not assumptions.

## 3. Distinguish Layout Intent From Legacy Implementation Details

Treat the existing C++ implementation as **evidence of behavior**, not automatically the
definition of the desired architecture. Distinguish actual UI intent (centered on screen, bottom
anchored, fixed-size panel, right-aligned status display, repeated item grid, relative
positioning, screen-edge positioning) from legacy implementation artifacts (magic coordinates,
manual screen-width calculations, hardcoded offsets, version-specific coordinate adjustments,
duplicated positioning logic, texture dimensions used as layout dimensions, 4:3 assumptions,
manual scaling calculations, special-case resolution branches). Do not blindly reproduce
implementation artifacts.

Canonical example: `x = (screenWidth - 640) / 2;` should be recognized as intent = "horizontal:
centered, width: 640 logical units," not preserved as arithmetic.

## 4. Do Not Assume Legacy Behavior Is Correct

The objective is not necessarily to reproduce every legacy implementation detail exactly. If
legacy behavior is poor on widescreen/ultrawide/other resolutions, distinguish **Legacy
behavior** from **Identified intent** from **Desired modern behavior** (e.g. legacy: HUD
stretches full-viewport; identified intent: fixed-width gameplay control area; desired modern
behavior: fixed logical width + bottom-center anchoring). Retain the Legacy Theme's visual
identity while letting the new layout system provide better responsive behavior. Document any
intentional compatibility change.

## 5. Make Layout Intent a First-Class Concept

Do not model UI primarily as absolute screen coordinates. Every major component should have an
explicit layout intent: what it's anchored to, how it's aligned, what determines its size,
fixed-size vs. fluid, whether it stretches, viewport-change/UI-scale-change behavior, and whether
its default position is customizable. This should result in declarative RML/RCSS, not C++
coordinate calculations (`x = screenWidth / 2 - 450;`, `if (screenWidth > 2560) ...`). **Think in
terms of layout relationships and intent, not screen coordinates.**

## 6. Analyze Related Components as a Layout Hierarchy

Don't analyze every element in isolation when its layout depends on neighbors. Determine the
overall composition first (e.g. Screen → Main HUD → Player Status / Experience / Action Bar /
Controls), which elements are children of the same container vs. independently anchored vs.
relative to another element vs. fixed/fluid/repeated/optional/theme-dependent, then reproduce
that hierarchy in RML rather than converting every legacy element into an independently
positioned absolute element.

## 7. Responsive and Aspect-Ratio-Aware Layout

The UI must behave intentionally across 4:3, 16:9, 16:10, ultrawide/21:9+ — not simply stretch to
fill the viewport. Each component needs an intentional behavior: **fixed-size** (inventory,
character window, dialogs, item slots, buttons — preserve logical dimensions, reposition by
anchor), **edge-anchored** (chat, minimap, quest tracker, status panels — stay attached to their
edge/corner), **centered** (central HUD, target frame, boss health bar, interaction prompts —
stay centered regardless of viewport width), **fluid/stretchable** (chat background, flexible
panels, some HUD containers — only when explicitly designed to). Do not introduce a universal
"scale everything to screen width" strategy.

## 8. HUDs Must Not Be Forced to Stretch

A component should be able to change from a stretched layout to a fixed/centered layout without
changing C++ — e.g. a full-width legacy HUD becoming a narrower fixed-width centered (or
left/right-anchored) bar on ultrawide, achievable through layout definitions/themes rather than
C++ coordinate calculations.

## 9. Centralized UI Scaling

UI scale (e.g. 75/100/125/150%, exact range TBD via implementation/testing) must be a first-class
user preference: applied consistently across the UI, persisted through the game's config system,
working across resolutions/aspect ratios, not requiring individual components to implement their
own scaling, ideally runtime-changeable — via the most appropriate RmlUI-native approach, not
independent per-component scaling calculations. **Viewport size and UI scale must remain
explicit, distinct concepts** (e.g. 3440x1440 @ 100% vs. 3440x1440 @ 125% are different scale
settings on the same viewport). Changing UI scale should not change a component's underlying
layout intent.

## 10. Preserve and Integrate Existing Draggability

Do not make redesigning/replacing the existing RmlUi drag system a primary goal of this
amendment. Instead: inspect how it works, preserve it where already appropriate, ensure it works
with the new declarative layout system, ensure theme-defined default layouts don't conflict with
user-dragged positions, ensure dragged positions continue to behave correctly across
resolution/aspect-ratio/UI-scale changes, and reuse the existing drag infrastructure rather than
building separate per-component drag implementations. The key concern is the interaction between
**theme default layout + existing user drag position + viewport/resolution + UI scale**. Don't
let runtime dragging force the whole architecture into raw screen-coordinate positioning — prefer
anchor+offset / normalized position / logical coordinates over storing arbitrary physical screen
coordinates where the existing system can be improved as part of this integration. **But do not
rewrite the existing drag system solely for theoretical architectural purity** — preserve working
behavior unless repository analysis demonstrates a concrete problem.

## 11. Separate Default Layout From User Layout

Distinguish **default layout** (RML/RCSS/theme/component configuration) from **user-customized
layout** (existing drag system, user preferences, UI scale, optional visibility/layout
preferences): Theme/RML/RCSS → Default Layout → Existing User Layout Overrides → Final Runtime
Layout. This prevents user customization from becoming hardcoded into the theme or component
implementation.

## 12. Unify UI Components Across Game Versions

Do not automatically preserve historical `Legacy UI`/`New UI`/version-specific UI boundaries in
the new architecture. Identify actual reusable UI concepts and consolidate — `LegacyButton`/
`NewButton` → `Button` (with visual presentation via theme), `LegacyWindow`/`NewWindow` →
`Window`, and likewise for item slots, inventory, panels, progress bars, tooltips, dialogs, HUD
elements, tabs, lists, scroll containers, etc. If two implementations genuinely differ in
behavior/structure, isolate that difference behind a clean abstraction rather than duplicating
the whole component. **Organize the new UI around reusable UI responsibilities, not historical
game versions.**

## 13. Legacy UI Becomes the Legacy Theme

The existing legacy appearance must remain supported, but not as a separate UI framework —
implement it as a **Legacy Theme** on top of the unified UI architecture (Unified UI Components →
Legacy/Modern/Custom Theme → theme-specific assets/styling/layout). The Legacy Theme should
preserve the existing visual identity as closely as practical (sprites, textures, icons, fonts
where applicable, windows, buttons, HUD appearance, visual effects, layout behavior where
required), integrated into the unified RmlUI system rather than a parallel legacy architecture.

## 14. Legacy Assets Should Remain First-Class Assets

Don't unnecessarily discard/rewrite existing legacy sprites/textures. Where practical, the Legacy
Theme should reference existing assets through the RmlUI resource/asset system, mapped to RmlUI
concepts (images, backgrounds, borders, icons, textures, decorators, etc.). If a legacy rendering
behavior can't be reproduced directly with normal RmlUI/RCSS, isolate that compatibility
requirement behind the UI framework rather than spreading legacy-specific rendering hacks
throughout every component.

## 15. Themes Are Presentation Packages, Not Just Color Schemes

A theme should be capable, where practical, of controlling colors, fonts, font sizes,
backgrounds, borders, sprites, textures, icons, buttons, windows, panels, progress bars, item
slots, tooltips, scrollbars, HUD presentation, spacing, dimensions, layout, alignment,
visibility, and component-specific presentation — not just colors/fonts/textures. **A theme is a
presentation layer, potentially including RML/RCSS/layout definitions and assets.** Don't force
every theme to use exactly the same visual structure if a theme genuinely requires a different
layout.

## 16. Prefer RML/RCSS for Theme and Layout Customization

As much as reasonably possible, visual/layout customization should be possible without modifying
C++ — HUD position/width/alignment, window size/position, button appearance, inventory layout,
item slot appearance, fonts, colors, textures, sprites, icons, spacing, margins, padding,
visibility — through RML/RCSS/theme configuration/assets rather than C++. ("Move the HUD to
bottom-center" → RML/RCSS. "Use the legacy HUD sprite" → theme asset. "Change how HP is
calculated" → C++.) Keep this separation clear.

## 17. Separate UI Structure From Styling

Avoid putting large amounts of styling directly into RML markup (`<div style="left: 123px; top:
456px; width: 900px;">`) — prefer classes (`<div class="main-hud">`) with presentation/layout
defined through RCSS. This makes theme overrides and modding significantly harder otherwise.
Likewise avoid generating large amounts of presentation-specific RML from C++ when static/
declarative RML is sufficient.

## 18. Support Theme Overrides and Modding

Design the resource system so users/modders can override UI resources without recompiling: Base
UI → Selected Theme → Optional User/Mod Override. A custom theme shouldn't need to duplicate the
entire UI — e.g. a mod providing only `theme.rcss`/`hud.rcss`/custom assets while inheriting
everything else from the base theme. Establish a deterministic, documented resource precedence
(e.g. user/mod override → selected theme → base/default UI → legacy/default game assets).

## 19. Theme Overrides Should Support Layout, Not Just Assets

The modding system should support layout customization, not just "replace this PNG" — moving/
resizing the HUD, changing anchoring, window dimensions, moving the minimap, rearranging HUD
elements, changing item-grid dimensions, spacing, alignment, hiding decorative elements, font
sizes — without modifying C++ wherever practical. This is one of the primary reasons layout must
be declarative.

## 20. Reusable UI Components

Create reusable components for recurring concepts (Window, Panel, Button, Icon, ItemSlot,
ItemGrid, ProgressBar, HealthBar, ManaBar, ExperienceBar, Tooltip, Dialog, Tab, TabBar, List,
ScrollContainer, Notification, HUDContainer — exact set derived from the actual repository).
**Build behavior once and allow presentation to vary by theme** — a `Button` shouldn't need to
become `LegacyButton`/`ModernButton`/`CustomButton` merely because its visual presentation
differs.

## 21. Design Tokens / Shared Theme Variables

Where supported by RmlUI, establish centralized UI design variables/tokens for common properties
(`ui-font`, `ui-font-size`, `ui-window-background`, `ui-window-border`, `ui-panel-padding`,
`ui-item-spacing`, `ui-button-height`, `ui-primary-text`, `ui-secondary-text`, `ui-scale`, etc.),
using RmlUI's supported capabilities rather than unnecessary custom infrastructure, so themes can
change broad aspects of the UI without editing every individual selector.

## 22. Keep Game Logic Out of Themes

Themes/RML should not become an uncontrolled interface into arbitrary game state. Expose
controlled data/events from C++ (player HP/level, inventory items, party members, game state
→ UI; button clicked/item selected/window closed/action requested → C++). Avoid exposing
arbitrary internal game objects directly to RML — keeps the UI layer decoupled for future
refactoring.

## 23. Avoid Resolution-Specific Hacks

No `if (width == 1920) x = 500;`-style branching, and no multiple hardcoded coordinate systems
per aspect ratio, for normal layout behavior. Prefer anchoring, alignment, logical dimensions,
flex/layout relationships, responsive rules, min/max dimensions, theme-specific layout — the UI
should behave correctly at resolutions that were not explicitly anticipated.

## 24. Define a Project-Wide Layout Policy

Before migrating a large number of screens, document: reference/design resolution or logical
coordinate strategy, UI scaling strategy, minimum supported resolution, aspect-ratio/ultrawide
policy, fixed-size/fluid-size conventions, anchor/centering conventions, existing drag behavior/
conventions, user-position persistence strategy, theme override rules, resource precedence. Don't
let each migrated screen invent its own scaling/positioning system.

## 25. Validate Across Resolutions, Aspect Ratios, Themes, and User Customization

Test representative screens at 800x600, 1280x720, 1920x1080, 2560x1440, 3440x1440, 3840x2160, and
UI scales 75/100/125/150%; across Legacy/Modern/Custom-or-Test themes; and across default layout,
existing dragged components, resolution change after dragging, UI scale change after dragging,
theme change after dragging, reset/default behavior where supported. **The Custom/Test Theme
should intentionally look substantially different from the Legacy Theme** — this reveals
accidental coupling between component implementation and a particular visual design. Pay
particular attention to HUD positioning/stretching, screen-edge elements, centered elements,
dragged elements, inventory grids, windows, dialogs, chat, minimap, tooltips, item slots, text
wrapping, clipping, overlap, minimum usable dimensions, UI scale behavior, persistence behavior.

## 26. Architecture Audit Before Broad Refactoring

Before major structural changes, inspect the existing repository/RmlUI migration: existing UI
managers, legacy/new UI hierarchy, version-specific implementations, duplicate components,
existing coordinate/scaling systems, existing drag implementation, resolution/input handling,
renderer integration, RmlUI context/document management, existing RML/RCSS, asset system, legacy
sprite/texture loading, configuration system, existing theme implementation, existing user
preference mechanisms. Then propose the target architecture based on the actual repository — do
not blindly impose a predetermined directory structure/abstraction hierarchy, and do not rewrite
functioning infrastructure merely to satisfy this amendment; prefer incremental improvement.

## 27. Required Migration Workflow

For each significant migrated UI: (1) inspect legacy implementation, (2) trace related UI/
component code, (3) identify original layout behavior, (4) separate intent from implementation
artifacts, (5) document layout intent, (6) identify related components/hierarchy, (7) identify
reusable/common components, (8) identify theme-specific presentation, (9) identify responsive
behavior, (10) identify existing drag/user-customization behavior, (11) design the RML hierarchy,
(12) implement layout in RML/RCSS, (13) implement appearance in the selected theme, (14) connect
behavior/state through C++, (15) integrate with the existing drag system, (16) persist/restore
user layout through the existing preference mechanism, (17) test against the original UI, (18)
test across resolutions/aspect ratios, (19) test UI scaling, (20) test Legacy/Modern/Custom
themes, (21) test user-customized positions, (22) remove obsolete legacy/version-specific
implementation where safe. **Do not skip steps 1–5.** The migration should be driven by
understanding the original UI's intent, not mechanical translation of legacy code.

## 28. Definition of Done

A migrated UI is not complete merely because it renders through RmlUI. It's properly migrated
when: its original layout intent has been understood and documented; that intent is explicitly
represented in the new architecture; positioning doesn't depend on a single resolution; behavior
across aspect ratios is intentional; UI scaling works consistently; it contains no unnecessary
hardcoded coordinate calculations; presentation is theme-controlled; assets can be overridden
where appropriate; it uses reusable components; it doesn't unnecessarily duplicate historical UI
implementations; it integrates with the existing drag mechanism where applicable; existing
user-customized positions continue to work; user positions remain sensible across resolution/
aspect-ratio/UI-scale changes; the Legacy Theme reproduces the required legacy appearance; a
Modern Theme can present the component differently; a Custom Theme can substantially modify its
appearance/layout without C++ changes; game behavior remains appropriately isolated in C++.

## 29. Final Architectural Goal

```text
GAME / C++ → Unified UI API → UI Components → RML (structure/composition)
  → RCSS (layout + responsive behavior) → { Themes (Legacy/Modern/Custom assets),
  Mod Overrides (custom assets/RML) } → Default Runtime Layout
  → Existing User Customization / Dragged Positions → Final Runtime UI
```

**C++** owns: game state, UI state, game/business logic, events, actions, data binding, component
behavior, lifecycle, RmlUI integration, existing drag interaction, user preference persistence.
**RML/RCSS** owns: UI structure, layout, anchoring, sizing, alignment, spacing, responsive
behavior, default component placement, visual styling, theme presentation. **Theme/Mod Assets**
own: sprites, textures, icons, fonts, visual resources, theme-specific styling/layout, optional
RML/RCSS overrides. **User Preferences** own: UI scale, existing user-adjusted positions, optional
visibility/layout preferences.

### Final Design Principle

Do not recreate the legacy UI framework inside RmlUI. Use the migration to establish a single
declarative UI system: historical implementations converge into Unified Components → {Legacy
Theme, Modern Theme, Custom Themes/Mods} → Existing User Customization.

> **First understand the original layout intent. Then express that intent declaratively in
> RML/RCSS. Presentation belongs to the theme, assets belong to the theme/mod, behavior belongs
> to C++, and user customization belongs to a separate persisted preference layer.**

A future developer should be able to change "bottom stretched" to "bottom centered, fixed width"
or "left anchored" to "right anchored" by changing RML/RCSS/layout definitions rather than
modifying game C++. A user should be able to use the existing drag functionality to customize a
component, have that customization persist, and still have the component behave sensibly when
resolution/aspect ratio/UI scale changes. A theme/mod author should be able to substantially
redesign the UI without understanding the historical implementation of every previous MU Online
UI version. The resulting architecture should make **the easy path the maintainable path**:
adding a new UI should naturally use the unified components, declarative layout, centralized
scaling, existing drag infrastructure, and theme/mod system rather than introducing another
version-specific UI implementation.

## 30. Theme Identity Must Never Be a C++ Branch Condition

If C++ genuinely must behave differently depending on the active theme (a real, narrow case —
e.g. render ordering forces a choice between two visually-equivalent techniques), branch on a
**declared theme capability/property** (a boolean or enum the theme itself states, e.g. "wants
filled backgrounds instead of borders"), never on the theme's **name** (`GetActiveThemeName() ==
"modern"`). A name check silently stops applying the moment a differently-named theme wants the
same behavior, and coupling behavior to an identity string is exactly the kind of hidden,
undiscoverable dependency §18–19's override system is meant to prevent. A theme wanting
non-default C++ behavior should be **stating** that want (a manifest property, a data flag), not
being **recognized by name**. See `STATUS.md` for known current violations of this rule.

---

**Next**: [`legacy-theme-modernization.md`](legacy-theme-modernization.md) — an amendment
resolving one recurring question this document leaves implicit: when legacy-theme UI behavior
currently lives in C++, when should maintaining/migrating it mean moving that behavior into
RML/RCSS, and when does it genuinely stay in C++. Then [`STATUS.md`](STATUS.md) for what's
actually implemented against both documents today, known gaps, and open conflicts with other
standing decisions on this branch.
