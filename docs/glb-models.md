# GLB monster models

A monster model can be drawn from a glTF 2.0 binary (`.glb`) instead of its
BMD. The BMD stays: it gives the model its actions (key counts, play speed,
loop and lock flags), so action durations and every frame-based game check
are unchanged, and it is the fallback when a file is refused. Nothing changes
until a model file table names a file: the shipped table has no entries.

Code: `src/source/Data/GameData/ModelData/` (the table),
`src/source/Data/DataHandler/ModelData/` (loading at start),
`src/source/Render/Models/Gltf/` (the loader and the skinned model).

---

## The model file table

`src/bin/Data/Models/ModelFiles.json`, read once on the loading screen:

```json
{
  "formatVersion": 1,
  "models": [
    {
      "model": "MODEL_ELITE_YETI",
      "file": "Monster/Monster14.glb",
      "actions": { "2": { "clip": "walk", "crossfade": 3 } },
      "lod": { "screenHeights": [0.2, 0.1], "hysteresis": 0.1 }
    }
  ]
}
```

| Field | Meaning |
|---|---|
| `model` | The model's code name, one of the monster models (`MODEL_MONSTER01 + N` names of `_enum.h`). |
| `file` | The glTF binary, relative to `Data/`. |
| `actions` | Optional. Per MU action index: the `clip` that plays it (default: the clip named `actionNN`, e.g. `action02`) and/or the `crossfade` into it in ticks (default: the playback rule's fade). |
| `lod` | Optional. Below the projected screen height `screenHeights[0]` (a fraction of the window's height) LOD 1 is drawn, below `screenHeights[1]` LOD 2 (descending, above 0); a level changes back to a finer one only above `h × (1 + hysteresis)` (0 ≤ hysteresis < 1). Defaults: `[0.2, 0.1]` and `0.1`. |

- No table, or an empty `models` list: every monster is drawn from its BMD
  and nothing is logged.
- An unknown model, a model named twice, a file found nowhere or a malformed
  table is a fatal problem: the game stops at start with a message, as for
  the effect data.
- An unknown field, or an action entry with neither a clip nor a crossfade,
  is a warning; the entry is kept.

### Developer overlay

When the environment variable `MU_DATA_OVERLAY` names a directory, its
`Data/` is searched before the game's `Data/`, for the table itself and for
every file the table names. The game's files stay untouched:

```sh
mkdir -p /tmp/overlay/Data/Models /tmp/overlay/Data/Monster
cp Monster14.glb /tmp/overlay/Data/Monster/
cp ModelFiles.json /tmp/overlay/Data/Models/
MU_DATA_OVERLAY=/tmp/overlay ./Main
```

The log then shows `Data overlay … is searched before …`.

## What the loader expects of a file

The loader matches the file to the model's BMD. It refuses a file (see
[Logs](#logs)) that does not follow these rules:

- **Self-contained:** one `.glb` with its buffers inside, no external URIs.
- **One skin** with a FLOAT MAT4 inverse bind matrix per joint, at most 200
  joints (`MAX_BONES`).
- **The BMD's bones by name:** every bone of the BMD is a joint of the same
  name (a dummy bone, which has no name in the BMD, is `dummy<index>`), under
  the same parent. Other joints are allowed; they are appended after the BMD's
  bones, parents first, and may not have a BMD bone below them. Joint names
  are unique.
- **Joints only rotate and move:** no scale or mirror on joints, in their
  keys, or on the nodes above the skeleton.
- **Space:** glTF (x, y, z) is MU (x, −z, y).
- **Clips:** one per action, named `actionNN` unless the table names
  another; LINEAR interpolation, key times in order, finite values. A clip
  is played over the BMD action's key count, sampled by phase, so its own key
  count is free. An action without a clip shows the rest pose. A clip named
  in `actions` must exist.
- **Meshes:** indexed triangles with POSITION, NORMAL, TEXCOORD_0,
  JOINTS_0 (UNSIGNED_BYTE or UNSIGNED_SHORT) and WEIGHTS_0, finite values,
  no negative weights, all on the skin. Up to four weights per vertex skin
  positions and normals, on the GPU and wherever the game reads vertices.
  LOD 0 needs at most as many meshes as the BMD may have (`MAX_MESH`).
- **LODs (optional):** mesh nodes with `"extras": {"lod": 1}` or `2` on the
  same skin; LOD 2 needs LOD 1, and mesh *i* of a LOD stands in for BMD mesh
  *i* (from one mesh up to LOD 0's count). The LOD drawn also casts the
  planar shadow.
- **Textures:** a PNG the material's base colour texture carries in the file
  is the mesh's texture. A mesh without one uses the texture its primitive's
  or material's `"extras": {"texture": "<name>"}` names in the monster's
  texture folders (`Data/Monsters/Models.json`), opened as for the BMD. Embedded images are PNG, at most
  2048 × 2048.
- **Normal maps (Enhanced lighting only):** a material's `normalTexture`
  (PNG, tangent space, +Y up, its `scale` applied to xy) needs a TANGENT
  attribute (4 components, w = ±1 for the handedness). Classic lighting
  ignores it.

A monster's head bone (`headBone` in `Data/Monsters/Models.json`, an index
into the BMD's bones) is found in the file by that BMD bone's name, and the
Yeti's breath bone by `Box03`. A name the model has no bone for falls back to
the root bone, with one log line.

## Logs

| Log line | Meaning |
|---|---|
| `[data] Model MODEL_… is drawn from <file>` | The file replaced the BMD's bones and meshes. |
| `[render] Model file <file> is not used, <reason>; the BMD stays.` | The file was refused; the monster is drawn from its BMD. |
| `[render] Image <name> of <file> is not used, <reason>; its meshes keep their texture.` | An embedded image could not be decoded (not a PNG, unreadable, or above 2048 × 2048). |
| `[render] Image <name> of <file> could not be loaded as texture.` | The decoded image could not be uploaded. |
| `[render] Model MODEL_… (<file>): mesh N has no texture <name>` | A mesh names a texture that is not in the monster's folder and carries no image. |
| `[render] Model <name> has no bone "<bone>"; the root bone stands in for it.` | A by-name bone lookup missed (logged once per model and name). |
| `[data] Data overlay <dir> is searched before <dir>` | `MU_DATA_OVERLAY` is in effect. |

## Tests

- `tests/render/test_glb_model.cpp` (`glb_model_tests`) loads
  `tests/render/data/Monster{14,01,33}.glb`, exports of the Elite Yeti, Bull
  Fighter and Bali BMDs without their textures, and compares bones, rest pose,
  meshes, animation and skinning with the BMDs; it also covers the refusals,
  LODs, normal maps, embedded images and the GPU vertex packing.
- `tests/data/test_model_files.cpp` (`model_files_data_tests`) covers the
  table, its fatal problems and warnings, and the overlay.
- `tests/render/test_animation_sampling.cpp` and
  `test_animation_playback_rule.cpp` cover the playback rule BMD and GLB
  models share.
