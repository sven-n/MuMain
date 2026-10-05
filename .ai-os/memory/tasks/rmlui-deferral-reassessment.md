# RmlUi tracked-deferral reassessment

Date: 2026-10-04. HEAD: `3309b4ba8`, plus current working tree.
Scope: `docs/rmlui-ui-system/tracked-deferrals.md`, checked against source, markup,
allowlist and recorded validation. Existing uncommitted Social edits were inspected
where relevant and left untouched. This is a source/document review, not runtime sign-off.

## Follow-up on 2026-10-05

The original recommendation below is a dated snapshot, not the current work queue.
Current source and the completed rollout show:

| Original priority | Current disposition |
|---|---|
| 1. Item-hotkey slot geometry | Resolved in `b22326020`: the RmlUi slot boxes supply icon geometry and interaction. Runtime scale checks remain useful validation. |
| 2. Theme-driven dock placement | Resolved by the workspace placement rollout through `e789fb469`: both themes place docks, panel-stage windows and the HUD shell from slots. Player docking was explicitly deferred. |
| 3. Letter portrait compositing | Resolved in `2e7eb2e2d`: the portrait is rendered into an offscreen target used inside the letter document. Inventory paperdoll/tooltip ordering is a separate issue. |
| 4. Remaining native presentation | The bounded quest, Catapult and Castle/Guard arithmetic was removed in `d5960f116`. Gens Ranking's native scrollbar was replaced by an RmlUi scroll pane in the current batch; its native text wrapper remains for line breaks. MU Helper threshold-gauge input remains the main constrained interaction case. |

The broader event-only, theme-switch, drag/restore and item/skill-use checks are
still validation work. The low-priority CObject registry and Social base are still
present. The next interaction candidate is MU Helper's threshold gauges, but the
stock RmlUi slider still maps positions incorrectly inside a transformed dock.
That integration constraint needs a tested solution before replacing its native input.
Gens Ranking also needs an in-game scroll check in both themes and at a non-100 % scale;
the build and asset guards passed, but no Gens account was exercised.

Other pilot claims checked against current source:

- The modern buff tooltip no longer contains the hardcoded white colour. Buff
  right-click cancellation remains deferred.
- The old `GenericConfirmDialog` input-field constants and the System Log
  `back_color` binding are gone.
- Gens and United Marketplace no longer have `display-list port` allowlist reasons.
  The current guard reports 174 RML documents and 76 allowlisted entries.
- Inventory paperdoll paint order and the item-tooltip overlap remain separate
  native/RmlUi layering issues. The letter portrait fix did not settle them.
- Counter-scaled sharp text still uses bound geometry; the pinned RmlUi engine's
  adoption question for stylesheet arithmetic remains open.

The detailed tables below preserve the original 2026-10-04 evidence. Treat their
`Current assessment` cells as historical, using this follow-up for present status.

## Original recommendation (2026-10-04 snapshot)

Start with **item-hotkey slot geometry ownership** as a bounded implementation task.
It removes a demonstrated theme limitation without replacing native 3D rendering.
Then design **theme-driven dock spacing and position restoration** as the larger
architecture task. Develop **3D content compositing inside RmlUi** as a separate
renderer initiative with a portrait proof of concept first.

Run targeted validation alongside each change. The broad missing runtime coverage
is a release-confidence priority, not evidence that every untested screen is broken.

## Original priorities (2026-10-04 snapshot)

| Order | Work | Why / scope |
|---|---|---|
| 1 | Item-hotkey slot geometry | `MainFrameWindow.cpp:1365` still renders items at `10 + i*38, 443, 20x20`. Read the corresponding RmlUi slot geometry and reconcile hover/click geometry in the same task. Validate all four slots at multiple scales/resolutions. Keep native 3D icons. |
| 2 | Theme-driven dock placement | `WindowSystem.cpp:20` still fixes column width at 190; PanelColumnX feeds creation of many windows before document layout. Themes can resize a panel but cannot independently set its neighbouring column positions. Design first-layout placement, resize/reflow and saved-position precedence together. Highest broad theming benefit, with higher interaction risk. |
| 3 | Native 3D content within document stacking | Letter portraits render after the whole main RmlUi context; family-front selection cannot place them below unrelated covering windows. A renderer-backed texture exposed to the document could solve this. Prove one portrait's lifecycle, clipping, depth and performance first; do not assume inventory icons/paperdoll automatically follow. |
| 4 | Remaining native interaction and presentation ownership | GensRanking's mirrored scrollbar; MU Helper threshold gauges; bounded tab/row index arithmetic. Pick one screen at a time. MU Helper has a real transform/slider integration constraint, so it is not a stock-control substitution. |
| Parallel | Runtime validation and parity decisions | Exercise event-only windows, theme switching while open, drag/restore across scale changes, and actual item/skill activation at scale extremes. Record results per scenario. Review original-behavior differences separately from defects. |
| Opportunistic | Small styling/dead-code cleanup | Modern buff tooltip token; quest heading bold derivation; unused generic input constants. Low effort, low architectural benefit. |
| Low | Naming, class/file splits and old bases | MainFrame file split, root-field spellings, SocialWindowCore base dissolution and CObject/INTERFACE registry replacement. No demonstrated benefit justifies wholesale rewrites now. |

