# Reusable Component Catalog

What already functions as a reusable UI primitive (`architecture-principles.md` §20), named and
pointed at its file, so a port checks here before inventing a one-off mechanism. Entries under
"Does not exist" are real gaps: the next window that needs one defines its shape (§26).

## Window / Panel

Every window is a `mu::ui::window::CObject` owning one or more RmlUi documents. Two shapes
(`building-new-ui.md`'s "Shape of the kit"):

- **Pure RmlUi** — `CMsgWin`, `RememberPasswordPrompt`, `CCharInfoBalloonMng`, and most windows.
- **Live 3D items** — the inventory family (`CMyInventory`, `CTrade`, `CStorageInventory`,
  `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`,
  `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd`). One document each: the frame,
  the item grids ("Item grids" below), an `ItemCameraTarget` image the window draws its items and
  remaining native effects into, the stack counts, titles, buttons and text (`SyncRootTransform()`,
  since the grids still hit-test in reference coordinates).

Visual frame primitives are theme-specific, not shared (correct per §15 — presentation is the
theme's job, not the component's):

- `modern`: `themes/modern/base.rcss`'s `.modern-frame`/`.modern-panel`/`.modern-frame-accent`/
  `.modern-inset`, plus the `-crimson` palette variant (`themes/modern/README.md` has the
  full token table these draw from).
- `legacy`: `themes/legacy/base.rcss`'s sprite-based 3-part `.panel-cap-top`/`.panel-cap-bottom`/
  `.panel-middle`.

A second, narrower shared frame exists for one specific window family: the `PanelColumnX()`-docked,
single-document windows that visually read as one group on screen —
`character_info`, `my_quest_info`, `pet_info`, `party_info` today. Their `#panel`/frame sprites/exit
button/tooltip shape/group-box corner-and-fill technique (legacy) and forged-dialog panel gradient/
shell-edge/groove/header-rail (modern) are byte-identical, so they link a shared
`docked_panel_frame.rcss` (both themes) instead of each re-declaring it, so windows docked side by
side read as one family (STATUS.md's checklist item 7). **A new window joining this same `PanelColumnX` dock group should link this partial too**,
not copy-paste a fifth version — check its current window list before assuming it doesn't apply.
`CMyInventory` and the other item windows keep frames of their own.

## Button

Real shared contract across both themes already — `.btn`/`.btn-ok`/`.btn-cancel`/`.btn.disabled`,
same class names, same state model, each theme's own `base.rcss`. `.btn-ok` gets each theme's
"primary/hero" treatment (see `themes/modern/README.md`'s accents); plain
`.btn` stays neutral. The C++ native button classes are for native-only content
(`building-new-ui.md`).

Inside a panel scaled by `transform: scale(root_scale)`, `.btn`'s `dp` sizes would scale twice.
Modern has `.modern-btn-px` (and `.modern-checkbox-px` for checkboxes), the same recipe in
reference `px` with no size of its own; the MU Helper windows use both.

A counter-scaled button label (`.sharp-text`) centres itself on its button with `.sharp-middle`,
and `.sharp-centre` when the theme gives no width (`base.rcss`, both themes; `engine-findings.md`
has why).

## Counter-scaled text

Text inside a panel scaled by `transform: scale(root_scale)` is drawn sharp by laying it out in
physical pixels and cancelling the panel's scale. `#panel` binds the scale as a custom property,
`data-style---root-scale="root_scale"`, beside its transform; base.rcss's `.sharp-text` (origin top
left) and `.counter-scaled` (the element's own origin) apply `scale(calc(1 / var(--root-scale)))`.
The layer's own lengths are physical, so the theme states them as reference px times the scale:
`#title { width: calc(160px * var(--root-scale)); }`, or `calc(100% * var(--root-scale))` for the
width of the box it sits in. Only its font size (the native text size) stays bound. A panel that
stretches per axis (a Hud layout's `#screen`, `gens_ranking`) also binds `--root-scale-y`, and its
layers take `.counter-scaled-xy`.

## Checkbox

`.checkbox-row`/`.checkbox-box`/`.checkbox-box.checked`/`.checkbox-label`, both themes' `base.rcss`
— same shared-contract shape as Button.

## Text field

Use a **stock RmlUi `<input>`**. There is no custom element, no C++ text widget and no wrapper
framework — the vendored engine already provides the editable buffer, caret, selection, clipboard,
`maxlength`, tab focus, `change` events and IME composition. Consumers today: `CMyShopInventory`'s
shop name, `CCharMakeWin`'s character name, `CGenericConfirmDialog`'s `Mode::Text` field, and
`CLoginWin`'s username/password pair (also proving `type="password"` and Tab navigation),
`CMsgWin`'s resident-password prompt, and `CChatInputBox`'s chat/whisper-target pair.

**Two things a field inside a window that opens and closes has to get right**, both learned from
`CChatInputBox`:

1. **Focus after the document is visible, not before.** `CSystem::Show()` runs a window's
   `OpenningProcess()` *before* `ShowInterface()`, so `IsVisible()` is still false there and
   `Element::Focus()` lands on a hidden document and is lost. It must also happen after
   `SyncDocumentVisibility()`'s own `Show()`, which defaults to `FocusFlag::Auto` and would blur the
   field again. Arm a one-shot latch in `OpenningProcess()`; consume it in the sync, immediately
   after the visibility call.
2. **A window that handles keys while its field is focused claims the field's document.**
   While the player types, `CManager::UpdateKeyEvent()` gives keys only to the window whose
   `TakesTypingFrom(document)` returns true for the document holding the focused field
   (`RmlUiRuntime::GetTypingDocument()`). Without the override, Enter, Escape and history never
   arrive. This is the role the focused `CUITextInputBox`'s `HWND` used to play.

**Two rules that are invisible at compile time and will silently break a field:**

1. **Never call `ElementDocument::Show()` per frame on a document containing an `<input>`.** It
   defaults to `FocusFlag::Auto`, which focuses the document and blurs the field — one frame after
   every click, so the field looks focusable but swallows every keystroke. Use
   `UI::RmlBridge::SyncDocumentVisibility()` (`UI/RmlBridge/RmlDocumentVisibility.h`), which only
   acts on an actual visibility transition. This caused the original failure *and* a later
   regression in `CLoginWin::Render()`.
2. **Declare the field after any frame art that overlaps it.** RmlUi paints siblings in document
   order, so a field placed where an old zero-size position anchor sat renders *behind* the frame —
   no visible text or caret, which reads as "can't focus" even though hover and focus are correct.

For a field that can be disabled, bind `data-attrif-disabled` to its semantic state and style
`:disabled`. A dimmed class or an overlay alone does not prevent focus or editing. Chat's whisper
target uses `data-attrif-disabled="!whisper_send"`, keeping its value when whispering is off.

Division of ownership:

- **RML/RmlUi** owns the edit buffer, focus, caret, selection, IME composition display, text
  clipping/scrolling and hit testing. Bind the value two-way with `data-value="<model field>"`; do
  not poll the element each frame.
- **RCSS** owns appearance. `.text-field` is the shared primitive in both themes' `base.rcss`
  (`.text-field`, `.error`, `:disabled`, and the `selection` child for the selection range). It
  deliberately carries **no** position or size — each consumer adds a local class for its own box,
  the way `.my-shop-title-field` does. There is deliberately **no `:focus` rule**: the native fields
  these replaced drew no focus outline, so the blinking caret is the only focus cue. `:focus` works
  if a screen ever wants one, but add it per consumer rather than to the shared class.
- **C++** owns the semantic value and the rules about it: the length cap (set the `maxlength`
  attribute from code, as `ApplyShopTitleLimit()` does, so the limit can't drift per theme), any
  character filtering, and what counts as valid. Surface an invalid value as model state that
  toggles `.error` — never set a colour from C++.

Notes for later consumers: use `type="password"` for masked input (the same `.text-field` styling
applies), and set `type` from C++ when one field serves both (see `ApplyInputFieldConfig()` — changing
`type` rebuilds the element's `InputType` and drops its value, so set type and limit *before* the
value). `autofocus` makes `ElementDocument::Show()`'s `FocusFlag::Auto` focus the field on open — the
declarative replacement for a native `GiveFocus()`, but **only for genuinely modal screens**: on an
ordinary window it swallows every hotkey the moment the window opens, so My Shop deliberately omits
it and is click-to-focus. Filtering that RmlUi has no equivalent for (digits-only) belongs in C++;
enforce it by rejecting the keystroke, not by correcting the value afterwards: a capture-phase
`textinput` listener on the document runs before the focused widget's own listener, and
`StopPropagation()` there leaves the caret untouched. That listener is shared:
`UI::RmlBridge::AttachNumericInputFilter(doc)` (`RmlNumericInputFilter.h`) filters every field that
carries `text-field--numeric`, so a window marks its fields and attaches once; `KeepDigitsOnly()` is
the read-side half. Writing a
filtered value back onto the element instead re-enters `OnValueAttributeChanged()` and resets the
caret to index 0. Clipboard paste raises no `textinput`, so filter on read as well. SDL3 IME is
handled once, centrally, by `RmlUiRuntime`'s installed `TextInputMethodEditor_SDL` plus
`RmlUiSystemInterface::ActivateKeyboard()` — a consumer needs no IME code of its own.

A focused field suspends every window's key handling (`CManager::UpdateKeyEvent()`) except the
window that claims its document, so a window that should still close on Esc from inside its field
overrides `TakesTypingFrom()` to return `document == m_RmlView.Document()`. A field in a hidden
document never counts as typing (`RmlUiRuntime::GetFocusedTextField()`), so closing the window
gives the hotkeys back at once; blurring on hide still clears the field for the next open.

## Scrolling pane

`.scroll-pane` (both themes' `base.rcss`). Put it on whatever element owns scrollable content and
RmlUi generates the scrollbar itself, as real child elements the theme styles. Consumers:
`CGenericConfirmDialog`'s `.gcd-text-col` — the case this was generalized from — `CChatLogWindow`'s
`#lines`, which is where it is actually *visible* (the dialog only scrolls when its content outgrows
the panel, which is hard to provoke), and `CMoveCommandWindow`'s `#list`, the one that replaced a
real hand-rolled scrollbar (thumb travel, grab offset, a three-state drag enum) rather than adding a
new one.

The pane needs a bounded height to scroll within (explicit `height`, `max-height`, or a stretched
flex child). That stays the consumer's own layout; `.scroll-pane` sets only the two rules below.

**Two requirements it exists to stop you rediscovering.** Both cost a debugging round the first
time, and both fail silently:

- `overflow: hidden auto` is `overflow-x, overflow-y` in that order. This engine only instantiates
  a `scrollbarvertical` when **overflow-y** is Auto/Scroll (`Layout/ContainerBox.cpp`). Write
  `auto hidden` and no scrollbar is ever created, at any content length.
- `pointer-events: auto` is mandatory. `base.rcss`'s `body { pointer-events: none; }` inherits
  down, and `slidertrack`/`sliderbar` are children of the pane — without it the scrollbar renders
  but cannot be dragged.

These are generated elements, not pseudo-elements, so ordinary selectors reach them
(`scrollbarvertical`, `slidertrack`, `sliderbar`, `sliderarrowdec`/`inc`, `scrollbarhorizontal`).

**Theme chrome.** `legacy` uses native's own art — the track is `newui_scrollbar_m` as a ninepatch
and the thumb is `newui_scroll_on`, with `filter: brightness(0.7)` on `:active` matching the
`RGBA(179,179,179)` tint `CChatLogWindow::RenderFrame()` applies while the thumb is held.
`modern` states the same affordance as a flat rail in its metal-rail palette.

The well's two end caps are native's real 7x3 `newui_scrollbar_up`/`_down` sprites, carried by
`sliderarrowdec`/`sliderarrowinc` — RmlUi generates an element at each end of a scrollbar and sizes
`slidertrack` between them, which is exactly native's cap/tile/cap construction. They are
`pointer-events: none`: native has no arrow buttons, these are the closing ends of the well.
**Their `width` is `100%`, not a fixed length, on purpose** — a consumer that hides its scrollbar by
zeroing the `scrollbarvertical`'s own width (`chat_log.rcss` does, for F5) would otherwise be left
with two floating caps. `WidgetScroll::FormatElements()` builds each arrow's box against the
scrollbar's own content size, so the percentage tracks whatever that width currently is. `modern`
needs no caps — its track is a solid fill that already reads closed — so its arrows stay collapsed.

One deliberate simplification remains on the legacy side, the same class as `character_info.rcss`'s
summary-box frame: the middle slice is one stretched ninepatch rather than a literal repeat-tile,
because this build has no verified repeat-tiling pattern. Native's thumb overhang (15-wide thumb
over a 7-wide track) is also not reproduced, because that needs the track narrower than the
scrollbar element and RmlUi sizes `slidertrack` itself.

**Two traps a consumer has to handle itself**, both found the hard way in `CChatLogWindow`:

- **`pointer-events: auto` on the pane blocks click-to-move.** `Core::Input::IsMouseOverUI()` gates
  world clicks on `Context::IsMouseInteracting()`, which is true for any auto element *hovered*, not
  clicked. A pane overlaying the world must set `pointer-events: none` on itself and its content,
  then opt the scrollbar back in at **every** level (`scrollbarvertical`, `slidertrack`,
  `sliderbar`) — they inherit from the pane, so one `none` above silently disables the drag.
  Anything the pane then loses to `none` (hover styling, per-element clicks) has to move to C++.
- **Never pin the scroll position per frame.** Applying `SetScrollTop` every frame — to follow new
  content, say — overrides the user's own drag and wheel continuously and reads as a dead
  scrollbar. Use a one-shot latch consumed on the frame after the content changed.

**Inside a `transform: scale(root_scale)` panel**, the pane has to counter-scale itself out of that
transform (`.counter-scaled` with `transform-origin: left top`, its own `width`/`height` as
`calc(Npx * var(--root-scale))`; Counter-scaled text above) — `guard_window`'s lists are the
reference. Two
things follow, both non-obvious:

- **The net render transform at the pane is identity**, so its contents are laid out in real
  pixels. That is the point — text in a dense list must stay at the native renderer's own physical
  size, which grows 11pt→16pt while the dock transform grows to 2.25x. It also means the scrollbar
  keeps `.scroll-pane`'s `dp` width rather than needing a reference-px override.
- **Give rows an explicit bound width, not `100%`.** A percentage resolves against the pane's
  content box, which shrinks by the scrollbar's width the moment one appears — every column in the
  row then shifts. Bind the same reference width the pane uses and let `overflow-x: hidden` clip it.

Column positions *inside* such a row are best written as percentages of that bound width: they then
need no binding of their own and hold at every scale.

**What it does not cover.** RmlUi scrolls *DOM content*, so a window keeping its own line-window
model in C++ is not a drop-in consumer: it has to put the lines in the DOM and let RmlUi own the
scroll position. `CChatLogWindow` did exactly that: `DataViewFor::Update()` is incremental (an
append creates one element), and a front-removal re-runs every line's text binding.

## Layout utilities

Not named in §20's own list, but the closest thing to a real cross-window primitive that exists
today (`layout-and-scaling.md` is the full reference): `.anchor-{top,bottom}-{left,right}`,
`.center-{x,y,both}`, `.stretch-{x,y,both}`, `.hidden`, `.layout-anchor` — both themes' `base.rcss`,
identical class names and behavior in each.

## Data binding

`RmlModelBinder<T>` (`UI/RmlBridge/RmlModelBinder.h`) owns the `Model` instance and its
`Rml::DataModelHandle`, and exposes `MarkDirty()` so packet-handler/action-controller code doesn't
need to know RmlUi's binding API. Windows reach it through their `ThemedView` (below):
`GetModel()`, `MarkDirty()`, and `Binder()` for the `SyncField`/`SyncRootTransform` helpers.

## Semantic colours

Where a value's colour depends on what it means, C++ binds the meaning and each theme's RCSS holds
the colour: a `data-class-*` binding toggles a class, and the legacy theme uses the original values.
Examples:
- Zen wealth tier: `GameLogic::Items::ClassifyGoldAmount()`, `.gold-*`;
- trade partner level bucket: `.level-bucket-*`;
- quest row kind: reported by `CQuestMng::GetRequestRewardText()`, `.row-*`;
- character-select balloon name status: `.name-*`;
- outlaw level: `CHARACTER::PK` bound as `pk_level`, `#name.pk-*`;
- where an attribute's total comes from: `AttributeSource()` → `"potion"`/`"boosted"`/`"base"`,
  `.attr-potion`/`.attr-boosted` over the base colour on `.attr-value`.

**This is the shape to reach for whenever C++ is about to build a colour.** The last two came from
`CCharacterInfoWindow`, which had 57 `MakeColorRgba()` calls and pushed six colour strings through
its model -- so the theme could not restyle a stat or an outlaw level, and modern's own
`#name { color: token(text-warm) }` had never once taken effect, because a `data-style-*` binding is
an inline property and inline beats every stylesheet rule in this build (`engine-findings.md`).
Note what is *not* on this list: that window's derived-stat rows still carry `line.color`, because
they are a transcribed display list rather than a value with a meaning -- see
`tracked-deferrals.md`'s ownership-boundary entry for that distinction.

Do not unpack the engine's packed text colours into CSS: they are `A<<24 | B<<16 | G<<8 | R`, and
reading them as RGB swaps red and blue.

## Theming

`UI::RmlBridge` (`RmlTheme.h`): `ThemedDocumentLoader` (the one way a document is loaded, callable
only from `ThemedView` — makes "add a theme" a drop-a-folder operation),
`GetActiveThemeName()`, `ThemeUsesNativeTextSize()` (a declared theme capability,
`architecture-principles.md` §30 — see `theming-and-modding.md` for the pattern to follow for any
future capability flag). See `theming-and-modding.md`'s "Forking a theme's RML" section for the
per-theme RML/RCSS override mechanism itself, not a separate component but part of this same
theming layer.

**Every themed document belongs to a `UI::RmlBridge::ThemedView<Model>`**
(`UI/RmlBridge/RmlThemedView.h`; `ThemedView<>` for documents without a model). A window declares
it as a member with its model name, a binding function (`BindRmlModel(c, model)`), its document
paths and options, calls `Ensure()` where it builds and `Release()` where it tears down:

```cpp
void BindRmlModel(Rml::DataModelConstructor& c, PetInfoRmlModel& model);
UI::RmlBridge::ThemedView<PetInfoRmlModel> m_RmlView{"pet_info",
    [this](Rml::DataModelConstructor& c, PetInfoRmlModel& model) { BindRmlModel(c, model); },
    {{"Data/Interface/RmlUi/pet_info.rml"}}};
```

- **Builds once, when it can.** `Ensure()` creates the model, then loads each document into its
  context (the main one, or the spec's getter: `BackgroundOrMainContext` for a frame behind native
  3D). It waits while a context is missing and retries a failed load on the next call. It registers
  for theme switches on its first call.
- **A theme switch keeps the model** and rebuilds the documents over it: typed text, selections
  and labels survive with no copying. Each document that was visible is shown again with the
  options' `modal`/`focus` and `stacking` (`Front`/`Back`). Owners rebuild from the bottom of their
  context's stack up, so the stacking survives too; a window with several views builds the one
  that must end up underneath first (`CGenericConfirmDialog`'s chrome).
- **Hooks.** `afterBuild` runs after every build (drag handles, input filters, `maxlength`, cached
  elements). `afterReload` runs after a theme switch's rebuild, for what the window sized or placed
  from the old theme (`CLoginWin`, `CCharSelMainWin`, a `UI::Placement::Invalidate()`).
  `beforeUnload` runs before every unload, theme switch or release (listeners, attached controls,
  cached element pointers).
- **`Release()`** blurs a focused field it owns (unloading drops the focus without a blur and
  leaves every hotkey dead), unloads the documents, removes the model and stops following theme
  switches. `Hide()` keeps them for the next `Show()`. Both, and `Document()`, are safe after
  `Rml::Shutdown()`, which a static window's destructor reaches.
- **One document per instance**: `modelPlaceholder` rewrites the markup's `data-model="…"` to the
  view's name, given late with `SetModelName()` when it depends on the owner (the friend views).

Shapes to copy: `CCharacterInfoWindow` (one document), `CMyInventory` (two views, content and
background frame), `CMainFrameWindow` (two documents, one model), `CGenericConfirmDialog` (three
views), `ChatRoomView` (per instance), `ReconnectDialog.cpp` (a namespace-scope view, no window).

## List / repeated rows

RmlUi's `data-for` binding against a `std::vector<T>` model field — the pattern for any "N rows
of the same shape" content. Two references:
`CBuffStrip`'s buff-icon strip (a simple array) and `CMyQuestInfoWindow`'s quest list
(`my_quest_info.rml`/`.rcss`, ported off `CUICurQuestListBox`/`CUIQuestContentsListBox` — also
proves `server_select.rml`'s click-a-row-to-select-it pattern on top of the same binding). No
generic "ListBox" C++ wrapper exists (and none is needed) — each window binds its own row-shaped
struct directly, the same way `RmlModelBinder<T>` is used everywhere else.

## Dialog

`mu::ui::window::CGenericConfirmDialog`/`GenericDialogConfig` (`UI/Dialogs/GenericConfirmDialog.h`,
`generic_confirm_dialog.rml`/`.rcss` both themes) — a real config-driven scaffold, not a per-dialog
hand-built RML/RCSS pair: one C++ class + one document, shown with different `GenericDialogConfig`
content per call, no new subclass or new `.rml` per dialog. **The struct itself is the field
reference** (`GenericConfirmDialog.h`, each field commented at its declaration) — don't duplicate
that list here, it'll drift; skim the header before adding a new field. Worth knowing before
reading it cold:
- Buttons are three fixed, role-named slots, not a positional pair —
  `primaryLabel`/`onPrimary` (always shown), optional `secondaryLabel`/`onSecondary` (a real second
  action, e.g. "Decrease", never Esc-bound), and `showCancel`/`cancelLabel`/`onCancel` (always
  dismiss semantics, fires on Esc).
- `KeepOpen()` lets `onPrimary`/`onSecondary` veto their own click (invalid typed input, etc.) —
  the dialog stays open exactly as it was, as if the click never happened. Needed by any consumer
  migrating a native dialog whose `OkBtnDown` could return "keep this open" instead of closing.
- `item3D` (a live 3D item-preview snapshot) draws into `#gcd_item3d` through a
  `UI::Items::ItemCameraTarget`, framed by its slot (`#gcd_item3d_slot`); the image is far larger
  than the slot, so a long model reaches past the panel as it did natively.
- Single active instance, not a real stack — a second `Show()` call while one is open queues
  instead of replacing it; see the class's own header comment for why that's not a functional
  regression from what it replaces.

Built to replace `UI/Dialogs/CommonMessageBox.h`/`CustomMessageBox.h`'s native `TMsgBoxLayout<T>`
family — see `migration-ledger.md`'s Dialog family table for what's left.

`mu::ui::window::CGenericMenuDialog`/`GenericMenuConfig` (`UI/Dialogs/GenericMenuDialog.h`,
`generic_menu_dialog.rml`/`.rcss` both themes) — sibling primitive for the "arbitrary list of N
labeled action buttons" shape (a multi-option menu, not two/three fixed named slots) that
`CGenericConfirmDialog` doesn't cover. One C++ class + one document, shown with a
`GenericMenuConfig` value (title, body lines, a button vector each with its own optional label/
tooltip/per-button lines/compact flag, an optional `columns` grid width, `onCancel`). Frame/border/
header chrome lives in the shared `window_shell` `<template>` (both themes), not duplicated per
dialog. Proven on 13 real dialogs (`migration-ledger.md`'s Dialog family table has the current
list) — the two remaining "multi-option menu" classes (`CGuild_ToPerson_Position`,
`CGemIntegrationDisjointMsgBox`) stay message boxes drawn by `MessageBoxView` because their shape
doesn't fit this primitive's plain "click closes" model (simultaneous radio-select, an embedded
jewel list). A `MessageBoxView` box can name its kind, which becomes `#panel`'s class, and mark
buttons as theme-placed: the appointment box's four buttons are placed by `message_box_view.rcss`. Several consumers chain a second `Show()` from inside a button's own
`onClick` — closing this menu and immediately opening a different one (or the same one with
different content) — a reentrant pattern proven by the Trainer menu pair, the Gem Integration
jewel-type→mix-amount flow, and Elpis's text-only variant; `GenericMenuDialog.h`'s own header
documents why this is safe (buttons always close on click, so there's no `KeepOpen()`-style veto
to interact with). `GenericMenuConfig::systemMenu` marks the in-game system menu (`system-menu` on
`#panel`): legacy lays it out like the native box, modern fills the screen like the options screen,
with a heading and close action of its own (`system_menu_label`, `close_label`, `gmd_cancel`, the
same as Esc), so it leaves out the button marked `MenuButton::dismiss` (Cancel).

## Dragging

`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h`) — drags a panel by a handle using RmlUi's
drag events, with an `OnDragEnd` hook. The handle needs `pointer-events: auto`; controls inside a
whole-panel handle opt out with `drag: block` (both `base.rcss` set it on
`input, select, textarea, .btn, [data-event-click]`). Helpers for transform-centred dialogs:
`ResetDraggedPosition()` (re-centre on open), `KeepInsideWindow()` (pull back a panel dropped
partly off screen); the `dragged` class drops `.center-both`'s transform (`engine-findings.md`).
Drag positions are not saved; they last for the session. Who drags is decided in `window-placement.md` section 8. Only drags of the handle itself move
the panel: a control inside it with its own `drag` (a level gauge's hit area) is the drag element
for its own drags, and those are ignored even though they bubble up to the handle.

## Native content inside a document

`UI::RmlBridge::RenderTarget` (`UI/RmlBridge/RmlRenderTarget.h`/`.cpp`) — native drawing rendered
into a texture a document shows like any other image, so it sits at its element's own depth: under
a window that covers it, beneath the text and tooltips drawn over it. This is the default for live
3D that belongs to one window. Consumers: the letter portrait (`CUIPhotoViewer`), the item
hotkeys (`CItemHotKey`) and the character-creation preview (`CCharMakeWin`, a character with the
scene camera saved and restored around it).

**Inventory items into a target**: `UI::Items::ItemCameraTarget` (`UI/Inventory/ItemCameraTarget.h`).
Its drawer runs under the item camera the original drew UI items with — identity view, 1° field
of view over the whole window (`SetFieldOfView()` for the cash shop's 2°) — with the projection
cropped to the image's drawn box, so each
`RenderItem3D()` call lands in the texture exactly where it would have landed on screen, every
per-item offset and angle included. The drawer keeps its own coordinates: window pixels by
default, or a window's layout space when constructed with that `CObject` (or a transform source:
the cash shop's message boxes pass the message box layout).
`Sync(image, enabled)` once a frame sizes the target to the image's drawn box (through any
transform on it or its ancestors, which `GetAbsoluteOffset()` leaves out;
`UI::RmlBridge::DrawnContentBox()`, `RmlElementBox.h`, does the same for any element), points its `src` at the target and
keeps it invisible until the first frame. It brackets the drawer with `SaveCameraPerspective()`/
`RestoreCameraPerspective()`: item rendering overwrites `g_Camera` and moves `MousePosition`, the
origin of the ray picking casts, and leaving it moved stops click-to-move. A model may reach past
its slot, as natively, so a slot's image can be larger than the slot with the drawer framing the
item by the slot's own box (the confirm dialog's `item3D`). Consumers: the item hotkeys, the
confirm dialog's item preview, `CNPCQuest` and `UI/Events/EventItemEntryView`
(`SetItemDrawer()`: a panel-sized `#entry_item` its window's own item code draws into). The item
on the cursor has a document of its own above every window (`UI/Inventory/CursorItemLayer.h`,
`cursor_item.rml`), at the original's 10.9 so it also stays over the message boxes.

Construct one with a drawer, size it to an element with `Resize(width, height)` in physical pixels
(RmlUi box sizes already are), `SetEnabled()` it while the window is shown, and set an `<img>`'s
`src` to `Source()`. The drawer runs once a frame from the renderer's offscreen seam
(`SetOffscreenRenderCallback`), inside a capture of exactly the target's size that brings its own
viewport, depth and transparent clear — so it sets only a projection for `width / height` and
draws. `UI::Social::PhotoViewerControl` is the worked example of the element side.

What it takes care of, so a caller does not:

- **A capture recorded early would land on the frame.** `FlushRenderCommands()` draws native
  commands mid-frame, and a flushed command is drawn before anything can mark it as captured. The offscreen seam runs at the top of `EndFrame`, after the last flush.
- **Resizing never shows an empty or freed texture.** A new size takes a new texture, and
  `Source()` moves to it only once it has been drawn. The old one is released a few frames after
  the switch, since a new `src` reaches RmlUi a frame or two later, and the release itself waits
  for the start of a frame: releasing an owned texture mid-frame drops that frame's whole replay.
- **Sources never repeat**, unlike texture ids, so a stale RmlUi cache entry cannot resolve to a
  recycled texture.

Alpha in a target is coverage: the renderer's alpha-style blend modes accumulate it over and its
additive and multiplying modes leave it (`MuRendererSDLGpu.cpp`'s blend table), so RmlUi's
premultiplied composite lands each one on whatever is behind the image as it would have on screen;
an additive glow adds to the panel under it instead of painting a black box.

**Item grids** (`UI/Inventory/ItemGridModel.h`, `ItemGridGeometry.h`, both themes'
`item_grid.rcss`). A `CInventoryCtrl`'s `Render()` computes `Cells()` (each cell's tint, drop
preview and stack count, from the native logic), and the window draws its items into its own
`ItemCameraTarget` with `Render3D()`. Native 2D a drawer draws lands in the target too, where it
would have on screen (`SetOffscreen2DRect()`), for a window's remaining effects. The window binds the cells (`RegisterItemGridCells()`) and
its markup lays them out: `.item-grid` (cells, then the frame) at the grid's anchor, the window's
`.item-view` image over it, then `.item-grid-counts` at the same place. The geometry is the
theme's: `CInventoryCtrl::FollowGrid()` reads the grid's box and its first `.item-cell`'s margin
box each frame into a `UI::Items::GridGeometry`, which the hit tests, the drop target, the items'
boxes, the tooltip and the window's effects use. The pitch is one custom property per theme
(`--item-cell`, the original's 20), and a window sizes each grid as `calc(N * var(--item-cell))`.
A dragged item holds its pickup point in cells (`UI::Items::PickupAnchor`), so it targets a grid of
another pitch correctly. `CMyInventory` is the worked example, with its equipment slots as
`slot_states` classes on their anchors and each slot's `#slot_*_item` box where the equipped item
draws and the pointer hits it.

## Native content above RmlUi

`Winmain.cpp`'s `SetPostRmlUiCallback` opens a `LOAD_OP_LOAD` render pass after RmlUi's main
context has closed: the only layer above RmlUi. It draws the cursor. Window content does not go
there: what it draws stands over every panel, including ones that should cover it, so native
drawing that belongs to a window goes into a render target.

## Native 3D viewer input

`UI::Social::PhotoViewerControl` (`UI/Social/PhotoViewerControl.h`/`.cpp`) — drag-to-turn,
right-click-to-reset and the "?" help toggle for a `CUIPhotoViewer` standing in an RmlUi slot,
driven from the document rather than from the viewer's own native mouse handling.

The reason it has to exist is worth knowing generally: **a native window behind an RmlUi document
never sees a mouse press.** `Context::ProcessMouseButtonDown` returns `!IsMouseInteracting()`, false
whenever anything at all is hovered, and Winmain's event pump only calls `HandleMouseButton()` when
RmlUi lets the event propagate — so `MouseLButtonPush` is never set for a click over a panel. The
wheel is not routed through RmlUi at all, which is why wheel-driven controls keep working natively
and press-driven ones silently do not. If a ported window still relies on native press handling for
anything, it is already broken; check it.

The slot takes `pointer-events: auto`, mousedown starts the gesture, and mousemove/mouseup are
listened for on the **document** so a drag leaving the slot keeps tracking. Deltas convert RmlUi px
to native reference px through `FloatingWorkspaceTransform().scaleX`, the same ratio that places the
viewer, so the feel holds at any UI scale.

## Tooltip

`UI::RmlBridge::Tooltip` (`UI/RmlBridge/RmlTooltip.h`/`.cpp`, `tooltip.rml` +
`themes/{legacy,modern}/tooltip.rcss`) — the single shared tooltip primitive. One always-on-top
RmlUi document in the main context (explicit `z-index: 9999` — the actual z-order fix; a document
with the default `z-index: auto`, every other document in this codebase, paints in plain DOM/show
order, so a native tooltip queued through the legacy 3D-camera effect system could always be
painted over by RmlUi's own main-context pass), with a `Show(Config, Owner)`/`Hide(Owner)`
free-function API callable from native code exactly as easily as an RmlUi hover callback — the
reason a still-fully-native window doesn't need its own RmlUi document just to show a tooltip.
`LineColor` is the union of every prior mechanism's palette (10 foreground + 4
background-highlight colors), and `Line::Kind{Text, HalfSpacer, FullSpacer}` replaces the old
"sniff the first character of a native text buffer" spacer convention with an explicit field.
`Config` also carries per-line `TextAlign{Left, Center}`, `centerHorizontally` (whether the anchor
is the panel's left edge or horizontal center), an `AnchorPoint{BelowLeft, AboveLeft}` grow
direction, and a real measure-then-clamp pass so a tooltip near any screen edge stays fully
on-screen (every prior mechanism clamped horizontally at best, some not at all). `Owner` is an
opaque per-caller token so one caller's per-frame `Hide()` can't clobber a different caller's
`Show()` from earlier the same frame — see the header's own comment for the real bug this shape
fixed. Callers show it every frame while hovered: `Show()` rebuilds and measures only when the
content, its native metrics or the `dp` ratio changed (one context update to create the rows, then
the tooltip document's own layout), and otherwise only places the measured panel at the anchor.

Migrated onto it: the item/pet tooltip (`RenderItemInfo()`/`RenderRepairInfo()`,
`Engine/Object/ZzzInventory.cpp`), the generic button tooltip (`CTooltip`,
`UI/Widgets/Window/Tooltip.h`/`.cpp` — `CButton`'s existing `ChangeToolTipText()` forwarding is
unchanged, only what happens internally moved), the skill-hotkey tooltip (`MainFrameWindow.cpp` —
`g_pSkillList`'s own hover slot), the inventory Set/Socket option tooltip (`MyInventory.cpp` — its
old embedded RmlUi implementation was deleted outright, not left running as a second mechanism),
two smaller hover tooltips (`MasterLevel.cpp`, `CursedTempleSystem.cpp`) and the buff strip.

**A static hint on an element of a document** is markup: `data-hint="text"` on the element, or
`data-attr-data-hint="x"` for text the model binds. `UI::RmlBridge::DocumentHints`
(`RmlDocumentHints.h`) shows it with no per-window code: once a frame, before the context updates,
it finds the hovered element's nearest ancestor with a non-empty `data-hint` and shows that as a
`Box::ButtonHint`, anchored to the element's drawn box with `CTooltip`'s geometry
(`RmlTooltipPlacement.h`: centred 3 units right of the element, 2 units off it). It goes above the
element, below it when there is no room above, and then starts under the cursor sprite, which
hangs below the point it marks. Its unit is the panel's root transform scale for a document laid out
in reference px, and the `dp` ratio (times the workspace slot's scale for a `workspace-placed` part)
for a `dp` document, so the legacy theme's native text size follows each window. Polling the
hovered element leaves nothing to unregister: a hidden document, a theme switch or a moved panel
just stops matching. The element needs `pointer-events` (a decorative bar or row may have to opt
back in). Every button and bar hint in the RmlUi windows is one; `EventItemEntryView`'s buttons
take theirs from `Button::hint`.

**A hint built from game state** (lines, colours or values per frame) uses
`UI::RmlBridge::ElementTooltip` (`RmlElementTooltip.h`). The element forwards
`data-event-mouseover="x_hover(i)"` / `data-event-mouseout="x_leave"` to the window's model, which
calls `Enter()` / `Leave()`; each frame the window passes the hovered element's lines to `Show()`,
which anchors the tooltip to the element's drawn box (`CBuffStrip`). `Leave()` counts only the
element's own mouseout: a child's bubbles to it while the element is still hovered.

**Placement.** `Config::flipAnchorY` is where a tooltip grows the other way when it does not fit on
its own side of the anchor (a button hint: the button's other edge); without it, `Show()` only
shifts the tooltip back on screen. `CTooltip` and `DocumentHints` both set it.

**Owners.** Every caller passes an owner token, and hides only its own. The item information
tooltip's is `UI::Tooltip::ItemInfoOwner()` (`LegacyTextListTooltip.h`), the default of
`ShowLegacyTextList()` / `HideLegacyTextList()`: a window that stops showing an item hides that and
nothing else. An ownerless `Hide()` would hide any caller's tooltip, which is how the inventory
once cleared every hint while it was open.

**Legacy theme layout.** The legacy theme overrides `tooltip.rml`
(`themes/legacy/tooltip.rml`) to follow the original `RenderTipTextList()`:
- native text size;
- rows one native text height tall, each starting 1.1 heights below the previous one, so
  highlight bars keep their gaps;
- the widest line plus 2 units of side padding;
- a 1-unit opaque frame around an 80% fill.

RmlUi's own FreeType metrics round to whole pixels, which accumulates over many rows. So
`Show()` reads the row height from the native text renderer
(`CUIRenderTextSDLTtf::LineHeight()`), without selecting a font on the shared renderer.
It measures under `Config::transform` when a caller anchors with a non-ambient transform (the
skill-hotkey tooltip's dp-ratio reference frame), otherwise under the ambient transform.
`Config::fixedWidth` keeps a fixed text width where the original had one (the inventory Set and
Socket option tooltips). The anchor places the inner (padding) box; the frame sits outside it.

`UI::Skills::Tooltip::Render()`/`BuildModelForSlot()`/`ToRmlBridgeLines()`
(`UI/HUD/Skills/SkillTooltip.h`/`.cpp`) is the shared skill/pet-command tooltip *content* builder —
deliberately not in `SkillTooltipModel.h`, which is also shared with the standalone MuEditor (ImGui)
tool and has no RmlUi dependency to pull in. `ToRmlBridgeLines()` converts a resolved `Model` into
`UI::RmlBridge::Tooltip::Line`s; `MainFrameWindow.cpp`'s skill-hotkey tooltip and
`SiegeWarBase.cpp`'s guild-skill tooltip build a `Config` from it. `Render()` has no callers and is
kept on purpose; the MU Helper skill picker shows no hover tooltip, as native never did.

**Deliberately not on this primitive**: `HelpWindow.cpp`/`ItemExplanationWindow.cpp` stay on native `RenderTipTextList()` on purpose: they render unconditionally while their own window
is open rather than on hover, so they don't fit this primitive's owner-token model (the newest
`Show()` always wins, which assumes a momentary, naturally mutually-exclusive hover tooltip) — a
second, non-competing primitive for them was scoped and rejected as not worth duplicating most of
this primitive's positioning/clamping logic for two low-traffic windows.

## Skill icons

`UI::Skills::Icon::ResolveSkillIcon()` (`UI/HUD/Skills/SkillIconAtlas.h`) maps a skill to its
20x28 cell, lit or grey, and `IconSpriteName()` names the sprite in `skill_icons.rcss` or
`master_skill_icons.rcss`. Link both, build `"image(" + sprite + ")"` in C++ and bind it with
`data-style-decorator`. It is the one resolver for every skill icon: the HUD (`CSkillList`), the
master tree and the MU Helper windows (`UI::MuHelper::SkillIconDecorator()`, always lit).

## Level gauge

Native's fill-by-level bar, in both `base.rcss`: `.level-gauge` (the rail), `.level-gauge-fill`
and `.level-gauge-hit` (a child of the rail that takes the pointer); the consumer sizes and places
all three. Legacy draws the original art — `newui_option_volume01/02` for 10 levels,
`newui_option_effect03/04` for 5 (`.level-gauge.effect-gauge`) — with one sprite rect per level,
since a clipped wrapper doesn't clip absolutely-positioned children here. A fill shows its level
through a class, `level-<n>`, which each theme maps to a width (and, in legacy, a sprite); the MU
Helper detail window binds its own width instead.

The hit child binds `mousedown`, `drag` and `mousescroll` to one callback that passes each event
to `UI::RmlBridge::ApplyLevelGaugeEvent()` (`RmlLevelGauge.h`) with the window's own pointer-to-
level rule: a press or drag sets the level from where the pointer is on the drawn bar (correct
inside a transformed panel, unlike a stock `<input type="range">`, `engine-findings.md`), the wheel
steps it. Consumers: the MU Helper detail window's thresholds, the options window's sound, music
and effect-limit gauges.

## Feature operations for networking and game code

Packet handlers and other non-UI code reach the UI through small free-function headers that
include nothing from RmlUi or the window classes, so replacing the UI framework changes their
`.cpp` files, not the callers. Each operation runs the owner's existing call sequence; callers
keep decoding and protocol meaning (result codes become named enum values before the call), pass
decoded values only, and strings or spans are borrowed for the call. Current headers:
`UI/Chat/ChatMessages.h` (`UI::Chat`), `UI/Core/WindowAccess.h` (`UI::Windows`, over
`Core/Globals/InterfaceList.h`), `UI/Dialogs/ConfirmRequest.h` (`UI::Dialogs::ShowConfirm`),
`UI/Social/SocialUpdates.h`, `UI/Inventory/{InventoryContents,TradeUpdates,StorageUpdates,
MixUpdates,ShopUpdates}.h`, `Guild/GuildUpdates.h`, `UI/HUD/HudUpdates.h`, `UI/Combat/SiegeUpdates.h`,
`UI/Events/{Doppelganger,EmpireGuardian,CryWolf,LuckyCoin,Kanturu,CursedTemple}Updates.h`,
`UI/NPCs/NpcDialogueUpdates.h`, `UI/Quests/QuestUpdates.h`, `UI/Options/OptionUpdates.h`,
`UI/MuHelper/MuHelperUpdates.h`, `UI/Windows/LoginSceneUpdates.h`, and the older
`UI/Chat/Whisper.h` and `UI/HUD/Notices.h`, which already had this shape. Plain OK message boxes and the
NPC menu dialogs are already free functions in `UI/Core/WindowCommon.h` (reached through the
precompiled header; it includes nothing). `Network/Server/WSclient.cpp` includes only these headers
from the UI, and its compiler include tree reaches no RmlUi header; keep it that way. Add an
operation to the family's header when a caller needs one; do not add a generic UI command or event
interface.

## Does not exist as a reusable primitive yet

Recorded here so a future session doesn't assume otherwise — each of these is still ad hoc,
per-window, or entirely unbuilt:

- **ProgressBar / HealthBar / ManaBar / ExperienceBar** — `main_frame.rcss`'s gauge fills are
  per-window CSS. `title_scene.rml`'s loading bar uses RmlUi's built-in `<progress>`
  (`SetValue()`/`SetMax()` from C++) — a proven raw element, not a shared component.
- **Notification, HUDContainer** — not needed by any window yet. (Scrolling is `.scroll-pane`,
  lists are `data-for`, text fields are `<input>` + `.text-field`, all above.)

## Tab / TabBar

Proven on `COptionWindow` (6 tabs), `CMyQuestInfoWindow` (3 tabs), `CPetInfoWindow` (2 tabs). Same shape every
time, no reusable C++ wrapper needed (matches this catalog's general "each window binds its own"
convention): an `int active_tab` model field, one `.tab-btn` per tab with
`data-class-active="active_tab == N"` and `data-event-click="window_select_tab(N)"`, and each tab's
content wrapped in a panel with `data-class-hidden="active_tab != N"`. `RmlClickSelectTab(int)` is
the C++-side handler name convention. Legacy theme swaps a sprite decorator on `.active`
(`my_quest_info.rcss`'s `.tab-btn-quest.active { decorator: image(myquest-tab-small); }` — real
sprite-art tabs, one frame per state); modern swaps a flat
`background-color: token(accent-steel)` instead (no sprite art needed). Start from
`my_quest_info.rml`/`.rcss` (3 tabs, plain content panels) or `pet_info.rml`/`.rcss` (2 tabs, sprite-
based tab art) rather than inventing the mechanism again.

## Using this catalog

Before building a new one-off mechanism for a window port: check this list first. If the concept
you need already exists above, reuse it. If it's listed as a gap, that's a signal you may be the
first real use case defining its shape — follow `architecture-principles.md` §27's workflow
(understand intent, then design the RML/RCSS/C++ split) rather than copying whatever the nearest
existing window happens to do, and update this catalog once the pattern is proven.
