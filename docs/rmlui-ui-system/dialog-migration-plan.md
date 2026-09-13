# `CommonMessageBox`/`CustomMessageBox` → `CGenericConfirmDialog` migration plan

Tracked, resumable checklist for porting the ~137 remaining native confirm-dialog classes onto the
`CGenericConfirmDialog` primitive (see `component-catalog.md`'s "Dialog" section for what the
primitive supports, and `STATUS.md`'s "What's migrated" for how/why it was built). This file is the
per-class worklist; `STATUS.md`'s "Tracked deferral" entry stays the short pointer to it.

Primitive shape, as a reminder for what "FITS" means below: a fixed list of plain text lines (each
optionally bold), and either an OK-only or OK+Cancel button set, with callbacks on click. No
keypad, no progress bar, no 3D item preview, no 3+-button menus.

Full inventory pass done 2026-09-13 (`CommonMessageBox.h`/`.cpp` read in full; `CustomMessageBox.h`/
`.cpp` characterized by sampling — see that section's own confidence note). Re-verify a class's
`SetLayout()`/`Create()` at port time rather than trusting this list blindly if its shape looks
even slightly off from the table — this is a starting point, not a guarantee.

## Already ported (proof pass, done)

- [x] `CGuildOutPerson` (`Guild/GuildInfoWindow.cpp`) — OK-only.
- [x] `CQuestGiveUpMsgBoxLayout` (`UI/Quests/MyQuestInfoWindow.cpp`'s `RmlClickGiveUp()`) — OK/Cancel.
- [x] `CGuildRequestMsgBoxLayout` (`Network/Server/WSclient.cpp`'s `ReceiveGuild()`) — OK/Cancel.

## `CommonMessageBox.h` — FITS (plain OK/OkCancel + `AddMsg` text, ~62 classes)

Batch these first — same shape as the 3 already proven, pure config-data ports. Grouped roughly by
call-site area so a batch can be done (and built/verified) per group rather than all 62 at once.

Zero-call-site classes are flagged `DEAD?` — confirm still-dead at port time (grep again) and
prefer deleting outright over porting if so; don't spend primitive-config effort on unreachable code.

### Guild — done (2026-09-13)

- [x] `CGuildBreakMsgBoxLayout` — OkCancel — `Guild/GuildInfoWindow.cpp` (guild-disband confirm)
- [x] `CGuildPerson_Get_Out` — OkCancel — `Guild/GuildInfoWindow.cpp` (kick-member confirm)
- [x] `CGuildPerson_Cancel_Position_MsgBoxLayout` — OkCancel (no `AddMsg`) — `Guild/GuildInfoWindow.cpp` (cancel-rank confirm)
- [x] `CUnionGuild_Break_MsgBoxLayout` — OkCancel — `Guild/GuildInfoWindow.cpp` (alliance-remove confirm)
- [x] `CUnionGuild_Out_MsgBoxLayout` — OK — `Guild/GuildInfoWindow.cpp` (alliance-master-can't-withdraw notice)
- [x] `CGuildRelationShipMsgBoxLayout` — OkCancel (no `AddMsg`) — `Guild/GuildInfoWindow.cpp`'s
  `ReceiveGuildRelationShip()` (alliance/hostility request approve-decline). Preserved the original
  "already busy → auto-decline instead of showing" branch (was `!g_MessageBox->IsEmpty()`, now
  `g_pGenericConfirmDialog->IsVisible()`) rather than letting it fall into the queue — this one
  specific call site needed that carried over, not a change to the primitive itself.
- [x] `CCanNotUseWordMsgBoxLayout` — OK (no `AddCallbackFunc`, default close only) — `Guild/UIGuildMaster.cpp`
- [x] `CGuildFireMsgBoxLayout` — deleted outright as dead code (confirmed zero call sites;
  `UIGuildInfo.cpp`'s `DoFireAction()` already sets `DeleteGuildIndex` directly without going
  through this class) — not ported, per the plan's own dead-code-candidate guidance. Its
  now-unused `DeleteGuildIndex`/`s_nTargetFireMemberIndex` externs were also removed from
  `CommonMessageBox.cpp`.

All 8 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references. Build + both RmlUi verification scripts still to run for this batch (see
"How to use this file" step 5) before considering it fully done.

### Trade / shop / inventory actions — done (2026-09-13)

- [x] `CTradeMsgBoxLayout` — OkCancel — `UI/Inventory/Trade.cpp` (incoming trade request)
- [x] `CTradeAlertMsgBoxLayout` — OkCancel — `UI/Inventory/Trade.cpp` (scam-alert confirm before
  trading). Original colored each of its 4 lines individually (3x orange warning, 1x red) via
  `TMsgBoxLayout`'s per-`AddMsg` color param; `GenericDialogConfig::Line` only has bold/not-bold,
  no arbitrary color, so all 4 collapsed to bold as a deliberate simplification (documented inline
  at the call site too). If this loses too much visual urgency in practice, the real fix is a
  scoped color/severity extension to `GenericDialogConfig::Line`, not a one-off hack here.
- [x] `CPersonalshopCreateMsgBoxLayout` — OkCancel — 2 call sites, `Engine/Object/ZzzInventory.cpp`
  and `UI/Inventory/MyShopInventory.cpp` (open-personal-shop confirm)
- [x] `CFenrirRepairMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`. Its
  `CFenrirRepairMsgBox` helper subclass (existed only to stash 2 caller-side int indices for the
  callback) was deleted entirely too — the indices now live in the `onPrimary` lambda's capture
  instead.
- [x] `CLuckyItemMsgBoxLayout` — OkCancel — `UI/Inventory/LuckyItemWnd.cpp` (variable 2-5 line
  body built from a runtime lookup table, first line bold — ported as a loop over `cfg.lines`)
- [x] `CMixCheckMsgBoxLayout` — OkCancel — `UI/Inventory/MixInventory.cpp` (combine-items confirm)
- [x] `CUseReviveCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CUsePortalCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CReturnPortalCharmMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`
- [x] `CUseSantaInvitationMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`

All 10 declarations + implementations removed from `CommonMessageBox.h`/`.cpp` (11 including the
`CFenrirRepairMsgBox` helper); grep-confirmed zero remaining references. Build + both RmlUi
verification scripts passed for this batch.

### Network/server-triggered (`WSclient.cpp`) — done (2026-09-13)

- [x] `CPartyMsgBoxLayout` — OkCancel — `Network/Server/WSclient.cpp` (party invite)
- [x] `CGuildWarMsgBoxLayout` / `CBattleSoccerMsgBoxLayout` — OkCancel — merged into one call site
  (`ReceiveDeclareWar`, same two `SendGuildWarResponse` callbacks for both, only body text differs)
- [x] `CServerImmigrationErrorMsgBoxLayout` — OK — `Network/Server/WSclient.cpp`
- [x] `CGemIntegrationUnityResultMsgBoxLayout` / `CGemIntegrationDisjointResultMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CGemIntegrationUnityCheckMsgBoxLayout` (4 call sites) / `CGemIntegrationDisjointCheckMsgBoxLayout`
  (1 call site) — OkCancel — actually called from `UI/Dialogs/CustomMessageBox.cpp` (native
  `CGemIntegrationUnityMsgBox`/`CGemIntegrationDisjointMsgBox` menu button handlers, which stay
  native — only the check-dialog-they-open was ported); `CustomMessageBox.cpp` needed the
  `GenericConfirmDialog.h` include added
- [x] `CChaosCastleTimeCheckMsgBoxLayout` — OkCancel (2 call sites, 1-2 dynamic lines) —
  `Network/Server/WSclient.cpp`
- [x] `CHarvestEventLayout` / `CWhiteAngelEventLayout` — OkCancel — `Network/Server/WSclient.cpp`
- [x] `CDuelCreateErrorMsgBoxLayout` (2 call sites) / `CDuelWatchErrorMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CDoppelGangerMsgBoxLayout` (4 call sites, each with its own body lines incl. blank-line
  spacers) — OK — `Network/Server/WSclient.cpp`
- [x] `CGatemanFailMsgBoxLayout` (2 call sites) — OK — `Network/Server/WSclient.cpp`
- [x] `COsbourneMsgBoxLayout` — OK — `Network/Server/WSclient.cpp`. Original colored its 2 lines
  differently (red bold, gold bold); collapsed to bold=true for both, same simplification as
  `CTradeAlertMsgBoxLayout` above.
- [x] `CCry_Wolf_Dont_Set_Temple1` / `CCry_Wolf_Wat_Set_Temple1` — OK — `Network/Server/WSclient.cpp`
- [x] `CSantaTownLeaveMsgBoxLayout` — OkCancel — `CSantaTownSantaMsgBoxLayout` (2 call sites) —
  OkCancel (no `AddMsg` in `SetLayout`, body supplied per call site) — `Network/Server/WSclient.cpp`
- [x] `CUseRegistLuckyCoinMsgBoxLayout` / `CRegistOverLuckyCoinMsgBoxLayout` /
  `CExchangeLuckyCoinMsgBoxLayout` / `CExchangeLuckyCoinInvenErrMsgBoxLayout` — OK —
  `Network/Server/WSclient.cpp`
- [x] `CEmpireGuardianMsgBoxLayout` (8 call sites across 2 functions, all OK-only with per-site
  dynamic text) — `Network/Server/WSclient.cpp`

`CQuestCountLimitMsgBoxLayout` intentionally **not** touched — still compiled out
(`#ifdef ASG_ADD_TIME_LIMIT_QUEST`, never defined); leave as dead code unless that macro is ever
enabled. All 25 real classes' declarations + implementations removed from `CommonMessageBox.h`/
`.cpp`; grep-confirmed zero remaining references. Build + both RmlUi verification scripts passed
for this batch.

(Gem integration check dialogs `CGemIntegrationUnityCheckMsgBoxLayout`/
`CGemIntegrationDisjointCheckMsgBoxLayout` — done, see the Network group above; the menu variants
they're opened from live in `CustomMessageBox` and stay native.)

### Siege / castle / CryWolf / combat events — done (2026-09-13)

- [x] `CMapEnterWerwolfMsgBoxLayout` / `CMapEnterGateKeeperMsgBoxLayout` — OkCancel (only OK wired;
  Cancel defaults to close) — `Engine/Object/ZzzInterface.cpp`. **Known regression**: both
  originally called `pMsgBox->LockOkButton()` to visually disable OK while a quest-state gate
  wasn't met — `GenericDialogConfig` has no disabled-button concept yet, so `onPrimary` now
  re-checks the same quest state and silently no-ops instead (functionally safe, cosmetically a
  downgrade: the button no longer looks disabled). Revisit if a locked/disabled-button field gets
  added to the primitive.
- [x] `CCastleMsgBoxLayout` (10 call sites across `UpdateGateManagingTab`/`UpdateStatueManagingTab`/
  `UpdateTaxManagingTab`) — OkCancel — `UI/Combat/CastleWindow.cpp`. Added one file-local
  `ExecuteCastleMsgBoxRequest()` helper (mirrors the original single shared `OkBtnDown`'s switch
  over `GetCurrMsgBoxRequest()`) reused by all 10 `onPrimary` lambdas, instead of duplicating the
  10-case switch at each site.
- [x] `CSiegeLevelMsgBoxLayout` / `CSiegeGiveUpMsgBoxLayout` — `UI/Combat/GuardWindow.cpp`
- [x] `CGatemanMoneyMsgBoxLayout` — `UI/NPCs/GatemanWindow.cpp`
- [x] `CMaster_Level_Interface` — OkCancel — `UI/HUD/MasterLevel.cpp`. Preserved the mouse-button
  state reset (`MouseLButton`/`MouseLButtonPop`/`MouseLButtonPush = false`) in both `onPrimary` and
  `onSecondary` — original did this on both OK and Cancel, presumably to stop the triggering click
  from falling through to the skill tree underneath.
- [x] `CCry_Wolf_Get_Temple` / `CCry_Wolf_Set_Temple` / `CCry_Wolf_Set_Temple1` /
  `CCry_Wolf_Dont_Set_Temple` / `CCry_Wolf_Destroy_Set_Temple` / `CCry_Wolf_Ing_Set_Temple` /
  `CCry_Wolf_Result_Set_Temple` — `Engine/Object/ZzzInterface.cpp`, `Network/Server/WSclient.cpp`,
  `UI/Events/CryWolf.cpp`. `CCry_Wolf_Set_Temple1`/`CCry_Wolf_Dont_Set_Temple` had no external call
  site (only chained from `CCry_Wolf_Get_Temple`'s own Cancel/OK) — ported as `Show()` calls made
  directly inside `CCry_Wolf_Get_Temple`'s `onSecondary`/`onPrimary` lambdas rather than as
  separate classes/call sites. Preserved one original quirk verbatim: `CCry_Wolf_Set_Temple`'s OK
  button was wired to `CCry_Wolf_Get_Temple::OkBtnDown` (not its own handler) in the native code —
  the ported call site duplicates that same handler logic with a comment explaining why.
  `CCry_Wolf_Result_Set_Temple`'s columnar scoreboard (header + up to 5 rank rows + blank spacer
  lines + summary) ported as plain text lines, colors collapsed (see `CTradeAlertMsgBoxLayout`
  note above for the same simplification) — readability not yet verified in-game, flagged for the
  general in-game verification pass. Porting this also exposed and fixed a pre-existing, unrelated
  type bug in `UI/Events/CryWolf.cpp`: it declared `extern BYTE HeroClass[5]` (should have been
  `CLASS_TYPE`, matching the real definition in `GMCrywolf1st.cpp` and `CommonMessageBox.cpp`'s own
  now-deleted `extern` decl) — harmless before since the file never actually used `HeroClass`, but
  a link error the moment it did. Fixed in place.

All 14 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references (one explanatory code comment mentions a deleted class name in prose, not a
real reference). Build + both RmlUi verification scripts passed for this batch.

### Dead-code candidates — confirmed dead and deleted (2026-09-13)

- [x] `CServerLostMsgBoxLayout` — its `OkBtnDown` did `SendMessage(g_hWnd, WM_DESTROY, 0, 0)`
  (quit-on-disconnect) — real-looking logic, but re-confirmed zero call sites anywhere; the actual
  disconnect flow must go through a different, already-existing mechanism. Deleted.
- [x] `CInfinityArrowCancelMsgBoxLayout` — re-confirmed zero call sites (its own header/cpp were
  the only references) despite non-trivial `g_iCancelSkillTarget` logic. Deleted.
- [x] `CBuffSwellOfMPCancelMsgBoxLayOut` — re-confirmed zero call sites. Deleted.
- [x] `CUnitedMarketPlaceMsgBoxLayout` — re-confirmed zero call sites. Deleted.

(`CGuildFireMsgBoxLayout` was the same story, already handled in the Guild batch above.)

All 4 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`. This closes out
every `CommonMessageBox.h` class this plan originally classified as FITS — nothing left to port
or delete in that file. `CustomMessageBox.h`'s ~76 classes (see below) and the other tracked
subsystems (`CUIPopup`, `GameShop/MsgBoxIGS*.h`) remain.

## `GenericDialogConfig` extensions — done (2026-09-13); `item3D` (5) and `input.Mode::Text` (9) now have consumers, `title`/`severity`/`input.Mode::NumericKeypad`/`progress` still don't

`GenericDialogConfig`/`CGenericConfirmDialog` (`UI/Dialogs/GenericConfirmDialog.h/.cpp`) grew
optional `title`, `severity` (Normal/Warning/Error, look-and-feel only), `input`
(`InputField::Mode::Text`/`NumericKeypad`), `progress` (duration-only, matching every real native
progress-bar call site being fire-and-forget), and `item3D` (an `ITEM` snapshot) — one struct, not
five sibling classes, per the design session that preceded this. `CGenericConfirmDialog` itself now
also implements `I3DRenderObj` (registers with `g_pNewUI3DRenderMng` in `Create()`) so `item3D` can
actually render via a `Render3D()` that reads an invisible `#gcd_item3d_anchor`'s live screen
position and converts it back to reference space (`UI::Scaling::LogicalX/Y`) before calling
`RenderItem3D()` — the same overall native-3D-inside-RmlUi split `CItemHotKey` proves, without that
class's extra delta-correction math (not needed for a brand-new anchor with no legacy hardcoded
coordinates to reconcile). `Mode::Text` hosts the shared `g_pSingleTextInputBox` portable widget,
positioned from a second anchor (`#gcd_input_anchor`) the same way CharMakeWin.cpp's own name/
password fields are — only the `g_iChatInputType == 1` path (the actual runtime default) is wired;
the older raw-global-buffer `== 0` path is a deliberate, documented gap. `Mode::NumericKeypad` is
pure RmlUi (10 shuffled-digit buttons + delete, reproducing `CKeyPadMsgBox`'s own anti-shoulder-
surfing shuffle). Layout geometry (input row size, keypad grid, progress bar) is a first-pass
placement inside the existing fixed 230x160dp panel, not yet visually verified in-game against a
real consumer — expect to need tuning once the first class using each field is actually ported.

**`item3D` is now proven end-to-end (2026-09-14)**, both the primitive and its rendering: the 5
`C3DItemCommonMsgBox`-derived classes below all consume it, in-game-tested (both themes) after
fixing 4 real bugs surfaced only by that testing — see `STATUS.md`'s own entry for the full
writeup, summarized: a CSS cascade-tie hiding bug, an anchor `position:absolute` flex-flow bug, a
`GetAbsoluteOffset()`-ignores-`transform` positioning bug (`PanelTranslateCorrection()`), and —the
big one— item3D rendering invisibly *behind* the dialog's own opaque panel background, since
RmlUi's main context always composites after the legacy 3D pass that draws it. That last one took
three real attempts to close out: a post-RmlUi callback approach crashed twice and was abandoned;
the fg/bg RmlUi document split that replaced it then surfaced a *second*-order bug of its own once
tested with another window open behind the dialog (NPC Shop) — the panel bled through beneath the
shop's own foreground content, because the shared background context every other window's bg doc
uses renders once, globally, before any window's own 2D content, not just this dialog's. Fixed for
real with a third, dedicated `Rml::Context` for this dialog's own panel, rendered at a precise point
in `CManager::Render()`'s own z-sorted loop (right before the shared 3D camera's turn) rather than
the generic background-layer hook. `title`/`severity`/`progress` remain **infrastructure only** —
no consuming class ported onto either yet, every field still defaults to unset/empty, so every
already-ported call site is unaffected by that.

**`input.Mode::Text` is now also proven end-to-end (2026-09-14)**: all 9 `CTextInputMsgBox`-derived
classes (see "Text input" below) consume it. This surfaced one real primitive gap:
`CGenericConfirmDialog::KeepOpen()`, letting `onPrimary`/`onSecondary` veto a click's `Resolve()`
for input validation failures, matching every native `OkBtnDown`'s own `CALLBACK_CONTINUE`
convention — see that section's own writeup. `Mode::NumericKeypad` still has zero consumers (the
"Numeric keypad" batch below is separate, unstarted work).

## `CommonMessageBox.h` — 3D-item-preview classes (6 classes)

All `TMsgBoxLayout<C3DItemCommonMsgBox>` — call `Set3DItem(pItem)`. The primitive's own `item3D`
support unblocked these — done 2026-09-13:

- [x] `CHighValueItemCheckMsgBoxLayout` — OkCancel, two live call sites (`UI/Inventory/
  InventoryActionController.cpp`'s double-click sell path, and `UI/NPCs/NPCShop.cpp`'s
  drag-into-shop `InventoryProcess()` path — both found and ported; the plan's original per-class
  grep had only caught the first one).
- [x] `CUseFruitMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`. `onPrimary`
  still creates the native `CUseFruitCheckMsgBoxLayout` chain dialog (a quantity stepper, different
  class, out of scope here) exactly as the old `OkBtnDown` did.
- [x] `CUsePartChargeFruitMsgBoxLayout` — OkCancel — `UI/Inventory/InventoryActionController.cpp`.
- [x] `CPersonalShopItemBuyMsgBoxLayout` — OkCancel — `UI/Inventory/PurchaseShopInventory.cpp`.
- [x] `CGambleBuyMsgBoxLayout` — OkCancel — `UI/NPCs/NPCShop.cpp`.

All 5 declarations + implementations removed from `CommonMessageBox.h`/`.cpp`; grep-confirmed zero
remaining references. Per-line custom `RGBA()` colors from the original `AddMsg()` calls are not
preserved (`GenericDialogConfig::Line` only has `bold`, matching every earlier FITS batch) — only
the bold/non-bold distinction carries over, styled via the theme's existing warm/secondary text
tokens. Build (162/162, zero new warnings) + both RmlUi verification scripts passed.

- [ ] `CPersonalShopItemValueCheckMsgBoxLayout` — **still native.** Not a `item3D`-only port: its
  caller (`CustomMessageBox.cpp`'s `CPersonalShopItemValueMsgBoxLayout::ProcessOk`, a `CTextInputMsgBox`
  price-entry dialog not yet ported) calls `SetItemValue(iInputZen)` on the instance before showing
  it, and `OkBtnDown` reads that value back via `GetItemValue()`. Needs a numeric price-value field
  on `GenericDialogConfig` (or reuse of the `input`/`NumericKeypad` shape) once the `CTextInputMsgBox`
  family this chains from is itself in scope — not designed now.

## `CustomMessageBox.h` (~76 classes: 46 layout wrappers + ~30 underlying box classes)

Characterized by sampling (medium-high confidence — see caveats below), not a full per-class read
like `CommonMessageBox.h` got. **Estimated FITS: 0** (one near-miss, see below) — this entire family
needs its own extensions before any meaningful batch can be ported. Re-survey with a full read
before starting real work here; don't trust the exact class list below as final.

- [x] `CDialogMsgBoxLayout`/`CDialogMsgBox` — done (2026-09-13). One live call site
  (`Network/Server/WSclient.cpp`'s `ReceiveServerCommand`, `Cmd1 == 5` — an NPC/server dialog-line
  popup, "shell + caller fills in body via `AddMsg` after construction" pattern, same as
  `CGuildRequestMsgBoxLayout` from the original proof batch). Ported straight to `ButtonSet::Ok`
  with `primaryLabel = I18N::Game::ConversationIsOver` (the real native button caption — not
  literally "End" as this row's earlier note guessed) and one dynamic line
  (`I18N::Dialog::Lookup(Data->Cmd2)`). No primitive change needed; `primaryLabel` already covered
  a non-"OK" label. Declarations + implementations removed from `CustomMessageBox.h`/`.cpp`;
  grep-confirmed zero remaining references. Build + both RmlUi verification scripts passed.
  - **`mu::ui::window::CreateOkMessageBox()`** (`UI/Core/WindowCommon.h/.cpp`) — done (2026-09-13).
    A *third*, previously-untracked pattern: a standalone helper that directly `new`s a base
    `CCommonMessageBox` (not a `TMsgBoxLayout<>` subclass) for a one-off OK-only message. Unlike
    every other row in this file, this needed **zero call-site edits** — it's a single centralized
    function called from ~90 sites across ~19 files (`WSclient.cpp` alone accounts for most of
    them), and every call site just calls `CreateOkMessageBox(text)` with the same signature, so
    redirecting the one function body to `g_pGenericConfirmDialog->Show(...)` migrates all of them
    at once. `dwColor` (only ever overridden by 2 identical call sites using a warning-red
    highlight — `InventoryActionController.cpp`/`MainFrameWindow.cpp`, same message both times)
    collapses to bold when non-default, same simplification as `CTradeAlertMsgBoxLayout`/
    `COsbourneMsgBoxLayout`. `fPriority` (never overridden by any caller) is now unused — kept in
    the signature so no caller needed touching. Confirmed via grep that no caller anywhere uses
    the `bool` return value, so it now unconditionally returns `true`. Build + both RmlUi
    verification scripts passed.
- **Text input** (`CTextInputMsgBox`-based) — **done (2026-09-14)**: all 9 classes ported onto
  `GenericDialogConfig::InputField::Mode::Text` (`CTradeZenMsgBoxLayout` — `Trade.cpp`;
  `CZenReceiptMsgBoxLayout`/`CZenPaymentMsgBoxLayout`/`CStorageUnlockMsgBoxLayout` —
  `StorageInventory.cpp`; `CPersonalShopItemValueMsgBoxLayout` — factored into a shared
  `ShowPersonalShopItemValueDialog()` free function (`MyShopInventory.h`/`.cpp`), since 3 of its 4
  call sites already live there (the 4th, `ZzzInventory.cpp`'s `OpenPersonalShopMsgWnd(2)`, is
  unreachable in practice — its only caller always passes 1 — but was still updated to keep the
  function compiling); `CPersonalShopNameMsgBoxLayout` — `ZzzInventory.cpp`;
  `CCastleWithdrawMsgBoxLayout` — `CastleWindow.cpp` (reads its own typed amount directly, not via
  `ExecuteCastleMsgBoxRequest()`'s generic switch, since none of that switch's 10 existing cases
  need input); `CStorageLockMsgBoxLayout` — chained from the still-native
  `CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown` (`CustomMessageBox.cpp`, unchanged — a PIN-entry
  keypad dialog, separate "Numeric keypad" batch below), which now captures the just-entered 4-digit
  PIN directly in the closure instead of round-tripping it through
  `CTextInputMsgBox::SetPassword()`/`GetPassword()`; `CGuildBreakPasswordMsgBoxLayout` — factored
  into a shared `ShowGuildBreakPasswordDialog()` file-local helper (`GuildInfoWindow.cpp`) for its 3
  call sites. `CTextInputMsgBox` itself (the now-unused native base class) and its own
  `INPUTBOX_TYPE_NUMBER`/`_TEXT`/`INPUTBOX_WIDTH`/`_HEIGHT`/`_TEXTLIMIT` constants were deleted too
  (grep-confirmed dead once all 9 subclasses were gone). Build + both RmlUi verification scripts
  passed.
  - **Primitive extension needed, `CGenericConfirmDialog::KeepOpen()`**: every native
    `CTextInputMsgBox`-derived `OkBtnDown`/`ReturnDown` validates its typed input (empty field, a
    zero/unparsed amount) and returns `CALLBACK_CONTINUE` — leaving its own MsgBox open for the
    user to retry — rather than closing unconditionally like every plain-text confirm dialog does.
    `GenericConfirmDialog` had no equivalent, so `onPrimary`/`onSecondary` can now call
    `g_pGenericConfirmDialog->KeepOpen()` to veto that click's `Resolve()`: nothing gets
    hidden/reset/advanced, `m_Active` is restored exactly as it was, as if the click never
    happened. Used by 7 of the 9 classes in this batch (checked per-class against its own native
    `ProcessOk` — not assumed uniform: `CGuildBreakPasswordMsgBoxLayout` specifically does NOT veto
    on empty input, it always closes and just logs an error message, so it deliberately never calls
    `KeepOpen()`). One known, accepted simplification: native only plays `SOUND_CLICK01` on the
    success path, never on a `CALLBACK_CONTINUE` retry, while `CGenericConfirmDialog::Update()`
    plays it unconditionally before `Resolve()` even knows whether `onPrimary` will veto —
    cosmetic-only (an extra click sound on a failed retry), not worth plumbing sound timing through
    the veto path for.
  - Per-class WEBZEN.COM password fields (`CStorageLockMsgBoxLayout`/`CStorageUnlockMsgBoxLayout`/
    `CGuildBreakPasswordMsgBoxLayout`) are `masked = true` but deliberately NOT `numericOnly` —
    confirmed against native: despite `Create()`'s `dwInputType` parameter being
    `INPUTBOX_TYPE_NUMBER` for all three, none of them ever called
    `SetInputBoxOption(UIOPTION_NUMBERONLY | ...)` the way every zen-amount dialog does, so real
    (alphanumeric) WEBZEN passwords were never actually digit-restricted natively either.
  - Every dialog in this batch reuses `GenericConfirmDialog.cpp`'s existing fixed real-pixel input
    widget size (`kInputFieldWidth`/`kInputFieldHeight` = 150x18) regardless of native's own
    per-class width (native varies 50-130 reference px, plus a `g_iLengthAuthorityCode`-scaled width
    for the 3 password fields) — same "one struct, not per-dialog geometry" simplification as
    `progress`/`title`. In-game-tested (2026-09-14) and confirmed working, position included.
  - **In-game-testing bug, 2026-09-14**: the field was completely invisible on first test (typed
    text unreadable, no visible box at all) — `UpdateTextInputWidget()`'s `InputBoxConfig` never
    overrode the text color, which defaults to opaque black, invisible against this dialog's own
    dark panel fill. The exact same gotcha `CharMakeWin.cpp`'s own `#input_text_anchor` field
    already hit and documented in its own code, just not one this new call site had inherited.
    Fixed by setting the same light-cream text color `LoginWin.cpp`/`CharMakeWin.cpp` already use
    (255,230,210), plus a visible dark recessed background fill (`.gcd-progress-track`'s own
    `#100c06`, reused via the widget's own `backR/G/B/Alpha` rather than CSS) since -- unlike
    those two windows, whose native sprite chrome frames the input row -- this dialog's anchor has
    no visual frame of its own, so the field needs to paint its own affordance to read as a
    clickable box at all.
- **Numeric keypad** (`CKeyPadMsgBox`-based) — **done (2026-09-14)**: re-inventoried all 5 classes
  (grep, not trusted from this row's own old list) and found only 3 with live call sites, all in
  `UI/Inventory/StorageInventory.cpp`. `CPasswordKeyPadMsgBoxLayout` (vault PIN verify, 2 call
  sites — a locked-vault item-move guard and `CZenPaymentMsgBoxLayout`'s own onPrimary from last
  batch) → `ShowVaultPinVerifyDialog()`. `CStorageLockKeyPadMsgBoxLayout` (vault-lock flow, step 1:
  choose new PIN, 1 call site) → `ShowStorageLockPinDialog()`. `CStorageLockCheckKeyPadMsgBoxLayout`
  (step 2: PIN re-entry confirm, 0 external call sites — only ever chained from step 1) →
  `ShowStorageLockPinConfirmDialog(firstPin)`, called from `ShowStorageLockPinDialog()`'s own
  `onPrimary`; on match, this now directly builds the already-ported WEBZEN-password `Mode::Text`
  `Show()` config inline (moved verbatim from where it used to live inside the native
  `CStorageLockCheckKeyPadMsgBoxLayout::OkBtnDown`) rather than a third separate call site. All 3
  file-local helpers reuse `KeepOpen()` for incomplete input and an inline adjacent-char check for
  `IsAllSameNumber()` (native: 4-in-a-row rejection, `CreateOkMessageBox(...)`, closes normally —
  not a `KeepOpen()` case).
  - **`CStorageLockFinalKeyPadMsgBoxLayout`/`CStorageUnlockKeyPadMsgBoxLayout` — confirmed dead,
    deleted, not ported**: zero call sites anywhere (grep-confirmed) — both superseded by the
    `Mode::Text`-based WEBZEN password dialogs already ported last batch
    (`CStorageLockMsgBoxLayout`/`CStorageUnlockMsgBoxLayout`), left behind as unreachable leftovers.
  - `CKeyPadMsgBox` itself (the native base class, plus its own `CKeyPadButton`/
    `CDeleteKeyPadButton` helper classes and the `KEYPAD_TYPE_*` enum) deleted too once all 5
    subclasses were gone — grep-confirmed no other consumer, same full-cleanup discipline
    `CTextInputMsgBox` got last batch. `MAX_KEYPADINPUT`/`MAX_PASSWORD_SIZE` (shared `_define.h`
    constants `CKeyPadMsgBox` also used) were left alone — still used by unrelated code
    (`g_lpszKeyPadInput` in `ZzzInventory.cpp`).
  - Build (zero new warnings) + both RmlUi verification scripts passed. `Mode::NumericKeypad` is
    now proven building the same way `Mode::Text` was last batch, but **not yet in-game-tested** —
    given last batch's own invisible-text bug on `Mode::Text`'s first real exercise, treat this the
    same way: ask the user to test both live flows (locked-vault item/zen withdrawal PIN verify,
    and the full vault-lock chain: PIN choose → confirm match/mismatch/all-same-digit → WEBZEN
    password) before considering it actually done, not just built.
- **3D item preview**: `CUseFruitCheckMsgBoxLayout` — `item3D` now exists, see above.
- **Gem-selection menus** (bespoke multi-button, not OK/Cancel): `CGemIntegrationMsgBoxLayout`,
  `CGemIntegrationUnityMsgBoxLayout`, `CGemIntegrationDisjointMsgBoxLayout`.
- **Fixed-format result/ranking tables** (custom `RenderTexts`, no `AddMsg`):
  `CBloodCastleResultMsgBoxLayout`, `CDevilSquareRankMsgBoxLayout`, `CChaosCastleResultMsgBoxLayout`.
- **Multi-option menus** (3+ named action buttons): `CChaosMixMenuMsgBoxLayout`,
  `CTrainerMenuMsgBoxLayout`, `CTrainerRecoverMsgBoxLayout`, `CElpisMsgBoxLayout`,
  `CSystemMenuMsgBoxLayout`, `CCherryBlossomMsgBoxLayout`, `CSeedMasterMenuMsgBoxLayout`,
  `CSeedInvestigatorMenuMsgBoxLayout`, `CDelgardoMainMenuMsgBoxLayout`, `CLuckyTradeMenuMsgBoxLayout`,
  `CResetCharacterPointMsgBoxLayout`, `CGuild_ToPerson_PositionLayout`.
- **Progress bar / timed auto-close** (`CProgressMsgBox`/`CCursedTempleProgressMsgBox`-based, needs
  a progress-bar concept — not built): `CCrownSwitchPopLayout`, `CCrownSwitchPushLayout`,
  `CCrownSwitchOtherPushLayout`, `CSealRegisterStartLayout`, `CSealRegisterSuccessLayout`,
  `CSealRegisterFailLayout`, `CSealRegisterOtherLayout`, `CSealRegisterOtherCampLayout`,
  `CCrownDefenseRemoveLayout`, `CCrownDefenseCreateLayout`, `CCursedTempleHolicItemGetLayout`,
  `CCursedTempleHolicItemSaveLayout`.
- **Duel dialogs** (bespoke, likely countdown rendering): `CDuelMsgBoxLayout`, `CDuelResultMsgBoxLayout`.

## Other native dialog subsystems (out of `CommonMessageBox`/`CustomMessageBox`, tracked here too)

- [x] `CUIPopup` (`UI/Dialogs/UIPopup.h`, `g_pUIPopup`) — done (2026-09-13). Every real call site
  (grepped, not just the header's declared capability) used only `POPUP_OK` (5 sites, `ResultFunc`
  always `NULL`) or `POPUP_YESNO` (4 live sites with a callback + 1 dead one) — no live
  `POPUP_OKCANCEL`/`POPUP_INPUT`/`POPUP_TIMEOUT` anywhere. All 9 live sites ported:
  `Guild/UIGuildInfo.cpp` (6: alliance-master-can't-disband/withdraw notices, guild-disband,
  member-disband-rank, member-fire, alliance-ban confirms) and `Guild/UIGuildMaster.cpp` (1:
  guild-mark-edit-not-activated notice), `Network/Server/WSclient.cpp` (2: hunt-zone-enter
  fail/no-authorization notices). `POPUP_YESNO` → `ButtonSet::OkCancel` with
  `primaryLabel/secondaryLabel = I18N::Game::Yes/No` (first live use of a non-OK/Cancel label —
  `GenericDialogConfig` already supported it, no primitive change needed). Each old free-function
  callback's body was folded directly into `onPrimary`/`onSecondary`, same convention as the
  `CommonMessageBox.h` batches.
  - Also deleted as confirmed-dead (not ported): `CUIGuildMaster::ReceiveGuildRelationShip` +
    its `DoGuildRelationReplyAction` callback + the `m_dwEditGuildMarkConfirmPopup`/
    `m_dwGuildRelationShipReplyPopup` members + `CUIGuildMaster::CloseMyPopup()` — grep confirmed
    `WSclient.cpp`'s `ReceiveGuildRelationShip` packet handler actually calls
    `g_pGuildInfoWindow->ReceiveGuildRelationShip(...)` (the already-ported `CGuildInfoWindow`,
    from the earlier Guild batch), never this one; it was dead leftover from before that port.
  - Two now-pointless `g_pUIPopup->CancelPopup()` defensive calls removed from `WSclient.cpp`
    (`ReceiveHuntZoneEnter`'s two branches, `ReceiveCrownState`'s entry) — nothing left for them to
    cancel once their neighboring `SetPopup(...POPUP_OK...)` calls moved off `CUIPopup`.
  - **`CUIPopup` itself is *not* deleted** — `UIGuildInfo.cpp`'s `POPUP_CUSTOM` "Appoint"
    sub-guild-master/battle-master picker (`DoAppointAction`/`RenderAppoint`, a bespoke 2-option
    picker plus its own OK/Cancel) stays fully native, same DOESNT_FIT category as
    `CChaosMixMenuMsgBoxLayout` etc. `CUIGuildInfo::CloseMyPopup()`/`m_dwPopupID` also stay
    unchanged — they now exclusively (and correctly) govern just that surviving popup.
  - **Optional follow-up, not done**: `UIPopup.cpp`/`.h`'s `POPUP_OK`/`POPUP_OKCANCEL`/
    `POPUP_YESNO`/`POPUP_INPUT`/`POPUP_TIMEOUT` branches in `UpdateInput()`/`Render()`/`PressKey()`
    are now unreachable (zero live callers pass those flags anymore) but were left in place —
    trimming them is a separate, slightly riskier change to a still-alive shared class, not blocking
    this batch.
  - Also removed one unrelated pre-existing dead `extern int DoBreakUpGuildAction_New(POPUP_RESULT)`
    declaration found in `CommonMessageBox.cpp` while working in this area (no definition, no
    caller, anywhere — stale leftover, unrelated to this session's own `DoBreakUpGuildAction`).
- [ ] `GameShop/MsgBoxIGS*.h` — 10 `CMessageBoxBase` subclasses, all built on `CMsgBoxIGSCommon`
  (buy confirm, buy-package w/ live 3D preview, buy-select-item, generic OK/Cancel, delete-item
  confirm, gift-storage-info, send-gift + confirm, storage-item-info, use-buff/use-item confirm).
  **This is the one family in the whole inventory with a genuine title**: `CMsgBoxIGSCommon::
  Initialize(pszTitle, pszText)` (`GameShop/MsgBoxIGSCommon.h/.cpp`) renders `m_szTitle` in bold at
  a fixed top offset, separate from the body text below it — a real title/body split, not just a
  bold first line the way `CCommonMessageBox`'s bold `AddMsg` lines are. `GenericDialogConfig.title`
  now exists (see "`GenericDialogConfig` extensions" above) — the plain OK/Cancel ones in this
  family are unblocked; the 3D-preview ones also need that extension (also now built) reviewed
  together, since
  they share the same base class).
- **Explicitly out of scope for `CGenericConfirmDialog`** (would need their own primitives if ever
  ported): `CHelpWindow`, `CWindowMenu`, `CChatCommandWindow` (`UI/Dialogs/`) — help overlay,
  per-window popup menu, command picker; none are confirm-dialog shaped.

## How to use this file

1. Pick a batch (a grouping above, or a handful of related `[ ]` rows).
2. Re-check each class's `SetLayout()`/`Create()` still matches the noted shape (things drift).
3. Swap the call site(s) to `g_pGenericConfirmDialog->Show({...})`, following the pattern the 3
   already-ported call sites established.
4. Grep to confirm zero remaining references, delete the native class's declaration + implementation
   from `CommonMessageBox.h`/`.cpp` (or `CustomMessageBox.h`/`.cpp`).
5. Check the row off here, and run both `Tools/check_rml_rcss_syntax.py` /
   `check_rml_rcss_drift.py` plus a full build before considering the batch done.
6. Flag in-game verification for whoever can launch the client — same as the original 3.
