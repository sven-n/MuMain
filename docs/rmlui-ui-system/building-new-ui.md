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
| Sprite widgets | `CButton : CSprite`, `CGaugeBar`, `CWin`, `CWinEx : CWin` | `UI/Widgets/{Button,GaugeBar,Win,WinEx}.h` | **Closed.** `CWin`/`CWinEx` have zero live subclasses anywhere in the tree — every window that still uses these is `CNewUIObj`-derived today and only kept these as composed *members* from before its Phase 1-3 migration off `CWin`. Don't add a new consumer. |
| `CUIControl` family | `CUIControl : CUIMessage`, `CUIButton`, `CUITextListBox<T>`, `CUITextInputBox`, `CUIChatInputBox`, `CUIBaseWindow : CUIControl`, `CUIWindowMgr`, `CRadioButton`, `CUISlideHelp`/`CSlideHelpMgr` | `UI/Widgets/UIControls.h`, `UI/Party/UIWindows.h` | **Fully live**, but effectively closed to new *window-manager* consumers — `CUIBaseWindow`/`CUIWindowMgr`'s only live subsystem is the friend/mail/chat-room feature in `UIWindows.cpp` (see below). `CUITextInputBox` is the one class here still legitimately reused by brand-new `CNewUIObj` windows (`NewUIGuildMakeWindow`, `NewUIMyShopInventory`, etc.) — there's no `CNewUIObj`-tier equivalent yet, so this is a sanctioned exception, not technical debt to avoid. |
| `SEASON3B::CNewUIObj` tier | `CNewUIObj : INewUIBase`, `CNewUIManager`, `CNewUIButton`/`CNewUIRadioButton`/`CNewUIRadioGroupButton`/`CNewUICheckBox`/`CNewUIComboBox`/`CNewUIScrollBar`/`CNewUITextBox`/`CNewUIChatInputBox` | `UI/Core/{NewUIBase,NewUIManager}.h`, `UI/Widgets/NewUI*.h` | **The default for all new work.** This is the toolkit the other ~88 in-game HUD/inventory/combat/event/NPC/option/quest windows already use. |

## Quick decision guide for a new window, dialog, or HUD panel

1. **Base class: `SEASON3B::CNewUIObj`.** Always. Never `CWin`/`CWinEx` — see above, that's a
   closed set with no live subclasses left to imitate.
2. **Register it** with the scene's `CNewUIManager` (`AddUIObj(INTERFACE_KEY, this)`) the same way
   every other `CNewUIObj` window does — see any file in `UI/HUD/`, `UI/Inventory/`, etc. for the
   pattern, or `newui-tier-adapter.md`'s "adapter shape" section for the full method contract
   (`Render()`/`Update()`/`UpdateMouseEvent()`/`UpdateKeyEvent()`/`GetLayerDepth()`).
3. **Widgets:** see the cheat sheet below. Default to the `CNewUI*` family; drop to
   `CUITextInputBox` only for single-line text entry, since nothing in the `CNewUIObj` tier's own
   widget set fills that need yet.
4. **Folder: by feature domain, not by toolkit.** `UI/Combat/`, `UI/Inventory/`, `UI/Events/`,
   `UI/HUD/`, `UI/NPCs/`, `UI/Party/`, `UI/Quests/`, `UI/Character/`, `UI/Options/`. `UI/Widgets/`
   is for genuinely generic, feature-agnostic controls only (not a catch-all). `UI/Dialogs/` is for
   modal/message-box-style windows. `UI/Windows/` is the closed, already-migrated `CWin`-heritage
   set from `docs/newui-legacy-merger.md` Phases 1-3 — don't add new windows there.
5. **Porting/wrapping an existing big legacy subsystem instead of writing one from scratch?** Wrap
   it behind a thin `CNewUIObj` adapter whose methods forward into the legacy implementation,
   rather than reimplementing it or inventing a second parallel manager. `CNewUIFriendWindow`
   (owns and forwards to `CUIWindowMgr`, see below) is the template — it's the same shape
   `newui-tier-adapter.md` documents for porting a window's *rendering* to RmlUi, just applied one
   layer earlier (wrapping the object lifecycle before the render target changes at all).

## Widget cheat sheet (new `CNewUIObj` windows)