## Main tracked entries

| Entry | Assessment | Evidence / disposition |
|---|---|---|
| CObject adapter tier and INTERFACE lookup | **Still present; rationale stale.** | CObject/CManager and WindowSystem's per-window registry remain. The text's “only 2 data points” and “~88 still-unported” rationale is obsolete after the migration. Keep the registry question, but require concrete extension/lifecycle requirements before replacing it. |
| MainFrame rename / split | **Rename resolved; split remains.** | MainFrameWindow.h declares CItemHotKey, CSkillList, CMainFrameWindow. CSkillList has RmlUi icon integration now; “fully legacy Phase 2” is stale. File splitting is maintainability work, not a theme enabler. |
| CUIControl residual bases | **Valid, low priority.** | SocialWindowBase/PhotoViewer still derive from CUIControl; their identity, state and queue remain used. Do not reopen the entire retired widget-family project. |
| Audit departures from original behavior | **Still valid as an audit initiative.** | Recorded choices span shared scrollbar appearance, highlight painting, reward preview interaction and MU Helper behavior. This pass does not re-certify every parity example. Port completion is not a decision review. |
| C++ / RML ownership boundary | **Largely remediated, with specific residue.** | Current guard passes 176 RML documents with 82 allowlisted entries. That number is not a violation count. Static index arithmetic and duplicated slot/dock geometry remain; dynamic/transform bridges require individual justification. Historical remediation lists must not be treated as a fresh backlog. |

## Pilot rows and smaller claims

