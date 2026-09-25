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
| `CUIControl` family | `CUIControl : CUIMessage`, `CUIButton`, `CUITextListBox<T>`, `CUITextInputBox`, `CUIChatInputBox`, `CUIBaseWindow : CUIControl`, `CUIWindowMgr`, `CRadioButton`, `CUISlideHelp`/`CSlideHelpMgr` | `UI/Widgets/UIControls.h`, `UI/Party/UIWindows.h` | **Fully live; one part transitional, one part a real-but-narrower permanent exception.** `CUIBaseWindow`/`CUIWindowMgr`'s only live subsystem — the friend/mail/chat-room feature in `UIWindows.cpp` (see "The `UIWindows.cpp`/`CFriendWindow` pattern" below) — is **not** a peer to the native-3D/world-overlay boundary: [`ui-target-architecture.md`](ui-target-architecture.md) Section H item 15 is explicit that this shape is transitional and shrinking, same Rule 3 standing as the `mu::ui::window` widget family itself. It has no scheduled port today (a real, self-contained legacy subsystem, not dead code, and not worth a speculative rewrite ahead of a concrete reason), but the end state is 3 shapes — RmlUi-only 2D, hybrid RmlUi/native-3D, and world-overlay — not 4, so a future port of this subsystem to RmlUi is in scope, just unscheduled. `CUITextInputBox` is Section E's "Type-2 companion object" — a real interactive widget RmlUi can't yet host (caret/selection, IME composition with candidate-window positioning, multiline wrap, password masking, tab-navigation, focus routing) — so it's the one class here still legitimately reused by brand-new `mu::ui::window::CObject` windows (`NewUIGuildMakeWindow`, `NewUIMyShopInventory`, etc.). Its eventual replacement is RmlUi's own native `<input>`/`<textarea>` form controls, not a new native widget-tier class — building a `mu::ui::window::CTextInput` would be a third kind of native companion, which Section E explicitly warns against. That RmlUi-native-input work is its own separate design effort (IME composition through RmlUi's DOM model is an open question), not a toolkit-retirement task. `CUIButton`/`CUITextListBox<T>` are still real and live too — confirmed a much bigger footprint than this row used to suggest: `CUIButton` (`UIGuildInfo`/`UIGuildMaster`/`UIPopup`/`UIWindows`) and ~20 `CUITextListBox<T>` subclasses spanning guild/quest/inventory-mix/in-game-shop/letter systems, not just the friend/mail/chat subsystem. First concrete shrink: `CMyQuestInfoWindow` (`UI/Quests/MyQuestInfoWindow.h`/`.cpp`) no longer consumes `CUICurQuestListBox`/`CUIQuestContentsListBox`/`CUIButton`-tier `CButton`s — its quest-list, tab-switching, and contents/reward panel are RmlUi data-bound now (`my_quest_info.rml`/`.rcss`), following `CBuffStrip`'s proven `data-for` list pattern and `server_select.rml`'s proven click-a-row-to-select-it pattern. Those two list-box classes still have two other live consumers (`QuestProgress.h`, `QuestProgressByEtc.h`) — natural next targets, same pattern. One piece of `CMyQuestInfoWindow` stayed native on purpose: a selected reward-item row's item-info popup (`::RenderItemInfo()`, a per-frame native draw) is a permanent hybrid-shape boundary for that one feature, not a porting gap. **Correction, 2026-09-13**: the un-migrated `CUITextListBox<T>` consumers aren't confined to legacy pre-`CObject` code — `CGuildInfoWindow` (`CUINewGuildMemberListBox`), `CMixInventory` (`CUISocketListBox`/`CUIUnmixgemList`), and `CInGameShop` (`CUIInGameShopListBox`/`CUIBuyingListBox`/`CUIPackCheckBuyingListBox`) are already on `mu::ui::window::CObject` and still reach into this family for list content — see `ui-target-architecture.md`'s new Rule 11/item 8b, and `tracked-deferrals.md`'s entry for the full remaining-consumer list. `CUIGuildInfo`/`CUIGuildMaster` (`Guild/UIGuildInfo.h`/`UIGuildMaster.h` — the *other*, `CUIControl`-rooted guild implementation, distinct from the live `CGuildInfoWindow`/`CGuildMakeWindow` pair) look like dead code superseded by those two: a full-codebase grep found zero instantiations of either class anywhere (no `new`, no member declaration). Not yet given the same zero-consumer verification `CWin`/`::CButton`/`CSlider` got before deletion — treat as suspected, not confirmed, until that pass runs. `CUIButton`'s other live cluster, `CUIPopup` (`g_pUIPopup`, called from `WSclient.cpp`'s generic server-error popups), duplicates `CCommonMessageBox`'s job — two live "generic confirm dialog" systems, not yet reconciled; moving those call sites to `CCommonMessageBox` would retire `CUIPopup` and, with it, `CUIButton`'s last confirmed-live consumer. |
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
| Native-only UI (stopgap, not a 4th permanent shape) | `CFriendWindow` (`UI/Party/FriendWindow.h`) | Thin `CObject` adapter forwarding into a live legacy subsystem — see "The `UIWindows.cpp`/`CFriendWindow` pattern" below. Transitional shape (Rule 3): copy this only when wrapping an existing native subsystem that isn't being ported to RmlUi in the same pass, not as a template for new 2D UI. |

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
   `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Quests/`, `UI/Character/`, `UI/Options/`. `UI/Widgets/`
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

