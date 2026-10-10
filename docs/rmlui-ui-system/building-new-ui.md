# Building New UI: Windows, Dialogs, HUD Panels, and Widgets

A decision guide for "which base class / widget class / folder do I use when adding something
new under `UI/`?", and how a window's C++ side plugs into RmlUi. Read `architecture-principles.md`
first for the philosophy; this doc is the concrete C++ object-layer answer (`component-catalog.md`
covers the parallel RmlUi/RCSS layer).

For anything with a visible presentation, RmlUi + `base.rcss`'s shared classes (`.btn`,
`.checkbox-box`, `.text-field`, `data-hint`) is canonical. The `mu::ui::window` widget family
(`UI/Widgets/Window/*.h`) is a **transitional bridge for content that must stay native** — live
3D-camera content, world-anchored overlays, or a documented native companion — not an alternative
to RmlUi for ordinary 2D chrome. "Shape of the kit" below has the reasoning and where the line
sits.

## Toolkits and their current roles

| Toolkit | Base class(es) | Real home | Status |
|---|---|---|---|
| Sprite widgets | *(none left)* | — | **Closed.** `CWin`/`CWinEx`, `CGaugeBar`, `CSlider` and the sprite `CButton` are deleted. |
| Friend/mail/chat base | `CUIBaseWindow`, `CUIWindowMgr` (both over `CUIMessage`) | `UI/Social/SocialWindowBase.h` | **The family's own base, nothing more.** The `CUIControl` toolkit it came from is deleted, widgets and base alike. What survives is the identity, parent, state, geometry and message queue `CUIWindowMgr` runs the friend/mail/chat windows through. |
| `mu::ui::window` tier | `CObject : IObject`, `CManager`, `CTextBox` | `UI/Core/{WindowObject,WindowManager}.h`, `UI/Widgets/Window/*.h` | **`CObject`/`CManager` are the base for all new work.** The widget family is for native-only content. |

## Shape of the kit

```
1. UI Runtime     — CObject / CManager (lifecycle, dispatch, depth/key order, show/enable)
2. UI Geometry    — UI::Scaling::UITransform (scale inputs, measuring units),
                    and the theme's workspace (window-placement.md)
3. UI Components  — RmlUi + base.rcss components; the mu::ui::window widgets only for
                    content that stays native
4. Presentation   — RmlUi/RCSS, CSprite (native visuals), CUIRenderText
5. Application UI — CTrade, CMyInventory, CMainFrameWindow, … — composes 1–4
```

- **`CObject` stays thin — no geometry, rendering, input or styling fields.** `CCharInfoBalloonMng`
  has no static rect at all (a per-frame `WorldToScreen()` projection), so a base-class geometry
  field would be meaningless for it. A window with a real rect owns a `WindowGeometry` value
  (composition) and calls `Contains()` from its own `UpdateMouseEvent()`; per-row or per-tab
  sub-rects stay inline. `CObject`'s shown/active split (`UpdateWhileShown()`/
  `UpdateWhileActive()`) is the same opt-in shape.
