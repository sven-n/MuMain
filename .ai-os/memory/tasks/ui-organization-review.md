# UI file, directory, and namespace organization review

## Review disposition: closed, findings implemented or carried forward

Reconciled on 2026-10-04. The report below remains the pre-change review snapshot.

- Widget guidance, HUD placement and Social view filenames: implemented in
  [organization cleanup](ui-organization-cleanup.md), which records build status.
- Remaining SocialWindowManager type extraction: carried forward to
  [Social manager cleanup](ui-social-manager-cleanup.md) as optional maintenance.
- Legacy structural debt and validated boundaries: retained as documented context;
  no broad restructuring selected.

This closure records disposition, not completion of the optional follow-up.

Date: 2026-10-04. Branch: `dev/rmlui-ui-system`. Reviewed HEAD: `9a56ff680`.

## Structural Summary

The structure is mostly coherent. Feature code generally lives under UI domains;
shared RmlUi integration and rendering have distinct homes. The main problems are
historical placement and a few migration naming inconsistencies, not systemic
directory duplication. No HIGH severity organizational issue was established.

This review covers the current tree, not just recent changes. Inventory: 393 tracked
files under `src/source/UI`, 12 under `src/source/Render/RmlUi`, and 434 under
`src/bin/Data/Interface/RmlUi`. Manual review concentrated on suspicious overlaps,
declarations, consumers, registration, and build paths; this is not a line-by-line
review of every file or a complete audit of non-UI subsystems.

The renderer's actual path is `src/source/Render/RmlUi`, not `src/Render/RmlUi`.
Recent commits already moved friend/mail/chat-room code from Party to Social,
renamed the old UIControls/UIWindows files, and separated concrete social-window
headers. Those changes are credited here rather than recommended again.

## Structural Map

Paths below are relative to `src/source` unless otherwise specified.

| Location | Observed responsibility |
|---|---|
| `UI/Inventory`, `Combat`, `Events`, `NPCs`, `Quests`, `Character`, `Options` | Feature controllers, local models and helpers |
| `UI/HUD` | Persistent screen UI, chat/system log windows, notice ticker, and skill helpers |
| `UI/Chat` | Chat text dispatch, commands, and whisper helpers |
| `UI/Party` | Party list/info windows and layout/model helpers |
| `UI/Social` | Friends, mail, chat rooms, their local manager/base, and portrait integration |
| `UI/Dialogs` | Modal and message-box infrastructure and implementations |
| `UI/Windows` | Closed legacy scene/window cohort, including login, server selection, credits and system menu |
| `UI/Widgets/Window` | Transitional controls, plus two misplaced complete windows |
| `UI/Widgets` | Residual sprite Button, text search, and UI definitions |
| `UI/Core` | Window lifecycle, dispatch, scene coordination, geometry and legacy open/close policy |
| `UI/RmlBridge` | Shared model binding, themed loading, document state, input and geometry helpers |
| `UI/Scaling` | Shared UI coordinate transforms |
| `Render/RmlUi` | Context runtime, rendering/system adapters and renderer-specific decorators |
| `MUHelper` / `UI/MuHelper` | Bot engine/configuration data / its settings UI |
| `src/bin/Data/Interface/RmlUi` | Shared documents/templates; theme directories own styles and optional document overrides |

## High-Value Structural Issues

### [MEDIUM] Two complete windows remain under reusable widgets

**Category:** Misplaced.

**Locations:** `src/source/UI/Widgets/Window/{ChatInputBox,SlideWindow}.{h,cpp}`;
`src/source/UI/HUD/{ChatLogWindow,SlideTicker}.{h,cpp}`.

**Current structure:** Full chat-input and notice windows sit beside generic Button,
ScrollBar and Tooltip implementations.

