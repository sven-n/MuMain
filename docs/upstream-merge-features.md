# Upstream merge: features and smoke-test guide

> **Context:** this branch (`cross-platform-sdl-migration-merged`) was merged with 406 upstream commits in merge commit `f5d1d73e` (2026-04-22), plus two fixup commits (`1cceb074` include path fix, `1eeac9aa` SDL3 label-cache stub). This document lists every named upstream PR that landed, what it does, and how to verify it in-game.
>
> **Status legend:** ✅ fully ported · ⚠️ ported but stubbed on SDL3 · 🔍 needs runtime verification

## Quick smoke-test sequence

A single 15-minute play session exercising all of the below in order:

1. Create a new character → login → logout → log back in (exercises PR #313)
2. Walk in a straight line across a long map stretch (PR #324)
3. Kill a few mobs, pick up drops (PR #317)
4. Sit next to an item drop pile, read tooltips (PR #321, on SDL3 runs uncached)
5. Right-click to drop, sell, stack, repair items (PR #311)
6. Open inventory, inspect a Divine Stick if you have one (PR #306)
7. Toggle Mu Helper self-defense in a party, stand among monsters and party members (PRs #304 / #305)
8. Cast Nova or Hellfire next to a mob pile (self-position skills)
9. Play into high master levels if your character supports it (PR #316)

If all 9 feel normal, the merge is almost certainly sound. Bugs introduced by merge-resolution tend to surface as obvious misbehavior in one of these specific paths.

---

## Mu Helper — PRs #304, #305

**Source PRs:** `eduardosmaniotto/bugfix/helper-heal`, `eduardosmaniotto/bugfix/helper-attack`
**Ported files:** `src/source/Gameplay/NPCs/MuHelper.cpp`, `src/source/Gameplay/NPCs/MuHelper.h`
**Status:** ✅ ported, 🔍 needs runtime verification

### Bugs fixed

1. **Heal party index stuck on one member.** The heal-party index only advanced when `pChar == NULL` (member offline/out-of-range). Any successful heal path returned early and never incremented — so the helper would re-heal the same member on every tick and ignore the other four. Fix captures the heal result in `iHealResult`, always increments the index, then returns the result.

2. **Self-defense targets non-monsters.** `if (m_config.bUseSelfDefense)` forcibly cast `pTarget->Object.Kind = KIND_MONSTER` and assigned it as target — meaning the helper could attack party members, NPCs, or players in defense mode. Fix adds `IsMonster(pTarget)` gate and removes the Kind coercion.

3. **`TargetX/TargetX` typo.** `ComputeDistanceFromTarget` read `pTarget->TargetX, pTarget->TargetX` for the second point (should have been `TargetY`). Meant the "next position" distance was really the "current X, current X" distance — usually wrong, sometimes right by coincidence.

4. **Self-position AoE skills wouldn't fire.** Nova, Hellfire, Inferno, etc. target the caster's own position, not a monster. The skill-execution path required a valid target to be set, and failed validation on these skills. Fix adds `IsSelfPositionSkill()` predicate and branches into a path that uses `Hero->PositionX/Y` as the skill target, still validating the tracked monster target for life-check purposes.

5. **Pickup dedup.** `SendPickupItemRequest` was sent every frame the item was in range, flooding the server. Fix guards with `SendGetItem == -1` token.

### Expected behavior

- Toggle Mu Helper with `U`. In a party of 5, stand so all members are in range, one low-HP. Helper should rotate heals across members, not fixate.
- Enable self-defense and stand near a party member below 50% HP. Helper should NOT target them.
- Pick a Summoner/MG/Wizard with Nova or Hellfire. Helper should cast the AoE skills, not just melee.
- Walk past a drop pile. Helper should send one pickup request per item, not spam.

---

## Divine Stick of Archangel — PR #306

**Source PR:** `olisikh/fix/divine_stick`
**Ported files:** `src/source/Gameplay/Items/ZzzInventory.cpp`, `src/source/Data/ZzzInfomation.cpp`, `src/source/Data/ZzzOpenData.cpp`
**Status:** ✅ ported, 🔍 needs runtime verification

### Bug fixed

Divine Stick of Archangel was not treated as an Archangel weapon in several classification paths: wing-type check, item-name color (purple for Archangel gear), tooltip color, model registration, texture loading. Result: the item rendered incorrectly in-world and in UI.

### Implementation

Two helper functions `IsDivineArchangelWeaponItem(itemType)` and `IsDivineArchangelWeaponModel(modelType)` added at top of `ZzzInventory.cpp`. Every site that previously enumerated "sword or CB or staff or scepter of Archangel" was replaced with the helper call, which now also includes the stick.

Model and texture registrations added in `ZzzOpenData.cpp`:
```cpp
gLoadData.AccessModel(MODEL_DIVINE_STICK_OF_ARCHANGEL, L"Data\\Item\\", L"Archangelus");
gLoadData.OpenTexture(MODEL_DIVINE_STICK_OF_ARCHANGEL, L"Item\\");
```

### Expected behavior

- Obtain a Divine Stick of Archangel (e.g., via dev inventory).
- Item appears in 3D world with correct model/texture (not a missing-asset fallback).
- Name in inventory renders purple, same as Divine Sword of Archangel.
- Tooltip treats it as a wing-class item where relevant.

---

## Wandering Merchant Zyro — PR #310

**Source PR:** `olisikh/fix/wandering_merchant`
**Ported file:** `src/source/Gameplay/Characters/ZzzCharacter.cpp`
**Status:** ✅ ported

### Bug fixed

Wandering Merchant Zyro type was not in the `IsForcedNpcMonsterType` allowlist, so it was classified as a monster → attackable, hostile AI behaviors, etc. Fix adds `MONSTER_WANDERING_MERCHANT_ZYRO` to the switch's NPC cases.

### Expected behavior

- When Zyro spawns (specific map/event), he's classified as an NPC — talkable, not attackable, no hostile pathing.

---

## Right-click inventory actions — PR #311

**Source PR:** `eduardosmaniotto/feature/right-click-drop`
**Ported files:**
- `src/source/UI/Windows/Inventory/NewUIMyInventory.{cpp,h}` (significantly rewritten)
- `src/source/UI/Windows/Inventory/IInventoryActionContext.h` (new; was at repo root, moved during merge)
- `src/source/UI/Windows/Inventory/InventoryUtils.h` (new)
- `src/source/UI/Windows/Inventory/NewUIInventoryActionController.{cpp,h}` (new)
- `src/source/UI/Framework/NewUIInventoryCtrl.{cpp,h}` (support)
- `src/source/UI/Windows/Inventory/NewUIInventoryExtension.{cpp,h}` (support)
- `src/source/UI/Windows/Inventory/NewUIStorageInventory.{cpp,h}` (support)
- `src/source/UI/Windows/Inventory/NewUIStorageInventoryExt.{cpp,h}` (support)
- `src/source/UI/Framework/NewUICustomMessageBox.cpp`, `NewUICommonMessageBox.cpp`, `NewUINPCShop.cpp`

**Status:** ✅ ported (architecturally significant), 🔍 **highest-priority runtime check**

### Feature

Right-click on an inventory item performs a context-sensitive action: auto-sell at NPC shop, auto-repair at repair NPC, stack with identical item elsewhere, consume (potions), apply jewel (on compatible target), drop on floor, or move to inventory extension/storage. Previously these required drag-and-drop.

### Architecture change

The previous implementation had four `static` helpers on `CNewUIMyInventory` — `RepairItemAtMousePoint`, `ApplyJewels`, `TryStackItems`, `TryConsumeItem` — plus a `HandleInventoryActions` entry point. These were extracted into a new `CNewUIInventoryActionController` class, and `CNewUIMyInventory` now implements an `IInventoryActionContext` interface that exposes the pieces the controller needs (`GetRepairMode`, `IsEquipable`, `ResetMouseLButton/RButton`, `FindEmptySlot`, `IsRepairEnableLevel`). `CNewUIMyInventory::HandleInventoryActions` now just delegates to `m_ActionController.HandleInventoryActions(targetControl)`. Controller is initialized via `m_ActionController.SetContext(this)` in `Create()`.

### Bounds fix

Several `if (iSourceIndex != -1)` checks in `UIControls.cpp`, `NewUICommonMessageBox.cpp`, and `NewUINPCShop.cpp` were tightened to `if (iSourceIndex >= MAX_EQUIPMENT_INDEX && iSourceIndex < MAX_MY_INVENTORY_EX_INDEX)`, preventing action attempts on equipment slots or invalid indices.

### Expected behavior

- Right-click potion → consumed.
- Right-click weapon at repair NPC → repaired + zen deducted.
- Right-click item at NPC shop with appropriate sell conditions → sold.
- Right-click jewel on enchantable item → consumed, item gets the jewel option.
- Right-click stackable consumable near an existing stack → merged into the stack.
- Right-click in empty storage while storage open → moved from inventory to storage.
- Right-click on equipped gear → **should be blocked** (MAX_EQUIPMENT_INDEX range check).

### Risk callout

During the merge, ~1500 lines of helper implementations were removed from `NewUIMyInventory.cpp` in favor of the controller class. Symbol-level audit confirms every removed method has a counterpart in `CNewUIInventoryActionController`, but the order of operations in `HandleInventoryActions` needs verification — if anything silently regressed, this is the code path to bisect first.

---

## Skill position — PR #312

**Source PR:** `itayalroy/fix_skill_pos`
**Ported file:** `src/source/Gameplay/Characters/ZzzCharacter.cpp`
**Status:** ✅ ported (via `DetermineMonsterObjectKind` extraction)

### Implementation

Part of a broader refactor that extracted the monster-kind classification (previously a giant if-else chain) into a function `DetermineMonsterObjectKind(type)` in an anonymous namespace. The skill-position fix landed as part of the logic reorganization and the helper `IsForcedNpcMonsterType`.

### Expected behavior

- When you cast a targeted skill on a monster, the visual/server target matches what you clicked (not a nearby one).

---

## Login scene race — PR #313

**Source PR:** `itayalroy/fix_login_scene_race`
**Ported file:** `src/source/Scenes/LoginScene.cpp`
**Status:** ✅ ported

### Bug fixed

When the server replied fast after login, the character-list population could race with `ReleaseLogoSceneData()` → character list got cleared right after arrival → empty character select screen.

### Fix

`ReleaseLogoSceneData()` moved earlier — before the character-request message is sent, not after. Comment added: "Tear down the login scene data before asking the server for the account characters, otherwise a fast reply can be cleared again."

### Expected behavior

- Login on a low-latency connection (or a local dev server). Character select screen consistently shows your characters on first appearance — no flicker-clear.

---

## Consecutive walk packets / post-move angle snap — PR #314 + walk jitter PR #324

**Source PR:** `itayalroy/fix_push`, `itayalroy/fix_walk_jitter`
**Ported file:** `src/source/Gameplay/Characters/ZzzCharacter.cpp`
**Status:** ✅ ported

### Bugs fixed

1. **Walk jitter.** The character-position update used `+ FPS_ANIMATION_FACTOR` where it should have been `* FPS_ANIMATION_FACTOR`. Effectively adding a scale-per-frame bias instead of multiplying by it → micro-oscillation in walk direction at certain angles.

2. **Post-move angle snap.** Character's facing angle was snapping when two move packets arrived in quick succession, before the first completed.

3. **Stop walking on server move-position.** When the server sent a reposition, the client should interrupt the local walk path.

### Expected behavior

- Walk diagonally for 30+ tiles. No visible jitter, straight-line motion.
- In a laggy spot (or stress-inducing crowded area), character doesn't pop-rotate.

---

## Experience bar overflow — PR #316

**Source PR:** `olisikh/fix/exp_bar_overflow`
**Ported files:** `src/source/Network/WSclient.cpp`, `src/source/UI/Windows/HUD/NewUIMainFrameWindow.{cpp,h}`, `src/source/Gameplay/Characters/CharacterManager.h`
**Status:** ✅ ported

### Bug fixed

At high master levels, experience totals exceeded 32-bit `DWORD` capacity and wrapped to 0 or negative. Master level 400+ would show an empty or weirdly-full exp bar.

### Fix

- `m_dwPreExp` / `m_dwGetExp` widened from `DWORD` to `__int64`.
- ~190 lines of saturating 64-bit arithmetic helpers added to `WSclient.cpp` (`GetNormalLowerBound`, `GetMasterLowerBound`, `SaturatingAddToUpper`, `ClampToInterval`).
- `IsMasterExperienceActive(classType, level)` method added to `CCharacterManager`, gated at level 400.
- `constexpr int kMasterExperienceUnlockLevel = 400` added in `CharacterManager.cpp`.
- Exp-bar progress math in `NewUIMainFrameWindow.cpp` clamped via `std::clamp(fGapProgress, 0.0, 1.0)`.

### Expected behavior

- Master-level character at 400+ shows a sensible exp bar that increases monotonically.
- No visible tearing when crossing level-transition boundaries.
- Hovering exp bar tooltip shows current/next values that make sense (no negatives, no wrap).

---

## Phantom items — PR #317

**Source PR:** `olisikh/bug/phantom_items`
**Ported files:** `src/source/Network/WSclient.cpp`, `src/source/UI/Framework/NewUIInventoryCtrl.cpp`
**Status:** ✅ ported

### Bug fixed

Occasionally a pickup packet would be recorded by the client but the server would reject it (full inventory, moved between packets, etc.). The client displayed the item as if received, creating a "phantom" — UI showed the item, but the server didn't know about it → the item couldn't be interacted with, and the slot appeared occupied.

### Fix

1. New `RequestInventorySync()` helper + `shouldResyncInventory` flag in `WSclient.cpp`. On any pickup response that indicates a mismatch, client sends an inventory-refresh request to re-sync.
2. In `CNewUIInventoryCtrl`, the tooltip-triggering code validates the slot via `FindItem()` before acting — previously it trusted `m_pdwItemCheckBox[idx] != 0` which could be out-of-sync.
3. New `ClearSlotKey()` and `RequestInventoryRefresh()` helpers added as public API on `CNewUIInventoryCtrl` to allow the validation path to clear the ghost slot and request re-sync.

### Expected behavior

- Pick up items in a full inventory. Items that can't fit either don't appear or immediately vanish — no phantom slots left behind.
- If a phantom does appear (edge case), inventory auto-resyncs within one packet round-trip.

---

## Phoenix Soul drop + Lucky Set constants — PR #318

**Source PR:** `olisikh/fix/phoenix_soul_cant_drop`
**Ported files:** `src/source/Gameplay/Items/ZzzInventory.cpp`, `src/source/Core/mu_define.h`
**Status:** ✅ ported

### Bug fixed

Phoenix Soul item sits at index 73 in its item slot. The Lucky Set armor check previously hardcoded the range `62..72` (11 items), which was correct for Lucky Set armor pieces but was being compared with item types that could equal 73 (Phoenix Soul), erroneously including it in the "can't drop" Lucky Set category.

### Fix

Constants extracted to `mu_define.h` with explicit documentation:
```cpp
constexpr int LUCKY_SET_ARMOR_START_INDEX = 62;
constexpr int LUCKY_SET_ARMOR_COUNT = 11;  // Indices 62-72 (Phoenix Soul is at 73)
```
`Check_LuckyItem` in `ZzzInventory.cpp` rewritten to use the constants plus `% MAX_ITEM_INDEX` instead of the magic `512`, so that the range is exclusive of 73.

### Expected behavior

- Phoenix Soul item can be dropped, sold, traded, personal-shopped like any non-Lucky item.
- Actual Lucky Set armor (Helm/Armor/Pants/Gloves/Boots indices 62–72) still blocked from sell/personal-shop/trade per game rules.

---

## Ancient item Mu Helper pickup — PR #320

**Source PR:** `olisikh/fix/ancient_glow`
**Ported file:** `src/source/Gameplay/NPCs/MuHelperData.cpp`
**Status:** ✅ ported

### Bug fixed

`MuHelperData.cpp`'s serialization was asymmetric:
- **Write direction** (client → server): `netData.SetItem = gameData.bPickAllItems ? 1 : 0`
- **Read direction** (server → client): `gameData.bPickAncient = (bool)netData.SetItem`

The UI exposed a "pick ancient" toggle, but the client sent the state of a different flag (`bPickAllItems`, which the UI didn't set). Changing the UI toggle had no effect on what the helper picked up.

### Fix

One-line change in the write path:
```cpp
netData.SetItem = gameData.bPickAncient ? 1 : 0;   // was: bPickAllItems
```

### Expected behavior

- Open Mu Helper config, toggle "Pick Ancient Items". Helper's pickup behavior reflects the toggle across restart.

---

## Ground-item-label texture cache — PR #321 ⚠️ SDL3 regression

**Source PR:** `olisikh/fix/tooltips_fps`
**Ported file:** `src/source/Gameplay/Items/ZzzInventory.cpp`
**Status:** ⚠️ **ported on Windows, stubbed on SDL3 builds**

### Feature

Caches rasterized ground-item label textures (`FillRect` + `TextOut` + `glTexImage2D` on Windows) keyed by `GroundItemLabelCacheKey` (type, level, excellent flags, ancient discriminator, set-item presence, option flags). Avoids re-rendering the same label every frame when nothing changed.

Budget-limited via `SetGroundItemLabelBuildBudget(32)` (set in `NewUINameWindow.cpp`) — max 32 label-builds per frame — and LRU-pruned via `PruneGroundItemLabelCache`.

### SDL3 status

The cache uses Windows GDI (`FillRect`, `GetTextExtentPoint32`, `TextOut`) and raw OpenGL (`glGenTextures`, `glTexImage2D`) — neither available in the SDL3/macOS build. On SDL3 the three cache functions (`CreateGroundItemLabelTexture`, `RenderGroundItemLabelTexture`, `DeleteGroundItemLabelTexture`) are gated behind `#ifdef MU_ENABLE_SDL3` and stubbed; `RenderItemName` has a fallback path that calls `g_pRenderText->RenderText()` directly per frame.

**Perf implication:** in dense-drop scenes on macOS/Linux, expect slightly lower FPS than Windows. See `docs/ground-item-label-sdl3-cache-story-draft.md` for the follow-up story that restores this on SDL3.

### Expected behavior (both platforms)

- Ground-drop labels render above items with correct name, color per rarity, background for excellent/ancient.
- Windows: stable FPS even in dense piles.
- SDL3/macOS: labels render correctly; FPS may dip in piles of 50+ drops.

---

## Devil's beam effect positioning — PR #323

**Source PR:** `itayalroy/fix_beam_effect`
**Ported file:** `src/source/RenderFX/ZzzEffect.cpp`
**Status:** ✅ ported

### Bug fixed

The devil-summoned beam effect spawned at the wrong position (too close to caster, wrong elevation). Multiple position calcs were off — a mix of missing offsets and wrong RNG ranges.

### Fix

Adjusted position deltas: `Position[0] += rand()%100 + 200` (was smaller range), `Position[2] += 130.f + rand()%32` for mid-effect, `Position[2] += 400.f` for end-effect, etc.

### Expected behavior

- Summon devil → cast its special → beam effect appears at the correct spatial location relative to the summon, not under its feet.

---

## Miscellaneous bug fixes in the merge (no dedicated PR reference found)

### Random.cpp cleanup
- **File:** `src/source/Core/Random.cpp`
- Removed `Engine31` adapter struct. Uses `std::mt19937` directly for `RangeInt`. No behavior change on MSVC; suppressed a clang-tidy narrowing warning and a minor runtime cost.

### ZzzEffectJoint VectorScale double-scaling
- **File:** `src/source/RenderFX/ZzzEffectJoint.cpp`
- `VectorScale(p, FPS_ANIMATION_FACTOR, p)` followed by `VectorAddScaled(..., FPS_ANIMATION_FACTOR)` applied the animation factor twice on thunder-joint position updates. Main removed the redundant `VectorScale` call.

### GetSkillByBook POISON underflow
- **File:** `src/source/Data/ZzzInfomation.cpp`
- Function `GetSkillByBook(Type)` used `Type - ITEM_SCROLL_OF_METEORITE` as switch key. `ITEM_SCROLL_OF_POISON` sits at `ITEM_ETC + 0`, one slot before `ITEM_SCROLL_OF_METEORITE`, so the subtraction yielded `-1` → `SkillAttribute[-1]` dereference. Main rewrote the function to switch directly on `Type` with explicit `ITEM_SCROLL_*` case labels.

### CNewUICharacterInfoWindow defenseSuccessRate
- **File:** `src/source/UI/Windows/Character/NewUICharacterInfoWindow.cpp`
- The character-info blocking-rate display used `CharacterAttribute->SuccessfulBlocking` directly. Main computes a local `defenseSuccessRate` that includes master-skill additions, and the tooltip strings now show the correct combined value.
- **Expected behavior:** Open character info (C), check the defense-success / blocking rate tooltip. Numbers match actual in-combat blocks, including bonuses from Increase-Defense master skills.

### Monster kind classification refactor
- **File:** `src/source/Gameplay/Characters/ZzzCharacter.cpp`
- The giant `if (Type == X) o->Kind = ...` chain (40+ lines) replaced by a single call `o->Kind = DetermineMonsterObjectKind(Type)`. Helper functions `IsForcedNpcMonsterType` and `DetermineMonsterObjectKind` live in an anonymous namespace. Behavior identical; easier to extend.

### Master experience unlock at level 400
- **Files:** `src/source/Gameplay/Characters/CharacterManager.{cpp,h}`
- New constant `kMasterExperienceUnlockLevel = 400` + method `CCharacterManager::IsMasterExperienceActive(class, level)`.

---

## Files to watch if something seems wrong

If a tester reports a regression, bisect toward these files first — they had the heaviest conflict resolution and are the most likely to have subtle merge errors:

1. `src/source/Gameplay/Items/ZzzInventory.cpp` — 27 conflict hunks. Wing classification, ground-item labels, Lucky Set checks.
2. `src/source/UI/Windows/Inventory/NewUIMyInventory.cpp` — 1500 lines of static helpers removed. Right-click drop flow.
3. `src/source/Gameplay/NPCs/MuHelper.cpp` — 9 ported upstream hunks, 200 lines touched. Any Mu Helper misbehavior.
4. `src/source/Network/WSclient.cpp` — 4 ported hunks including the saturating arithmetic helpers. Exp bar and pickup flow.

## Cross-reference

- Merge commit: `f5d1d73e`
- Include path fix: `1cceb074`
- SDL3 label-cache stub: `1eeac9aa`
- SDL3 cache-restore story draft: `docs/ground-item-label-sdl3-cache-story-draft.md`
