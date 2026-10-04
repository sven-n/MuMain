# Reusable Component Catalog

Closes `architecture-principles.md` §20's gap ("no reusable-component catalog exists as such") —
an honest inventory of what already functions as a reusable UI primitive today, named and pointed
at its actual file, so a future port checks here before inventing a new one-off mechanism. This is
a snapshot, not a promise: entries marked "doesn't exist yet" are real gaps, not placeholders for
work already planned — the next window that actually needs one is what should define its real
shape (§26), not this document guessing ahead of a real use case.

Read `architecture-principles.md` first if you haven't — §20 is the principle this document
audits status against. See `STATUS.md` for how this fits the rest of the tracked gaps.

## Window / Panel

Three structural patterns exist today, not one unified `Window` component (`README.md`'s
"Coexistence patterns" section has the full detail on the first two) — but only the first two are
where things end up; the third is `CMyInventory` mid-migration, not a permanent category
(`ui-target-architecture.md` Section E's two-shape end state — pure RmlUi, or RmlUi with a native
live-3D seam — still holds):

- **`mu::ui::window::CObject`-tier window keeping legacy sprite widgets + RmlUi overlay** — these windows no
  longer derive from or hold a `CWin`/`CWinEx` instance at all (that base class has zero live
  subclasses left anywhere in the tree); what they kept from their pre-migration `CWin` days is
  just their sprite-widget *members* (`CButton`, `CGaugeBar`, `CWinEx` as a plain composed member
  in a couple of cases) for hit-testing bookkeeping, while RmlUi renders 100% of the visible
  chrome. Used by `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`, `CCharSelMainWin`, `CCharMakeWin`,
  `CServerSelWin`. See `docs/rmlui-ui-system/building-new-ui.md` for the full widget-toolkit map —
  this is a closed, historical set of windows, not a pattern for new ones to follow.
- **Pure RmlUi** — no legacy widget members at all. Used by `RememberPasswordPrompt`,
  `CMsgWin`, `CCharInfoBalloonMng`.