## Widget cheat sheet (native-only content on new `mu::ui::window::CObject` windows)

Applies only to content that must stay native (see the note above) — for anything with an RmlUi
presentation, skip this table and use RmlUi + `base.rcss`'s `.btn`/`.checkbox-box`/`.tooltip`
instead.

| Need | Use | Header | Don't confuse with |
|---|---|---|---|
| Button | `mu::ui::window::CButton` | `UI/Widgets/Window/Button.h` | `::CButton` (sprite toolkit, closed), `CUIButton` (`CUIControl` family) — three unrelated classes, same bare name, disambiguated only by namespace |
| Radio button | `mu::ui::window::CRadioButton` (+ `CRadioGroupButton` to coordinate a set) | `UI/Widgets/Window/Button.h` | `::CRadioButton` (`UIControls.h`, no base, unrelated) — same situation as `CButton` |
| Checkbox | `mu::ui::window::CCheckBox` | `UI/Widgets/Window/Button.h` | — |
| Dropdown | `mu::ui::window::CComboBox` | `UI/Widgets/Window/ComboBox.h` | Deliberately base-less by design (see its own header comment) — don't force it onto `CObject` |
| Scroll bar | `mu::ui::window::CScrollBar` | `UI/Widgets/Window/ScrollBar.h` | — |
| Multi-line read-only text | `mu::ui::window::CTextBox` | `UI/Widgets/Window/TextBox.h` | — |
| Chat input | `mu::ui::window::CChatInputBox` | `UI/Widgets/Window/ChatInputBox.h` | Internally still uses `CUITextInputBox` for the actual entry field — that's expected, not a bug |
| Single-line text entry | **stock RmlUi `<input>`** + shared `.text-field` | `themes/*/base.rcss`, `themes/*/my_shop.rml` | The convention for new UI — bind with `data-value`, style with `.text-field`, keep `maxlength`/validation in C++. See `component-catalog.md`'s "Text field". `CUITextInputBox` is the fallback for **unmigrated** windows only, not a choice for new ones |
| Progress/gauge bar | *(none yet as a reusable wrapper — `CGaugeBar` is sprite-toolkit-only, closed)* | — | RmlUi's own built-in `<progress>` element (`RmlUi/Core/Elements/ElementProgress.h`, registered by `Factory.cpp` with no extra setup) is a real, proven option now — `title_scene.rml`'s loading bar uses it, with `SetValue()`/`SetMax()` called directly from C++. `main_frame.rcss`/`server_select.rcss`'s own gauges predate that and still use a plain div + `data-style-width`, not retrofitted — check `component-catalog.md`'s "doesn't exist yet" list before inventing a third pattern |
| Scrollable list of rows | *(no native-tier wrapper — don't build one)* | — | `CUITextListBox<T>` (`UI/Widgets/UIControls.h`, `CUIControl` family) is the legacy answer and is closed to new consumers (`ui-target-architecture.md` Rule 11) — including from a window already on `mu::ui::window::CObject`, which doesn't exempt it. The real answer is RmlUi's `data-for` binding: `CBuffStrip`'s buff-icon strip and `CMyQuestInfoWindow`'s quest list (ported off `CUICurQuestListBox`/`CUIQuestContentsListBox`) are the two proven references |
| MU Helper bot-engine window | `mu::ui::window::CUIMuHelper` | `UI/Core/WindowMuHelper.h` | Deliberately kept its `UI` — the plain-stripped `CMuHelper` would collide with `MUHelper::CMuHelper`, the actual bot-logic engine this window displays/controls (a real, unrelated class, not a duplicate) |

## Resolved name collisions

Three pairs of classes shared nearly the same name across toolkits, purely by historical accident
— none of them were duplicates of each other or interchangeable. This was resolved with real
namespaces instead of prefix soup: `namespace SEASON3B` (itself a
literal historical-version name) split into `mu::ui::window` for this tier's classes (`mu::` is
this project's own already-established top-level namespace — `mu::platform`, `mu::log` — so this
extends existing convention rather than inventing a new one), leaving the sprite toolkit and
`UIControls.h` family unnamespaced as before. Two unqualified `CButton`s and two unqualified
`CRadioButton`s in different namespaces need no awkward compound name at all once they're
qualified — the namespace itself disambiguates:

