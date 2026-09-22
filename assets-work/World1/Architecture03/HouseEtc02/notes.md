# HouseEtc02 — low masonry annex

Owner: ASTRA architecture worker. Branch `codex/lorencia-architecture-final`.
Date: 2026-09-22. Offline production and validation complete; real client verification pending.

Actual World1 identity: low masonry annex; 1 placements. Exact positions, rotations, scale and tile coordinates are retained in `original/placements.json`.

250 → 250 triangles. Accepted BMD stays byte-identical to the pinned current baseline; only shared painted texture containers change. All authored recesses are 0.55 units deep, constrained inside existing face boundaries. Every original vertex, origin, orientation, silhouette extremum and modular anchor remains retained. Selected face pairs and perimeters are listed in `validation/blender.json`. This is conservative structural edge refinement; the shared painting pass supplies the principal visual change.

Bind bounds before: `[[-302.53, -299.57, -0.15], [297.88, 408.2, 332.48]]`.
Bind bounds after: `[[-302.53, -299.57, -0.15], [297.88, 408.2, 332.48]]`.
Material slots, unchanged order: `['tile_wood02.jpg', 'tile_ston04.jpg', 'c_wall04.jpg', 'tile_wood03.jpg', 'tile_house01.jpg', 'tile_ston01.jpg', 'horse_drawn_01.jpg', 'tile_ston06.jpg']`.

Every texture dependency and image dimension:

- `c_wall04.jpg`: [512, 512], RGB
- `horse_drawn_01.jpg`: [256, 256], RGB
- `tile_house01.jpg`: [512, 512], RGB
- `tile_ston01.jpg`: [512, 512], RGB
- `tile_ston04.jpg`: [512, 512], RGB
- `tile_ston06.jpg`: [512, 512], RGB
- `tile_wood02.jpg`: [512, 512], RGB
- `tile_wood03.jpg`: [512, 512], RGB

Shared painted families are bridge_01 (stone arch/coping and beast relief atlas with isolated orb region), tree_04 (cut log end grain), tile_ston06 (left-half vertical timber, upper-right ornamental crest, lower-right plaster), tile_wood02 (horizontal weathered oak boards), tile_ston04 (large limestone/plaster masonry) and tile_02 (riveted blackened iron). Filenames, normalized atlas layouts and existing UV/material roles are retained. bridge_01 is 1024×512; the five square paintings are 512×512 RGB. All other dependencies retain exact current-baseline bytes. `bridge_shadow01` is the unchanged 2×32 black RGBA gradient, alpha 8–255, with original container padding and geometry. The prior accepted awning alpha remains exact where consumed.

Frozen texture hashes: `validation/modular-anchors.json`. Shared consumer ownership: `../dependency-ownership.json`.

Exact rig/action contract retained:

```text
bone 0: "ihix01" parent=-1
bone 1: "Box03" parent=-1
bone 2: "Mesh03" parent=-1
action 0: keys=1 lock=0
```

Full comparison: **EQUIVALENT**. Remodeled geometry is intentionally DIFFERENT; all 17 readonly BMDs compare EQUIVALENT and remain byte-identical. Skeleton plus actual action comparison: EQUIVALENT. Bone names, order, indices, parents, bind transforms, action count/key count/lock and every local key transform are unchanged. Local samples: `[{'file': 'HouseEtc02.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}, {'file': 'HouseEtc02_a00.smd', 'bone_samples': 3, 'frames': 1, 'max_position_component': 0, 'max_euler_component_radians': 0}]`. Hierarchical action matrices checked: 3; maximum matrix delta 0; maximum posed-bound delta 0.

Every original corner survives with maximum component delta 0.0. 250 protected triangles retain positions, bone bindings, UVs and normals; maximum protected UV delta 0, normal direction delta 0 degrees. `validation/authored-bindings.json` checks new panels against the final BMD for intended bones/positions/UVs where this is an authored model. `validation/raw-bone-bindings.json` checks actual raw vertex/normal bone ownership; readonly results remain exactly original.

Engine contracts: Bridge01 retains CollisionRange -50 and fires at local (90,-200,30) and (90,200,30). Shared-material consumers retain House03/HouseWall02 light_02 mesh4 additive surfaces, House04 tile_space01 mesh8 V-scroll, House05 ston02 mesh2 V-scroll, and HouseWall05/06 HeroTile4 fade. Mesh indices, geometry and UVs of those effect surfaces remain original. `../engine-contract.txt` contains source excerpts. No flame particles are manufactured as client evidence.

Sources: untouched BMD, all original containers, unwrapped images, bmdconv info/SMD/action/manifest, official-import packed blend and placements in `original/`. Packed `source.blend` retains export-excluded REF_ORIGINAL, original rig/actions and an editable REF_HIGH_POLY candidate. Final editable PNG and game JPEG/TGA images are in `textures/`; final BMD and complete containers in `exports/`. Exact built-in imagegen prompts, unchanged generated masters and reproduction metadata are in `../paintings/`.

Official mu_bmd_export.py exports authored sources. Supported bmdconv SMD/manifest packaging then restores original protected corner records, panel-boundary coordinates and bind/action data to avoid Blender roundtrip quantization. Authored interior geometry and bevel normals remain. Raw official exports and final packing inputs/logs are retained, and their full compare is EQUIVALENT. Readonly BMDs bypass export and are copied exactly. Original-versus-remodeled comparison remains DIFFERENT.

Evidence: full validation/info/compare, isolated rig comparison, local pose/matrix/posed-bound, raw binding, material/UV/alpha, ownership/hash and source packing logs in `validation/`. `review/comparison.png` shows matching-camera original versus actual reimported final BMD, wireframe and reduced-scale previews. Animated readonly models retain additional three-pose sheets. Neutral diffuse versus labeled additive approximations are retained for indexed effects. Actual World1 placement assemblies are in `../review/`.

No shared runtime or client was used. Blender camera, lighting, additive effects, illustrative scroll offsets, roof omission, grass and 190-unit figure are explicitly offline approximations. Pending: actual client loading, lighting/filtering, alpha blending/scroll/fade, animations, bridge traversal/collision, flame placement and matching 1920×1080 client screenshots. Terrain placement, height, walk data, baked lighting and alpha strips remain unchanged.
