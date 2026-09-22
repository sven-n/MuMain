# CartHay01 — carts and bundled straw

Owner: ASTRA environment artist `/root/groundcover`, `codex/lorencia-cart-hay`.
Starting integration revision 8094616d. Six assets, 23 World1 placements. Offline production
and validation complete; client verification pending. Shared documentation belongs to coordinator.

| Model | Actual identity | Placements | Triangles before → after |
|---|---|---:|---:|
| Carriage01 | Covered-back cart with hanging lantern and additive light pane |4|280 →472|
| Carriage02 | Empty two-wheel handcart |7|258 →450|
| Carriage03 | Handcart with two adjacent casks |0|346 →538|
| Carriage04 | Cart loaded with three tied straw bundles |2|390 →882|
| Straw01 | Three tied bundles in a stack |8|132 →432|
| Straw02 | Eight tied bundles in a crossed pile |2|352 →1152|

All six retain their original footprint, origin, orientation, per-bone bounds and mesh order.
The cart frames, shafts, seat/platform surfaces, axles, wheel contact geometry, casks and
lantern/light geometry remain. Shaped iron axle sleeves add useful hub definition inside
original bounds. Shared cart wood and cask artwork are intentionally frozen for their
separately coordinated full dependency passes.

Original straw is an open hexagonal shell with crossed interior faces. The replacement is
closed, rounded and tied, with pinched rope profiles, frayed outer tips and packed cut ends.
Every bundle retains its original arrangement, orientation and rigid bone. Slight individual
shape variation avoids identical local vertices across bones in the legacy converter.
The owned straw atlas now includes side fibers, cut-end chaff and a spare chaff patch; all
three consumers were remodeled/re-UVed together. Initial oversized end-grain artwork was
revised to smaller broken straw fibers. Raw iterations and prompts remain under textures/.

Three paintings were made with the built-in imagegen skill and packaged at 512x512:
horse_drawn_02.OZT, horse_drawn_03.OZJ, grass_01.OZT. The iron cage has 62.7% alpha coverage
at cutoff 64 versus 66.4% original; sparse missing pixels stay within the original cage cards.
Generated straw alpha is retained, confined to frayed tips. Transparent RGB is padded from
visible neighbors without changing alpha. OZT is 32-bit uncompressed bottom-left; the additive
RGB light uses black background and restrained amber. Original texture filenames/casing remain.
All alpha coverage and RGB measurements are in textures/final/*-audit.json.

Complete dependencies and consumer ownership:

- horse_drawn_02.OZT and horse_drawn_03.OZJ: Carriage01 only, repainted here.
- grass_01.OZT: Carriage04, Straw01, Straw02, repainted here as one group.
- horse_drawn_01.OZJ: frozen, all four carts retain this shared texture unchanged.
- tub.OZJ: frozen, Carriage03 retains this shared texture unchanged.

Exact changed game files under src/bin/Data/Object1 are Carriage01.bmd, Carriage02.bmd,
Carriage03.bmd, Carriage04.bmd, Straw01.bmd, Straw02.bmd, horse_drawn_02.OZT,
horse_drawn_03.OZJ and grass_01.OZT. No other game files, runtime files or engine code changed.

Source controls: MapManager.cpp:1065–1067 loads these series; ZzzObject.cpp:4635 sets
Carriage01 BlendMesh=2. Its original horse_drawn_03.jpg two-triangle pane remains mesh 2,
with original positions, UVs and bone. Carriage01 retains 14 bones including Dummy05 and all 21
keys with lock 0. The other five retain their 2/4/5/3/8 bones and one-key action, lock 0.
No dummy receives geometry. No placement, terrain, collision, interaction or effect anchor
is changed. Original placement records are archived verbatim per model.

All six full BMD comparisons correctly report DIFFERENT for geometry; skeleton/action-only
comparisons report EQUIVALENT. Every name, order, parent, action/frame/lock entry and local
translation/rotation matrix was checked. Precise before/after bounds, counts, complete compare
outputs, wrapper checks and local-motion logs are retained under each validation directory.
Source bounds are exact; actual exported coordinates differ only by float/SMD precision
(max 0.001682 units across preserved cart corners, strict .005-unit audit tolerance).
Four pre-existing collapsed UV triangles on Carriage01 wood were opened within a quarter
original atlas texel. All other retained cart UVs, especially alpha/light UVs, remain exact.
Final exports have no collapsed UV/geometry triangles or winding/normal disagreements.

Additional raw BMD audit matters for this batch. Legacy SMD2BMD.cpp compares local positions
and normals without including their bone index. Identical authored bundle-local positions
were therefore merging across differently posed bones. Slight per-bundle variation fixes that
without changing bounds. Every saved authored vertex is checked against final BMD-expanded
corners for both intended bone and position, in both directions.

The original Carriage01 already shares normal nodes 10:0 despite differing action rotations
(max world-matrix component delta 0.147809). Tiny per-bone tangent adjustments to authored
split normals prevent that merge in the replacement. Final Carriage01 has zero raw normal-node
mismatches. Remaining shared normal nodes on static models are proven to have exactly equal
world rotations on every key. No converter or engine edit was made. raw_bindings.py is a
read-only binary parser; audit_bindings.py records all-key normal-node proofs.

Each packed source.blend includes immutable numeric REF_ORIGINAL with saved-file fingerprint
proof, untouched armature/action metadata, packed original/final images and excluded editable
REF_HIGH_POLY. Originals, original info/imports, full texture dependencies, source scripts,
generation prompts, raw art, editable PNGs and exported game files are preserved.

Each review/comparison.jpg shows matching original/exported-BMD cameras, a reduced view and
terrain staging proxy. Reverse views and wireframes are separate. Carriage01 action frames 0,
10 and 20 are included. Terrain repeats 100 units and a 190-unit figure are illustrative staging;
engine terrain UV density depends on texture width and is not reproduced. Additive mesh 2 uses
an explicitly labeled render-only approximation in Blender, never a new game material flag.
review/placements compares original world transforms for paired carts, an elevated hay stack,
a loaded cart and tilted crossed hay. Relative translation/rotation/scale remain exact there;
surrounding terrain/buildings and client lighting are absent. Carriage03 has no placements.

installed-source-files.json records nine owned hashes. ownership-check.json proves the other
315 tracked Object1/World1 files unchanged, including frozen shared dependencies and all
terrain/light/placement/alpha data. No runtime installation or client session occurred.
Client loading, lighting, additive behavior, alpha/filtering, all motion and surrounding
occlusion remain pending; no asset is marked client-verified.

Reproduction in this assigned worktree: bundled Python prepare.py, import_originals.py,
package_textures.py; Blender inspect_originals.py and build_sources.py; Python export_validate.py;
Blender audit_sources.py; Python audit_exports.py and audit_bindings.py; Blender render_review.py
and render_placements.py; Python write_notes.py and finalize.py. Use Blender --python-exit-code 1
and Python -B. Official import/export subprocesses disable bytecode writes. Generation is not
deterministic; packaging reproduces the retained chosen paintings. All scripts stay in this batch.