**Evidence:** Both derive from `mu::ui::window::CObject`, register through
`CManager::AddUIObj` (`ChatInputBox.cpp:78`, `SlideWindow.cpp:35`), and load their own
`chat_input.rml` / `slide_notice.rml` documents. `WindowSystem.cpp` constructs them
as windows. ChatInputBox receives chat/system-log windows and owns message modes,
history and send behavior. SlideWindow wraps the HUD's SlideTicker. These are not
generic text-entry or sliding-container controls.

**Problem:** Developers following a widget example can copy a whole feature window;
developers changing HUD chat or notices must search across unrelated locations.

**Recommended structure:** Move these two header/source pairs to `UI/HUD`, beside
their existing log/ticker peers. Keep class names and namespaces. This is the
smallest correction under the current HUD organization. A later decision to group
all chat presentation under `UI/Chat` should move that complete cohort together,
not create another partial split now.

**Migration impact:** Four file moves; includes in their implementations,
`UI/Core/WindowSystem.h`, and `UI/HUD/ChatLogWindow.h`. CMake recursively discovers
client sources with `CONFIGURE_DEPENDS`; preserve document paths, interface IDs,
registration and namespaces. Recheck documentation and all path references at
execution time, then compile/link. No behavior redesign required.

**Confidence:** High.

### [MEDIUM] Placement guidance still points new work toward a transitional toolkit

**Category:** Ambiguous Boundary.

**Locations:** `docs/rmlui-ui-system/building-new-ui.md`, especially the introduction,
quick decision guide item 3, and naming discussion.

**Current structure:** The introduction correctly requires RmlUi/base.rcss for
visible presentation, but item 3 says to default to the `mu::ui::window` widget
family. The naming discussion also calls that family's Button/RadioButton the
ones to use for new work.

**Evidence:** These instructions coexist with item 4's correct feature-domain
placement and the governing architecture policy. The same document explains that
native controls are transitional. Its namespace warning still discusses a global
CRadioButton declared by SocialWindowCore, although that class was retired.

**Problem:** The guide provides competing answers to where new reusable UI belongs
and which generation to copy. This can recreate organizational debt even while
existing directories remain unchanged.

**Recommended structure:** Retain the directories. Make the quick guide and naming
examples consistently say RML/RCSS first, and scope native-widget examples to
documented native companions. Remove the obsolete RadioButton collision example.

**Migration impact:** Documentation only. Preserve the CObject registration guidance;
it describes object lifecycle, a different choice from presentation widgets.

**Confidence:** High.

### [LOW] Social view filenames hide their distinction from window adapters

**Category:** Misleading Name.

**Locations:** `src/source/UI/Social/{ChatRoom,LetterRead,LetterWrite}.{h,cpp}`;
`FriendWindowView.{h,cpp}` and the corresponding `*Window` files.

**Current structure:** ChatRoom.h declares ChatRoomView; LetterRead.h declares
LetterReadView; LetterWrite.h declares LetterWriteView. FriendWindowView.h declares
the plural FriendWindowViews coordinator. Adapter declarations now have their own
ChatRoomWindow/LetterReadWindow/LetterWriteWindow headers.

**Evidence:** Each of the three View classes implements Rml::EventListener and is
included by its corresponding window implementation. FriendWindowViews synchronizes
a list of windows and is included by SocialWindowManager.cpp.

**Problem:** Searching by class name or choosing between a view and its adapter
requires remembering filename exceptions. The singular coordinator filename is
particularly easy to mistake for one window's presentation.

**Recommended structure:** Rename the three pairs to `ChatRoomView`, `LetterReadView`,
`LetterWriteView`, and the coordinator pair to `FriendWindowViews`. Keep them in
Social. FriendShell already matches its class name; do not rename it merely for symmetry.

**Migration impact:** Eight file renames, direct include updates and a comment in
FriendWindow.cpp; no class or namespace changes. Client sources are globbed;
recheck explicit test/build references and compile after renaming.

**Confidence:** High.

### [LOW] SocialWindowManager still contains several separately meaningful types

**Category:** Legacy Organization.

**Locations:** `src/source/UI/Social/SocialWindowManager.h/.cpp`.

