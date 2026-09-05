# Building New UI: Windows, Dialogs, HUD Panels, and Widgets

A decision guide for "which base class / widget class / folder do I use when adding something
new under `UI/`?" The codebase has three widget toolkits that accumulated over three different
eras and, on the surface, look interchangeable — several classes even share almost the same name
across toolkits. They aren't interchangeable, and this doc exists so a new window doesn't
accidentally reach for a closed, historical one. Read `architecture-principles.md` first for the
overall philosophy this follows; this doc is the concrete "what do I actually type" answer for the
C++ object layer specifically (`component-catalog.md` covers the parallel RmlUi/RCSS layer).

## The three toolkits

| Toolkit | Base class(es) | Real home | Status |
|---|---|---|---|
| Sprite widgets | `CButton : CSprite`, `CGaugeBar`, `CWin`, `CWinEx : CWin` | `UI/Widgets/{Button,GaugeBar,Win,WinEx}.h` | **Closed.** `CWin`/`CWinEx` have zero live subclasses anywhere in the tree — every window that still uses these is `mu::ui::window::CObject`-derived today and only kept these as composed *members* from before its Phase 1-3 migration off `CWin`. Don't add a new consumer. |
| `CUIControl` family | `CUIControl : CUIMessage`, `CUIButton`, `CUITextListBox<T>`, `CUITextInputBox`, `CUIChatInputBox`, `CUIBaseWindow : CUIControl`, `CUIWindowMgr`, `CRadioButton`, `CUISlideHelp`/`CSlideHelpMgr` | `UI/Widgets/UIControls.h`, `UI/Party/UIWindows.h` | **Fully live**, but effectively closed to new *window-manager* consumers — `CUIBaseWindow`/`CUIWindowMgr`'s only live subsystem is the friend/mail/chat-room feature in `UIWindows.cpp` (see below). `CUITextInputBox` is the one class here still legitimately reused by brand-new `mu::ui::window::CObject` windows (`NewUIGuildMakeWindow`, `NewUIMyShopInventory`, etc.) — there's no equivalent yet in that tier, so this is a sanctioned exception, not technical debt to avoid. |
| `mu::ui::window` tier | `CObject : IObject`, `CManager`, `CButton`/`CRadioButton`/`CRadioGroupButton`/`CCheckBox`/`CComboBox`/`CScrollBar`/`CTextBox`/`CChatInputBox` | `UI/Core/{NewUIBase,NewUIManager}.h`, `UI/Widgets/NewUI*.h` | **The default for all new work.** This is the toolkit the other ~88 in-game HUD/inventory/combat/event/NPC/option/quest windows already use. |

## Quick decision guide for a new window, dialog, or HUD panel

1. **Base class: `mu::ui::window::CObject`.** Always. Never `CWin`/`CWinEx` — see above, that's a
   closed set with no live subclasses left to imitate.
2. **Register it** with the scene's `mu::ui::window::CManager` (`AddUIObj(INTERFACE_KEY, this)`)
   the same way every other window in this tier does — see any file in `UI/HUD/`, `UI/Inventory/`,
   etc. for the pattern, or `newui-tier-adapter.md`'s "adapter shape" section for the full method
   contract (`Render()`/`Update()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`/`GetLayerDepth()`).
3. **Widgets:** see the cheat sheet below. Default to the `mu::ui::window` widget family; drop to
   `CUITextInputBox` only for single-line text entry, since nothing in this tier's own widget set
   fills that need yet.
4. **Folder: by feature domain, not by toolkit.** `UI/Combat/`, `UI/Inventory/`, `UI/Events/`,
   `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Quests/`, `UI/Character/`, `UI/Options/`. `UI/Widgets/`
   is for genuinely generic, feature-agnostic controls only (not a catch-all). `UI/Dialogs/` is for
   modal/message-box-style windows. `UI/Windows/` is the closed, already-migrated `CWin`-heritage
   set from `docs/newui-legacy-merger.md` Phases 1-3 — don't add new windows there.
5. **Porting/wrapping an existing big legacy subsystem instead of writing one from scratch?** Wrap
   it behind a thin `mu::ui::window::CObject` adapter whose methods forward into the legacy
   implementation, rather than reimplementing it or inventing a second parallel manager.
   `CNewUIFriendWindow` (owns and forwards to `CUIWindowMgr`, see below) is the template — it's the
   same shape `newui-tier-adapter.md` documents for porting a window's *rendering* to RmlUi, just
   applied one layer earlier (wrapping the object lifecycle before the render target changes at
   all).

## Widget cheat sheet (new `mu::ui::window::CObject` windows)