- **`::CButton`** (sprite toolkit, `CSprite`-derived, closed) vs. **`CUIButton`** (`CUIControl`
  family) vs. **`mu::ui::window::CButton`** (the one to use for new work).
- **`::CRadioButton`** (`UIControls.h`, no base) vs. **`mu::ui::window::CRadioButton`** (the one to
  use for new work).
- **`MUHelper::CMuHelper`** (the actual bot-logic engine) vs. **`mu::ui::window::CUIMuHelper`**
  (the window that displays/controls it) — this one kept a disambiguating name (`CUIMuHelper`, not
  the plain-stripped `CMuHelper`) rather than relying on the namespace alone, since both classes
  are legitimately reachable from similar contexts and a bare `CMuHelper` reference could plausibly
  mean either.

**A subtlety worth knowing if you're writing code in this tier**: a file that does `using namespace
mu::ui::window;` at file/global scope (common — most files in this tier do, since their own class
definitions rely on it for everything else unqualified) can still hit an "ambiguous symbol" error
if it also transitively includes `UIControls.h`, because that makes both `::CRadioButton` and
`mu::ui::window::CRadioButton` visible unqualified in the same translation unit. Fix by explicit
qualification at the actual use site (`::CRadioButton` if you mean the `UIControls.h` one,
`mu::ui::window::CRadioButton::` on a method *definition* if you mean this tier's one) — this bit
`UIControls.h`'s own internal members and `Window/Button.cpp`'s own method definitions during Phase
5 itself, both fixed at the source rather than by removing the `using namespace`.

## Confirmed dead — don't resurrect these as a pattern

- **`CSlider`** (`UI/Widgets/Slider.h`, composed a `CButton` + `CGaugeBar`) — deleted 2026-09-05,
  confirmed zero consumers anywhere in the tree. If a slider control is genuinely needed again,
  design it for the `mu::ui::window` tier fresh rather than reviving this.
- **`UIDefaultBase`** — deleted during the `UI/` directory restructure, fully inert (`#ifdef`-gated
  on a macro that was never defined).

## The `UIWindows.cpp` / `CFriendWindow` pattern — a legitimate stopgap, not confusion or a dead end

`UI/Party/UIWindows.h/.cpp` (`CUIBaseWindow`, `CUIWindowMgr`, `CUIFriendWindow`, mail, chat-room
list) is a fully live, self-contained legacy subsystem — not dead code, not superseded. It's
reached through exactly one seam: `CFriendWindow : public mu::ui::window::CObject` owns one
`CUIWindowMgr*` and every one of its public methods is a one-line forward into it. Mail
(`CUILetterReadWindow`/`WriteWindow`) and the chat-room list (`CUIChatRoomListTabWindow`) have no
`mu::ui::window`-native reimplementation anywhere — this file is their only implementation. If you
ever need to touch the friend/mail/chat-room feature, this is the file; don't build a second one.

This is not a fourth permanent architectural shape alongside RmlUi-only/hybrid-3D/world-overlay —
see the Reference screens section above. It has no scheduled port (a real subsystem with real
users, not worth rewriting speculatively), but a future RmlUi port is in scope, not excluded the
way the native-3D/world-overlay boundary is. Wrap, don't reimplement, until that port happens.

## Cross-references

- [`ui-target-architecture.md`](ui-target-architecture.md) — the canonical/transitional
  framing this doc's widget guidance follows, the full RmlUi-vs-native boundary reasoning, and the
  broader UI-kit migration plan this is one item of.
- [`newui-tier-adapter.md`](newui-tier-adapter.md) — how to port a window's *rendering* to RmlUi
  once it exists (a separate, later step from choosing its base class here).
- [`architecture-principles.md`](architecture-principles.md) — the overarching design philosophy.
- [`component-catalog.md`](component-catalog.md) — the RmlUi/RCSS-layer component catalog, the
  parallel axis to this doc's C++ object layer.
