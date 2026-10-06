# Building New UI: Windows, Dialogs, HUD Panels, and Widgets

A decision guide for "which base class / widget class / folder do I use when adding something
new under `UI/`?", and how a window's C++ side plugs into RmlUi. Read `architecture-principles.md`
first for the philosophy; this doc is the concrete C++ object-layer answer (`component-catalog.md`
covers the parallel RmlUi/RCSS layer).

For anything with a visible presentation, RmlUi + `base.rcss`'s shared classes (`.btn`,
`.checkbox-box`, `.tooltip`, `.text-field`) is canonical. The `mu::ui::window` widget family
(`UI/Widgets/Window/*.h`) is a **transitional bridge for content that must stay native** — live
3D-camera content, world-anchored overlays, or a documented native companion — not an alternative
to RmlUi for ordinary 2D chrome. "Shape of the kit" below has the reasoning and where the line
sits.

## Toolkits and their current roles

| Toolkit | Base class(es) | Real home | Status |
|---|---|---|---|
| Sprite widgets | *(none left)* | — | **Closed.** `CWin`/`CWinEx`, `CGaugeBar` and `CSlider` are deleted; the sprite `::CButton` has no production consumer (still covered by `tests/ui/test_ui_scaling.cpp`). Nothing to add a consumer to. |
| Friend/mail/chat base | `CUIBaseWindow`, `CUIWindowMgr` (both over `CUIMessage`) | `UI/Social/SocialWindowBase.h` | **The family's own base, nothing more.** The `CUIControl` toolkit it came from is deleted, widgets and base alike. What survives is the identity, parent, state, geometry and message queue `CUIWindowMgr` runs the friend/mail/chat windows through. |
| `mu::ui::window` tier | `CObject : IObject`, `CManager`, `CButton`/`CRadioButton`/`CRadioGroupButton`/`CCheckBox`/`CComboBox`/`CScrollBar`/`CTextBox` | `UI/Core/{WindowObject,WindowManager}.h`, `UI/Widgets/Window/*.h` | **`CObject`/`CManager` are the base for all new work.** The widget family is for native-only content. |

## Shape of the kit

