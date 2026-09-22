# Architecture03 independent review

2026-09-22. Reviewer: ASTRA independent reviewer, `codex/lorencia-monuments`.
Reviewed worker commit `e8fef936736ce8bb2d4c54d0ae910c52c5eb7d3b` read-only in
`/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-architecture-final`.

**ACCEPT for offline integration.** The three remodeled pieces retain their identity and assembly
boundaries. Six coordinated paintings cover all 20 consumers. The 17 accepted compatibility BMDs
remain byte-identical to the pinned baseline. No blocker found; real client observation remains pending.

## Fresh independent checks

- Eighty converter validations pass: original and final reference/action SMD pairs for all 20 models.
  Complete comparisons reproduce three intentional DIFFERENT results and 17 EQUIVALENT results.
  Every reference SMD header and complete action SMD pair is byte-identical, including House04's
  40 keys, House05's 30 keys and Tent01's 36 keys. Bone names, order, hierarchy, bind transforms
  and all local action records are preserved.
- Three final game BMDs match their delivered exports. All 17 compatibility BMDs match their
  archived baseline and installed source-checkout BMDs byte for byte. Every archived BMD matches
  pinned baseline `41f28d5bd29530b2a6957e5024df3265955026cd`.
- Raw BMD records independently confirm zero vertex/normal cross-node pairs in all three authored
  final models. Every readonly raw binding record matches its original; exact BMD identity also
  protects behavior that expanded SMD normals alone cannot establish.
- Independent read-only Blender inspection confirms packed images and hidden, correctly marked
  REF_ORIGINAL collections in all 20 source files. Authored new panel triangles were independently
  aligned against final SMD triangles with exact material, intended bone and UV correspondence:

| Model | Triangles before / after | New panel triangles checked | Maximum authored/export position component delta |
|---|---:|---:|---:|
| HouseWall03 | 20 / 36 | 20 | 0.000007201660 |
| Bridge01 | 196 / 276 | 100 | 0.000015273437 |
| BridgeStone01 | 74 / 106 | 40 | 0.000930866211 |

The documented 0.002-unit authored/export tolerance accommodates the timber bridge's floating-point
transform reconstruction. It does not relax protected original connections. Independent checks retain
every original corner on its original bone, with maximum component delta 0.000031 across all 20 models.
All 3,663 unmodified original triangles retain UVs within 0.00001. Fresh extractions of the three final
BMDs are byte-identical to their archived validation SMDs. The supported post-export SMD/manifest
packaging restores protected corner records and original rig/actions; raw official intermediates remain.

## Modular geometry and texture scope

Reviewed explicit panel-face selection and perimeter-preservation code, modular-anchor reports, full
comparisons, local matrix/posed-bound reports and final contracts. Recesses only affect selected face
interiors. Original end planes, arch openings, beast heads, log ends, braces, shadow meshes, doorway
contacts and all original perimeter vertices are retained. Placement, collision and terrain files remain
byte-identical; no visual compensation through gameplay-data changes occurred.

The full-suffix dependency check confirms `tree_04.jpg` resolves to `tree_04.OZJ` for BridgeStone01
and Tree07 only. Tree12's distinct `tree_04.tga` / `tree_04.OZT` foliage is unchanged. Other complete
paint consumer groups match dependency-ownership.json and are contained in these 20 reviewed models.
All 72 delivered dependency containers pass a fresh mu_texture loader check. The original
`bridge_shadow01.OZT` and every other unowned game container remain byte-identical. Six paintings
are RGB: bridge_01 is 1024 by 512 and the other five are 512 by 512. Existing alpha/effect materials
retain their original containers, material slots and UVs.

Exactly nine game paths differ: Object1/HouseWall03.bmd, Bridge01.bmd, BridgeStone01.bmd,
bridge_01.OZJ, tree_04.OZJ, tile_ston06.OZJ, tile_wood02.OZJ, tile_ston04.OZJ and tile_02.OZJ.
All 315 other tracked World1/Object1 files independently match the pinned baseline byte for byte.

## Visual evidence and limits

Viewed the three individual original/export comparison sheets with wireframes and reduced-scale views;
batch-review.jpg; both compatibility overviews covering all 17 readonly consumers; texture-comparison.jpg;
House04, House05 and Tent01 action sheets; and the stone-bridge, log-bridge, west-door and town-interior
assembly comparisons. The original double arches, projecting beast heads, paired logs and doorway
clearance remain recognizable and connected. Shared timber, masonry, end grain and forged iron keep
consistent muted material roles. The black bridge-shadow planes are present in both baseline and final
views and retain their original alpha gradient; this review makes no new client blend claim.

Evidence resides under assets-work/World1/Architecture03 in the reviewed worktree. Assemblies use
unchanged World1 placement transforms. Camera, flat terrain/figure proxies, additive effects, scrolling
and roof fade are explicitly offline approximations; terrain, collision, lighting and unrelated assets
are omitted from those previews. No runtime installation or client session was performed. Client
verification remains pending. Source checkout was read-only; only this review document is committed.