| Need | Use | Header | Don't confuse with |
|---|---|---|---|
| Button | `SEASON3B::CNewUIButton` | `UI/Widgets/NewUIButton.h` | `CButton` (sprite toolkit, closed), `CUIButton` (`CUIControl` family) — three unrelated classes, similar names |
| Radio button | `SEASON3B::CNewUIRadioButton` (+ `CNewUIRadioGroupButton` to coordinate a set) | `UI/Widgets/NewUIButton.h` | `CRadioButton` (`UIControls.h`, no base, unrelated) |
| Checkbox | `SEASON3B::CNewUICheckBox` | `UI/Widgets/NewUIButton.h` | — |
| Dropdown | `SEASON3B::CNewUIComboBox` | `UI/Widgets/NewUIComboBox.h` | Deliberately base-less by design (see its own header comment) — don't force it onto `CNewUIObj` |
| Scroll bar | `SEASON3B::CNewUIScrollBar` | `UI/Widgets/NewUIScrollBar.h` | — |
| Multi-line read-only text | `SEASON3B::CNewUITextBox` | `UI/Widgets/NewUITextBox.h` | — |
| Chat input | `SEASON3B::CNewUIChatInputBox` | `UI/Widgets/NewUIChatInputBox.h` | Internally still uses `CUITextInputBox` for the actual entry field — that's expected, not a bug |
| Single-line text entry | `CUITextInputBox` | `UI/Widgets/UIControls.h` | Sanctioned exception — no `CNewUIObj`-tier equivalent exists yet |
| Progress/gauge bar | *(none yet — `CGaugeBar` is sprite-toolkit-only, closed)* | — | Check `component-catalog.md`'s "doesn't exist yet" list before inventing one |

## Known name collisions (until Phase 5 resolves them)

Two pairs of classes share nearly the same name across toolkits today, purely by historical
accident — they are not duplicates of each other and not interchangeable:

- **`CButton`** (sprite toolkit, `CSprite`-derived, closed) vs. **`CUIButton`** (`CUIControl`
  family) vs. **`CNewUIButton`** (`SEASON3B`, the one to use for new work).
- **`CRadioButton`** (`UIControls.h`, no base) vs. **`CNewUIRadioButton`** (`SEASON3B`, the one to
  use for new work).

`docs/newui-legacy-merger.md`'s Phase 5 records the planned fix: real namespaces
(`UI::Sprite`, `UI::Controls`, `UI::Window` — replacing the literal historical-version name
`SEASON3B`) so these resolve by C++ scoping instead of by prefix. Not done yet — this section is
the interim guardrail until it is.

## Confirmed dead — don't resurrect these as a pattern

- **`CSlider`** (`UI/Widgets/Slider.h`, composed a `CButton` + `CGaugeBar`) — deleted 2026-09-05,
  confirmed zero consumers anywhere in the tree. If a slider control is genuinely needed again,
  design it for the `CNewUIObj` tier fresh rather than reviving this.
- **`UIDefaultBase`** — deleted during the `UI/` directory restructure (`docs/newui-legacy-merger.md`),
  fully inert (`#ifdef`-gated on a macro that was never defined).

## The `UIWindows.cpp` / `CNewUIFriendWindow` pattern — a legitimate exception, not confusion

`UI/Party/UIWindows.h/.cpp` (`CUIBaseWindow`, `CUIWindowMgr`, `CUIFriendWindow`, mail, chat-room
list) is a fully live, self-contained legacy subsystem — not dead code, not superseded. It's
reached through exactly one seam: `CNewUIFriendWindow : public CNewUIObj` owns one
`CUIWindowMgr*` and every one of its public methods is a one-line forward into it. Mail
(`CUILetterReadWindow`/`WriteWindow`) and the chat-room list (`CUIChatRoomListTabWindow`) have no
`CNewUIObj`-native reimplementation anywhere — this file is their only implementation. If you ever
need to touch the friend/mail/chat-room feature, this is the file; don't build a second one.

## Cross-references

- [`newui-legacy-merger.md`](../newui-legacy-merger.md) — the `CUIMng`/`CWin` retirement history,
  the `UI/` directory restructure, and Phase 5's planned namespace scheme.
- [`newui-tier-adapter.md`](newui-tier-adapter.md) — how to port a `CNewUIObj` window's
  *rendering* to RmlUi once it exists (a separate, later step from choosing its base class here).
- [`architecture-principles.md`](architecture-principles.md) — the overarching design philosophy.
- [`component-catalog.md`](component-catalog.md) — the RmlUi/RCSS-layer component catalog, the
  parallel axis to this doc's C++ object layer.
