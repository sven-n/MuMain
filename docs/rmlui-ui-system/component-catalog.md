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
`.btn` stays neutral. This is the RCSS-layer contract only — the C++ side has three unrelated
button classes of its own (`CButton`, `CUIButton`, `mu::ui::window::CButton`); see
`docs/rmlui-ui-system/building-new-ui.md` for which one to use and why they aren't duplicates of
each other.

## Checkbox

`.checkbox-row`/`.checkbox-box`/`.checkbox-box.checked`/`.checkbox-label`, both themes' `base.rcss`
— same shared-contract shape as Button.

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

## Theming

`UI::RmlBridge` (`RmlTheme.h`): `LoadThemedDocument()` (the one entry point every migrated window
uses instead of `Context::LoadDocument` directly — makes "add a theme" a drop-a-folder operation),
`GetActiveThemeName()`, `ThemeProvidesOwnIconChrome()` (a declared theme capability,
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

## List / repeated rows

RmlUi's `data-for` binding against a `std::vector<T>` model field — the proven pattern for any
"N rows of the same shape" content, and the sanctioned replacement for `CUITextListBox<T>`
(`UI/Widgets/UIControls.h`, `CUIControl` family): `ui-target-architecture.md` Rule 11 says not to
reach for that legacy class in new code, this is what to reach for instead. Two proven references:
`CBuffStrip`'s buff-icon strip (a simple array) and `CMyQuestInfoWindow`'s quest list
(`my_quest_info.rml`/`.rcss`, ported off `CUICurQuestListBox`/`CUIQuestContentsListBox` — also
proves `server_select.rml`'s click-a-row-to-select-it pattern on top of the same binding). No
generic "ListBox" C++ wrapper exists (and none is needed) — each window binds its own row-shaped
struct directly, the same way `RmlModelBinder<T>` is used everywhere else. See
`tracked-deferrals.md`'s "Tracked deferral: `CUIControl` family... full retirement" entry for the
~18 `CUITextListBox<T>` subclasses still waiting on this port, one per window, same pattern each
time.

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
  read that, not a paraphrase, before touching anything `item3D`-adjacent.
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

`UI::Skills::Tooltip::Render()`/`BuildModelForSlot()`/`ToRmlBridgeLines()`
(`UI/HUD/Skills/SkillTooltip.h`/`.cpp`) is the shared skill/pet-command tooltip *content* builder —
deliberately not in `SkillTooltipModel.h`, which is also shared with the standalone MuEditor (ImGui)
tool and has no RmlUi dependency to pull in. `ToRmlBridgeLines()` converts a resolved `Model` into
`UI::RmlBridge::Tooltip::Line`s (the `LineColor` switch every caller used to hand-roll); three
callers now build a `Config` from it directly instead of calling `Render()`'s native
`RenderTipTextList()` draw: `MainFrameWindow.cpp`'s skill-hotkey tooltip, and (this round)
`SiegeWarBase.cpp`'s guild-skill tooltip (Siege War). `Render()` itself is kept for one remaining
caller, but that caller is dead code, not a migration gap: `WindowMuHelper.cpp`'s
`CMuHelperSkillList::RenderSkillInfo()` (the MU Helper bot config window — distinct from the
already-migrated `CMuHelperBar`) is only ever invoked through an `if (m_bRenderSkillInfo && ...)`
guard whose fields are never set to anything but their constructor defaults anywhere in the file —
confirmed unreachable, not merely unmigrated. Left as-is (not deleted, not wired up) per explicit
decision; revisit only if this window's skill-hover tooltip is ever actually wanted as a live
feature.

**Deliberately not on this primitive**: `CBuffStrip`/`CMuHelperBar`'s own hover tooltip is still a
separate, CSS-only `:hover` mechanism (plain text, no per-line color) — deferred because it lives
in a `dp`-based coordinate system, unlike every other caller's reference-pixel one; see
`tracked-deferrals.md`'s pilots-to-revisit table. `HelpWindow.cpp`/`ItemExplanationWindow.cpp` also
stay on native `RenderTipTextList()` on purpose: they render unconditionally while their own window
is open rather than on hover, so they don't fit this primitive's owner-token model (the newest
`Show()` always wins, which assumes a momentary, naturally mutually-exclusive hover tooltip) — a
second, non-competing primitive for them was scoped and rejected as not worth duplicating most of
this primitive's positioning/clamping logic for two low-traffic windows.

## Does not exist as a reusable primitive yet

Recorded here so a future session doesn't assume otherwise — each of these is still ad hoc,
per-window, or entirely unbuilt:

- **ItemSlot / ItemGrid** — the slot *chrome* (border/hover highlight/count/cooldown overlay) has
  no reusable RmlUi component yet, but the pattern to build one isn't unproven: it's the same
  RmlUi-overlay-plus-native-icon split `CSkillList` (Phase 2) already validated for skill icons.
  **Correction, 2026-09-06**: this entry previously called `CItemHotKey`'s icons "3D-camera-
  composited" and framed the whole slot as "still 100% legacy 2D rendering... the next real
  candidate to prove a pattern against" — wrong on both counts. Traced to source
  (`CItemHotKey::RenderItems()` → `RenderItem3D()`/`ZzzInventory.cpp` →
  `RenderObjectScreen(MODEL_...)`), the icon is a genuine **live 3D model render**, the same
  technique `CCharMakeWin`'s character-preview panel uses — Section E of
  [`ui-target-architecture.md`](ui-target-architecture.md) puts that in the *permanent*,
  no-RmlUi-equivalent bucket, not the temporary sprite-atlas one. So only the slot chrome around
  the icon is a real "prove the pattern" candidate; the icon itself stays native permanently, same
  as `CCharMakeWin`'s preview.
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
  already proves the pattern, it just isn't fully adopted yet.

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
