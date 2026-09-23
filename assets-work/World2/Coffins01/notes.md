# Dungeon coffin construction studies

Final artistic decision: retain both original game assets and reject the proposed replacements for insufficient visual gain. No game replacement accepted or installed. The original body and lid remain in `src/bin/Data/Object2`. Baseline revision: `7b808473`. The pair has 53 actual placements (33 body, 20 lid).

Object21 is the open six-sided coffin body (40 baseline triangles, root Box01); Object22 is its separate tapered lid (20 triangles, root Object01). Their local origins and footprints are offset, so context evidence uses actual placements without snapping. Object35 is a bucket and excluded. The shared Object2/wood01.OZJ atlas remains byte-identical; the six other consumers are outside writable ownership.

The first 156-triangle lid study gave modest board separation but insufficient placed-view improvement. Its low shoulder expanded the original tapered envelope in tilted placements; preserved under experiments/Object22-study01 with rejection recorded. The corrected lid keeps the full four-triangle underside and all twelve corner anchors; all original convex support planes and all twenty actual placed bounds pass at 0.0003 tolerance. It was rejected after coherent paired review: deeper seams and small end notches do not meaningfully improve actual placed readability.

The first 364-triangle body study replaces only the six outer walls with broad framed board relief. All 28 original floor, complete mating rim, and inner-wall triangles remain numerically exact in position, UV, winding, and normals, along with all 24 original corners. Individual diffuse renders show only subtle improvement; increased triangle count is not acceptance. Independent review rejected this replacement for insufficient visible improvement; no further arbitrary detail pass is planned.

Both current exports pass official converter validation, exact raw 32-byte model identity, unchanged root names/actions, per-corner authored material/bone/position/UV/winding correspondence, actual raw normal-owner/world-direction proof, nonzero UV areas and frozen texture hashes. Every actual rotated placement preserves bounds and ground minimum. These are offline technical findings only.

Official import/export delegates to tools/blender/mu_bmd_import.py and mu_bmd_export.py. official_adapter.py handles only CP949 legacy model-name bytes through the converter's supported name argument and ASCII action-manifest records; engine and converter code are untouched. Sources contain packed wood01 and hidden REF_ORIGINAL.

Reproduction uses pipeline.py prepare/prototype, body_prototype.py, validate.py, audit_source.py, contact_proof.py, uv_proof.py, review.py, context_review.py, and final_evidence.py. Supply BLENDER, MU_BMDCONV, BLENDER_USER_SCRIPTS and BLENDER_USER_CONFIG pointing to the verified official tools. Context runs in a fresh Blender process. Scripts do not install game files.

All images are offline diffuse reviews, not client evidence. New Dungeon client verification remains pending.

## Final baseline retention rationale

The original pair already has a distinctive tapered coffin outline, convincingly open body volume and wall thickness, clear painted timber courses and corner framing. Actual offset pair 4122/4123 and steeply tilted pair 683/685 preserve this identity. The new studies mainly deepen dark grooves and add small lid notches; their additional geometry does not justify replacing the baseline. Retention is based on these visual merits, not converter equivalence or prior completion labels. The frozen shared painting and exact interfaces remain appropriate.

The final independent report is retained as independent-review.json. Proposed candidate sources, exports, validations and comparisons are rejected study artifacts, not installed deliverables.
