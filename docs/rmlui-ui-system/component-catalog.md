# Reusable Component Catalog

Use this catalog to choose an existing mechanism and find a working reference before adding
UI. Entries distinguish **components** (reusable C++ behavior), **style contracts** (shared
RML/RCSS classes), and **patterns** (bindings each feature owns). Theme-specific styles and
domain adapters have narrower scopes than general controls.

[Building New UI](building-new-ui.md) covers ownership, lifecycle, input and validation.
[Architecture Principles](architecture-principles.md) governs the RML/RCSS/C++ responsibilities.
Exact configuration fields and API contracts belong in the linked source headers.

## Window / Panel

Choose the owner using [Building New UI](building-new-ui.md#choose-a-component-and-an-owner).
For presentation, select a frame that fits the window family:

| Mechanism | Scope and reference |
|---|---|
| `base.rcss` frame styles | Theme-specific chrome: modern's `.modern-frame`, `.modern-panel`, `.modern-frame-accent` and `.modern-inset`; legacy's `.panel-cap-top`, `.panel-middle` and `.panel-cap-bottom` |
| `window_shell.rml` template | Shared shell for `dp`-authored documents, including menus and social windows. Link the template and use `template="window_shell"` on the body; see `generic_menu_dialog.rml` |
| `docked_panel_frame.rcss` | Shared partial in both themes for the right-docked character, quest, pet and party panels. Check its selectors before joining the family; inventory windows have their own frames |

The shell supplies frame structure; it is not a window owner or a universal replacement for
reference-pixel panels. Theme-specific frame details belong in the assets and
[Theming & Modding](theming-and-modding.md).

[`BindWindowClose()`](../../src/source/UI/RmlBridge/RmlWindowClose.h) binds `window_close` for
the frame's corner close action and other exit controls. Pass a window ID for normal hiding,
or a callback when closing has domain semantics, such as cancelling a trade.

## Button

**Style contract:** `.btn`, `.btn-ok`, `.btn-cancel` and `.btn.disabled` in both themes'
`base.rcss`. Apply role classes alongside `.btn`; the theme supplies their appearance.
Bind disabled state and validate the action in C++, even though `.disabled` blocks pointer input.

Server selection uses `.server-group-btn` (with `.checked`) and `.server-option-btn`
(with `.gray` / `.orange` category colors), supplied by both themes' `base.rcss`.
Use a `.button-label` child for the pressed text offset. `server_select.rml` is the reference;
its stylesheet owns button and gauge geometry. Legacy uses the original `cha_bt` group
and `server_b2_all` server sprites at their native aspect ratios within the current two-column layout.

Icon buttons have a narrower contract: `.btn-icon`, `.btn-icon-primary`, `.btn-icon-label` and
`.btn-icon-caption` are used by the login and character-selection documents. The modern base
supplies shared styling; legacy supplies consumer-specific sprite styling. Copy the relevant
`login_main.rml` or `char_sel_main.rml` structure rather than assuming identical theme styling.

For root-transformed reference-pixel panels, modern supplies `.modern-btn-px` and
`.modern-icon-btn-px` with `.modern-icon-btn-glyph-px`. The MU Helper and inventory documents
are references. These are modern-only styles; `.modern-icon-btn-px` also carries default
geometry, so inspect it before reuse. Use reference-pixel styles where `dp` sizes would scale twice.

Counter-scaled labels use `.sharp-middle` and, where appropriate, `.sharp-centre`; see below.

## Checkbox

**Style contract:** `.checkbox-row`, `.checkbox-box`, `.checkbox-box.checked` and
`.checkbox-label` in both themes' `base.rcss`. A feature binds its boolean state to the checked
class and handles the toggle through a RmlUi event. These classes supply presentation, not a
C++ checkbox object or automatic state management.

The MU Helper documents demonstrate the structure and state bindings. Modern's
`.modern-checkbox-px` is the reference-pixel variant for transformed panels.

## Text field

Use stock RmlUi `<input>` with `.text-field`. RmlUi supplies editing, caret, selection,
clipboard, password masking and IME composition. The feature owns the semantic value,
validation and limits; RCSS owns the field's appearance and consumer-specific dimensions.

- Bind the value with `data-value`; do not poll the element every frame.
- Set length limits from the owning code. When changing `type`, configure type and limits before
  setting the value: changing type rebuilds the input implementation.
- Use `data-attrif-disabled` and style `:disabled` when editing must stop. A dimmed class alone
  does not disable an input. Surface invalid state with `.error`.
- Focus once after visibility and content synchronization. Use transition-based visibility,
  blur on hide, and implement `TakesTypingFrom()` when the owner needs Enter, Esc or history.
  See [input and visibility](building-new-ui.md#connect-visibility-input-and-actions).
- Use `autofocus` only when immediate typing is intended; it suspends ordinary window hotkeys.

For digits-only fields, mark `text-field--numeric` and attach
[`AttachNumericInputFilter()`](../../src/source/UI/RmlBridge/RmlNumericInputFilter.h) after each
document build. It rejects typed characters without rewriting the caret. Clipboard paste bypasses
`textinput`, so also use `KeepDigitsOnly()` when reading numeric input and validate its range.
Consumers need no separate IME integration.

References: `login.rml` for password and Tab navigation, `my_shop.rml` for a name field,
`chat_input.rml` for focused-window key handling, and `GenericConfirmDialog.cpp` for a field
reconfigured per invocation. Keep overlapping frame art earlier than editable controls in
markup so it does not cover their text or caret.

## Counter-scaled text

**Style contract:** use this when text inside a root-transformed panel must retain its physical
font size. Bind `data-style---root-scale="root_scale"` on the root. Both themes' `base.rcss`
provide `.sharp-text` (top-left origin) and `.counter-scaled` (the element's own origin), which
cancel that scale. Per-axis transforms use `--root-scale-y` and `.counter-scaled-xy`.

The theme expresses the layer's lengths in RCSS, for example
`width: calc(160px * var(--root-scale));`; native font size may remain model-bound.
Do not bind static dimensions multiplied by `root_scale` from C++.

Use one `.sharp-text.sharp-flow` layer for stacked lines, with `--flow-w` and optional `--flow-h`.
`.centred` centres that flow vertically. Separate counter-scaled siblings retain physical layout
sizes and do not form an ordinary scaled text flow. `npc_quest` is a reference.

`.sharp-middle` centres a label vertically on its button; `.sharp-centre` centres it horizontally.
Coordinate rules and scale validation belong in [Layout, Anchoring & Scaling](layout-and-scaling.md).

### Marquee and shrink-to-fit

Use `.marquee` for a single-line label whose box must stay fixed. Its first child contains the
text; overflow shows `..`, and pointer presence scrolls the full text back and forth even if the
label lets clicks through. See [`RmlMarquee.h`](../../src/source/UI/RmlBridge/RmlMarquee.h) and
the MU Helper config labels. Prose that can grow should wrap or scroll instead.

Use `.native-fit` for legacy-compatible shrink-to-fit text. The theme sets a one-line box and
inherited font size; the shared pass shrinks text to fit within the native minimum. It applies
only under the `NativeTextSize` capability and requires physical-pixel text geometry.
See [`RmlNativeTextFit.h`](../../src/source/UI/RmlBridge/RmlNativeTextFit.h) and
`generic_menu_dialog.rml`. This is distinct from marquee or wrapping.

Use `.text-fit` for explicit shrink-to-fit in either theme, including the map-name banner.
It uses the inherited font size as its maximum and reduces that size without transforming glyphs.
Give the element a bounded width, direct text children and `white-space: nowrap`; physical-pixel
layout keeps the fitted glyphs sharp.

## Scrolling pane

**Style contract:** `.scroll-pane` in both themes' `base.rcss`. RmlUi scrolls document content
and generates the scrollbar elements. The consumer supplies a bounded height through `height`,
`max-height` or constrained flex layout. Use `data-for` for repeated content.

The shared class sets `overflow: hidden auto` and `pointer-events: auto`. Overflow order matters:
only vertical Auto/Scroll creates a vertical scrollbar. Generated scrollbar parts use ordinary
selectors such as `scrollbarvertical`, `slidertrack` and `sliderbar`.

A pane over the world claims pointer interaction across its area. For click-through content,
set the pane and content to `pointer-events: none`, then opt the scrollbar back in at every
level. Element hover/click behavior lost this way needs an explicit alternative; `chat_log.rcss`
and `CChatLogWindow` demonstrate this specialized arrangement.

Set scroll position once after changed content has synchronized. Continuous pinning overrides
the user's wheel and drag. For bottom-up lists, use a flex column with `margin-top: auto` on the
first item; `justify-content: flex-end` can push early lines outside the scroll area.
Use `align-items: flex-start` for per-line backgrounds.

When a transformed panel's dense text must retain physical size, counter-scale the pane and
express its dimensions through `calc(... * var(--root-scale))`. Give column rows an explicit
RCSS width when scrollbar appearance must not shift their columns; a `100%` row shrinks with the
pane's available content width. `guard_window` is the counter-scaled reference;
`move_command` is a selectable-list reference.

## List / repeated rows

**Pattern:** bind a `std::vector<T>` with RmlUi `data-for`, registering its row type and array
with the model. The feature owns row data and selection; markup supplies events and RCSS supplies
selected/hover states. There is no shared C++ ListBox wrapper.

Use `buff_strip.rml` for a simple repeated strip, `my_quest_info.rml` for structured rows, and
`server_select.rml` for row selection. Editable repeated entries need care when removing rows:
RmlUi preserves elements by array index, not identity. See
[Engine Findings](engine-findings.md#rmlui-build).

## Tab / TabBar

**Pattern:** keep the active tab authoritative in the feature, bind it to the model, toggle each
button's active class, and hide inactive content:

```xml
<div class="tab-btn" data-class-active="active_tab == 0"
     data-event-click="select_tab(0)">First tab</div>
<div data-class-hidden="active_tab != 0">...</div>
```

Callback names are feature-specific. This is a binding pattern, not a shared TabBar class;
`.tab-btn` styling is consumer-specific. Copy `my_quest_info.rml` for three content panels or
`pet_info.rml` for two sprite-art tabs. Each theme styles the active state independently.

## Level gauge

**Component and style contract:** `.level-gauge`, `.level-gauge-fill` and `.level-gauge-hit`
in both themes' `base.rcss`, with
[`ApplyLevelGaugeEvent()`](../../src/source/UI/RmlBridge/RmlLevelGauge.h) for interaction.
The consumer sizes the rail and fill, and binds `mousedown`, `drag` and `mousescroll` on the hit
child to its callback.

The helper maps presses and drags against the drawn bar, steps wheel input, clamps to
`0..maxLevel`, and returns a value only when changed. Supply the feature's pointer-to-level
mapping. Its wheel handler stops propagation so a containing scroll pane does not also scroll.

Reflect the fill using `level-N` classes or justified state-driven width. The options window's
sound/music/effect gauges and MU Helper detail thresholds are references. The transformed-pointer
constraint is explained in [Engine Findings](engine-findings.md).

## Dialog

Choose the component by interaction shape, not by the legacy class name.

### Confirmations and prompts

[`CGenericConfirmDialog` / `GenericDialogConfig`](../../src/source/UI/Dialogs/GenericConfirmDialog.h)
is the shared configuration-driven component for confirmations, text/PIN entry, timed progress
and optional item/image previews. One active dialog is shown at a time; later requests queue.
Configure it rather than adding a subclass or a document for each prompt.

Buttons have explicit roles: Primary, optional Secondary, and optional Cancel. Enter invokes
Primary. Esc invokes Cancel when configured, otherwise Primary; it never invokes Secondary.
Timed progress dialogs hide buttons and do not accept Esc dismissal.

Callbacks can call `KeepOpen()` to veto resolution, including Cancel. Read entered text through
`GetInputText()` and validate it before accepting. A text field with a `maxValue` gets a Max
button in the Secondary slot that fills in that value, read on each click; typing is not clamped
to it. Requests can carry an `isValid` predicate; expired requests dismiss without callbacks. IDs
support pending checks and cancellation, while tags support automation answers. Exact fields and
contracts remain in the header.

The current component owns one model/document view. Live item previews use `ItemCameraTarget`
inside that document. References: `GenericConfirmDialog.cpp`, `generic_confirm_dialog.rml`,
and each theme's override/styles. Non-UI callers use
[`ShowConfirm`](../../src/source/UI/Dialogs/ConfirmRequest.h).

### Menus of actions

[`CGenericMenuDialog` / `GenericMenuConfig`](../../src/source/UI/Dialogs/GenericMenuDialog.h)
is the sibling component for an arbitrary list of labelled actions. It also queues requests
behind one active menu. An enabled button runs its callback and closes; Esc uses `onCancel`.
There is no resolution veto. A callback may enqueue the next menu, as the Gem Integration flow does.

Use configuration for content and supported layout variants. `kind` identifies compatibility
presentation (the original box the menu copies, `data-kind`, which also switches the document to
the native-frame layout), while `systemMenu` selects the system-menu presentation. Every caller
also sets `id`, which only names the menu (`data-id` on the body) so a theme can target one menu
without new layout rules; it changes nothing by itself. Column support is currently
limited to the implemented two-column variant; consult the header before adding another shape.
Frame structure comes from `window_shell.rml`. References: `generic_menu_dialog.rml`, its themed
assets, and the menu construction in `CustomMessageBox.cpp`.

### Existing message-box integration

[`MessageBoxView`](../../src/source/UI/Dialogs/MessageBoxView.h) renders existing message-box
owners with their callbacks, role-labelled lines, tables, buttons and progress. Its `kind` lets
the theme choose compatibility layout. Use it when maintaining those owners; prefer the generic
components for new interactions that fit their shapes.

The [migration ledger](migration-ledger.md) records existing dialog destinations.
Preview commands belong in its [event-preview reference](migration-ledger.md#event-preview-reference).

## Tooltip

**Component:** one shared hover-tooltip document. Choose the entry point by content:

| Need | Mechanism and reference |
|---|---|
| Static or model-bound element hint | `data-hint` or `data-attr-data-hint`; the shared `DocumentHints` pass finds the hovered element's nearest hinted ancestor |
| Game-state content anchored to an element | [`ElementTooltip`](../../src/source/UI/RmlBridge/RmlElementTooltip.h); bind enter/leave events and refresh content while hovered, as `CBuffStrip` does |
| Direct hover content from native or feature code | [`Tooltip::Show(Config, Owner)`](../../src/source/UI/RmlBridge/RmlTooltip.h); pass lines and an anchor in window pixels |
| Persistent help shown while a window is open | The feature's own document, such as `ItemHelpView`; this does not use the shared hover-tooltip owner |

Hint elements need pointer interaction. Dynamic element tooltips ignore bubbled mouseout events
from children that do not represent leaving the tracked element.

Use a stable owner token when callers compete, and hide only that owner's tooltip. The API permits
ownerless calls, whose hide behavior can affect another caller. Refresh direct per-frame hover
content appropriately; `Config::refreshEachFrame` lets it disappear when refresh stops.

The shared tooltip measures and clamps to the viewport. `flipAnchorY` allows growth from the
opposite edge when needed. It is safe to call `Show()` repeatedly; unchanged content reuses its
measurements. Exact options and palette belong in the header; depths belong in `RmlStackingOrder.cpp`.

Skill tooltip content remains domain-specific in `UI/HUD/Skills/SkillTooltip.h`; it builds lines
for this presentation mechanism. Item help and the help window remain separate persistent views.

## Dragging

**Component:** [`MakeDraggable()`](../../src/source/UI/RmlBridge/RmlDraggable.h) attaches a
RmlUi handle to an absolutely positioned panel. Attach after every document build. The helper
sets the handle's drag and pointer properties; controls inside it opt out with `drag: block`
(the shared base styles cover inputs, buttons and click handlers).

A nested control's own drag, such as a level gauge, does not move the enclosing panel.
`ResetDraggedPosition()` restores theme placement; `KeepInsideWindow()` clamps a dropped panel.
Use move/end callbacks when the owner must synchronize position or commit a preference.
Eligibility, workspace behavior and persistence policy belong in [Window Placement](window-placement.md).

## Layout utilities

**Style contract:** `.anchor-{top,bottom}-{left,right}`, `.center-{x,y,both}`,
`.stretch-{x,y,both}`, `.hidden` and `.layout-anchor` in both themes' `base.rcss`.
Use [Layout, Anchoring & Scaling](layout-and-scaling.md) for units and placement recipes.

Shared geometry helpers are integration tools, not layout sources:

| Helper | Use |
|---|---|
| [`DrawnContentBox()` / `DrawnBox()`](../../src/source/UI/RmlBridge/RmlElementBox.h) | Read transformed element geometry in window pixels for native drawing or interaction |
| [`SlotPlacement` / `FillPlacementSize`](../../src/source/UI/RmlBridge/RmlPanelGeometry.h) | Apply workspace placement and fill dimensions to managed panels |
| [`RegisterWorkspaceDocument()`](../../src/source/UI/RmlBridge/RmlWorkspaceParticipant.h) | Register a `dp`-sized document root with the workspace |

Placement configuration and registration recipes live in [Window Placement](window-placement.md).

## Data binding

**Component:** [`RmlModelBinder<T>`](../../src/source/UI/RmlBridge/RmlModelBinder.h) owns model
state and its data-model handle. Windows normally reach it through `ThemedView` using
`GetModel()`, `MarkDirty()` and `Binder()`.

Use [`SyncField()` / `SyncFieldFrom()`](../../src/source/UI/RmlBridge/RmlSyncField.h) to assign
and dirty only changed fields. The first accepts a value; the second reconciles a field from a
staged model:

```cpp
SyncField(view.Binder(), &Model::activeTab, "active_tab", activeTab);
SyncFieldFrom(view.Binder(), &Model::activeTab, "active_tab", updated);
```

The member and value types must match. Register row structs and arrays whenever the model is
created. Model creation and theme reload ownership are covered below.

## Semantic colours

**Pattern:** bind meaning and let themed RCSS choose the colour. For example, character
attributes bind `potion`/`boosted` state to `.attr-potion`/`.attr-boosted`; wealth tiers select
`.gold-*` classes. `character_info.rml` and both themes' styles are references.

Avoid inline `data-style-color` for theme-owned states: it overrides stylesheet rules.
Resolved compatibility content palettes are a narrower case; conversion details belong in
[Engine Findings](engine-findings.md#compatibility-text-and-tooltip-metrics).

## Theming

**Component:** [`ThemedView<Model>`](../../src/source/UI/RmlBridge/RmlThemedView.h) owns each
feature's model and themed documents. Use `ThemedView<>` when no model is needed.

```cpp
UI::RmlBridge::ThemedView<Model> view{
    "feature",
    [](Rml::DataModelConstructor& constructor, Model& model) {
        BindModel(constructor, model);
    },
    {{"Data/Interface/RmlUi/feature.rml"}}};
```

Call `Ensure()` to build: it creates the model before loading markup, waits for unavailable
contexts and retries failed loads. `Release()` unloads documents, removes the model and unregisters
reload handling. `Hide()` retains the built view. A theme reload preserves model values and
rebuilds documents, restoring visibility with the configured focus and stacking options.

Use `afterBuild` to attach controls/listeners and field configuration, `beforeUnload` to clear
attachments and cached elements, and `afterReload` for geometry-dependent refresh. Per-instance
views use `modelPlaceholder` and `SetModelName()` before model creation; `ChatRoomView` is a reference.

Examples: `CCharacterInfoWindow` (one document), `CMainFrameWindow` (multiple documents sharing a
model), `CGenericConfirmDialog` (one document with an item preview), and `ReconnectDialog.cpp`
(a feature-owned view without a window class).

Theme resource resolution, markup overrides and declared capabilities belong in
[Theming & Modding](theming-and-modding.md). Do not branch presentation on a theme name.

## Native content inside a document

### Render targets

**Component:** [`RenderTarget`](../../src/source/UI/RmlBridge/RmlRenderTarget.h) draws native
content into a texture shown by a document image. Use it for live 3D that belongs to a window,
so overlapping documents can cover it normally.

Provide a drawer, resize to the element's drawn size in physical pixels, enable only while needed,
and bind `Source()` to the image. The drawer configures its projection and draws into the supplied
viewport. `Source()` is empty until the first rendered frame. `PhotoViewerControl` is an element-side
reference; the renderer owns capture scheduling and texture lifetime.

Native content belonging to a window must not use the post-RmlUi cursor pass.
The [frame lifecycle](README.md#frame-lifecycle-render-seams) describes rendering order;
[render-target findings](engine-findings.md#render-target-integration) cover capture and blending.

### Item-camera content

[`ItemCameraTarget`](../../src/source/UI/Inventory/ItemCameraTarget.h) wraps a render target
with the UI item camera. Its drawer uses window pixels; `Sync(image, enabled)` reads the drawn
image box, sizes the target and sets its source. The camera is cropped to that box and restored
after drawing. `SetFieldOfView()` handles features that use a different item-camera field of view.

References: `CItemHotKey`, the confirm dialog's item preview and `EventItemEntryView`.
Use drawn slot geometry when framing an item; do not copy RCSS dimensions into C++.

### Item grids

**Domain integration:** `ItemGridModel.h`, `ItemGridGeometry.h` and both themes' `item_grid.rcss`
connect the inventory controller's state to themed grids. Register cells with
`RegisterItemGridCells()` and bind `Cells()` for tints, drop previews and stack counts. Native
items and remaining effects draw into the window's item-camera target.

`CInventoryCtrl::FollowGridPx()` reads the drawn grid and cell pitch into window-pixel
`GridGeometry`. Rendering, hit tests, drop targets and tooltips use that geometry. RCSS owns
`--item-cell` and grid dimensions. `PickupAnchor` stores a drag's hold point in cells so it works
between grids with different pitches. `CMyInventory` is the reference, including equipment slots.

## Native 3D viewer input

**Domain adapter:** [`PhotoViewerControl`](../../src/source/UI/Social/PhotoViewerControl.h)
connects a letter portrait's native viewer to document gestures: drag-to-turn, right-click reset
and its help toggle.

The slot claims pointer interaction; document-level move/up listeners keep tracking when a drag
leaves it. Gesture deltas convert to `dp` for scale-independent feel. Use document events rather
than legacy mouse-press flags for content inside an interactive RmlUi panel. Wheel events also
route through the UI consumer; the [input-routing findings](engine-findings.md#document-input-routing)
explain propagation.

## Skill icons

**Domain helper:** [`ResolveSkillIcon()` / `IconSpriteName()`](../../src/source/UI/HUD/Skills/SkillIconAtlas.h)
resolve normal/master skill atlas sprites and lit/grey states. Link `skill_icons.rcss` and
`master_skill_icons.rcss`, then bind the resolved `image(...)` decorator.
The skill HUD, master tree and MU Helper provide references. Skill meaning and availability stay
in the feature; this helper resolves presentation assets.

## Sprite digits

**Domain helper:** [`DigitCells()`](../../src/source/UI/RmlBridge/RmlDigitCells.h) resolves a
number to texel rectangles in a digit strip. Bind the returned array; the theme lays out the
cells and chooses the image. Non-digit characters reserve an empty cell. `CryWolf` and
`CursedTempleSystem` are references. Use ordinary text unless compatibility needs sprite digits.

## Specialized views

These adapters serve existing feature families; they are not general window bases.

| View | Scope and reference |
|---|---|
| [`EventItemEntryView`](../../src/source/UI/Events/EventItemEntryView.h) | Shared entry-window presentation, text/buttons and optional live item slots; Doppelganger and Empire Guardian are references |
| [`ItemHelpView`](../../src/source/UI/Inventory/ItemHelpView.h) | Persistent item-explanation lines and level tables; item and set-item explanation windows own separate documents |

Modern's `.modern-wallet-px` and `.modern-control-well` are theme-specific styles for inventory
money strips and inset control backgrounds. They supply appearance and may carry default geometry;
inspect `base.rcss` and the inventory/login consumers before reuse.

## Feature operations for networking and game code

Non-UI callers use small feature-operation headers with decoded values and no RmlUi/window-class
dependency. Examples are `UI/Chat/ChatMessages.h`, `UI/Inventory/TradeUpdates.h` and
`UI/Dialogs/ConfirmRequest.h`. Extend the appropriate family's header when a caller needs another
operation; keep protocol decoding and result-code meaning with the caller.

Do not introduce a generic UI command interface or expose presentation internals to packet
handlers. The [architecture policy](architecture-principles.md) governs this boundary.

## Concepts without a shared primitive

Use the existing mechanism appropriate to the feature rather than assuming every concept needs
a wrapper:

- Loading progress can use stock RmlUi `<progress>` (`title_scene.rml`). HUD health, mana and
  experience fills are window-specific markup/RCSS; interactive level gauges use the helper above.
- Lists and tabs use the binding patterns above; fields use stock inputs and shared styles.
- Notifications have feature-owned presentations, such as `UI/HUD/Notices`; there is no general
  notification component or HUD container to instantiate.

## Using this catalog

Reuse a listed mechanism within its scope. When a new mechanism proves useful across consumers,
add its purpose, essential constraints and a working reference here. Keep configuration fields
in headers, engine quirks in Engine Findings, and migration history in the migration ledger.