| Need | Use | Header | Don't confuse with |
|---|---|---|---|
| Button | `mu::ui::window::CButton` | `UI/Widgets/NewUIButton.h` | `::CButton` (sprite toolkit, closed), `CUIButton` (`CUIControl` family) — three unrelated classes, same bare name, disambiguated only by namespace |
| Radio button | `mu::ui::window::CRadioButton` (+ `CRadioGroupButton` to coordinate a set) | `UI/Widgets/NewUIButton.h` | `::CRadioButton` (`UIControls.h`, no base, unrelated) — same situation as `CButton` |
| Checkbox | `mu::ui::window::CCheckBox` | `UI/Widgets/NewUIButton.h` | — |
| Dropdown | `mu::ui::window::CComboBox` | `UI/Widgets/NewUIComboBox.h` | Deliberately base-less by design (see its own header comment) — don't force it onto `CObject` |
| Scroll bar | `mu::ui::window::CScrollBar` | `UI/Widgets/NewUIScrollBar.h` | — |
| Multi-line read-only text | `mu::ui::window::CTextBox` | `UI/Widgets/NewUITextBox.h` | — |
| Chat input | `mu::ui::window::CChatInputBox` | `UI/Widgets/NewUIChatInputBox.h` | Internally still uses `CUITextInputBox` for the actual entry field — that's expected, not a bug |
| Single-line text entry | `CUITextInputBox` | `UI/Widgets/UIControls.h` | Sanctioned exception — no equivalent exists yet in the `mu::ui::window` tier |
| Progress/gauge bar | *(none yet — `CGaugeBar` is sprite-toolkit-only, closed)* | — | Check `component-catalog.md`'s "doesn't exist yet" list before inventing one |
| MU Helper bot-engine window | `mu::ui::window::CUIMuHelper` | `UI/Core/NewUIMuHelper.h` | Deliberately kept its `UI` — the plain-stripped `CMuHelper` would collide with `MUHelper::CMuHelper`, the actual bot-logic engine this window displays/controls (a real, unrelated class, not a duplicate) |

## Resolved name collisions (Phase 5, 2026-09-05)

Three pairs of classes shared nearly the same name across toolkits, purely by historical accident
— none of them were duplicates of each other or interchangeable. `docs/newui-legacy-merger.md`'s
Phase 5 resolved this with real namespaces instead of prefix soup: `namespace SEASON3B` (itself a
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
`UIControls.h`'s own internal members and `NewUIButton.cpp`'s own method definitions during Phase
5 itself, both fixed at the source rather than by removing the `using namespace`.

## Confirmed dead — don't resurrect these as a pattern

- **`CSlider`** (`UI/Widgets/Slider.h`, composed a `CButton` + `CGaugeBar`) — deleted 2026-09-05,
  confirmed zero consumers anywhere in the tree. If a slider control is genuinely needed again,
  design it for the `mu::ui::window` tier fresh rather than reviving this.
- **`UIDefaultBase`** — deleted during the `UI/` directory restructure (`docs/newui-legacy-merger.md`),
  fully inert (`#ifdef`-gated on a macro that was never defined).

## The `UIWindows.cpp` / `CNewUIFriendWindow` pattern — a legitimate exception, not confusion

`UI/Party/UIWindows.h/.cpp` (`CUIBaseWindow`, `CUIWindowMgr`, `CUIFriendWindow`, mail, chat-room
list) is a fully live, self-contained legacy subsystem — not dead code, not superseded. It's
reached through exactly one seam: `CNewUIFriendWindow : public mu::ui::window::CObject` owns one
`CUIWindowMgr*` and every one of its public methods is a one-line forward into it. Mail
(`CUILetterReadWindow`/`WriteWindow`) and the chat-room list (`CUIChatRoomListTabWindow`) have no
`mu::ui::window`-native reimplementation anywhere — this file is their only implementation. If you
ever need to touch the friend/mail/chat-room feature, this is the file; don't build a second one.

## Cross-references

- [`newui-legacy-merger.md`](../newui-legacy-merger.md) — the `CUIMng`/`CWin` retirement history,
  the `UI/` directory restructure, and Phase 5's namespace/rename work.
- [`newui-tier-adapter.md`](newui-tier-adapter.md) — how to port a window's *rendering* to RmlUi
  once it exists (a separate, later step from choosing its base class here).
- [`architecture-principles.md`](architecture-principles.md) — the overarching design philosophy.
- [`component-catalog.md`](component-catalog.md) — the RmlUi/RCSS-layer component catalog, the
  parallel axis to this doc's C++ object layer.
