# Building New UI: Windows, Dialogs, HUD Panels, and Widgets

A decision guide for "which base class / widget class / folder do I use when adding something
new under `UI/`?" The codebase has three widget toolkits that accumulated over three different
eras and, on the surface, look interchangeable — several classes even share almost the same name
across toolkits. They aren't interchangeable, and this doc exists so a new window doesn't
accidentally reach for a closed, historical one. Read `architecture-principles.md` first for the
overall philosophy this follows; this doc is the concrete "what do I actually type" answer for the
C++ object layer specifically (`component-catalog.md` covers the parallel RmlUi/RCSS layer).

**Read this before the toolkit table below**: for anything with a visible presentation, RmlUi +
`base.rcss`'s shared classes (`.btn`, `.checkbox-box`, `.tooltip`) is canonical — not a fourth
option alongside the three toolkits here. The `mu::ui::window` tier's own widget family
(`Widgets/Window/*.h`, the cheat sheet below) is a **transitional bridge for content that must stay
native** — live 3D-camera-viewport content (e.g. `CCharMakeWin`'s character-preview panel),
world-anchored overlays, or a documented Type-1/Type-2 RmlUi-companion pattern (a redundant
click-detector behind a real RmlUi button, or a real widget RmlUi can't host yet like
`CUITextInputBox`) — not a permanent alternative to RmlUi for ordinary 2D chrome. See
[`ui-target-architecture.md`](ui-target-architecture.md) for the full reasoning and the
cross-check against `architecture-principles.md` that established this. **This doc's base-class
guidance below is unchanged** (`mu::ui::window::CObject`, always) — what changed is which widgets a
*new* window reaches for once it has one.

## The three toolkits

| Toolkit | Base class(es) | Real home | Status |
|---|---|---|---|
| Sprite widgets | *(none — `CButton` and `CGaugeBar` both fully closed)* | — | **Fully closed, zero consumers anywhere.** `CWin`/`CWinEx` are fully deleted now (`UI/Widgets/{Win,WinEx}.h/.cpp` removed outright) — they'd already reached zero live subclasses and zero remaining composed members anywhere (`CSysMenuWin`/`CCharMakeWin`/`CServerSelWin` were the last three holders; all provably dead — never rendered, any real state moved to a plain struct read by RmlUi's data binding). The sprite-tier `CButton`'s last consumer was `CCreditWin::m_btnClose` — Stage 1 of that screen's RmlUi port (`credit_win.rml`/`.rcss`: background/deco/logo/close button) retired it, so `::CButton` had zero remaining consumers from that point on. Stage 2 finished the rest of `CCreditWin`: the scrolling credit text (department/team/names, opacity-faded via the model instead of `g_pRenderText` + a black hide-overlay sprite) and the illustration crossfade — the latter's first attempt used two `<img>`s with a C++-swapped `data-attr-src`, but `<img>`/`ElementImage` didn't fill its CSS box correctly in this engine (root cause never pinned down); the working version instead uses two named `@spritesheet`-backed decorators, C++-swapped via `data-style-decorator` the same way `CBuffStrip`'s buff icons already do, plus a model-pushed `opacity` — `CCreditWin` is fully off the sprite toolkit and its `Render()` override is a no-op. That pass also registered a second RmlUi font face (`NanumGothic-Regular.ttf`, `RmlUiRuntime.cpp`) for the credit names, since they aren't guaranteed ASCII the way every other RmlUi string ported so far has been — full CJK/Cyrillic RmlUi text coverage beyond that one face remains a separate, pre-existing, project-wide gap (every other ported window's legacy theme still hardcodes `"Liberation Sans"`), not something this pass tried to fix. `CGaugeBar`'s last consumer was `TitleSceneUI.cpp`'s splash-screen loading bar — ported to RmlUi's own built-in `<progress>` element (`title_scene.rml`/`.rcss`; `Factory.cpp` registers it in Core, no extra setup needed), with `SetValue()`/`SetMax()` called directly from C++ each frame — no data-model binding needed, `<progress>` already owns that state. Its `left`/`top`/`width`/`height` are pushed from C++ as real `px` too, not static `dp`: this scene's 13 background tiles are still native `CSprite`, scaled via the original `fScaleX`/`fScaleY` (800x600-reference, independent per axis, unclamped) math, which RmlUi's shared `dp` unit (640x480-reference, one uniform clamped/damped factor, `UITransform.cpp`'s `ViewportFitScale()`) can't reproduce at arbitrary window sizes — confirmed live, the first `dp`-based attempt only lined up at exactly 640x480. Recomputing the identical `fScaleX`/`fScaleY` math `CGaugeBar::Create()`/`SetPosition()` used and pushing it as `px` keeps the bar pixel-exact with the sprites at any resolution, the same reasoning `login_main.rcss`'s own `#panel` uses for pushing its geometry from C++. `GaugeBar.h/.cpp` are deleted outright, same treatment `CWin`/`CWinEx` got. (`CMsgWin`/`CLoginMainWin`/`CCharSelMainWin`/`CLoginWin` used to also hold sprite `CButton`s as a deliberate redundant-click-detection companion behind their real RmlUi buttons; that companion path has been dropped from all four — RmlUi is now the sole click path, and `CLoginWin`'s checkbox state moved to plain `bool`s.) Nothing left to add a new consumer to. |
| `CUIControl` family | `CUIControl : CUIMessage`, `CUIBaseWindow : CUIControl`, `CUIWindowMgr`, `CUIPhotoViewer` | `UI/Social/SocialWindowCore.h` | **Retired down to its base (2026-10-04).** `CUIButton`, `CUITextListBox<T>` and all ~20 of its subclasses, `CUITextInputBox`, `CUIChatInputBox`, `CRadioButton`, `CUISlideHelp`/`CSlideHelpMgr`, `CUIGuildInfo`/`CUIGuildMaster`/`CUIPopup` are **deleted** -- `SocialWindowCore.h` went 1,488 -> 186 lines. Do not reach for anything in this family when building new UI: text fields are a stock RmlUi `<input>` with the shared `.text-field`, lists are a `data-for` binding in a `.scroll-pane`, buttons are `.btn`. What survives is `CUIControl` and its UI-message plumbing, still the base of `CUIBaseWindow`/`CUIPhotoViewer` -- the friend/mail/chat family's own window manager, which is fully ported to RmlUi documents and keeps that base only for position/state/message bookkeeping. Taking those two off it is what deletes the header; see `tracked-deferrals.md`. |
| `mu::ui::window` tier | `CObject : IObject`, `CManager`, `CButton`/`CRadioButton`/`CRadioGroupButton`/`CCheckBox`/`CComboBox`/`CScrollBar`/`CTextBox`/`CChatInputBox` | `UI/Core/{WindowObject,WindowManager}.h`, `UI/Widgets/Window/*.h` | **`CObject`/`CManager` are the default base class for all new work** — this is the toolkit the other ~88 in-game HUD/inventory/combat/event/NPC/option/quest windows already use. Its own **widget family is transitional**, for native-only content only (see the note above) — for anything with an RmlUi presentation, use RmlUi + `base.rcss` instead. |

## Reference screens — copy these, not an arbitrary neighboring window

`ui-target-architecture.md` Section H, item 15: one named, already-verified example per shape. The
end state is **3 permanent shapes** — RmlUi owns all ordinary 2D UI, a hybrid RmlUi/native-3D split
for content with a live 3D-camera-viewport or world-anchored piece, and world-overlay UI for
per-frame-projected content with no static 2D rect. "Native-only UI" is listed as a fourth row
below because it has a real reference implementation worth copying *today*, not because it's a
fourth permanent destination — it's the stopgap shape for a not-yet-ported legacy subsystem, and
shrinks to zero as that subsystem migrates. Start from whichever matches what you're building
instead of copying the nearest existing window, which may predate current conventions:

| Shape | Reference | Why |
|---|---|---|
| RmlUi-only 2D UI | `CMsgWin` (`UI/Windows/MsgWin.h`) | Ordinary screen-anchored modal, no `CWin` involvement. `RememberPasswordPrompt` for a free-function variant with no reusable state. This is the default destination for every screen; the other two permanent shapes below are the only carve-outs. |
| Hybrid RmlUi/native 3D UI | `CItemHotKey` (`UI/HUD/MainFrameWindow.h/.cpp`) | RmlUi owns the slot chrome; the item icon stays a genuine live 3D render — the permanent boundary, not a porting gap. |
| World-overlay UI | `CCharInfoBalloonMng` (`Character/CharInfoBalloonMng.h`) | Per-frame `WorldToScreen()` projection, no static 2D rect. |
| ~~Native-only UI~~ | — | **No longer a shape.** `CFriendWindow` was this row's only example; its windows are RmlUi documents as of 2026-10-04. Three shapes remain: RmlUi-only 2D, hybrid RmlUi/native-3D, and world-overlay. |

See `ui-target-architecture.md` Section H item 15 for the full reasoning behind each pick.

## Quick decision guide for a new window, dialog, or HUD panel

1. **Base class: `mu::ui::window::CObject`.** Always. Never `CWin`/`CWinEx` — see above, that's a
   closed set with no live subclasses left to imitate.
2. **Register it** with the scene's `mu::ui::window::CManager` (`AddUIObj(INTERFACE_KEY, this)`)
   the same way every other window in this tier does — see any file in `UI/HUD/`, `UI/Inventory/`,
   etc. for the pattern, or `newui-tier-adapter.md`'s "adapter shape" section for the full method
   contract (`Render()`/`Update()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`/`GetLayerDepth()`).
3. **Widgets:** see the cheat sheet below. Default to the `mu::ui::window` widget family; for text
   entry use a stock RmlUi `<input>` with the shared `.text-field` class, not `CUITextInputBox`.
4. **Folder: by feature domain, not by toolkit.** `UI/Combat/`, `UI/Inventory/`, `UI/Events/`,
   `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Social/`, `UI/Quests/`, `UI/Character/`, `UI/Options/`. `UI/Widgets/`
   is for genuinely generic, feature-agnostic controls only (not a catch-all). `UI/Dialogs/` is for
   modal/message-box-style windows. `UI/Windows/` is the closed, already-migrated `CWin`-heritage
   set — don't add new windows there.
5. **Porting/wrapping an existing big legacy subsystem instead of writing one from scratch?** Wrap
   it behind a thin `mu::ui::window::CObject` adapter whose methods forward into the legacy
   implementation, rather than reimplementing it or inventing a second parallel manager.
   `CFriendWindow` (owns and forwards to `CUIWindowMgr`, see below) is the template — it's the
   same shape `newui-tier-adapter.md` documents for porting a window's *rendering* to RmlUi, just
   applied one layer earlier (wrapping the object lifecycle before the render target changes at
   all).

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
    `[Capabilities]`, read via `ThemeProvidesOwnIconChrome()`/`ThemeUsesNativeTextSize()`. Better
    still, check for the content itself (`ThemeProvidesDocument()`) when that can answer.
11. **Expose a purpose-built view model, never a game object.** Currently true of all ~98
    registered structs — don't be the first exception.

**Two documents to compare before choosing a shape.** `mu_helper_config.rml` and
`guard_window.rml` are the same kind of window — same dock, same `docked_panel_frame.rcss`, both
tabbed, both with a native control retained underneath. The first binds *only* its root transform
and places ~65 controls by id in RCSS; the second binds its every label position, size, alignment,
weight and colour, and its RCSS can change almost nothing. Copy the first.

## Widget cheat sheet (native-only content on new `mu::ui::window::CObject` windows)

Applies only to content that must stay native (see the note above) — for anything with an RmlUi
presentation, skip this table and use RmlUi + `base.rcss`'s `.btn`/`.checkbox-box`/`.tooltip`
instead.

| Need | Use | Header | Don't confuse with |
|---|---|---|---|
| Button | `mu::ui::window::CButton` | `UI/Widgets/Window/Button.h` | `::CButton` (sprite toolkit, closed), `CUIButton` (`CUIControl` family) — three unrelated classes, same bare name, disambiguated only by namespace |
| Radio button | `mu::ui::window::CRadioButton` (+ `CRadioGroupButton` to coordinate a set) | `UI/Widgets/Window/Button.h` | `::CRadioButton` (`SocialWindowCore.h`, no base, unrelated) — same situation as `CButton` |
| Checkbox | `mu::ui::window::CCheckBox` | `UI/Widgets/Window/Button.h` | — |
| Dropdown | `mu::ui::window::CComboBox` | `UI/Widgets/Window/ComboBox.h` | Deliberately base-less by design (see its own header comment) — don't force it onto `CObject` |
| Scroll bar | `mu::ui::window::CScrollBar` | `UI/Widgets/Window/ScrollBar.h` | — |
| Multi-line read-only text | `mu::ui::window::CTextBox` | `UI/Widgets/Window/TextBox.h` | — |
| Chat input | `mu::ui::window::CChatInputBox` | `UI/Widgets/Window/ChatInputBox.h` | Internally still uses `CUITextInputBox` for the actual entry field — that's expected, not a bug |
| Single-line text entry | **stock RmlUi `<input>`** + shared `.text-field` | `themes/*/base.rcss`, `themes/*/my_shop.rml` | The convention for new UI — bind with `data-value`, style with `.text-field`, keep `maxlength`/validation in C++. See `component-catalog.md`'s "Text field". `CUITextInputBox` is the fallback for **unmigrated** windows only, not a choice for new ones |
| Progress/gauge bar | *(none yet as a reusable wrapper — `CGaugeBar` is sprite-toolkit-only, closed)* | — | RmlUi's own built-in `<progress>` element (`RmlUi/Core/Elements/ElementProgress.h`, registered by `Factory.cpp` with no extra setup) is a real, proven option now — `title_scene.rml`'s loading bar uses it, with `SetValue()`/`SetMax()` called directly from C++. `main_frame.rcss`/`server_select.rcss`'s own gauges predate that and still use a plain div + `data-style-width`, not retrofitted — check `component-catalog.md`'s "doesn't exist yet" list before inventing a third pattern |
| Scrollable list of rows | *(no native-tier wrapper — don't build one)* | — | `CUITextListBox<T>` (`UI/Social/SocialWindowCore.h`, `CUIControl` family) is the legacy answer and is closed to new consumers (`ui-target-architecture.md` Rule 11) — including from a window already on `mu::ui::window::CObject`, which doesn't exempt it. The real answer is RmlUi's `data-for` binding: `CBuffStrip`'s buff-icon strip and `CMyQuestInfoWindow`'s quest list (ported off `CUICurQuestListBox`/`CUIQuestContentsListBox`) are the two proven references |
| MU Helper configuration windows | `CMuHelperConfigWindow`, `CMuHelperDetailWindow`, `CMuHelperSkillPicker` | `UI/MuHelper/` | Named for what each window is; none can be mistaken for `MUHelper::CMuHelper`, the bot-logic engine they configure |

## Resolved name collisions

Three pairs of classes shared nearly the same name across toolkits, purely by historical accident
— none of them were duplicates of each other or interchangeable. This was resolved with real
namespaces instead of prefix soup: `namespace SEASON3B` (itself a
literal historical-version name) split into `mu::ui::window` for this tier's classes (`mu::` is
this project's own already-established top-level namespace — `mu::platform`, `mu::log` — so this
extends existing convention rather than inventing a new one), leaving the sprite toolkit and
`SocialWindowCore.h` family unnamespaced as before. Two unqualified `CButton`s and two unqualified
`CRadioButton`s in different namespaces need no awkward compound name at all once they're
qualified — the namespace itself disambiguates:

- **`::CButton`** (sprite toolkit, `CSprite`-derived, closed) vs. **`CUIButton`** (`CUIControl`
  family) vs. **`mu::ui::window::CButton`** (the one to use for new work).
- **`::CRadioButton`** (`SocialWindowCore.h`, no base) vs. **`mu::ui::window::CRadioButton`** (the one to
  use for new work).
- **`MUHelper::CMuHelper`** (the actual bot-logic engine) vs. its configuration window, now
  `CMuHelperConfigWindow`: a descriptive name rather than a stripped one, so a bare `CMuHelper`
  only ever means the engine.

**A subtlety worth knowing if you're writing code in this tier**: a file that does `using namespace
mu::ui::window;` at file/global scope (common — most files in this tier do, since their own class
definitions rely on it for everything else unqualified) can still hit an "ambiguous symbol" error
if it also transitively includes `SocialWindowCore.h`, because that makes both `::CRadioButton` and
`mu::ui::window::CRadioButton` visible unqualified in the same translation unit. Fix by explicit
qualification at the actual use site (`::CRadioButton` if you mean the `SocialWindowCore.h` one,
`mu::ui::window::CRadioButton::` on a method *definition* if you mean this tier's one) — this bit
`SocialWindowCore.h`'s own internal members and `Window/Button.cpp`'s own method definitions during Phase
5 itself, both fixed at the source rather than by removing the `using namespace`.

## Confirmed dead — don't resurrect these as a pattern

- **`CSlider`** (`UI/Widgets/Slider.h`, composed a `CButton` + `CGaugeBar`) — deleted 2026-09-05,
  confirmed zero consumers anywhere in the tree. If a slider control is genuinely needed again,
  design it for the `mu::ui::window` tier fresh rather than reviving this.
- **`UIDefaultBase`** — deleted during the `UI/` directory restructure, fully inert (`#ifdef`-gated
  on a macro that was never defined).

