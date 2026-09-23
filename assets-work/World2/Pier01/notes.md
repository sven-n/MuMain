# Dungeon carved pier study

Object04 is the 588-placement carved pier between Dungeon wall modules. Work began from the exact current/main `7b808473` model and textures on `codex/dungeon-pier`. Only the pier BMD and this study folder are owned. No game file has been replaced. Object16/17 were inspected read-only as texture consumers; they are entrance structures, not geometry variants of this pier.

## Structural and technical contract

The original model has 90 triangles, one Box01 root and one one-key action. Its raw 32-byte CP949 name is preserved using the same official import/export metadata adapter validated for Dungeon pottery. All geometry processing uses the official Blender/SourceTools pipeline. Original and current BMDs, packed sources and textures are retained, with hidden REF_ORIGINAL geometry in authored sources.

Original faces 0–9 and 82–89 are the protected pillar/backing/cap surfaces. The projecting relief is faces 10–81. The extreme nose tip stays at original world/UV coordinates, including atlas point approximately (32,34) on the 64×128 texture. All 588 placed support envelopes are checked after official export. Seam checks cover every original open-boundary segment with contiguous, nonoverlapping exported intervals, preserving seam UVs, binding and projected winding. Full authored-triangle and raw-normal correspondence, rig/action/name/material order, frozen textures and exact protected surfaces are checked before rendering. Baseline raw bindings, UV areas and corner-normal incidence have no legacy exceptions.

## Geometry studies

The first 576-triangle study used adaptive cubic relief patches and gentle broad brow/cheek changes. Technical checks passed, but the smoother upper face had insufficient visible return at reduced scale. The complete earlier result and scripts are retained in `studies/geometry01`.

The second 487-triangle study changes only 24 upper-mask faces. It retains 66 original faces, including the entire nose tip/underside, lower carving and structural block. It deliberately uses a broad connected brow ridge and recessed angular sockets instead of cubic smoothing. Technical checks and all placed bounds pass. Artist assessment: the stronger brow forms a faceted shelf that interrupts the painted forehead, while reduced-scale improvement remains weak. This candidate is not recommended for installation. Matching actual wall, corner and tilted contexts are retained for independent judgment.

Baseline retention is recommended on artistic grounds, confirmed by independent final assessment: the original hooked nose, elongated graphic mask and framed cap already read as a coherent repeated architectural marker beside the wall panels. Its pixelated painting is consistent with those nearby surfaces. Neither additional smoothing nor a stronger shelf improves that reading enough to justify replacement. This is not a claim that either rejected study completes a remake.

## Painting and complete dependency study

Both game textures remain frozen. The full Data inventory scans 5300 BMD-named files: 5114 readable model records (including empty models), 182 nonmodel headers and four 42-byte BMD-version-0 placeholders independently rejected by the converter. The exact `Object2/deep_wall03.OZJ` path has only Object04, Object16 and Object17 consumers. All same-name references were checked; there are none under other paths in this inventory.

Object16/17 each use four triangles for ornamental bands below their dragon heads. Their two separate UV islands repeat into approximately U .30–.70, V .0101–.2016. Read-only all-material front/reverse and isolated strip-location images are retained under `context`. Object04 cap faces 82–89 also use the atlas's lower region. Object04 backing/back faces 6–9 use the entire atlas, so repainting the portrait would necessarily change those owned flat portraits as well and require all-side review.

`texture-mask-proof.json` records conservative triangle/pixel-cell overlap, repeat addressing and a periodic two-pixel filtering guard. The protected consumer/cap union covers 1230 pixels, or 1566 with guard. Any touched 8×8 JPEG block is protected: 36 blocks remain original; 92 are separable. The texture is 64×128 RGB 4:4:4. Engine texture upload uses one mip level; OpenTexture defaults to repeat/nearest, and the guard also covers bilinear sampling. Periodic wrap to the top rows is explicitly protected.

The coordinator generated three noninstalled portrait masters. Masters 01 and 02 move the eyes/nose downward relative to the existing UV-projected nose. Master 03 introduces a human face and two stacked nose forms. All are rejected for identity/registration, not adopted. Exact prompts, masters, downsampled drafts and a numbered registration comparison are retained. The red marker in that diagnostic is the exact geometry-derived nose anchor; other feature-row estimates are visual observations, not automatic landmark detection.

Focused 64×128 coefficient-patching and FASTDCT-check scripts were prepared by adapting the accepted Fountain02 method with attribution. No painting passed the registration gate, so these scripts have not been used to package a replacement texture. Pixel-mask separability must not be misreported as an accepted or tested new painting.

All views are offline approximations. No personal Dungeon client verification is claimed. The final independent report in independent-final-direction.json rejects the487-triangle shelf and explicitly retains baseline Object04 for its convincing wall/corner/tilted architectural role. It does not accept any painting. This is a study-only checkpoint; no Data change is authorized by these results.