| Claim | Current assessment |
|---|---|
| MU Helper threshold gauges poll native input | **Valid.** MuHelperDetailWindow calls UpdateGauge. Vendored WidgetSlider combines event positions with absolute offsets/untransformed track dimensions. Preserve this as an integration issue requiring transformed-input tests. |
| Modern buff tooltip hardcodes white | **Valid.** `themes/modern/buff_strip.rcss:236` still says `color: #ffffff`. Token replacement is a small cleanup. Typography-token expansion remains a separate design choice. |
| Buff right-click cancellation / rich tooltip | **Still deferred.** BuffStrip.cpp explicitly records omitted right-click cancellation. Do not infer a behavioral fix from the RmlUi port. |
| CWin-tier restructuring | **Stale wording.** No CWin class or CWin-derived UI declaration found in current UI; the surviving concern is the CObject registry, not a second live CWin tier. |
| No validation matrix exists | **Resolved as an artifact; coverage incomplete.** validation-matrix.md exists, targets scale/hit-box failures, and records only limited runs. It explicitly excludes resolution, drag-state transitions and hot theme changes. Do not turn missing rows into claimed failures or erase user testing recorded elsewhere. |
| Drag/preference integration unaudited | **Still a valid coverage gap.** The matrix's “only inventory is draggable” is stale: Social's FriendShell installs MakeDraggable and processes drag events. Include the social family in future tests. |
| Main-frame background context | **Still justified for native item icons.** Skill icons have migrated. The background context is a rendering-order accommodation, not a reason to port all items immediately. |
| Item slot origin/pitch duplicated | **Valid, actionable.** Current RenderItems still uses literals. Scope a geometry readback task before considering a renderer rewrite. |
| Two main-frame theme documents lack drift tooling | **Tooling claim resolved.** Theme overrides are intentional and `check_rml_rcss_drift.py` is wired into CMake. Contract checking does not prove visual parity. Keep theme forks where their structures genuinely differ. |
| Actual item/skill use across scale/resolution unverified | **Not closed by this review.** Source cannot establish interactive sign-off; record tested combinations before closure. |
| Inventory paperdoll behind-icon chrome | **Still native.** RenderEquippedItem remains. Evaluate separately after a compositing primitive is proven; do not treat its live 3D content as ordinary RCSS art. |
| Inventory tooltip under RmlUi buttons | **Still a plausible documented limitation.** InventoryCtrl still calls RenderItemInfo and has no OverlayRender registration. Existing overlay infrastructure reduces implementation cost, but this pass did not reproduce overlap or prove a simple callback move preserves frame data/lifetime. |
| Circular orbs / arc visual study | **Optional visual work, not architectural debt.** Keep out of the priority queue unless that visual direction is chosen. |
| Letter portrait stacking | **Valid.** RenderOverlay3D selects the visible front family window and paints it in the post-context overlay. Existing uncommitted transform-scoping edits do not change this ordering. EnsureOffscreenColorTexture is still editor-gated in MuRendererSDLGpu.cpp:3067. A production portrait-texture path remains work. |
| Old FriendWindowView presentation/geometry exception | **Resolved.** Social views replaced the old native transcription layer; no friends/letters/chat-room geometry allowlist entries remain. The obsolete accepted-exception bullet conflicts with the later resolution in the same file. |
| Counter-scale widths and CSS math | **Constraint remains in the pinned engine.** HEAD pins RmlUi `22282190294773798228736e7f1728226563bac9`. This assessment did not check live upstream PR status or validate the fork prototype. Do not schedule adoption from old PR notes alone. |
| Five files can uniformly drop bold because all have semantic style | **Partially valid; uniform-fix claim wrong.** QuestRewardModel::Entry has style plus bold, and ToEntry derives bold from Heading. GenericConfirmDialog/GenericMenuDialog LineEntry instead contain text, bold and color, with no semantic style. Quest cleanup is bounded; generic dialogs need a caller-contract review. |
| GenericConfirmDialog 150x18 native anchor coupling | **Active coupling resolved; dead constants remain.** kInputFieldWidth/Height have declarations only. The document has a stock `gcd_input`; the old `.gcd-input-anchor` is not present in the inspected theme files. Remove stale constants/comments opportunistically, not as a layout refactor. |
| System-log back_color mixes preference and theme | **Old binding gone.** Searches found no back_color/backColor in its implementation or RML. Do not reopen the named binding defect from the historical paragraph. |
| Index arithmetic still affects all listed six documents | **List partly stale.** Guard and Castle tab positions, Catapult lines and Gens description rows still bind index arithmetic. Battle Soccer and server message rows no longer have the cited index-position binding in their current documents. |
| No display-list allowlist labels remain | **Incorrect inventory claim.** Two entries still use that wording: gens_ranking and united_market_place. Gens still mirrors CScrollBar thumb geometry. UnitedMarketPlace's current document has named elements and no scrollbar binding, so its reason is stale. Update reasons after reviewing actual fields; do not classify both as remaining full re-ports. |

## Validation priorities

The tracker explicitly lists unseen CryWolf, siege, Illusion Temple, event timer,
Doppelganger, spectator and Battle Soccer states. CryWolf still gates presentation
on IsCyrWolf1st. No new interactive evidence was generated here. A useful next
validation initiative is a repeatable way to reach these states, followed by
recorded tests; opening a window without satisfying its event state is insufficient.

For architecture changes, test theme layout changes as well as existing themes:
moving a hotkey slot or widening one dock panel is the direct test that ownership
has actually transferred to RCSS. Check click regions and tooltips, not just drawing.

## Minor confirmed bug outside this tracker

User reproduced portrait drag-release failure on the right side: drag outside the
letter window, release left mouse, return without pressing, rotation resumes.
Earlier left-side success does not close this case. Track as a small correctness
fix, independent of the larger render-to-texture proposal; no need to wait for it.
See [portrait follow-up](ui-portrait-drag-release-investigation.md).

## Validation performed and limits

- Source/markup searches plus targeted implementation reads for the entries above.
- Geometry guard passed: 176 RML, 82 allowlisted. Two display-list descriptions remain.
- Read matrix scope/results and CMake's drift-check registration.
- Read current uncommitted portrait-overlay transform change; did not modify source.
- No build, runtime tests or upstream PR verification for this documentation-only assessment.
- Some pilot entries remain evidence/validation tasks, not proven runtime defects.
