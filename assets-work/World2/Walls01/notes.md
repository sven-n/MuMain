# Dungeon wall crown refinement and pier retention

Object01 is accepted offline as a bounded crown refinement, not a substantial wall rebuild. Its new upper chamfer has coherent stone grain and planar masonry mitres. Object03 is retained unchanged under a separate visual decision. New client verification is pending; no client captures or runtime installation were performed by this worker.

| Asset | Disposition | Triangles | Placements |
| --- | --- | ---: | ---: |
| Object01 | Accepted upper crown refinement | 58 → 64 | 1,262 |
| Object03 | Retain existing tapered four-niche pier | 130 unchanged | 27 |

Object01 retains its deep dragon niche, broad shoulders and stepped silhouette. An 18-unit upper chamfer with a 12-unit inward top shift and 28-unit planar end mitres leaves a 111-unit central span. Only four original front/top faces were replaced; all 54 other faces retain their positions, UVs, normals and winding, including complete module ends, opening/jamb surfaces, plaque and ground contacts. Normal-distance gain is modest in the three-module view, but purposeful at the crown with only six additional triangles.

Object03 already has a substantial tapered foot, four deep framed niches, a decorated belt and a dressed recessed crown. These forms terminate the repeated wall coherently in the actual 960/964 assembly. Close-view grain stretching and atlas segmentation remain historical limitations; added subdivisions or a matching crown treatment were not justified. Retention is based on visual merits, not absence of writable scope.

## Protected contracts

Baseline is main `7b808473`. Object01 bind bounds remain [-150.126999, -61.038601, 0] to [149.873001, 53.459900, 291.606201]. Full end surfaces at x -150.126999 and +149.873001 join repeated wall placements spaced 300 units apart. The original plaque remains at y -12.882600. Every actual placed bound and convex support plane passes the 0.0003 numerical gate.

Both assets retain two material slots, one root bone, one action and one key. Object01 root is Box06; Object03 root is Box01. Original 32-byte CP949 internal names are preserved through the converter's supported name argument. Official exported Object01 uses BMD version 12; its baseline is version 10. Bone/action skeleton comparison is equivalent, with motion component delta zero. Geometry comparison correctly reports DIFFERENT.

Both texture containers remain byte-identical: Object2/deep_wall01.OZJ and deep_wall02.OZJ. Deep_wall01 has broad shared and excluded consumers; deep_wall02 serves this pair. No texture or other consumer changed. New upper stone facets sample an existing plain-stone patch at uniform physical UV density 0.00725988 per world unit. Their independently measured maximum density deviation is 2.40e-8. Original lower ornament and roof projection remain within 1e-6; this deliberate mapping improvement is limited to new facets.

## Evidence and reconstruction

Object01/source.blend contains editable mesh data, packed diffuse images and hidden REF_ORIGINAL. Untouched original and merged-baseline BMDs/imports are retained separately. Object01/exports contains the accepted BMD; all earlier experimental exports are rejected studies. Object03 has baseline/reference material and its independent retention rationale, not an authored replacement.

Object01/validation retains converter info/validation/comparison, exact raw-name and action proof, every authored/exported corner match, raw normal ownership/world-direction agreement, positive face-normal incidence, nonzero UV areas, affine/density checks and all 1,262 placed bounds. Object01/review includes original, baseline, candidate, reverse, reduced and exported topology images. The batch review directory contains identical-camera actual wall 958/959/960 and pier 964 comparisons. These are diffuse offline evidence, not client evidence.

Independent reports are Object01/independent-review.json and Object03/independent-review.json. provenance.json binds final source/export, frozen texture dependencies and all current evidence hashes. The first corrected 78-triangle inward profile and second 88-triangle broad profile are retained under experiments; both were rejected. The first was hard to read, the second introduced stretched stone sampling and scoop-like end transitions.

Run `python3 assets-work/World2/Walls01/reproduce.py` from the repository with BLENDER, MU_BMDCONV, BLENDER_USER_SCRIPTS and BLENDER_USER_CONFIG set to the verified Blender 5.2.2 / SourceTools 3.4.3 environment and repository converter. It verifies retained baseline hashes, uses the official import/export tools through the metadata-only adapter, builds the third study, runs strict checks, and regenerates matched reviews. Source packing timestamps may change source.blend bytes when reproduced; final accepted hashes are retained in provenance.json. No reproduction script installs game files.
