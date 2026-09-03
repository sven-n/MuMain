# UI Architecture Amendment — Legacy Theme Modernization Policy

Amends [`architecture-principles.md`](architecture-principles.md) — read that first. This file
governs one recurring question that doc leaves implicit: when legacy-theme UI behavior currently
lives in C++, when does modernizing/maintaining it mean moving that behavior into RML/RCSS, and
when does it genuinely stay in C++? Issued by the user.

## Legacy Theme Modernization Policy

The Legacy Theme is a compatibility and visual-preservation layer, **not an exemption from the
modern RmlUi architecture**.

Many legacy MU UI components may currently have layout, positioning, sizing, visibility, visual
states, or other UI-specific behavior implemented directly in C++. When maintaining or migrating
the Legacy Theme, these implementations should be progressively moved into the RmlUi scope
**wherever RML/RCSS can express the behavior without compromising gameplay logic or
compatibility**.

### Core Rule

> Preserve the legacy UI's intended appearance and behavior, but move its presentation and
> UI-specific implementation from C++ into RML/RCSS whenever RmlUi can represent it cleanly.

The objective is **not** to rewrite legacy behavior for the sake of modernization. The objective
is to separate legacy **intent and behavior** from legacy **implementation artifacts**.

### What Should Move to RmlUi

The following should generally be considered candidates for migration from C++ into RML/RCSS:

* Absolute screen coordinates used purely for presentation.
* Width and height calculations for UI elements.
* Anchoring and alignment.
* Centering and edge positioning.
* Spacing, margins, padding, and gaps.
* Resolution-specific positioning.
* 4:3/16:9 layout adjustments.
* UI scaling calculations.
* Stretching or fixed-size behavior.
* Visibility controlled purely by presentation state.
* Z-order where it is a presentation concern.
* Window/panel dimensions.
* Button and control geometry.
* Text positioning and alignment.
* Icon placement.
* Background and border presentation.
* Sprite/image placement.
* Progress-bar geometry.
* List/grid layout.
* Tooltip placement rules where expressible through the UI system.
* Responsive layout rules.
* Theme-specific dimensions and presentation.
* Visual states such as hover, active, disabled, selected, focused, etc.
* Layout variations that currently require C++ resolution checks.
* Theme-specific layout differences that can be expressed through RML/RCSS.

For example, legacy code such as:

```cpp
if (screenWidth >= 1920)
    SetPosition(320, screenHeight - 100);
else
    SetPosition(0, screenHeight - 100);
```

should be treated as a migration candidate.

If the actual intent is "HUD is bottom-aligned and either stretches across the screen or uses a
centered fixed-width layout depending on the theme," that intent should be represented through
RML/RCSS or theme layout definitions rather than preserved as C++ coordinate logic.

### What Should Remain in C++

Do **not** move actual game/application behavior into RmlUi merely because the Legacy Theme is
being modernized.

C++ should continue to own:

* Game state.
* Gameplay rules.
* Inventory/equipment state.
* Character state.
* Network state.
* Server-driven behavior.
* Input actions that have gameplay meaning.
* Game commands and actions.
* Data acquisition.
* UI lifecycle where necessary.
* Complex business/game logic.
* Event processing that cannot reasonably be represented by RmlUi.
* Integration with existing engine systems.
* Persistence of user preferences.
* Existing drag/customization integration where required by the engine.

The distinction should be:

**C++ determines what the UI means and what state it is in.**

**RmlUi determines how that state is presented and laid out.**

### Legacy C++ Behavior Classification

When encountering legacy UI code, classify each implementation into one of three categories:

#### 1. Preserve in C++

The behavior is genuine application/game logic or requires engine integration.

Example:

```text
When the player clicks the inventory button, open the inventory system.
```

The action remains C++.

#### 2. Move to RmlUi

The behavior exists primarily because the old UI framework lacked declarative layout/styling
capabilities.

Examples:

```text
Calculate X based on screen width.
Calculate Y based on screen height.
Move this panel when resolution changes.
Resize this window for widescreen.
Manually position children relative to a parent.
Change button appearance based on state.
```

These should be evaluated for implementation using RML/RCSS.

#### 3. Split Between C++ and RmlUi

Some legacy implementations contain both game logic and presentation logic.

Separate them.

For example:

```cpp
if (inventoryOpen)
{
    inventoryWindow.SetPosition(...);
    inventoryWindow.SetSize(...);
    inventoryWindow.SetTexture(...);
    UpdateInventoryItems();
}
```

The desired architecture is conceptually:

```text
C++:
    inventoryOpen = true
    update inventory data

RmlUi:
    display inventory window
    determine position
    determine dimensions
    apply theme
    lay out inventory items
    apply visual state
```

### Do Not Preserve Legacy C++ Layout Merely for Compatibility

Compatibility with the Legacy Theme does **not** mean preserving its historical implementation.

The following reasoning is insufficient:

> "The legacy theme originally did this in C++, so the legacy theme should continue doing it in
> C++."

Instead ask:

