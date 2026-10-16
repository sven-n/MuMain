# Building New UI: Windows, Dialogs, HUD Panels, and Widgets

Use this guide to choose an implementation pattern, connect a component to the client's
lifecycle and input, and validate it. Read [Architecture Principles](architecture-principles.md)
first; it defines the governing ownership and quality requirements.

Visible UI uses RmlUi and the shared classes in each theme's `base.rcss`. Start with the
[Component Catalog](component-catalog.md) for buttons, fields, lists, dialogs, tooltips and
native-content integration. Retired native widget families are not alternatives for new UI;
the [Legacy UI Class Map](migration-ledger.md) records their replacements.

## Choose a component and an owner

Describe the component's purpose, anchor, size, responsive behavior and relationship to nearby
windows before writing code. For legacy compatibility changes, also establish the original layout
intent using the [compatibility workflow](architecture-principles.md#27-required-workflow-for-legacy-compatibility-changes).

First check whether an existing primitive fits. Confirmations and text prompts use
`CGenericConfirmDialog`; menus of actions use `CGenericMenuDialog`. Configure those components
rather than introducing a subclass or document for each call. Their usage and exceptions are in
[Dialog](component-catalog.md#dialog). An element within an existing window normally belongs in
that window's document and model.

For independently managed UI, choose the owner by its lifecycle:

| Kind | Owner and integration | Reference |
|---|---|---|
| Gameplay window or HUD panel | Derive from `mu::ui::window::CObject`; register with the gameplay `CManager` and `CSystem` in `WindowSystem.cpp` | [`CCharacterInfoWindow`](../../src/source/UI/Character/CharacterInfoWindow.h) |
| Login or character-scene window | Use `CObject` with the existing scene lifecycle and `CSceneUICoordinator`; documents must be hidden or released on scene exit | [`CMsgWin`](../../src/source/UI/Windows/MsgWin.h) |
| Shared view with no independent window lifecycle | Keep a `ThemedView` in the owning feature module and expose feature operations | [`RememberPasswordPrompt`](../../src/source/UI/Windows/RememberPasswordPrompt.cpp) |
| Existing friend, mail or chat-room UI | Extend the family through `CFriendWindow` and its social-window manager | [Social window integration](migration-ledger.md#social-window-integration) |

`CObject` is the base for managed windows, not for every element or reusable helper. `CManager`
handles registration, dispatch, ordering and shared measuring units; RmlUi handles the document's
hierarchy, layout and element events. Use these existing owners rather than adding another
window manager or native container hierarchy.

Place C++ files in their feature domain: `UI/Character/`, `UI/Inventory/`, `UI/Combat/`,
`UI/Events/`, `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Social/`, `UI/Quests/`, `UI/Options/` or
`UI/MuHelper/`. Shared modal components belong in `UI/Dialogs/`; game-window integration helpers
belong in `UI/RmlBridge/`. `UI/Widgets/` is for feature-independent controls, and `UI/Windows/` is
the existing login/credit set. Name classes and assets for their responsibility and follow
[the repository coding rules](../CODING_RULES.md) for file and namespace conventions.

## Ownership: what the C++ side of a window may own

Keep the implementation responsibilities clear:

| Owner | Responsibility |
|---|---|
| Feature C++ | Game and UI state, view-model values, validated actions, lifecycle and preference persistence |
| RML | Element hierarchy, bindings and action requests |
| RCSS and theme assets | Layout, dimensions, anchors, appearance and visual states |
| Shared integration | Scale inputs, workspace placement, themed loading and native render targets |

Apply these rules when defining a model or handler:

- Expose purpose-built values and semantic state, not arbitrary game objects or a replay of native
  draw calls. For example, bind a wealth tier and let RCSS choose its color.
- Keep each selection or tab authoritative in one place. Bind it to the view rather than
  maintaining a second independent selection in a native control.
- Keep static coordinates, colors and alignment in RCSS. Inline `style` attributes and
  `data-style-*` bindings take precedence over stylesheet rules; use them only for justified
  state-driven values or shared integration. See [Engine Findings](engine-findings.md#rmlui-build).
- Native content follows the drawn element's geometry. Do not duplicate panel or item-slot
  constants in C++ and RCSS. Coordinate spaces and readback are covered by
  [Data-driven geometry and native companions](layout-and-scaling.md#data-driven-geometry-and-native-companions).
- Validate actions in C++, even when markup supplies an action name or index. Presentation must
  not bypass class restrictions, available points, inventory rules or server-facing validation.
- Keep domain behavior in its feature layer. Item move/split/stack rules and skill-tooltip content
  do not belong in a generic UI component. Non-UI callers use
  [feature operations](component-catalog.md#feature-operations-for-networking-and-game-code).
- Update both `legacy` and `modern`. Presentation differences belong in themed markup or RCSS;
  an engine behavior difference uses a declared capability, never a theme-name condition. See
  [Theming & Modding](theming-and-modding.md).

Static geometry is not made themeable by binding it from C++. State-driven geometry, such as
projected markers or tooltip anchors, needs a documented reason for the bound-geometry guard.
Reference-pixel lengths for counter-scaled text belong in RCSS using `calc()` and the shared
`--root-scale`; see [Counter-scaled text](component-catalog.md#counter-scaled-text). Current
compatibility exceptions are recorded in [Known limits](README.md#known-limits).

## Define the model and themed documents

A window's `UI::RmlBridge::ThemedView<Model>` owns its model and documents. It creates the model
before loading markup that references it, and participates in theme reloads. Use `ThemedView<>`
when there is no bound state. The declaration and hook recipes are in
[Theming](component-catalog.md#theming); exact options live in
[`RmlThemedView.h`](../../src/source/UI/RmlBridge/RmlThemedView.h).

Shared documents live under `src/bin/Data/Interface/RmlUi/`, with stylesheets and optional markup
overrides in `themes/legacy/` and `themes/modern/`. Link `base.rcss` for the shared interaction and
component rules. Use shared markup when it serves both presentations; theme-specific structure
follows [Shared markup and theme-specific structure](theming-and-modding.md#shared-markup-and-theme-specific-structure).

In the owner's initialization path, register the managed window and call the view's `Ensure()`.
It is cheap once built and retries when the context or document was unavailable. Concrete windows
provide their own `Create()` and `Release()` conventions; these are not methods declared by
`CObject`. Tear down the view and remove the window's manager registration when its owner releases
it. Follow the chosen reference's lifecycle rather than assuming all owners initialize alike.

Read current game state into the model in the owner's update path and mark changed bindings
dirty. Register model structs and arrays on every model creation; a static registration guard
breaks rebuilding. When reopening a scene resets model values, mark those changes dirty too.

A theme switch preserves model values while replacing documents. Reattach listeners, drag
handles, filters and field limits through `afterBuild`; refresh geometry-dependent state through
`afterReload`; clear cached element pointers and attached controls through `beforeUnload`.
Do not retain an element pointer across document replacement.

## Connect visibility, input and actions

For a pure RmlUi window, `Render()` returns `true` without drawing chrome; the runtime renders its
document. `CObject::Show(bool)` is virtual. A window may override it to synchronize its document,
or use `SyncDocumentVisibility()` in its update path. Synchronize only on visibility transitions:
calling `ElementDocument::Show()` every frame can steal focus from an input field.

`Update()` runs for every enabled window, shown or not, so `SyncRmlModel()` returns once the
window is hidden, right after `SyncDocumentVisibility()`. The model then catches up in the call
that shows the document. A window whose `Show(true)` shows the document itself calls
`SyncRmlModel()` first. Anything the window owns outside its document (a shared tooltip) is
released on that early return.

Use RmlUi event callbacks for visible controls. Repeated rows bind data with `data-for`; use the
catalog's [List / repeated rows](component-catalog.md#list--repeated-rows) and
[Tab / TabBar](component-catalog.md#tab--tabbar) patterns instead of inventing a native control.
Give interactive panels `pointer-events: auto`; `base.rcss` makes document bodies click-through
by default. Decorations over the world should not claim the pointer unless their interaction
requires it. Scroll-pane click-through behavior is in [Scrolling pane](component-catalog.md#scrolling-pane).

For managed-window mouse dispatch, `UpdateMouseEvent()` returns `false` only when it consumes
input. A pure RmlUi panel can return `!UI::RmlBridge::IsPointerOver(document)`; native content can
query its own element through `IsPointerWithin()` or `PointerIn()`.
[`RmlPointer.h`](../../src/source/UI/RmlBridge/RmlPointer.h) operates against drawn boxes, so
window code does not need to remap the pointer into a second coordinate system.

Keep only window-level keys, such as Esc and shortcuts, in `UpdateKeyEvent()`. Poll them through
`mu::ui::window::IsPress`/`IsRelease`/`IsNone`/`IsRepeat` (`WindowCommon.h`); `CInput::Instance()`
is for the existing login/character-select integration. Mouse events over a RmlUi panel may be
consumed before native button flags are set. Native viewer gestures therefore use document
events; see [Native 3D viewer input](component-catalog.md#native-3d-viewer-input).

A window that handles Enter, Esc or history while its field is focused must implement
`TakesTypingFrom(document)` for that field's document. Focus the field after the document is
shown and its content has synchronized, using a one-shot request rather than per-frame focus.
Blur focused fields when hiding their documents. Field binding, limits, numeric filtering and
IME behavior are detailed in [Text field](component-catalog.md#text-field).

Static hover hints use `data-hint`; hints built from game state use the shared tooltip API or
`ElementTooltip`. Both are covered by [Tooltip](component-catalog.md#tooltip). Avoid a separate
per-window tooltip renderer.

At gameplay input call sites, include `Core::Input::IsMouseOverUI()` when deciding whether a click
may act on the world. A panel click must not also move the character or attack a target behind it.

## Integrate placement and scene participation

Choose a workspace slot for windows that arrange with other windows, `.stage` for centered
reference-layout panels, `.hud-board` for HUD-relative content, or RCSS anchors for independent
screen placement. The theme owns the layout; shared scale functions supply its units.

Read [Units and placement](layout-and-scaling.md#units-and-placement) before choosing lengths.
Directly laid-out scalable UI uses `dp`; root-transformed panels use reference `px`. Do not apply
UI scale twice or add a per-window calculator. Workspace registration, content/fill sizing,
arrangement rules and dragging are detailed in [Window Placement](window-placement.md).

Gameplay windows update through the main-scene lifecycle. Scene windows use their scene owner's
updates; being a `CObject` does not itself restrict a component to `MAIN_SCENE`. A persistent
document can remain drawable when its C++ owner is no longer updating, so register its scene
policy and explicitly synchronize lifecycle visibility.

Main-scene documents are suspended outside that scene by the shared scene gate. HUD documents
also wait for world loading through `CSystem::SyncMainSceneHudVisibility()`; add the new HUD's
visibility sync there when needed. Login and character-scene owners hide or release their
documents on exit. A window available in multiple scenes must be updated by each relevant owner
and use the matching scene policy; the options window is the existing example.

### Stacking order

Add each new document to
[`RmlStackingOrder.cpp`](../../src/source/UI/RmlBridge/RmlStackingOrder.cpp) with its depth and
`DocumentScene` policy. Choose the depth by its relationship to existing panels, HUDs, dialogs
and tooltips, keeping native dispatch order consistent with the document. The table is the source
of truth for numeric depths; scene policies are defined in
[`RmlStackingOrder.h`](../../src/source/UI/RmlBridge/RmlStackingOrder.h).

RmlUi sorts documents by `z-index`. Show/focus order and
`SyncDocumentVisibilityInFront()`/`Behind()` affect documents at equal depth; they do not raise a
low-depth document above higher-depth windows. Workspace regions supply geometry, not stacking
priority. Modal keyboard priority is separate: higher `GetKeyEventOrder()` values run first, and
a handler returning `false` consumes the key. Use the existing generic dialogs as the modal
reference; the ordering constraint is explained in [Engine Findings](engine-findings.md#cobjectcmanager).

The rendering sequence is described in [Frame lifecycle](README.md#frame-lifecycle-render-seams).
Native content within a window must occupy that document's depth, as described below.

## Integrate native content when required

RmlUi owns ordinary 2D chrome, text, icons and layout. Live 3D drawing remains native, but its
image belongs in the document through `UI::RmlBridge::RenderTarget`; item-camera content uses
`UI::Items::ItemCameraTarget`. The element's drawn box supplies the target geometry, so content
follows scaling, placement and overlapping windows. See
[Native content inside a document](component-catalog.md#native-content-inside-a-document).

Useful references are [`CItemHotKey`](../../src/source/UI/HUD/ItemHotKey.h) for item icons and
[`CMyInventory`](../../src/source/UI/Inventory/MyInventory.h) for grids and equipment.
Keep inventory actions in their domain controller; the theme owns cell and slot geometry.
Draw effects that belong with the native content into the same target, rather than above the
entire RmlUi context.

World overlays calculate projected positions in C++ and draw labels through the shared
world-label layer. Follow
[`CCharInfoBalloonMng`](../../src/source/Character/CharInfoBalloonMng.h) for projected labels and
[`WorldLabelLayer`](../../src/source/UI/Character/WorldLabelLayer.h) for recorded overlay drawing.
Native hit-test or bookkeeping companions follow the drawn RmlUi geometry; retaining one is not
permission to move presentation back into C++.

When replacing existing visuals, remove their unused native drawing and widgets, but first check
for resources or state that other components still use. Legacy-specific rendering constraints
belong in [Engine Findings](engine-findings.md), and existing exceptions belong in
[Known limits](README.md#known-limits).

## Validate the component

Before considering a new or changed component complete:

1. Confirm that its layout intent is expressed in RML/RCSS and that its model exposes state,
   rather than static presentation. Reuse the shared primitives and update both themes.
2. Run the existing asset and integration guards from the repository root:

   ```powershell
   python tools/check_rml_rcss_syntax.py
   python tools/check_rml_rcss_drift.py
   python tools/check_rml_bound_geometry.py
   python tools/check_layout_transform_users.py
   ```

   The syntax guard also runs in CTest (`rml_rcss_syntax`). It catches markup and values RmlUi only
   reports in its log, such as a self-closed `data-for` element or `line-height: inherit`.

   Contract-checker scope and exception markers are described in
   [Theming & Modding](theming-and-modding.md#shared-markup-and-theme-specific-structure).
   A geometry exception needs a `<!-- bound-geometry: reason -->` comment; the guard rejects
   static values multiplied by `root_scale`, which belong in RCSS. Read the diagnostic and
   document a genuine state-driven need rather than bypassing the check. Passing a guard does
   not establish runtime correctness.
3. Build with the [supported build setup](../build/README.md) and run checks relevant to the change.
   The stacking table has `rml_stacking_order_tests`; workspace, model, theme-reload and input
   helpers have tests under `tests/ui/`. Add layout coverage where appropriate using the
   [headless text-layout audit](layout-and-scaling.md#the-headless-text-layout-audit).
4. Exercise open, close and scene transitions. Check Esc/Enter with a focused field, click-through
   to the world, scroll/drag gestures and tooltip placement where applicable.
5. Test both themes across representative resolutions and the
   [75%/150% UI-scale sweep](layout-and-scaling.md#checking-it-the-scale-sweep). Check corner cells
   of native grids, resizing, dragging across scale changes and switching themes while open.
   Compare original behavior when making a compatibility change.
6. Record tested scenarios and remaining gaps with the change. A screenshot or preview is not
   evidence that server-dependent actions work.

For event UI, `$preview` lists available previews, `$preview <event>` supplies sample state, and
`$preview off` ends the preview. This can expose layout without a live event; teleporting to an
event map alone does not supply its state. Use the
[event preview reference](migration-ledger.md#event-preview-reference) for additional commands,
and test against a running server for gameplay behavior. Scripted checks can use the
[control socket](../control-socket.md) and [in-game tester](../in-game-tests.md).

## Preview a document in a browser

`tools/rml_preview.py` shows a document without building or starting the client. It turns the
document and its theme's stylesheets into an HTML page:

- it resolves files through the theme chain, as the game does;
- it sizes `dp` and `rem` for a window size, as `UI::Scaling` does;
- it evaluates the data bindings against a JSON model;
- it draws the sprites from the game's own textures, with the bundled fonts.

With `--png` it also screenshots each page, using Chrome, Edge or Chromium.

```powershell
python tools/rml_preview.py generic_confirm_dialog --theme modern `
    --model tools/rml_preview_models/generic_confirm_dialog.json --png
```

The pages land in `build/rml-preview/`, one per state of the model.

**The model.** It is the binding names of the document's data model. A document with several
models takes them keyed by model name, and `{"states": {...}}` renders one page per state. Names
the model leaves out evaluate to nothing, and the tool lists them.
`tools/rml_preview_models/` holds example models; add one for a document you preview often.

**Other options.** `--window 1920x1080` and `--ui-scale 150` show other sizes, and
`--pixel-ratio 2` gives sharper screenshots.

**Limits.** The browser lays the page out, not RmlUi, so the preview is close to the game but not
identical. It leaves out:

- live 3D content (items, render targets);
- scrollbars and font effects;
- placement that C++ pushes at runtime;
- state that isn't in the model.

Use it to look at a layout, a theme change or a long translation quickly, or to attach a picture
to a pull request. Check the result in the game before you rely on details.

## Where detailed guidance lives

| Task | Detailed reference |
|---|---|
| Apply ownership and compatibility requirements | [Architecture Principles](architecture-principles.md) |
| Select a primitive or configure a view | [Component Catalog](component-catalog.md) |
| Choose units, read geometry or test scaling | [Layout, Anchoring & Scaling](layout-and-scaling.md) |
| Arrange windows, fill slots or integrate dragging | [Window Placement](window-placement.md) |
| Edit theme resources, tokens or document contracts | [Theming & Modding](theming-and-modding.md) |
| Diagnose an engine or native-integration constraint | [Engine Findings](engine-findings.md) |
| Resolve an old class or specialized compatibility path | [Legacy UI Class Map](migration-ledger.md) |
| Understand render passes and current exceptions | [System overview](README.md) |

Keep detailed recipes in their owning guide and link to them here. Source headers remain the
reference for exact APIs, configuration fields and numeric depth values.