- **`C3DRenderMng`-tier window keeping a fully-native frame + partial RmlUi overlay** — the visible
  panel frame/background is still 100% native sprite art (the theme RCSS's `#panel` rule carries no
  visual chrome of its own — position/size only); RmlUi overlays only specific interactive pieces
  (title bar, buttons, gold/text, tooltips) on top. Driven by paint order, not by choice: this
  window's equipped/held item renders as a live 3D-camera icon (`ui-target-architecture.md`
  Section E) at a native paint-order depth *behind* where the frame chrome sits, and RmlUi's main
  context always composites last in the frame — porting the frame chrome today would flip it to
  render in front of the item icon instead of behind it. Used by `CMyInventory` (Stage 1/3 chrome
  done; the equipment paperdoll's background/durability-tint/drag-highlight chrome deliberately
  stays native — Stage 2, skipped for this reason, see `tracked-deferrals.md`'s pilots-to-revisit table) and,
  as of 2026-09-13, the rest of the inventory-family `C3DRenderMng` sibling windows sharing the same
  constraint: `CTrade` (two independent grids, not just one), `CStorageInventory`,
  `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`,
  `CPurchaseShopInventory`, `CInventoryExtension` (1-4 grids). `RmlUiRuntime::RenderBackgroundLayer()`
  is what let all of these move — see `STATUS.md`'s "Known gaps" entry on that mechanism, now
  generalized into `mu::ui::window::CManager::Render()`'s z-sorted loop instead of each window
  wiring its own call. Market place (`UnitedMarketPlaceWindow`) isn't in this family — no
  `CInventoryCtrl`/item grid, just a confirmation dialog that borrows this frame's sprite constants
  cosmetically — still native, but not blocked by anything here.

Visual frame primitives are theme-specific, not shared (correct per §15 — presentation is the
theme's job, not the component's):

- `modern`: `themes/modern/base.rcss`'s `.modern-frame`/`.modern-panel`/`.modern-frame-accent`/
  `.modern-inset`, plus the `-crimson` palette variant (`modern-theme-visual-direction.md` has the
  full token table these draw from).
- `legacy`: `themes/legacy/base.rcss`'s sprite-based 3-part `.panel-cap-top`/`.panel-cap-bottom`/
  `.panel-middle`.

A second, narrower shared frame exists for one specific window family: the `PanelColumnX()`-docked,
single-document (no background-context split) windows that visually read as one group on screen —
`character_info`, `my_quest_info`, `pet_info`, `party_info` today. Their `#panel`/frame sprites/exit
button/tooltip shape/group-box corner-and-fill technique (legacy) and forged-dialog panel gradient/
shell-edge/groove/header-rail (modern) are byte-identical, so they link a shared
`docked_panel_frame.rcss` (both themes) instead of each re-declaring it — see `migration-ledger.md`'s
`CPetInfoWindow`/`CPartyInfoWindow` rows and `STATUS.md`'s dock-neighbor gap note for why this
exists. **A new window joining this same `PanelColumnX` dock group should link this partial too**,
not copy-paste a fifth version — check its current window list before assuming it doesn't apply.
`CMyInventory` is deliberately not part of it (separate `*_bg.rml` context, can't link it).

## Button

Real shared contract across both themes already — `.btn`/`.btn-ok`/`.btn-cancel`/`.btn.disabled`,
same class names, same state model, each theme's own `base.rcss`. `.btn-ok` gets each theme's
"primary/hero" treatment (see `modern-theme-visual-direction.md`'s Accent colors section); plain
`.btn` stays neutral. This is the RCSS-layer contract only — the C++ side has two unrelated
button classes of its own (`CButton`, `mu::ui::window::CButton`; a third, `CUIButton`, is deleted); see
`docs/rmlui-ui-system/building-new-ui.md` for which one to use and why they aren't duplicates of
each other.

Inside a panel scaled by `transform: scale(root_scale)`, `.btn`'s `dp` sizes would scale twice.
Modern has `.modern-btn-px` (and `.modern-checkbox-px` for checkboxes), the same recipe in
reference `px` with no size of its own; the MU Helper windows use both.

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
2. **A window whose field is focused must claim `RmlUiRuntime`'s address as its related window.**
   `CManager::UpdateKeyEvent()` dispatches only to windows whose `GetRelatedWnd()` matches the
   focused handle, and it reports a focused RmlUi `<input>` as `&RmlUiRuntime::Instance()`. Without
   `SetRelatedWnd()` matching that, the owning window stops receiving keys the moment the player
   starts typing — Enter, Escape and history all silently never arrive. This is the exact role the
   focused `CUITextInputBox`'s `HWND` used to play.

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

A focused field suspends every window's key handling (`CManager::UpdateKeyEvent()`), its own
included, so Esc and Enter never reach the window you're typing in. A window that should still close
on Esc from inside its field calls `UI::RmlBridge::ClaimKeyboardWhileTyping(*this, doc)` from
`Update()` (`RmlKeyboardFocus.h`); `CChatInputBox` hand-rolls the same claim and could adopt it.
Blur the focused field on every hide path as well, or hotkeys stay suspended after the window closes.

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
transform (`transform: scale(1 / root_scale)` with `transform-origin: left top`, its own `width`/
`height` bound as `value * root_scale` px) — `CMoveCommandWindow`'s `#list` is the reference. Two
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
scroll position. `CChatLogWindow` did exactly that — see `STATUS.md` for why that was safe
(`DataViewFor::Update()` is incremental) and what it cost.

## Layout utilities

Not named in §20's own list, but the closest thing to a real cross-window primitive that exists
today (`layout-and-scaling.md` is the full reference): `.anchor-{top,bottom}-{left,right}`,
`.center-{x,y,both}`, `.stretch-{x,y,both}`, `.hidden`, `.layout-anchor` — both themes' `base.rcss`,
identical class names and behavior in each.

## Data binding

`RmlModelBinder<T>` (`UI/RmlBridge/RmlModelBinder.h`) — the per-window model/binder lifecycle
wrapper every migrated window uses: owns the `Model` instance, creates the `Rml::DataModelHandle`
once, exposes `MarkDirty()` so packet-handler/action-controller code doesn't need to know RmlUi's
binding API directly.

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

`UI::RmlBridge` (`RmlTheme.h`): `LoadThemedDocument()` (the one entry point every migrated window
uses instead of `Context::LoadDocument` directly — makes "add a theme" a drop-a-folder operation),
`GetActiveThemeName()`, `ThemeUsesNativeTextSize()` (a declared theme capability,
`architecture-principles.md` §30 — see `theming-and-modding.md` for the pattern to follow for any
future capability flag). See `theming-and-modding.md`'s "Forking a theme's RML" section for the
per-theme RML/RCSS override mechanism itself, not a separate component but part of this same
theming layer.

**Every window that creates a themed document must call
`UI::RmlBridge::RegisterForThemeReload(this, [this]{ ReloadRmlTheme(); })` right next to its first
`BuildRmlUi()` call (typically inside `Create()`'s guard), and unregister
(`UI::RmlBridge::UnregisterForThemeReload(this)`) at the exact point, if any, it already calls
`RemoveUIObj(this)` in `Release()`.** This replaced an earlier virtual-override mechanism
(`IObject::ReloadRmlTheme()` + `CManager::ReloadAllRmlThemes()`'s sweep) that required every window
to remember an override the compiler couldn't enforce — 16 windows across the docked-window and
inventory families shipped with exactly that gap before being fixed (2026-09-20), which is what
motivated the registry. Stated honestly: a window can still forget to call
`RegisterForThemeReload()`, the same way it could forget to call `BuildRmlUi()` — what the registry
actually fixes is that a theme switch used to require sweeping multiple independent `CManager`
instances plus separate free-function calls from every trigger site (now down to one call,
`UI::RmlBridge::ReloadAllThemedDocuments()`, from a `RegisterForThemeReload`-owning theme-switch
callsite), not that per-window opt-in itself became mandatory. A handful of app/scene-lifetime
singleton windows (e.g. `CLoginWin`, `CGenericConfirmDialog`) never unhook from `CManager` at all —
those must never unregister either, so their registration simply outlives every `Release()` call,
mirroring their existing `CManager` lifetime.

The `ReloadRmlTheme()` method itself is unchanged in shape: factor the RmlUi setup already in
`Create()` — model binder registration + `LoadThemedDocument()`/`CreateBackgroundDocument()` — into
a private `BuildRmlUi()`, call it from `Create()`, then implement `ReloadRmlTheme()` as: if
`m_pRmlDoc` is null, return (never opened yet); otherwise destroy the model binder, `UnloadDocument()`
the old document, null the pointer (same for `m_pRmlBgDoc`/its binder if the window has one, via
`RmlUiRuntime::Instance().GetBackgroundContext()`), then call `BuildRmlUi()` again. A window with a
per-frame `SyncRmlModel()`-style poll (most of them) needs nothing further — the next frame
self-corrects visibility/live data. A window without one (`CServerSelWin` is the one exception
found so far) must also explicitly re-run whatever populates its model and re-apply visibility,
since nothing else will. Reference implementations: `CMainFrameWindow::ReloadRmlTheme()`
(`UI/HUD/MainFrameWindow.cpp`, main + background doc) and `CCharacterInfoWindow::ReloadRmlTheme()`
(`UI/Character/CharacterInfoWindow.cpp`, main doc only).

**Text the player has typed is not live data.** It exists only in the model, which `Destroy()`
resets, so no per-frame poll can bring it back. Copy each `data-value` field before `Destroy()`,
restore it after `BuildRmlUi()`, and `MarkDirty()` it before the rebuilt document is shown
(`CMyShopInventory`, `CLoginWin`, `CCharMakeWin`, `CMsgWin`, `CChatInputBox`, the MU Helper windows,
`CGenericConfirmDialog`).
Set the field's limits (`maxlength`, `type`) inside `BuildRmlUi()`, not in the code that opens the
dialog, so the rebuilt field has them before the value lands. Restore only in the reload path:
opening a dialog afresh should still clear it.

## List / repeated rows

RmlUi's `data-for` binding against a `std::vector<T>` model field — the proven pattern for any
"N rows of the same shape" content, and what replaced the native list family
the client used to carry (`CUITextListBox<T>`, deleted 2026-10-04 once its last subclass went).
Two proven references:
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
- `item3D` (a live 3D item-preview snapshot) renders on top of the panel via a foreground/
  background RmlUi document split, not a post-RmlUi callback — RmlUi's main context always
  composites last in the frame, so a single-document panel would always paint over the item
  instead of under it. The mechanism (a dedicated third `Rml::Context`,
  `RmlUiRuntime::RenderDialogBackgroundLayer()`, fired from `CManager::Render()`'s own loop right
  before the shared 3D camera's z-order) is fully documented in the class's own header comment —
  read that, not a paraphrase, before touching anything `item3D`-adjacent. Only an `item3D` dialog
  paints its chrome in that context: every other dialog shows the same background markup as a
  second document in the main context, pulled to the front right under its text, so a
  full-screen RmlUi window (the master skill tree) cannot cover the panel of a dialog opened over
  it.
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
list) — the two remaining native "multi-option menu" classes (`CGuild_ToPerson_Position`,
`CGemIntegrationDisjointMsgBox`) stay native because their actual shape doesn't fit this
primitive's plain "click closes" model (simultaneous radio-select, an embedded live inventory
list-selection widget). Several consumers chain a second `Show()` from inside a button's own
`onClick` — closing this menu and immediately opening a different one (or the same one with
different content) — a reentrant pattern proven by the Trainer menu pair, the Gem Integration
jewel-type→mix-amount flow, and Elpis's text-only variant; `GenericMenuDialog.h`'s own header
documents why this is safe (buttons always close on click, so there's no `KeepOpen()`-style veto
to interact with).

## Dragging

`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h`) — makes an RmlUi panel draggable-by-mouse with
zero legacy `CWin` dependency. **First real caller landed 2026-09-07**: `CMyInventory`'s title bar,
paired with a new generic persistence mechanism (`GameConfig::GetWindowPosition`/
`SetWindowPosition`, an `OnDragEnd` hook on `MakeDraggable` itself) any future draggable window can
reuse with one call each way — see `STATUS.md`'s "Known gaps" entry for the full mechanism and
what's still unaudited (behavior across a resolution/UI-scale/theme change post-drag).

## Native content inside a document

`UI::RmlBridge::RenderTarget` (`UI/RmlBridge/RmlRenderTarget.h`/`.cpp`) — native drawing rendered
into a texture a document shows like any other image, so it sits at its element's own depth: under
a window that covers it, beneath the text and tooltips drawn over it. This is the default for live
3D that belongs to one window. Consumers: the letter portrait (`CUIPhotoViewer`) and the item
hotkeys (`CItemHotKey`).

**An inventory item into a target**: `CItemHotKey::RenderSlot()` is the recipe. Set up the item camera
`C3DCamera::Render()` uses — identity view, 1° field of view — but crop its projection to a
rectangle the size of the target, and call `RenderItem3DWithHover()` with that rectangle. At that
field of view only the rectangle's size frames the item, so every per-item offset and angle in
`RenderItem3D()` applies exactly as on screen. Bracket it with `SaveCameraPerspective()`/
`RestoreCameraPerspective()`: item rendering overwrites `g_Camera` and moves `MousePosition`, the
origin of the ray picking casts, and leaving it moved stops click-to-move.

Construct one with a drawer, size it to an element with `Resize(width, height)` in physical pixels
(RmlUi box sizes already are), `SetEnabled()` it while the window is shown, and set an `<img>`'s
`src` to `Source()`. The drawer runs once a frame from the renderer's offscreen seam
(`SetOffscreenRenderCallback`), inside a capture of exactly the target's size that brings its own
viewport, depth and transparent clear — so it sets only a projection for `width / height` and
draws. `UI::Social::PhotoViewerControl` is the worked example of the element side.

What it takes care of, so a caller does not:

- **A capture recorded early would land on the frame.** `RmlUiRuntime` flushes native commands
  mid-frame for the background context, and a flushed command is drawn before anything can mark it
  as captured. The offscreen seam runs at the top of `EndFrame`, after the last flush.
- **Resizing never shows an empty or freed texture.** A new size takes a new texture, and
  `Source()` moves to it only once it has been drawn. The old one is released a few frames after
  the switch, since a new `src` reaches RmlUi a frame or two later, and the release itself waits
  for the start of a frame: releasing an owned texture mid-frame drops that frame's whole replay.
- **Sources never repeat**, unlike texture ids, so a stale RmlUi cache entry cannot resolve to a
  recycled texture.

One compositing difference to expect: native pipelines write straight alpha and RmlUi blends
premultiplied, so opaque pixels are exact and only partly transparent edges can differ slightly.

## Native content above RmlUi

`UI::RmlBridge::OverlayRender` (`UI/RmlBridge/RmlOverlayRender.h`/`.cpp`) — a registry of native
draw callbacks drained once per frame from `Winmain.cpp`'s `SetPostRmlUiCallback`, which opens its
own `LOAD_OP_LOAD` render pass after RmlUi's main context has closed. That pass is the **only**
layer above RmlUi. `Register(owner, draw)` / `Unregister(owner)`, drawing in registration order.
No window uses it today; the inventory's native item tooltip is the recorded candidate
(`tracked-deferrals.md`).

Prefer a render target for anything that belongs to one window. This seam is above the whole
context rather than at any window's depth, so what it draws stands over every panel, including
ones that should cover it, and nothing RmlUi draws can paint over it. 2D native text and quads are
proven here (`RenderCursor`), and skinned 3D works too, since the renderer re-stages bone data for
this pass.

Historical note: an earlier `SetPostRmlUiCallback` attempt for `CGenericConfirmDialog`'s item3D
crashed twice and was abandoned for a third-context document split (`GetDialogBackgroundContext()`).
The likely cause — the post-UI pass staging only vertex data, not bone rows — was fixed in the
renderer afterwards (`MuRendererSDLGpu.cpp`), and the seam has carried a skinned character since
2026-10-04. The context-split alternative only works for always-on-top content such as a modal, so
it does not generalize to a draggable, stackable window.

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
`themes/{legacy,modern}/tooltip.rcss`) — the single shared tooltip primitive, replacing what this
entry used to describe as four non-unified mechanisms (stale as of this update). One always-on-top
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
fixed.

Migrated onto it: the item/pet tooltip (`RenderItemInfo()`/`RenderRepairInfo()`,
`Engine/Object/ZzzInventory.cpp`), the generic button tooltip (`CTooltip`,
`UI/Widgets/Window/Tooltip.h`/`.cpp` — `CButton`'s existing `ChangeToolTipText()` forwarding is
unchanged, only what happens internally moved), the skill-hotkey tooltip (`MainFrameWindow.cpp` —
`g_pSkillList`'s own hover slot), the inventory Set/Socket option tooltip (`MyInventory.cpp` — its
old embedded RmlUi implementation was deleted outright, not left running as a second mechanism),
and two smaller hover tooltips (`MasterLevel.cpp`, `CursedTempleSystem.cpp`).

**Legacy theme layout (2026-09-24, #623).** The legacy theme overrides `tooltip.rml`
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
`UI::RmlBridge::Tooltip::Line`s (the `LineColor` switch every caller used to hand-roll); three
callers now build a `Config` from it directly instead of calling `Render()`'s native
`RenderTipTextList()` draw: `MainFrameWindow.cpp`'s skill-hotkey tooltip, and (this round)
`SiegeWarBase.cpp`'s guild-skill tooltip (Siege War). `Render()` itself now has **no callers**: its
last one, the MU Helper skill picker's `RenderSkillInfo()`, was unreachable and went with that port.
Kept on purpose rather than deleted; the picker shows no hover tooltip, as native never did.

**Deliberately not on this primitive**: `CBuffStrip`/`CMuHelperBar`'s own hover tooltip is still a
separate, CSS-only `:hover` mechanism (plain text, no per-line color) — deferred because it lives
in a `dp`-based coordinate system, unlike every other caller's reference-pixel one; see
`tracked-deferrals.md`'s pilots-to-revisit table. `HelpWindow.cpp`/`ItemExplanationWindow.cpp` also
stay on native `RenderTipTextList()` on purpose: they render unconditionally while their own window
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

A *gauge*, native's fill-by-level slider (`newui_option_volume01` back, `volume02` fill clipped to
`level * 10%`), is built in `mu_helper_common.rcss` (`.mh-gauge`/`.mh-gauge-fill`). Legacy uses one
sprite rect per level (`mh-gauge-fill-1..10`), because a clipped wrapper doesn't clip absolutely-positioned children
here. The input stays in C++, and a stock `<input type="range">` is not a drop-in replacement
inside a root-transformed panel — see `engine-findings.md` and its `tracked-deferrals.md` row.
Move it to `base.rcss` when a second window needs it.

## Does not exist as a reusable primitive yet

Recorded here so a future session doesn't assume otherwise — each of these is still ad hoc,
per-window, or entirely unbuilt:

- **ItemSlot / ItemGrid** — the slot *chrome* (border/hover highlight/count/cooldown overlay) has
  no reusable RmlUi component yet, but the pattern to build one isn't unproven: it's the same
  RmlUi-overlay-plus-native-icon split `CSkillList` (Phase 2) validated for skill icons (since
  2026-09-27 the skill icons themselves are RmlUi sprites too: `skill_icons.rcss`, `ResolveSkillIcon()`).
  An item icon is a live 3D model render (`RenderItem3D()`), which stays native, but it no longer
  has to sit outside the slot: the item hotkeys draw theirs into a `RenderTarget` the slot's
  `.item-icon` shows (see "Native content inside a document"), so a slot is one RmlUi element with
  RCSS deciding where its icon sits.
- **ProgressBar / HealthBar / ManaBar / ExperienceBar** — `main_frame.rcss`'s HP/MP/AG/SD/EXP
  gauge-fill rules (`#hp_fill` etc.) are ad hoc per-window CSS, not an abstracted, reusable bar
  component another window could reference. `title_scene.rml`'s loading bar uses RmlUi's own
  built-in `<progress>` element instead (`SetValue()`/`SetMax()` from C++, no model binding) — a
  real, proven raw-element option for a future gauge, but still not an abstracted shared component.
- **ScrollContainer, Notification, HUDContainer** — none of the currently migrated windows have
  needed one yet, so none exist. `CMainFrameWindow`'s still-legacy skill grid/pet-command row is
  the closest thing to a "grid" concept in the codebase, and it hasn't been abstracted either (see
  `tracked-deferrals.md`'s pilots-to-revisit entry for why its icon art stayed legacy 2D). **List
  moved out of this bucket 2026-09-13** — see the "List / repeated rows" section above; `data-for`
  already proves the pattern, it just isn't fully adopted yet. **Text field also moved out of this
  bucket** — see the "Text field" section above; stock `<input>` plus a shared `.text-field` class
  is the convention now, proven by `CMyShopInventory`.

## Tab / TabBar

**Moved out of "doesn't exist yet" (2026-09-19)** — proven on 3 windows now:
`COptionWindow` (6 tabs), `CMyQuestInfoWindow` (3 tabs), `CPetInfoWindow` (2 tabs). Same shape every
time, no reusable C++ wrapper needed (matches this catalog's general "each window binds its own"
convention): an `int active_tab` model field, one `.tab-btn` per tab with
`data-class-active="active_tab == N"` and `data-event-click="window_select_tab(N)"`, and each tab's
content wrapped in a panel with `data-class-hidden="active_tab != N"`. `RmlClickSelectTab(int)` is
the C++-side handler name convention. Legacy theme swaps a sprite decorator on `.active`
(`my_quest_info.rcss`'s `.tab-btn-quest.active { decorator: image(myquest-tab-small); }` — real
sprite-art tabs, one CRadioGroupButton frame per state); modern swaps a flat
`background-color: token(accent-steel)` instead (no sprite art needed). Start from
`my_quest_info.rml`/`.rcss` (3 tabs, plain content panels) or `pet_info.rml`/`.rcss` (2 tabs, sprite-
based tab art) rather than inventing the mechanism again.

## Using this catalog

Before building a new one-off mechanism for a window port: check this list first. If the concept
you need already exists above, reuse it. If it's listed as a gap, that's a signal you may be the
first real use case defining its shape — follow `architecture-principles.md` §27's workflow
(understand intent, then design the RML/RCSS/C++ split) rather than copying whatever the nearest
existing window happens to do, and update this catalog once the pattern is proven.