1. What was the original UI intent?
2. Is the current C++ implementation actually expressing that intent?
3. Is some of it merely an artifact of the old UI framework?
4. Can RML/RCSS express the same behavior?
5. Can it be moved without changing the externally observable legacy behavior?
6. Does keeping it in C++ create unnecessary coupling with the theme?

If RmlUi can represent the behavior cleanly, **prefer the RmlUi implementation**.

### Legacy Theme as a Compatibility Layer

The Legacy Theme should ultimately look conceptually like:

```text
                 Unified UI Components
                         │
                         ▼
                       RmlUi
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
        Legacy Theme          Custom Theme
              │                     │
       legacy sprites          custom assets
       legacy styling          custom styling
       legacy layout           custom layout
```

The Legacy Theme should preserve:

* Legacy visual identity.
* Legacy sprites and textures where appropriate.
* Legacy dimensions where they represent intentional design.
* Legacy positioning/alignment where appropriate.
* Legacy interaction semantics.
* Legacy visual states.
* Legacy component appearance.
* Legacy-specific behavior that genuinely cannot be generalized.

However, it should **not** preserve unnecessary:

* C++ coordinate calculations.
* Resolution branches.
* Manual layout calculations.
* Duplicated widget implementations.
* Theme-specific rendering code.
* C++ presentation state machines.
* Historical UI framework abstractions that RmlUi replaces.

### Legacy Behavior Does Not Automatically Become RmlUi Behavior

Migration must still respect behavioral boundaries.

Do not force a behavior into RML/RCSS simply because it is possible to move code.

For each candidate, evaluate:

```text
Legacy implementation
        ↓
Identify intent
        ↓
Separate game logic from presentation
        ↓
Can RmlUi express it cleanly?
        ├── Yes → move to RML/RCSS
        ├── Partially → split C++ and RmlUi responsibilities
        └── No → retain C++ integration
```

The goal is **architectural ownership**, not maximum migration percentage.

### Legacy Theme Migration Priority

Prioritize migration in this order:

**P0 — Architecture violations**

* C++ hardcoded resolution/layout logic.
* C++-driven presentation that prevents themes from controlling layout.
* Duplicated legacy/custom UI implementations.
* C++ code that directly manipulates presentation solely because of historical UI architecture.

**P1 — Theme/modding limitations**

* Layout that cannot be changed without recompiling.
* Theme-specific positioning implemented in C++.
* Theme assets tightly coupled to C++ implementation.
* Legacy components that cannot be reused by Custom Themes.

**P2 — Maintainability**

* Repeated coordinate calculations.
* Repeated scaling logic.
* Repeated window/layout logic.
* Version-specific implementations that can safely converge into unified RmlUi components.

**P3 — Cleanup**

* Historical abstractions with little remaining value.
* Minor presentation logic that can be simplified after the architecture is established.

### Definition of Done for Legacy Components

A migrated Legacy Theme component should, where technically practical:

* Preserve the intended legacy appearance.
* Preserve the intended legacy interaction semantics.
* Use the unified RmlUi component architecture.
* Keep layout in RML/RCSS rather than C++.
* Keep theme-specific presentation in the Legacy Theme.
* Reuse generic components shared with Custom Themes.
* Use legacy sprites/assets through the theme layer.
* Avoid resolution-specific C++ branches.
* Avoid component-specific scaling calculations.
* Allow layout changes without recompiling C++.
* Allow theme overrides where appropriate.
* Keep genuine gameplay behavior in C++.
* Integrate with the existing drag/customization system rather than replacing it.
* Maintain persisted user customization independently from theme defaults.

### Guiding Principle

The Legacy Theme should be treated as:

> **Legacy in appearance and compatibility, but not unnecessarily legacy in architecture.**

The desired end state is not:

```text
Legacy UI
    ↓
Legacy C++ implementation
```

It is:

```text
Legacy UI intent
      ↓
Unified RmlUi components
      ↓
Legacy Theme
      ↓
Legacy assets + Legacy RML/RCSS
```

This allows the project to preserve the original MU experience while progressively removing
historical UI implementation constraints.

**When migrating legacy code, preserve the behavior users recognize—not the implementation
technique that originally produced it.**

---

**Worked example on this branch**: `CNewUIMainFrameWindow`'s skill-hotkey number readout
(`RenderSkillIcon()`, `NewUIMainFrameWindow.cpp`) used to draw a legacy digit-sprite subscript in
C++, unconditionally for both themes. Retired entirely — `CNewUISkillList::GetHotKeySlotNumber()`
now just returns the assigned number (pure data, theme-agnostic), `SyncRmlModel()` binds it into
the shared model, and each theme's own `main_frame.rml`/`.rcss` decides where and how to show it
(modern: upper-left corner of the slot, matching its Q/W/E/R treatment; legacy: bottom-right,
matching the retired subscript's original position) via `.skill-hotkey-label`. Zero
`GetActiveThemeName()` checks anywhere in the result — the same worked example for the gauge-bar
numeric readout (`hp_text` "935 / 935" vs. legacy's own `hp_current_text` "935": both computed
unconditionally in C++, only which field each theme's markup binds differs).

**Next**: [`STATUS.md`](STATUS.md) for what's actually implemented against this document (and
[`architecture-principles.md`](architecture-principles.md)) today.
