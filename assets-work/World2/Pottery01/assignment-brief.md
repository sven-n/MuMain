# Read-only Dungeon readiness and proposed first family

No production authorization assumed; no game or repository asset changes. Inventory: /tmp/astra-dungeon-readiness.json. Official imported preview sheet: /tmp/astra-dungeon-views/readiness-sheet.jpg; six separate renders and packed preview blends in that folder. These are offline previews, not client verification.

Engine loading: MapManager.cpp1121–1129 maps WorldActive1 to Data/Object2 and zero-based object type i to Object(i+1).bmd, with textures resolved from Object2. All63BMDs inventoried (60 numbered + Bat01/Rat01/DungeonStone01). EncTerrain2.obj decoded read-only into4488 records,61 types. All29 distinct referenced texture filenames resolve locally (see exact inventory count). Nine type60 records have no Object61; ZzzObject.cpp4680 defines that hidden interactive pose marker. Do not create a missing model.

Static exclusions / special contracts:
- Bat01 and Rat01 are animated boids; DungeonStone01 is a spawned falling effect. MapManager.cpp53–57 loads these separately.
- Object12,23,24,25,36,54 have30/40-key animated actions. Exclude from initial static batch;23–25 also scrolling mesh1 in ZzzObject.cpp3899–3905.
- Object40/41/52 are hidden trap markers (types39/40/51), ZzzObject.cpp3912 and5110. Object41 also action1 four keys and loader override PlaySpeed .4. Exclude.
- Object53 is hidden falling-stone emitter (type52), ZzzObject.cpp3894.
- Object60 is interactive type59, ZzzObject.cpp4677. Keep out of first static pass.
- Object42/43 have wood plus fire0a meshes and fixed runtime flame anchors (types41/42, ZzzObject.cpp3906–3911). A later hybrid batch must freeze shell and anchor contracts.
- Alpha cobweb/steel sheets Object02/18 should be classified separately from opaque structural geometry.

Recommended bounded family: Object28/29/30 pottery,45 placements total (24+11+10), all one bone/one one-key action. Triangles516/78/288. Actual previews show angular vase shoulders, thin/coarse mouth rims, and broken pots that lack clear ceramic wall thickness. Start one standalone Object29 prototype: intentional continuous shoulder/neck curve, believable lip/throat thickness using the existing frozen painted atlas; preserve footprint, root, extrema and ground contacts. If clear actual-export reduced gain passes, propagate only demonstrably matching pot parts into Object28 composite and improve fracture thickness in Object30 while preserving broken contours/contacts. Keep deep_wall04 secondary meshes exact until identified in geometry. No automatic copying of all variants; Wells04 demonstrated distinct variant scales must retain their own contracts. Full source/export triangle, material/UV/bone/winding and world-normal proof required.

Texture dependency isolation: flower_vase.jpg is shared only by28/29/30. Freeze flower_vase.OZJ initially. Their deep_wall04.jpg secondary texture is shared by06/07/08/09/10/13/14/15/16/28/29/30/40/52 and must stay frozen. Record exact hashes from inventory before any later authorized work. Preserve model-name bytes: legacy Korean non-UTF8 names make official importer's UTF8 manifest-comment reader fail. Read-only preview shim parses only ASCII action records while delegating all geometry/texture work to official importer; no exporter/converter modification was made. A production pipeline must explicitly solve original name-byte preservation, not silently replace it.

Alternative first pair: Object21/22 open coffin and lid,53 placements (33+20),40/20triangles, one root/action/key. They are obvious slab construction in actual preview; meaningful board wall thickness, joints and shaped lid/bevel could pay off. wood01.jpg has8 consumers19/20/21/22/35/42/43/44; texture must remain frozen. Object35 is a separate upright bucket8placements76tri, not a third coffin. Model families must follow actual geometry rather than shared texture names.

Large modular architecture Object01 (1262placements),04(588),06/13(229),15(217) offers broader exposure but has wall contacts and shared atlas dependencies. Defer until a small family establishes Dungeon appearance. Bones/scattered bones Object47/48 and rocks49/50 are another later family; do not infer visual defect merely from placements/triangle counts.