**Current structure:** The header declares CFriendList, CLetterList, CUIWindowMgr,
and CUIFriendMenu alongside manager enums.

**Evidence:** Declarations begin at header lines 39, 66, 113 and 240. The friend and
letter types own collections; the menu derives from CUIBaseWindow. These are
distinct roles, unlike a manager's small private helper struct.

**Problem:** The filename conceals a menu implementation and list containers even
after the larger old UIWindows grab-bag was split.

**Recommended structure:** On the next focused change, extract FriendMenu first,
then FriendList/LetterList if they are being changed. Keep all under Social and
preserve the established global class names. Do not redesign the manager here.

**Migration impact:** Declaration/definition and include changes, plus dependency
review for shared enums and packet types. More churn than a simple file rename;
not justified as an automatic cleanup in this review.

**Confidence:** High.

## Similar / Overlapping Directory Review

| Pair/group | Conclusion | Evidence and reason |
|---|---|---|
| `UI/Windows` vs `UI/Widgets/Window` | **Move selected files** | Most of the latter are reusable transitional controls; the former contains complete legacy-cohort windows. Move the two exceptions above; do not consolidate both directories. |
| Two `Button` pairs | **Keep as-is** | Global CButton derives from CSprite; namespaced CButton belongs to the CBaseButton family. They are separate APIs/generations. The global header still has a scaling-test consumer (`tests/ui/test_ui_scaling.cpp:453` instantiates CButton), so zero production consumers does not mean deletion is a path-only cleanup. |
| `UI/Party` vs `UI/Social` | **Keep as-is** | Party now contains party list/info code; Social holds friends/mail/chat rooms. Recent moves resolved the prior misleading boundary, including namespace changes to UI::Social. |
| `UI/Chat` vs Social chat rooms vs HUD chat logs | **Keep as-is** | Chat helpers route gameplay messages and whispers; social rooms have their own window identity and connection; HUD owns persistent presentation. No evidence supports merging social rooms into gameplay chat. Revisit HUD/helper grouping only as a complete chat-placement task. |
| `UI/RmlBridge` vs `Render/RmlUi` | **Keep as-is** | Bridge helpers bind models/load documents/sync transforms. Runtime implements Core::Input::IUiInputConsumer and owns context/render integration. Similar technology names do not imply duplicate ownership. |
| `MUHelper` vs `UI/MuHelper` | **Keep as-is** | CMuHelper runs the bot; CMuHelperConfigWindow and UI::MuHelper's staged settings serve its UI. Case/name differences do not justify consolidation. |
| `Core/Input` vs `Input` | **Keep as-is** | Core holds event/key/IME/routing primitives. Input::Selection resolves game entities under the cursor. Distinct abstraction levels were verified in headers. |
| Shared RML vs theme directories | **Keep as-is** | RmlTheme.cpp tries a theme document override, falls back to shared markup, and resolves styles against the theme source URL. The duplication is part of resource lookup, not an accidental second source tree. |
| PhotoViewer vs PhotoViewerControl | **Keep as-is** | CUIPhotoViewer owns native 3D portrait content; UI::Social::PhotoViewerControl receives RmlUi gestures. Composition explains the separate files. |

## Namespace Review

- `UI::Social`, `UI::Party::List`, `UI::MuHelper`, and `UI::RmlBridge` match their
  conceptual domains. Recent Social relocation already corrected the old Party
  namespace; do not report it as outstanding.
- `mu::ui::window` spans Core, feature windows and transitional widgets. It denotes
  an established object family, not a directory named Window. Moving a file does
  not require renaming its symbols.
- `UI::Skills` under `UI/HUD/Skills` is explicitly supported by CODING_RULES:
  namespaces describe the domain rather than mechanically mirroring folders.
- Render::RmlUi free-function helpers coexist with global RmlUiRuntime/interface
  classes. Existing global C* adapters likewise coexist with domain-scoped models
  and views. These are historical conventions; no high-value namespace-only
  migration was established.
