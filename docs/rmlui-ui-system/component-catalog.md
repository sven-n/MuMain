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
  stays native — Stage 2, skipped for this reason, see `STATUS.md`'s pilots-to-revisit table) and,
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

## List / repeated rows

RmlUi's `data-for` binding against a `std::vector<T>` model field — the proven pattern for any
"N rows of the same shape" content, and the sanctioned replacement for `CUITextListBox<T>`
(`UI/Widgets/UIControls.h`, `CUIControl` family): `ui-target-architecture.md` Rule 11 says not to
reach for that legacy class in new code, this is what to reach for instead. Two proven references:
`CBuffStrip`'s buff-icon strip (a simple array) and `CMyQuestInfoWindow`'s quest list
(`my_quest_info.rml`/`.rcss`, ported off `CUICurQuestListBox`/`CUIQuestContentsListBox` — also
proves `server_select.rml`'s click-a-row-to-select-it pattern on top of the same binding). No
generic "ListBox" C++ wrapper exists (and none is needed) — each window binds its own row-shaped
struct directly, the same way `RmlModelBinder<T>` is used everywhere else. See `STATUS.md`'s
"Tracked deferral: `CUIControl` family... full retirement" entry for the ~18 `CUITextListBox<T>`
subclasses still waiting on this port, one per window, same pattern each time.

## Dialog

`mu::ui::window::CGenericConfirmDialog`/`GenericDialogConfig` (`UI/Dialogs/GenericConfirmDialog.h`,
`generic_confirm_dialog.rml`/`.rcss` both themes) — a real config-driven scaffold, not a per-dialog
hand-built RML/RCSS pair: one C++ class + one document, shown with different `GenericDialogConfig`
content (button set, body lines, OK/Cancel callbacks) per call, no new subclass or new `.rml` per
dialog. Built to replace `UI/Dialogs/CommonMessageBox.h`/`CustomMessageBox.h`'s ~140-class native
`TMsgBoxLayout<T>` family (see `STATUS.md`) — proven on 3 real dialogs first
(`Guild/GuildInfoWindow.cpp`'s alliance-master-can't-leave notice, `UI/Quests/
MyQuestInfoWindow.cpp`'s quest-giveup confirm, `Network/Server/WSclient.cpp`'s guild-invite
accept/decline) before porting the rest. Single active instance, not a real stack — a second
`Show()` call while one is open queues instead of replacing it; see the class's own header comment
for why that's not a functional regression from what it replaces.

## Dragging

`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h`) — makes an RmlUi panel draggable-by-mouse with
zero legacy `CWin` dependency. **First real caller landed 2026-09-07**: `CMyInventory`'s title bar,
paired with a new generic persistence mechanism (`GameConfig::GetWindowPosition`/
`SetWindowPosition`, an `OnDragEnd` hook on `MakeDraggable` itself) any future draggable window can
reuse with one call each way — see `STATUS.md`'s "Known gaps" entry for the full mechanism and
what's still unaudited (behavior across a resolution/UI-scale/theme change post-drag).

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
  [`ui-target-architecture.md`](../ui-target-architecture.md) puts that in the *permanent*,
  no-RmlUi-equivalent bucket, not the temporary sprite-atlas one. So only the slot chrome around
  the icon is a real "prove the pattern" candidate; the icon itself stays native permanently, same
  as `CCharMakeWin`'s preview.
- **ProgressBar / HealthBar / ManaBar / ExperienceBar** — `main_frame.rcss`'s HP/MP/AG/SD/EXP
  gauge-fill rules (`#hp_fill` etc.) are ad hoc per-window CSS, not an abstracted, reusable bar
  component another window could reference. `title_scene.rml`'s loading bar uses RmlUi's own
  built-in `<progress>` element instead (`SetValue()`/`SetMax()` from C++, no model binding) — a
  real, proven raw-element option for a future gauge, but still not an abstracted shared component.
- **Tooltip** — actively **four non-unified mechanisms** exist side by side (flagged in
  `newui-tier-adapter.md`'s pilots-to-revisit table): the skill-hotkey tooltip
  (`UI::Skills::Tooltip`, `SkillTooltipModel.h`), `CMyInventory`'s Set/Socket option tooltip
  (`UI::Inventory::Tooltip`, `ItemOptionTooltipModel.h` — added Stage 3, H7), `CBuffStrip`'s
  plain-text tooltip (a deliberate scope cut from the original's per-line-colored rich tooltip),
  and whatever the still-fully-legacy windows use. The first two already share the same shape
  (a fixed-buffer `Model`/`Line{text, color, isBold}` built with no drawing by a `BuildModel`-style
  function, consumed either by the legacy `TextList`/`RenderTipTextList` path or bound into RmlUi)
  — only the `color` enum's members differ (skill: White/Blue/Red/DarkRed; item-option:
  White/Blue/Yellow/Green/Purple), making them the natural starting point if/when this list is
  consolidated. Not bundled here — check this entry before adding a *fifth*.
- **Tab / TabBar, ScrollContainer, Notification, HUDContainer** — none of the currently migrated
  windows have needed one yet, so none exist. `CMainFrameWindow`'s still-legacy skill grid/
  pet-command row is the closest thing to a "grid" concept in the codebase, and it hasn't been
  abstracted either (see `STATUS.md`'s pilots-to-revisit entry for why its icon art stayed legacy
  2D). **List moved out of this bucket 2026-09-13** — see the "List / repeated rows" section above;
  `data-for` already proves the pattern, it just isn't fully adopted yet.

## Using this catalog

Before building a new one-off mechanism for a window port: check this list first. If the concept
you need already exists above, reuse it. If it's listed as a gap, that's a signal you may be the
first real use case defining its shape — follow `architecture-principles.md` §27's workflow
(understand intent, then design the RML/RCSS/C++ split) rather than copying whatever the nearest
existing window happens to do, and update this catalog once the pattern is proven.