- **`CManager` provides only measuring units.** Its dispatch runs every pass (`Update()`/
  `Render()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`) under one `ScopedActiveTransform` of
  `MeasuringUnits()` and leaves the pointer alone; it is topmost-first, consume-and-stop
  (`UpdateMouseEvent()` returns `false` only to consume). Placement is the theme's
  ([layout-and-scaling.md](layout-and-scaling.md#units-and-placement)).
- **Input**: `CInput` is built on `CNewKeyInput` (its keyboard queries forward to the
  `IsPress`/`IsRelease` free functions) — one root sampler plus a façade scoped to login and
  character select, whose mouse state is stale elsewhere. RmlUi's own events drive RmlUi elements
  only.
- **No native container layer.** `CManager`'s registry is flat; RmlUi's DOM gives every ported
  window containment and scroll clipping, so a native `Panel`/`ScrollContainer`/`HUDContainer`
  (principles §20) would serve only the shrinking native population and is not built.

**The RmlUi/native boundary.** Permanently native is only content with no RmlUi equivalent:
**live 3D** (item grids and icons, equipped items, character and item previews) and
**world-anchored positions** (`WorldToScreen()` projections). `UI::RmlBridge::RenderTarget` lets a
document *show* live 3D as an image at its element's depth; that does not make the 3D render
portable. That decides who draws the content, not where it may sit.
Everything else — chrome, layout, text, buttons, tooltips, sprite-atlas icons — is RmlUi; the test
is "is this a live 3D render or a world-space projection", not "is this hard to port". A ported
window may keep one kind of native companion: a control kept for hit-testing or for state other
code reads (the quick command menu's hover index, which the control socket observes).

## Reference screens — copy these, not an arbitrary neighbouring window

Three permanent shapes, following that boundary:

| Shape | Reference | Why |
|---|---|---|
| RmlUi-only 2D UI | `CMsgWin` (`UI/Windows/MsgWin.h`); `RememberPasswordPrompt` for a free-function variant | The default for every screen; the other two are the only carve-outs. |
| Hybrid RmlUi/native 3D UI | `CItemHotKey` (`UI/HUD/ItemHotKey.h/.cpp`) | RmlUi owns the slot chrome; the item icon stays a live 3D render — the permanent boundary, not a porting gap. |
| World-overlay UI | `CCharInfoBalloonMng` (`Character/CharInfoBalloonMng.h`) | Per-frame `WorldToScreen()` projection, no static 2D rect. |

## Quick decision guide for a new window, dialog, or HUD panel

1. **Base class: `mu::ui::window::CObject`.** Always.
2. **Register it** with `CManager` (`AddUIObj(INTERFACE_KEY, this)`) like every window in
   `UI/HUD/`, `UI/Inventory/`, etc. Implement it as in "The C++ side of an RmlUi window" below.
3. **Widgets: RmlUi and shared `base.rcss` classes.** The native cheat sheet below applies only
   to documented native companions.
4. **Folder: by feature domain, not by toolkit.** `UI/Combat/`, `UI/Inventory/`, `UI/Events/`,
   `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Social/`, `UI/Quests/`, `UI/Character/`, `UI/Options/`,
   `UI/MuHelper/`. `UI/Widgets/` is for genuinely feature-agnostic controls only; `UI/Dialogs/` for
   modal/message-box-style windows. `UI/Windows/` is the closed login/credit set — don't add there.
5. **Wrapping a big legacy subsystem instead of writing one?** Put a thin `CObject` adapter in
   front that forwards into it, rather than reimplementing it or inventing a second manager.
   `CFriendWindow` (owns and forwards to `CUIWindowMgr`) is the template.
6. **Scales come from `UI::Scaling`; placement from the theme.** No second
   `g_fScreenRate_x`-style global or hand-rolled reference scale, and no position from C++: a
   new window's document takes `.stage`, `.hud-board` or a workspace slot.
7. **Widget-level polling goes through `mu::ui::window::IsPress`/`IsRelease`/`IsNone`/`IsRepeat`**
   (`UI/Core/WindowCommon.h`); `CInput::Instance()` only for the login/character-select family's
   real-pixel needs (double-click, left-hand swap, raw cursor).
8. **Load documents through a `UI::RmlBridge::ThemedView`**; theme-specific behaviour is a
   `theme.ini` capability, never a theme-name branch. A static hint is `data-hint` on the
   element (`UI::RmlBridge::DocumentHints`); one built from game state uses
   `UI::RmlBridge::Tooltip` (`ElementTooltip` for an element of a document). Never a new per-window
   `RenderTooltip()` or a CSS `:hover` hint.
9. **Deprecated families get no new call sites, features or subclasses**, and no new native
   button implementation.

## The C++ side of an RmlUi window

`CObject`'s interface (`UI/Core/WindowObject.h`) is `Render()`/`Update()`/`UpdateMouseEvent()`/
`UpdateKeyEvent()`/`GetLayerDepth()`/`IsVisible()`, plus non-virtual `Create()`/`Release()`/
`Show()`. For a window whose visuals are an RmlUi document:

- **`Render()` draws nothing** and returns `true`; RmlUi renders the document in its own pass.
- **`UpdateMouseEvent()` claims the panel**, so a click on it doesn't also reach windows below or
  the world: the theme gives `#panel` `pointer-events: auto`, so RmlUi takes clicks on it, and the
  window returns `!UI::RmlBridge::IsPointerOver(document)` (`RmlPointer.h`), which needs no units
  (`CCharacterInfoWindow` is the shape). A window with native content under its panel tests that
  content's own element with `IsPointerWithin()` or `PointerIn()` instead.
- **`UpdateKeyEvent()`** keeps only real key behaviour (Esc to close, hotkeys).
- **A `UI::RmlBridge::ThemedView` member owns the documents and the model**
  (`UI/RmlBridge/RmlThemedView.h`): the model's binding function, the document paths and any
  setup. `Create()` calls its `Ensure()`, which builds once and is cheap after (`Create()` runs
  again on resolution change), and `Release()` calls its `Release()`. A theme switch needs nothing
  from the window: the view keeps the model's values, rebuilds the documents and shows again what
  was visible (`component-catalog.md`'s Theming section).
- **`Update()`** reads live game state into the model; it runs only during `MAIN_SCENE`.
- **Visibility.** Show/hide the document from `Show(bool)`, or from a per-frame sync through
  `UI::RmlBridge::SyncDocumentVisibility()` (transition-only — re-asserting `Show()` every frame
  steals focus from a text field; see `RmlDocumentVisibility.h`). A HUD document that stays shown
  across scenes must also be added to `CSystem::SyncMainSceneHudVisibility()`
  (`WindowSystem.cpp`) through a `SyncDocVisibility(bool sceneAllowsShow)` method: `CObject`'s
  `Update()` only runs in `MAIN_SCENE`, so nothing else hides it on the login and
  character-select screens.
- **Delete the native widgets the document replaces** (`CButton`s, `SetButtonInfo()`, tooltip
  helpers, `LoadImages()`/`UnloadImages()` no one else aliases). Once `Render()` paints nothing
  they cannot do anything; they are not redundancy.
- **Blur on hide** if the document has a text field, so it opens again without the old focus
  (`CChatInputBox::ClosingProcess()` is the shape). Hotkeys don't depend on it: a field in a hidden
  document never counts as typing.
- **Clicks on the world.** `Input/Selection.cpp` (`SelectObjects()`) and
  `Engine/Object/ZzzInterface.cpp` (`Attack()`) check `Core::Input::IsMouseOverUI()` beside the
  native `MouseOnWindow`/`mouseOnHud`/`CheckMouseUse()` flags, so every RmlUi document is covered
  automatically. A new gameplay call site that gates on those flags needs the same check.

## Ownership: what the C++ side of a window may own

`architecture-principles.md` §1 states the split; this section is the concrete test to apply while
writing, derived from an ownership audit of the migrated windows. The audit found game
behaviour well contained in C++ and presentation widely leaked out of RCSS — so these rules are
about the second direction, which is where new code actually goes wrong.

**Why it is a hard rule and not a preference.** `data-style-*` writes an *inline* property, and
this build resolves inline properties before any stylesheet rule with no `!important` escape
anywhere (see `engine-findings.md`). A coordinate or colour bound from the model is one **no theme
can ever override** — not by specificity, not by shipping its own copy of the document. Nothing
warns: the theme's rule simply has no effect.

**The same applies to a `style=` attribute in the markup**, which writes the same inline property.
Layout belongs in RCSS addressed by id or class, never in a `style=` attribute — not even for a
handful of repeated rows, which is where it keeps appearing. For rows whose count varies, give the
set its own container and let the theme address them with `:nth-child` (`engine-findings.md`).

1. **A model field is a fact about the game or the UI's state, never a rendering of it.** Expose
   `pk_level`, not `name_color`; `str_source = "potion" | "item" | "base"`, not `str_value_color`.
   The shape to copy already ships: `gold_tier` and `level_bucket` classify in C++ and let each
   theme own the colour (`base.rcss`'s `.gold-*`, `trade.rcss`'s `.level-bucket-*`).
2. **Never bind a coordinate that does not change with the data.** Static chrome positions belong
   in RCSS, addressed by id. Bind the root transform and genuinely per-frame values (marker
   positions, projected world labels) — nothing else. `Tools/check_rml_bound_geometry.py` enforces
   this at build time: a document binding `left`/`top`/`right`/`bottom`/`width`/`height` (or a
   custom property with a length unit) fails the build unless it says why in a
   `<!-- bound-geometry: <why> -->` comment, and expressions reading only
   `root_x`/`root_y`/`root_scale` are exempt. A number multiplied by `root_scale` fails it
   outright: a counter-scaled layer's lengths are `calc(Npx * var(--root-scale))` in RCSS
   (`component-catalog.md`'s Counter-scaled text). **If that check sends you here, the answer is almost
   always RCSS** — the marker is for geometry that is the state itself, not for a
   coordinate that was easier to push from C++. Lines of text that stack go in one `.sharp-flow` layer and flow;
   a list the original laid out by index (a zig-zag, a fan-out) binds the index as a unitless
   custom property and the theme turns it into a position; a native line height binds as a metric
   (`--line-height`), and the theme sizes rows from it.
3. **If you are writing `a / 2 - b / 2` or `y += stripHeight`, you are writing RCSS in C++.**
   Centering and stacking are what the layout engine is for.
4. **A constant that appears in both C++ and RCSS is a bug waiting.** Author it in RCSS and ask
   the drawn element (`RmlPointer.h`, or its box) from C++. That applies to native content's own
   size too, not just a panel's.
5. **Do not transcribe the native `Render()` into a list of `{text, x, y, width, align, bold,
   color}`.** That turns the model into a draw list and the document into a replayer, and it takes
   every one of these rules down with it. Reverse-engineer the layout intent
   (`architecture-principles.md` §2) and author it by id instead.
6. **Keeping a native control for hit-testing or data does not license C++ to own its
   presentation.** Once the window's `Render()` paints nothing, §2's native-geometry exception
   (RCSS owns geometry → C++ reads it → native follows) no longer covers that window — and the
   inverted flow, C++ constant → RmlUi position, is the thing that exception exists to avoid.
7. **State gets one authority.** Hold a selection or current tab in the model and let both the
   highlight and the page read it. Do not mirror a native control's index into a C++ member and
   keep the two in step by hand.
8. **Let markup request; make C++ decide.** A markup-supplied action name or index is fine —
   re-check the rule in C++ regardless, the way `CCharacterInfoWindow::RmlClickIncreaseStat()`
   re-checks `LevelUpPoint` and the class-dependent stat range before sending anything.
9. **Keep user preference, semantic state and theme decision in separate fields.** Bind an alpha
   and a boolean; let RCSS own the colour. Composing them into one CSS string hands the theme's
   share to C++ permanently.
10. **Branch on a declared theme capability, never a theme name** (§30) — `theme.ini`'s
    `[Capabilities]`, read via `ThemeUsesNativeTextSize()`. Better
    still, ship the content in shared markup and let a theme that doesn't want it hide it in RCSS.
11. **Expose a purpose-built view model, never a game object.** Currently true of every
    registered struct — don't be the first exception.

**The shape in one example.** The skill hotkey's number used to be a digit-sprite subscript drawn
in C++ for both themes. Now `CSkillList::GetHotKeySlotNumber()` returns the number,
`SyncRmlModel()` binds it, and each theme's `main_frame.rml`/`.rcss` places it through
`.skill-hotkey-label` (modern: top-left like its Q/W/E/R; legacy: bottom-right like the original).
Likewise the HP readout: C++ computes both `hp_text` ("935 / 935") and `hp_current_text` ("935"),
and each theme binds the one it wants — no theme-name check anywhere (§30, §31).

**Two documents to compare before choosing a shape.** `mu_helper_config.rml` and
`guard_window.rml` are the same kind of window — same dock, same `docked_panel_frame.rcss`, both
tabbed, both with a native control retained underneath. The first binds *only* its root transform
and places ~65 controls by id in RCSS; the second binds its every label position, size, alignment,
weight and colour, and its RCSS can change almost nothing. Copy the first.

### What may still bind geometry

The documents that still bind geometry, each with a `<!-- bound-geometry: <why> -->` marker: per-frame data (gauges, things that follow the
pointer or scroll, windows that grow with their content, projected markers) and text measured the
way the native renderer measured it. A list the server does not bound is not a reason: it flows in
RCSS, as the duel spectators and score marks do.
`root_x`/`root_y` and `panel_x`/`panel_y` stay apart: the first is the physical origin of a root
scaled uniformly by `root_scale`, the second a reference-unit position inside a stretched `.screen`.

`.sharp-text` counter-scaled tops, `MiniMap`/`WorldLabelLayer` marker coordinates, and
`TitleSceneUI`'s loading bar are justified hybrids, recorded in the README's Known limits.

Passing the guards establishes documented exceptions, not runtime correctness. The bound-geometry
guard covers the four box offsets and the two sizes only: a bound `color`, `decorator` or
`font-size` has the same override problem but is a judgement call per case, while a bound static
coordinate is nearly always layout that belongs in RCSS. Widen it when a bound colour bites. The
active UI transform has its own guard: `tools/check_layout_transform_users.py` keeps it to the
infrastructure (the window manager's measuring scope, the text renderer, the inventory's screen
scope, the tooltip's metric scope, each listed in the script with its reason).

## Checklist for every new port (principles §27's workflow, condensed)

1. **Layout intent traces to the original code's computed behaviour**, not its literal
   default-case numbers (§2–3) — `buff_strip.rml`'s header derives `x = (iScreenWidth - 200) / 2`
   from `SetPos()`'s four hardcoded pairs.
2. Uses the `dp` anchor/stretch/center classes (`layout-and-scaling.md`) or a workspace slot
   (`window-placement.md`) instead of C++-pushed rects, unless the position is genuinely per-frame.
   A bound per-frame offset uses the unit of the sibling static CSS (`dp` ≠ `px`).
3. Deliberate aspect-ratio/resolution behaviour: fixed, edge-anchored, centred or stretched (§7–8).
4. C++ owns state, binding, events and game behaviour; RCSS owns layout, sizing and position
   (§1, §16; `building-new-ui.md`'s ownership rules).
5. The C++ class and the RmlUi assets are named for what the component is, renamed at port time
   (§12).
6. Both themes in the same pass. A rendering technique new to the port is verified at runtime
   (`engine-findings.md`).
7. Reuses existing primitives (`component-catalog.md`). For the modern theme, match the windows it
   appears **beside** on screen (its dock group), not its nearest technical sibling; a docked
   panel links `docked_panel_frame.rcss`.
8. A new document gets its original window's layer depth in the stacking table (below).
9. A window whose native hit box comes from live RCSS is clicked through at a non-100 % UI scale
   (`layout-and-scaling.md`'s scale sweep).

## Stacking order

Every document's `z-index` is the layer depth of the original window (or render pass) it
replaces, from one table (`UI/RmlBridge/RmlStackingOrder.cpp`), set when a themed document loads.
The original drew its windows in ascending `GetLayerDepth()` order, then the notices, the scene
windows, the login scene's message box and the reconnect dialog; RmlUi sorts a context's
documents by `z-index` and keeps show/focus order only among equal depths, so
`SyncDocumentVisibilityInFront()`/`Behind()` and focus only order documents of one depth. Passes
outside the window list: object descriptions
and the map name 0.5, notices 20, scene windows 30 (balloons 29, the remember-password prompt 31),
loading and title screens 40, reconnect dialog 50. The shared tooltip is 10.69, above every window
and under the message boxes (10.7): the original drew each tooltip at its owner's depth, where the
chat log, the friends window and the HUD hid its rows. The top-right button row (the modern
theme's choice) is its own document (`main_frame_top.rml`, 1.05: over the names, under every
window, which dock over that corner). Native parts (item grids, 3D items) keep the native order
and stay under the main context. `rml_stacking_order_tests` checks that every document the sources
name has an entry.

## Legacy parity rules

Differences to the original found by the paired comparison suites and fixed in shared places, so
a new port inherits them:

- **Scene gate.** A window `CSystem` updates only in the main scene still has a live document in
  every scene. Its stacking-table entry marks it a main-scene document, and every such document is
  suspended outside the main scene (`SuspendMainSceneDocumentsOutsideMainScene()`). The HUD's
  documents also wait for the world to load (`CSystem::SyncMainSceneHudVisibility()`). A window
  the login and character scenes open too (the options window) is a `DocumentScene::Every`
  document instead: never suspended, at its own depth in the world and over the scene windows
  outside it (`ApplySceneStackingDepths()`).
- **Scroll thumb.** The legacy `.scroll-pane` thumb is the native 15x30 knob, not a proportional
  bar; a list the original scrolled one row per wheel notch takes `mousescroll` itself
  (`CMoveCommandWindow::RmlWheelList()`), since RmlUi scrolls 80 dp per notch.
- **Button hover text.** A document's `data-hint` and a native button's `CTooltip` both use the
  shared tooltip's `Config::Box::ButtonHint` (unframed, 2 units off the button, the other side
  when there is no room); the framed box is for `RenderTipTextList()`.
- **Hangul.** NanumGothic is a fallback face, so Korean game text draws in any family.
- **Alpha test.** Art the original drew under `EnableAlphaTest()` (reference 0.25) stays invisible
  while its fade is below a quarter (the Illusion Temple banner).
- **Scene windows re-created per visit.** A `Create()` that resets model fields must mark them
  dirty (the login fields) and reset what the original reset (the server list's chosen group).

## Lessons from shipped ports

Engine quirks are in [`engine-findings.md`](engine-findings.md); these are porting patterns.

**Input and focus**

- **A text field's window claims the field's document.** While the player types,
  `CManager::UpdateKeyEvent()` gives keys only to the window whose `TakesTypingFrom()` accepts the
  focused field's document (`RmlUiRuntime::GetTypingDocument()`); the chat line, chat command,
  Gold Bowman, MU Helper and friend windows claim theirs, or Enter/Escape never arrive.
- **Focus, scroll pins and scroll rewinds are one-shot latches**, done once on the frame after the
  view caught up — never per frame (that steals focus and kills the scrollbar). `CSystem::Show()`
  runs `OpenningProcess()` before `ShowInterface()`, so arm there and consume in the sync.
- **A native window behind an RmlUi document never sees a mouse press**
  (`ProcessMouseButtonDown` consumes it while anything is hovered), though the wheel still reaches
  it. Drive the gesture from the document (`UI::Social::PhotoViewerControl`).
- **`UpdateMouseEvent()` returns `false` only to consume.** `CManager` stops dispatching at the
  first `false`; a "not for me" guard must return `true` (an inventory guard once ate every drop
  into the trade grid).
- **A port can drop a side effect that was covering a bug** — `CUIMuHelper::Show()` released a
  dead startup input box's focus, and without it every hotkey stayed suspended. Look for what a
  removed call was hiding.

**Layout and text**

- **Click-through.** `IsMouseOverUI()` is true over any hovered `pointer-events: auto` element, so a
  scroll pane over the world is a wall; the chat log's lines are `pointer-events: none` with the
  scrollbar opted back in at every level, and hover/right-click moved to C++.
- **Bottom-up lists**: a flex column with `margin-top: auto` on the first item, not
  `justify-content: flex-end` (which pushes the earliest lines out of the scroll area); per-line
  backgrounds need `align-items: flex-start`.
- **A `.scroll-pane` inside a `transform: scale()` panel counter-scales itself out**
  (`.counter-scaled`, its box `calc(Npx * var(--root-scale))` in RCSS), with rows at an explicit
  width, not `100%` (`component-catalog.md`).
- **A counter-scaled layer's lengths are RCSS**: `#panel` binds `--root-scale`, base.rcss's
  `.sharp-text` / `.counter-scaled` cancel it, and the theme writes `width: calc(160px *
  var(--root-scale))`. A number multiplied by `root_scale` in RML fails the bound-geometry guard.
- **A label in a constricted box** that must keep native's position is a `.marquee`: one line,
  `..` when longer, the whole text scrolling on hover (`RmlMarquee.h`). Text that may take more
  lines wraps in a flow or a scroll pane instead (`event_entry`, `shop_notice.rcss`).
- **`MeasureText()` returns reference units**, not pixels; `RenderText()` shrinks text wider than
  its box — use `NativeTextPixelSizeInBox()` per text. Text the native renderer draws small is laid
  out at `CachedFontPointSize()` and scaled down (the login scene lines).
- **`overflow: hidden` does not clip under a panel's `transform: scale()`**; crop a bar with
  `decorator: image(<sprite> scale-none left top)` on an element of the shown width, or an
  untransformed box with `clip: always`.
- **Centre a counter-scaled label in RCSS**, not by a C++-measured top: `.sharp-middle` centres it on its button's height and `.sharp-centre` on its
  parent's width (`engine-findings.md`).
- **Draw a window in the scale its slot is sized in.** The event HUDs were placed in the HUD's
  UI-scaled units but drawn in the original's W/640 x H/480 stretch, so at 90 % they outgrew their
  slots; they now draw at the HUD's own scale.
- **Data expressions have no unary minus**: bind `-x` from C++.
- **Class-specific controls are one tested C++ table bound as flags**
  (`UI::MuHelper::ResolveClassFeatures()`), never RCSS — the themes cannot disagree.

**Drawing**

- **A window the original drew under every panel** (siege HUD, duel and battle-soccer boards)
  takes its low depth from the stacking table; **one with a live 3D preview** draws it into a render
  target in its own document (`UI::Items::ItemCameraTarget`, `UI/Events/EventItemEntryView`).
- **Native 3D inside a document** goes through `UI::RmlBridge::RenderTarget`, so it z-orders with
  the windows around it.
- **World-anchored or shared legacy drawing** goes through the world-label layer's
  `Overlay2DRecordScope`, which records `RenderText()`/`RenderColorQuadARGB()`/`RenderBitmap()`
  and replays them into pooled elements; `CObject::PrepareFrame()` records them before any window
  renders.
- **`EnableAlphaBlend()` is additive** (ONE, ONE): `decorator: additive-fill(<colour>)` /
  `additive-image(<colour> <image>)`.
- **Textures are premultiplied on load** (`RmlUiRenderInterface::LoadTexture`).
- **A texture cut by UVs, padded by the loader, or drawn mirrored** is an `<img rect="x y w h">` in
  texels (mirrored: `transform: scale(-1, 1)`).
- **A quad rotated in physical pixels** is a CSS `matrix()` built from three corners
  (`UI/HUD/MiniMapLayout`).
- **Image paths** in a themed document resolve from the theme folder (`../../../../Logo/…`); an
  absolute `/Interface/…` path misses `Data` and draws a white quad.
- **Animations the original stepped in `Render()`** move to `Update()`.
- **One document and model per instance** (chat rooms, letters) through `ThemedView`'s
  `modelPlaceholder` and `SetModelName()`.
- **A block-scope `extern` inside `mu::ui::window`** declares a namespace member, not the global.
- **Windows the original never showed** can carry latent crashes; exercise every size.

## Event windows outside their event: `$preview`

The event windows that draw only while a server runs their event are looked at with `$preview <event>`
(`UI/Events/EventPreview.cpp`; `$preview` lists them, `$preview off` ends one). It fills a window
through the setters its packets use and lets it draw off its map; the window sends nothing. Teleporting
a game master to the map passes the map check but brings no event state, and the GM move does not
create the siege minimap that a map join does. A preview is not a live event.

- **Illusion Temple HUD**: its corner part (time, mini map, skill panel) is `#corner` in
  `cursed_temple_system.rml`; a theme moves it with `left`/`top`, and C++ reads the offset back to
  place its markers, digits, buttons and hit tests. Modern moves it left of the worn-equipment icons.
  Its three hover tooltips show in both themes (native's kill-point zones start at each number's
  centre, kept).
- **Siege commander HUD**: `$preview siege` seeds members, NPCs and commands against the hero's
  zoom-1 crop -- inside, on each edge and just outside -- so both zooms' clipping is exercised.

The Blood Castle and Chaos Castle timers, the duel spectator list, the CryWolf result and the HUD
status texts (`$preview status`; the crown switch lines show only at the switches) and the event
result and progress boxes (`$preview bcresult`, `ccresult`, `dsrank`, `switchbox`) and the guild war
time and result (`$preview guildwar`) look right. `$preview kanturu` fills the Kanturu entry window
and `$preview notices` the centre-screen notices; `$dialog menu <name>` opens an NPC menu without its
NPC.
The Battle Soccer score, the duel frame, the Empire Guardian timer and the Doppelganger frame draw
outside their event (`$win soccer full`, `duel`, `empiretimer`, `doppelframe`) and were checked in
both themes.

## Naming

Name the C++ class and the RmlUi assets after what the window *is*, not the legacy tier or class
it came from, and rename at port time (`architecture-principles.md` §12):
`CNewUIHeroPositionInfo` became `CMuHelperBar` / `mu_helper_bar.rml`, with its `INTERFACE_*` key,
accessor and globals renamed to match. Port one legacy class as one component; split only where
the code already has two independent lifecycles.

## Widget cheat sheet (native-only content on new `mu::ui::window::CObject` windows)

Applies only to content that must stay native — for anything with an RmlUi presentation, skip this
table.

| Need | Use | Header | Don't confuse with |
|---|---|---|---|
| Multi-line read-only text | `mu::ui::window::CTextBox` | `UI/Widgets/Window/TextBox.h` | — |
| Single-line text entry | **stock RmlUi `<input>`** + `.text-field` | `themes/*/base.rcss` | Bind with `data-value`, keep `maxlength`/validation in C++; numeric fields use `UI::RmlBridge::NumericInputFilter`. See `component-catalog.md`'s "Text field" |
| Progress/gauge bar | RmlUi's built-in `<progress>` | — | `title_scene.rml`'s loading bar; check `component-catalog.md` before inventing another pattern |
| Scrollable list of rows | RmlUi `data-for` in a `.scroll-pane` | — | `component-catalog.md`'s "List / repeated rows" |

## Namespaces and look-alike names

- `MUHelper::CMuHelper` is the bot engine; `CMuHelperConfigWindow`/`CMuHelperDetailWindow`/
  `CMuHelperSkillPicker` (`UI/MuHelper/`) are its settings UI.
- `UI/HUD/ChatInputBox.h` is the complete chat-input window, not a reusable text-entry widget.
  `UI/HUD/SlideWindow.h` similarly owns the notice window beside its SlideTicker.

## Accepted as the base: the `CObject` tier

Every window is a `mu::ui::window::CObject` owned by `CManager`, registered by hand in
`WindowSystem.cpp` (creation, `INTERFACE_*` id, its `g_p*` macro). That machinery is kept as it
is, by decision: the migration never needed it to change. An RmlUi document is something a
`CObject` owns, and slots and fill placement plugged into it without trouble. Replacing it would
touch every window and every caller of the lookups for no change a player sees.

Revisit when a concrete need appears that this machinery cannot meet. The cheapest step then is
self-registration: each window declaring its id and name once, in place of the hand-kept
lists. Typed lookups in place of the `g_p*` macros, and the lifecycle moving onto the documents,
each stand alone after that.

## Kept out of the UI kit on purpose

Domain logic a component may call into, but which never moves into a generic UI type:

- **`CInventoryActionController`/item drag-drop** — inventory business logic (move/split/stack,
  server round-trips) that renders through UI.
- **`CUIManager`'s open/close exclusion** (`MUTEX_*` keys) — domain policy ("opening Inventory
  closes the personal shop purchase window"); don't merge it into `CManager`.
- **Skill-tooltip content** (`UI::Skills::Tooltip::BuildModelForSlot`) — rendering uses the shared
  tooltip; deciding what a skill tooltip says stays domain logic.
- **3D-camera and world-space rendering** (`Window3DRenderMng`/`I3DRenderObj`, `ScreenOverlay`) —
  the boundary above.

## Driving `CSprite` from native code

`CSprite` takes reference-resolution coordinates and applies the active transform's scale and the
live screen offset inside `Render()` (pre-scaling double-applies the offset). It bakes scale and
its Y-flip's `WindowHeight` basis in at `Create()`, with no live updater — rebuild it whenever
image, frame count, size, `WindowHeight` or scale change. `CButton::Render(true)`'s UV crop for
`MiniMap.cpp` is the one remaining `RenderImage()` path.

## The `CFriendWindow` seam

`UI/Social/SocialWindow*.h/.cpp` holds `CUIBaseWindow`, `CUIWindowMgr` and `CUIPhotoViewer`, reached
through one seam: `CFriendWindow : public mu::ui::window::CObject` owns one `CUIWindowMgr*` and
forwards into it. The windows themselves are RmlUi documents — the shell, each chat room and each
letter — and `SocialWindowManager.cpp` keeps the manager that arranges them, the UI-message queue
they talk over, and the native `CUIPhotoViewer` that draws a letter's sender.

- `CUIBaseWindow` (`SocialWindowBase.h`) is the family's own base: no drawing and no input of its
  own, since each window's RmlUi view drags, maximises and closes it. `CUIPhotoViewer` is a plain
  class its letter's document places.
- Don't `#include` `SocialWindowBase.h` without needing a symbol it declares.

## Cross-references

- [`architecture-principles.md`](architecture-principles.md) — the overarching design philosophy.
- [`component-catalog.md`](component-catalog.md) — the RmlUi/RCSS-layer components, the parallel
  axis to this doc's C++ object layer.
- [`engine-findings.md`](engine-findings.md) — why the Ownership rules are hard rules, and the
  engine quirks a port runs into.
- [`README.md`](README.md)'s Known limits — the windows that keep a recorded exception to those
  rules, each with its trigger.
