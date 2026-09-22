# Beer02 — apples, bottle, tankard and two bowls

Owner: ASTRA groundcover/tree artist, `codex/lorencia-tavern-details`.
Status: remodeled and painted, exported and validated offline. Client review pending.
World1 type 152, 2 placements retained in placements.json.
Complete dependencies, in original BMD mesh order: ('pot3.tga', 'apple.jpg', 'bottle.tga', 'plate.jpg', 'winecup.jpg').
Exact containers: ['pot3.ozt', 'apple.OZJ', 'bottle.ozt', 'plate.OZJ', 'winecup.OZJ'].
Texture dimensions: {'pot3.tga': [512, 512], 'apple.jpg': [512, 512], 'bottle.tga': [512, 512], 'plate.jpg': [512, 512], 'winecup.jpg': [512, 512]}.
Original archive checked byte-for-byte against starting revision 8b85343485ad231b4e1d6e96a6e196d7f0c08bd6.

Replacement: 952 triangles; original info in original/info.txt.
Bone names/order: ['ytut', 'Cylinder32', 'Cylinder33', 'Cylinder34', 'Cylinder35', 'ytut01']. Every original parent and local pose is retained.
One action, one frame, lock=0; original frame/action order is unchanged.
All new vertices have exactly one full-weight original rigid bone. No dummy bindings.
Mesh order and all original material filenames are preserved, including lower-case ozt paths.

Before bind bounds: [[-43.76, -28.36, -0.8], [44.47, 23.53, 61.47]].
After bind bounds: [[-43.76, -28.36, -0.8], [44.47, 23.53, 61.47]].
Every original material/bone component's bounds is also retained numerically. Rounded vessel
profiles are fitted separately, preserving individual arrangement and footprint. Tankard/jug
handles follow their original measured direction. Shelf heights and vessel contacts are retained.
Chair and stool keep the original seat surface polygons and foot corners. Seating proof:
{'status': 'not a seat'}.

Full comparison correctly reports DIFFERENT because geometry was rebuilt:

```
meshes 5 vs 5, bones 6 vs 6, actions 1 vs 1, triangles 232 vs 952
unmatched triangles: 230  max corner distance: 0.0001  max bone distance: 0.0000  differing bone names: 0
DIFFERENT
```

Skeleton-only comparison is EQUIVALENT. validation/local-motion.json proves exact node names,
indices, parents and frame lists; every bind/action local key is compared as translation and
Rz*Ry*Rx rotation matrices. The measured maxima are retained there. Action metadata is compared
explicitly. Original and replacement reference/action SMDs pass bmdconv validate.

Actual exported UV/normal/material audit: {'minimum_uv_triangle_area': 0.0003839999999999997, 'minimum_geometry_triangle_area': 1.3972222932559997, 'minimum_winding_normal_dot': 0.3912884531272465, 'zero_area_triangles': 0, 'winding_disagreements': 0}.
Legacy alpha-card geometry, UVs and rigid bone assignments remain within export precision;
paintings preserve transparent openings. Final alpha coverage at cutoff 64 is 37.1% vs 36.5%
for apple sprigs, 86.6% vs 84.0% for chair fretwork. RGB is padded under discarded alpha texels;
generated alpha is retained. Bottle retains its original 181..255 opacity field via nearest
resampling. OZT is uncompressed 32-bit bottom-left; opaque textures are RGB JPEG/OZJ. All final
containers pass tools/mu_texture.py check. No new material flags or rendering features.

source.blend is packed and retains the untouched imported geometry/UVs/skinning/transforms
in REF_ORIGINAL; source-audit.json compares a numeric fingerprint to the original import.
REF_HIGH_POLY retains excluded editable authored component meshes with fine-bevel modifiers.
Painted raster masters, editable PNGs, final game textures and exact imagegen prompts/raw
iterations are in parent textures/ and the prompt JSON files. All image artwork was produced
with the built-in imagegen skill; packaging performs technical resizing/encoding/alpha padding.

review/comparison.jpg provides matching original/exported-BMD cameras and reduced previews.
After views are imported from actual game exports. Separate wireframes are included. Ground
views use completed terrain at 100-unit repeats and a 190-unit proxy at an invented staging
position. The terrain density is a staging proxy; actual engine UV density depends on texture width.
These are offline Blender previews, not client evidence or actual placement captures.

Client lighting, material/alpha filtering, actual tavern occlusion and chair/stool sitting
contacts remain pending. No shared runtime, client, engine or collision data was modified.