```
1. UI Runtime     — CObject / CManager (lifecycle, dispatch, depth/key order, show/enable)
2. UI Geometry    — UI::Scaling::UITransform / UILayoutPolicy, the opt-in WindowGeometry,
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
- **`CManager` provides only a coordinate space.** Its dispatch wraps every `Update()`/`Render()`/
  `UpdateMouseEvent()`/`UpdateKeyEvent()` in a `ScopedActiveTransform` from `GetLayoutMode()`, and
  is topmost-first, consume-and-stop (`UpdateMouseEvent()` returns `false` only to consume).
  `UI::Scaling` (seven `LayoutMode`s) is the one coordinate-transform layer.
- **Input**: `CInput` is built on `CNewKeyInput` (its keyboard queries forward to the
  `IsPress`/`IsRelease` free functions) — one root sampler plus a façade scoped to login and
  character select, whose mouse state is stale elsewhere. RmlUi's own events drive RmlUi elements
  only.
- **No native container layer.** `CManager`'s registry is flat; RmlUi's DOM gives every ported
  window containment and scroll clipping, so a native `Panel`/`ScrollContainer`/`HUDContainer`
  (principles §20) would serve only the shrinking native population and is not built.

**The RmlUi/native boundary.** Permanently native is only content with no RmlUi equivalent:
**live 3D** (item grids and icons, equipped items, character and item previews) and
**world-anchored positions** (`WorldToScreen()` projections). The background context lets RmlUi
paint *behind* live 3D, and `UI::RmlBridge::RenderTarget` lets a document *show* it as an image;
neither makes the 3D render portable. That decides who draws the content, not where it may sit.
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
6. **Coordinates go through `UI::Scaling`.** No second `g_fScreenRate_x`-style global or
   hand-rolled reference scale; add a `LayoutMode` case if none fits (and a `UILayoutPolicy.cpp`
   case for the new `INTERFACE_*` key — `AddUIObj()` overwrites the window's own mode).
7. **Widget-level polling goes through `mu::ui::window::IsPress`/`IsRelease`/`IsNone`/`IsRepeat`**
   (`UI/Core/WindowCommon.h`); `CInput::Instance()` only for the login/character-select family's
   real-pixel needs (double-click, left-hand swap, raw cursor).
8. **Load documents through a `UI::RmlBridge::ThemedView`**; theme-specific behaviour is a
   `theme.ini` capability, never a theme-name branch. Tooltips use `UI::RmlBridge::Tooltip` or the
   `.tooltip` RCSS convention, never a new per-window `RenderTooltip()`.
9. **Deprecated families get no new call sites, features or subclasses**, and no new native
   button implementation.

## The C++ side of an RmlUi window

`CObject`'s interface (`UI/Core/WindowObject.h`) is `Render()`/`Update()`/`UpdateMouseEvent()`/
`UpdateKeyEvent()`/`GetLayerDepth()`/`IsVisible()`, plus non-virtual `Create()`/`Release()`/
`Show()`. For a window whose visuals are an RmlUi document:

- **`Render()` draws nothing** and returns `true`; RmlUi renders the document in its own pass.
- **`UpdateMouseEvent()` claims the panel**, so a click on it doesn't also reach windows below or
  the world: read the live RCSS size with `UI::RmlBridge::RefreshLogicalPanelSize()` and return
  `false` while the cursor is inside `WindowGeometry(m_Pos, size)` (`CCharacterInfoWindow` is the
  shape). RmlUi does the actual hit-testing of buttons and fields. Test it at a non-100 % UI scale
  (`layout-and-scaling.md`'s scale sweep).
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
writing, derived from an ownership audit of the migrated windows (findings and affected screens in
[`tracked-deferrals.md`](tracked-deferrals.md)'s ownership-boundary entry). The audit found game
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
   this at build time: a document binding `left`/`top`/`right`/`bottom`/`width`/`height` needs a line
   in `Tools/rml_bound_geometry_allowlist.txt` saying why, and expressions reading only
   `root_x`/`root_y`/`root_scale` are exempt. **If that check sends you here, the answer is almost
   always RCSS** — the allowlist is for geometry that genuinely varies per frame, not for a
   coordinate that was easier to push from C++.
3. **If you are writing `a / 2 - b / 2` or `y += stripHeight`, you are writing RCSS in C++.**
   Centering and stacking are what the layout engine is for.
4. **A constant that appears in both C++ and RCSS is a bug waiting.** Author it in RCSS and read it
   back with `UI::RmlBridge::RefreshLogicalPanelSize()` / `RefreshLogicalAnchorPosition()`
   (`RmlBridge/RmlPanelGeometry.h`). That applies to a native companion widget's own size too, not
   just a panel's.
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
| Button | `mu::ui::window::CButton` | `UI/Widgets/Window/Button.h` | `::CButton` (sprite toolkit, closed) — two unrelated classes, same bare name, disambiguated by namespace |
| Radio button | `mu::ui::window::CRadioButton` (+ `CRadioGroupButton` to coordinate a set) | `UI/Widgets/Window/Button.h` | — |
| Checkbox | `mu::ui::window::CCheckBox` | `UI/Widgets/Window/Button.h` | — |
| Dropdown | `mu::ui::window::CComboBox` | `UI/Widgets/Window/ComboBox.h` | Deliberately base-less (see its header) — don't force it onto `CObject` |
| Scroll bar | `mu::ui::window::CScrollBar` | `UI/Widgets/Window/ScrollBar.h` | — |
| Multi-line read-only text | `mu::ui::window::CTextBox` | `UI/Widgets/Window/TextBox.h` | — |
| Single-line text entry | **stock RmlUi `<input>`** + `.text-field` | `themes/*/base.rcss` | Bind with `data-value`, keep `maxlength`/validation in C++; numeric fields use `UI::RmlBridge::NumericInputFilter`. See `component-catalog.md`'s "Text field" |
| Progress/gauge bar | RmlUi's built-in `<progress>` | — | `title_scene.rml`'s loading bar; check `component-catalog.md` before inventing another pattern |
| Scrollable list of rows | RmlUi `data-for` in a `.scroll-pane` | — | `component-catalog.md`'s "List / repeated rows" |

## Namespaces and look-alike names

- `::CButton` is the closed sprite implementation; `mu::ui::window::CButton` is the native
  companion. Qualify where both are visible.
- `MUHelper::CMuHelper` is the bot engine; `CMuHelperConfigWindow`/`CMuHelperDetailWindow`/
  `CMuHelperSkillPicker` (`UI/MuHelper/`) are its settings UI.
- `UI/HUD/ChatInputBox.h` is the complete chat-input window, not a reusable text-entry widget.
  `UI/HUD/SlideWindow.h` similarly owns the notice window beside its SlideTicker.

## Accepted as the base: the `CObject` tier

Every window is a `mu::ui::window::CObject` owned by `CManager`, registered by hand in
`WindowSystem.cpp` (creation, `INTERFACE_*` id, its `g_p*` macro) and in `UILayoutPolicy.cpp` (its
layout mode). That machinery is kept as it is, by decision: the migration never needed it to
change. An RmlUi document is something a `CObject` owns, and slots, fill placement and the layout
modes plugged into it without trouble. Replacing it would touch every window and every caller of
the lookups for no change a player sees.

Revisit when a concrete need appears that this machinery cannot meet. The cheapest step then is
self-registration: each window declaring its id, name and layout mode once, in place of the hand-kept
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
- **3D-camera and world-space rendering** (`Window3DRenderMng`/`I3DRenderObj`, `WorldOverlay`) —
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
- [`tracked-deferrals.md`](tracked-deferrals.md) — the ownership-boundary entry: which shipped
  windows already violate those rules.