## The `SocialWindowManager.cpp` / `CFriendWindow` seam

`UI/Social/SocialWindow*.h/.cpp` holds `CUIBaseWindow`, `CUIWindowMgr` and `CUIPhotoViewer`, reached
through exactly one seam: `CFriendWindow : public mu::ui::window::CObject` owns one `CUIWindowMgr*`
and forwards into it. **The windows themselves are RmlUi now** (2026-10-04) -- the shell, each chat
room and each letter own a document and a data model, and `SocialWindowManager.cpp` keeps the manager that
arranges them, the UI-message queue they talk over, and the native `CUIPhotoViewer` that draws a
letter's sender.

So this is no longer a "native-only stopgap" to wrap rather than reimplement. Two things are worth
knowing if you touch it:

- `CUIBaseWindow`/`CUIPhotoViewer` still derive from `CUIControl`, which is the only reason
  `SocialWindowCore.h` still exists. Taking them off that base is what finishes the toolkit retirement
  (`tracked-deferrals.md`).
- The file is a grab-bag whose name says nothing about what it holds, and until 2026-10-04 it was
  `#include`d by ~30 files across maps, events, pets and scenes that used nothing from it. Those are
  cleaned up; don't add a new one without needing a symbol it declares.

## Cross-references

- [`ui-target-architecture.md`](ui-target-architecture.md) — the canonical/transitional
  framing this doc's widget guidance follows, the full RmlUi-vs-native boundary reasoning, and the
  broader UI-kit migration plan this is one item of.
- [`newui-tier-adapter.md`](newui-tier-adapter.md) — how to port a window's *rendering* to RmlUi
  once it exists (a separate, later step from choosing its base class here).
- [`architecture-principles.md`](architecture-principles.md) — the overarching design philosophy.
- [`component-catalog.md`](component-catalog.md) — the RmlUi/RCSS-layer component catalog, the
  parallel axis to this doc's C++ object layer.
- [`engine-findings.md`](engine-findings.md) — why the Ownership section's rules are hard rules:
  a `data-style-*` binding and a `style=` attribute are both inline properties, and inline beats
  every stylesheet rule in this build with no `!important` escape.
- [`tracked-deferrals.md`](tracked-deferrals.md) — the ownership-boundary entry: which shipped
  windows already violate those rules, and in what order they are worth fixing.