- Do not add a third namespace convention for the proposed file moves. The coding
  rule for new extracted free functions is narrower than a mandate to modernize
  every existing class.

## Recommended Refactors

1. Correct the conflicting new-UI placement/toolkit guidance.
2. Move ChatInputBox and SlideWindow beside their HUD peers in one focused batch.
3. Rename the Social view/coordinator files to match their actual classes.
4. Split remaining SocialWindowManager types when working on those responsibilities.

No source moves, namespace changes or resource moves were applied in this review.
The output is an evidence-backed review; avoid turning it into another broad migration.

## Legacy Structural Debt

- `Guild/GuildInfoWindow` and `Guild/GuildMakeWindow`, and `GameShop/InGameShop`,
  remain outside UI while using CObject and RmlUi. GuildInfoWindow registers with
  the same window manager as UI features. These locations group whole legacy
  domains; a future split must preserve adjacent data/transport dependencies.
  Record them as exceptions rather than automatically moving entire subsystems.
- UIManager and WindowManager have confusingly broad names, but distinct roles:
  CUIManager implements legacy open/close/mutual-exclusion policy; CManager
  registers objects and dispatches update/render/input. Do not merge them because
  both are managers. SceneUICoordinator is a separate scene lifecycle owner.
- UI/Core is broad but the sampled files have shared UI lifecycle/geometry roles.
  This pass did not establish a reason to split it into more directories.
- Events and Inventory are relatively large (57 and 44 tracked files), but size
  alone does not justify deeper folder trees. No arbitrary per-class directories.

## Validated Existing Structure

Keep feature-local models beside their windows, the Social/Party separation,
the backend/bridge split, the independent Scaling concern, and the theme override
hierarchy. Preserve paired headers/implementations. No new public/private include
tree or centralized directory for every RmlUi feature is needed.

## Proposed Organizational Rules

1. Put feature windows, models and local helpers beside their domain peers;
   UI/Windows is a closed historical cohort, not the destination for all new windows.
2. Reserve Widgets for feature-independent controls. A registered window with
   its own gameplay actions/document belongs with the corresponding feature or HUD.
3. Keep shared UI integration in RmlBridge and renderer/context adapters in Render/RmlUi.
4. Name new class files after their significant class; preserve useful View/Model/Window
   distinctions without renaming legacy class APIs solely for uniformity.
5. Use domain namespaces for new free-function extractions as CODING_RULES specifies;
   preserve established class-family namespaces during file moves.
6. Treat shared/theme resource paths as a loading contract. Preserve override/fallback
   behavior and run asset checks for any resource reorganization.

## Validation Performed

- Enumerated tracked UI, renderer and asset files; searched includes and namespaces
  across tracked source/tests/build files, including older code.
- Inspected suspicious headers, inheritance, adapter/view connections, CSystem
  construction, AddUIObj registration, theme override lookup and relevant consumers.
- Checked `src/CMakeLists.txt`: client source glob uses CONFIGURE_DEPENDS; reviewed
  explicit RmlUi test-source references. This review does not assume globbing makes
  all future path changes safe.
- Ran Windows `cmake --build out/build/windows-x64 --config RelWithDebInfo` through
  the existing VS setup wrapper. Exit 0. Incremental build; no C++ compilation/link
  was required. Log: `out/build/windows-x64/ui-organization-review-build.log`.
- Build checks passed: RML/RCSS syntax (176/254), geometry ownership (176 RML),
  theme contract drift (87 documents/124 files), shader staging, and renderer guard.
- The VS wrapper emitted a `vswhere.exe` lookup diagnostic before CMake ran.
  The successful incremental result does not prove a fresh compiler setup or clean
  rebuild. No compiler-environment change was made for this documentation-only review.
- BUILD_TESTING is OFF; no CTest, interactive game, alternate-platform or clean
  rebuild validation was performed. No behavioral changes, commits or pushes.
