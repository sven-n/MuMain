# MU Online Legacy Asset Conversion Pipeline

Technical documentation covering the reverse engineering, conversion, and validation
of legacy MU Online game assets from proprietary C++ formats to modern open standards.

## Development Workflow

After any change related to this Rust workspace, run `cargo check` before considering the task complete.
Preferred command from `rust/`:

```bash
cargo check --workspace
```

For detailed binary format specifications and JSON output schemas, see
[`rust/docs/client/assets_data.md`](../../docs/client/assets_data.md).

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Data Sources](#data-sources)
4. [Proprietary Formats Reverse-Engineered](#proprietary-formats-reverse-engineered)
   - [OZJ / OZT / OZB / OZP (Textures)](#ozj--ozt--ozb--ozp-textures)
   - [BMD (3D Models)](#bmd-3d-models)
   - [Terrain Data (Height, Map, Attributes)](#terrain-data)
   - [Scene Objects (.obj)](#scene-objects-obj)
   - [Camera Tour (.cws)](#camera-tour-cws)
5. [Encryption Algorithms](#encryption-algorithms)
   - [MapFileDecrypt](#mapfiledecrypt)
   - [Xor3Byte (BuxConvert)](#xor3byte-buxconvert)
   - [ModulusDecrypt (Season16+)](#modulusdecrypt-season16)
   - [LEA-256 ECB (Season20)](#lea-256-ecb-season20)
6. [BMD Format Deep Dive](#bmd-format-deep-dive)
   - [File Header and Versioning](#file-header-and-versioning)
   - [Struct Layouts (MSVC-Aligned)](#struct-layouts-msvc-aligned)
   - [Parsing Sequence](#parsing-sequence)
   - [Bone Fixup (Rest Pose Transform)](#bone-fixup-rest-pose-transform)
   - [Math Functions (Ported from C++)](#math-functions-ported-from-c)
   - [GLTF/GLB Emission](#gltfglb-emission)
   - [Edge Cases and Non-Model BMDs](#edge-cases-and-non-model-bmds)
7. [Terrain Height Format Deep Dive](#terrain-height-format-deep-dive)
   - [OZB Container Format](#ozb-container-format)
   - [8-bit Height Maps (Legacy)](#8-bit-height-maps-legacy)
   - [24-bit Height Maps (Extended)](#24-bit-height-maps-extended)
   - [Corrupted and Flat Files](#corrupted-and-flat-files)
8. [Conversion Pipeline](#conversion-pipeline)
   - [Scripts and Tools](#scripts-and-tools)
   - [Running the Pipeline](#running-the-pipeline)
   - [Output Directory Structure](#output-directory-structure)
9. [Conversion Results](#conversion-results)
10. [JSON Sidecar Formats](#json-sidecar-formats)
11. [Known Issues and Limitations](#known-issues-and-limitations)
12. [C++ Source References](#c-source-references)

---

## Overview

The MU Online Rust/Bevy client requires all game assets in open, universal formats.
The legacy C++ client stores assets in proprietary binary formats that are
undocumented and in some cases encrypted with multiple cipher layers.

The pipeline supports two data sources and converts a combined **~46,930 legacy files**
into modern formats:

| Legacy Format | Target Format | Count (Season20) | Status |
|---------------|---------------|------------------:|--------|
| OZJ/OZT/OZB/OZP/TGA/JPG/BMP | PNG | 18,634 | Done |
| BMD (3D models) | GLB (glTF Binary) | 12,258 | Done |
| OZB (terrain height) | JSON | 80 | Done |
| MAP (terrain tile mapping) | JSON | 82 | Done |
| ATT (terrain attributes) | JSON | 88 | Done |
| OBJ (scene objects) | JSON | 82 emitted | Partial (encryption unknown for Season20) |
| CWS (camera tour) | JSON | per-world | Done |
| (generated) terrain config | JSON | per-world | Done |

All conversions are performed by pure Python scripts. `Pillow` is required for
image processing. A C++ helper (`mu_terrain_decrypt`) is needed for Season16+
ModulusDecrypt terrain files and requires `libcryptopp`.

---

## Architecture

```
Legacy Data Sources (read-only)
  cpp/MuClient5.2/bin/Data/        (5.2 build — 11,991 files)
  /home/.../MuData/Season20/       (Season20 build — 34,939 files)
        |
        v
+-------------------+     +----------------------------+
| assets_convert.py | --> | Textures (PNG)             |
| (textures, terrain|     | terrain_height.json        |
|  map, att, obj,   |     | *.map.json                 |
|  cws, config)     |     | *.att.json (EncTerrainN)   |
+-------------------+     | scene_objects.json         |
        |                 | camera_tour.json           |
        |                 | terrain_config.json        |
        |                 +----------------------------+
        |
+-------------------+     +----------------------------+
| bmd_converter.py  | --> | 3D Models (GLB)            |
| (BMD parser,      |     | supports 0x0A/0x0C/0x0E/0F |
|  bone fixup, GLB) |     +----------------------------+
+-------------------+
        |
+-------------------------+
| mu_terrain_decrypt      |  (C++ helper, Crypto++)
| (Season16+ ModulusDecrypt|
|  for ATT/MAP files)     |
+-------------------------+
        |
        v
+-------------------+     +----------------------------+
| validate_assets.py| --> | Validation Report (JSON)   |
| (structural check)|     +----------------------------+
+-------------------+
        |
        v
    rust/assets/data/         (converted assets)
    rust/assets/reports/      (conversion reports)
```

Orchestrated by `convert_all_assets.sh`.

---

## Data Sources

| Source | Path | Files | Description |
|--------|------|------:|-------------|
| 5.2 build | `cpp/MuClient5.2/bin/Data/` | 11,991 | Legacy client, no `.obj` scene files |
| Season20 | `/home/.../MuData/Season20/` | 34,939 | Modern client with 82 `.obj` scene files, LEA-256 encrypted BMDs |

The pipeline can target either source (or both via `--legacy-fallback-root`).

---

## Proprietary Formats Reverse-Engineered

### OZJ / OZT / OZB / OZP (Textures)

The MU client wraps standard image formats with a small proprietary header:

| Extension | Inner Format | Header Size | Strip Method |
|-----------|-------------|-------------|--------------|
| `.OZJ`    | JPEG        | 24 bytes    | Strip first 24 bytes |
| `.OZT`    | TGA         | 4 bytes     | Strip first 4 bytes |
| `.OZB`    | BMP         | 4 bytes     | Strip first 4 bytes |
| `.OZP`    | PNG         | 4 bytes     | Strip first 4 bytes |

The header bytes are opaque padding; no encryption is applied to texture containers.
After stripping, the payload is a standard image file that can be decoded by any
image library.

**Source**: `MuClientTools16/_src_/Core/GraphicConvertor.h` (lines 71-165), `OZB.h`

The `GraphicConvertor<EXT, PAD>` template class defines `Encrypt()` (insert PAD bytes)
and `Decrypt()` (remove PAD bytes). OZB uses `PAD=4`, OZT uses `PAD=4`, etc.

### BMD (3D Models)

Binary Model Data format. See [BMD Format Deep Dive](#bmd-format-deep-dive).

### Terrain Data

| File | Format | Encryption (5.2) | Encryption (Season16+) |
|------|--------|-------------------|------------------------|
| `TerrainHeight.OZB` | 4-byte header + BMP | None | None |
| `EncTerrain*.map` | Binary terrain mapping | MapFileDecrypt | ModulusDecrypt (MAP\x01 header) |
| `EncTerrain*.att` | Binary terrain attributes | MapFileDecrypt + Xor3Byte | ModulusDecrypt + Xor3Byte (ATT\x01 header) |
| `Terrain*.att` | Binary terrain attributes (unencrypted) | Raw | Raw |
| `EncTerrain*.obj` | Binary scene objects | MapFileDecrypt | Unknown (Season20 unsolved) |
| `CWScript*.cws` | Camera tour waypoints | None | None |

### Scene Objects (.obj)

Scene object files contain per-world 3D object placement data. Each entry
specifies a model type, position, rotation, and scale. Entry sizes vary by
format version (30, 32, 33, 45, 46, or 54 bytes).

The converter uses a multi-candidate scoring system that tries several
decryption methods (MapFileDecrypt, raw, Xor3Byte combinations) and header
layout interpretations, selecting the candidate with the highest sanity score.

**Status**: Works for 5.2 build. Season20 `.obj` encryption is unsolved.

### Camera Tour (.cws)

CWScript files define camera flythrough paths for login/character-select
screens. Magic: `0x00535743` (ASCII `CWS\0`). Each waypoint struct is 28 bytes
containing grid index, camera position, delay, acceleration, and distance level.

The converter emits both a raw JSON (preserving original values) and a
normalized JSON (Bevy Y-up coordinates, seconds instead of frames).

---

## Encryption Algorithms

### MapFileDecrypt

Used for encrypted terrain files (`.map`, `.att`) and BMD version 0x0C files.

```
Key:  [0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
       0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2]
Seed: 0x5E

For each byte at index i:
    temp = data[i]
    data[i] = ((data[i] XOR key[i % 16]) - seed) & 0xFF
    seed = (temp + 0x3D) & 0xFF
```

**Source**: `MuCrypto.cpp:65-77`

### Xor3Byte (BuxConvert)

Simple 3-byte repeating XOR applied after MapFileDecrypt or ModulusDecrypt
for `.att` files:

```
Key: [0xFC, 0xCF, 0xAB]
For each byte at index i:
    data[i] ^= key[i % 3]
```

**Source**: `ZzzLodTerrain.cpp:113-119`

### ModulusDecrypt (Season16+)

Multi-cipher block decryption using 8 Crypto++ algorithms selected by two
algorithm bytes embedded in the encrypted payload. Implemented in C++ as
`mu_terrain_decrypt.cpp`, compiled against `libcryptopp`.

**Key**: `"webzen#@!01webzen#@!01webzen#@!0"` (32 bytes)

**Cipher selection** (`algorithm & 7`):

| Index | Cipher   |
|-------|----------|
| 0     | TEA      |
| 1     | ThreeWay |
| 2     | CAST-128 |
| 3     | RC5      |
| 4     | RC6      |
| 5     | MARS     |
| 6     | IDEA     |
| 7     | GOST     |

**Critical**: Keys are set using `T::DEFAULT_KEYLENGTH`, not the full 32-byte
key length.

**File detection**: Season16+ files have a 4-byte magic header (`ATT\x01` or
`MAP\x01`) before the encrypted body.

**Post-processing by file type**:
- **ATT**: Strip 4-byte magic -> ModulusDecrypt -> Xor3Byte
- **MAP**: Strip 4-byte magic -> ModulusDecrypt (no Xor3Byte)
- **BMD (0x0E)**: Body is wrapped in fake `MAP\x01` header for decryption.

**Build**: `g++ -O2 -o mu_terrain_decrypt mu_terrain_decrypt.cpp -lcryptopp`

**Source**: `MuCrypto.cpp:220-262`, `mu_terrain_decrypt.cpp`

### LEA-256 ECB (Season20)

LEA (Lightweight Encryption Algorithm) from KISA. Used for Season20 BMD files
(version `0x0F`).

| Property   | Value |
|------------|-------|
| Block size | 16 bytes |
| Key size   | 32 bytes (256-bit) |
| Rounds     | 32 |
| Mode       | ECB |
| Key (hex)  | `cc 50 45 13 c2 a6 57 4e d6 9a 45 89 bf 2f bc d9 39 b3 b3 bd 50 bd cc b6 85 46 d1 d6 16 54 e0 87` |

Encrypted payload size must be 16-byte aligned. Implemented in pure Python
(~500 files/min).

**Source**: `bmd_converter.py`, discovered via `xulek/muonline-bmd-viewer`

---

## BMD Format Deep Dive

### File Header and Versioning

Every BMD file starts with a 4-byte header:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 3 | Magic | ASCII `"BMD"` |
| 3 | 1 | Version | Format version byte |

**Version handling**:

| Version | Encryption | Era | Treatment |
|---------|-----------|-----|-----------|
| `0x0A` | None | Legacy | Unencrypted. Data starts at offset 4. |
| `0x0C` | MapFileDecrypt | v5.x | 4-byte LE encrypted-size at offset 4, encrypted data at offset 8. |
| `0x0E` | ModulusDecrypt | Season16+ | Same header layout as 0x0C. Requires `mu_terrain_decrypt` tool. |
| `0x0F` | LEA-256 ECB | Season20 | Same header layout as 0x0C. Size must be 16-byte aligned. |
| `0x00` | — | — | Corrupt/empty files. Skipped. |
| Non-`BMD` magic | — | — | Data table files (item.bmd, etc.). Not 3D models. Skipped. |

**Source**: `ZzzBMD.cpp` Open() at lines 2530-2625, Open2() at lines 2730-2830

### Struct Layouts (MSVC-Aligned)

The on-disk struct sizes match MSVC x86 alignment rules. Getting these wrong
causes all subsequent parsing to fail silently.

| Struct | On-Disk Size | Layout |
|--------|-------------|--------|
| **Vertex_t** | 16 bytes | `short Node` (2) + `pad` (2) + `float Position[3]` (12) |
| **Normal_t** | 20 bytes | `short Node` (2) + `pad` (2) + `float Normal[3]` (12) + `short BindVertex` (2) + `pad` (2) |
| **TexCoord_t** | 8 bytes | `float U` (4) + `float V` (4) |
| **Triangle_t2** | 64 bytes | See below |
| **Mesh header** | 10 bytes | `short[5]` (NumVerts, NumNorms, NumTCs, NumTris, Texture) |
| **Model header** | 38 bytes | `char Name[32]` + `short NumMeshs` + `short NumBones` + `short NumActions` |

**Triangle_t2 on-disk layout (64 bytes)**:

The C++ client reads `sizeof(Triangle_t)` = 36 bytes of useful data but advances
the file pointer by `sizeof(Triangle_t2)` = 64 bytes. The tools version uses
`DWORD Nothing[7]` padding.

```
Offset  Size  Field
0       1     Polygon (3=triangle, 4=quad)
1       1     Padding
2       8     VertexIndex[4] (short x 4)
10      8     NormalIndex[4] (short x 4)
18      8     TexCoordIndex[4] (short x 4)
26      38    Padding/lightmap (unused)
```

**Source**: `ZzzBMD.h` (struct definitions), `BMD.h` in MuClientTools16

### Parsing Sequence

After decrypting (if version 0x0C), the data is parsed sequentially:

```
1. Model header: Name(32 bytes) + NumMeshs(2) + NumBones(2) + NumActions(2)

2. Per mesh (NumMeshs times):
   a. Mesh header: NumVerts(2) + NumNorms(2) + NumTCs(2) + NumTris(2) + Texture(2)
   b. Vertices:   NumVerts   x 16 bytes (Vertex_t)
   c. Normals:    NumNorms   x 20 bytes (Normal_t)
   d. TexCoords:  NumTCs     x  8 bytes (TexCoord_t)
   e. Triangles:  NumTris    x 64 bytes (Triangle_t2 stride)
   f. TextureName: 32 bytes (null-terminated string)

3. Per action (NumActions times):
   a. NumAnimationKeys: 2 bytes (short)
   b. LockPositions: 1 byte (bool)
   c. If LockPositions: Positions[NumKeys] x 12 bytes (float[3])

4. Per bone (NumBones times):
   a. Dummy: 1 byte (bool)
   b. If not dummy:
      - Name: 32 bytes
      - Parent: 2 bytes (short)
      - Per action:
        - Positions[NumKeys] x 12 bytes (float[3])
        - Rotations[NumKeys] x 12 bytes (float[3])  ← radians!
```

**Source**: `ZzzBMD.cpp:2530-2620`, `BMD_SMD.cpp:330-556`

### Bone Fixup (Rest Pose Transform)

BMD vertices are stored in bone-local space. The `FixUpBones` function computes
world-space transforms by walking the bone hierarchy.

**Reference**: `BMD_SMD.cpp:156-195`

```
For each bone i (in order, parents before children):
    angle = bone[i].rotation[action=0][key=0] * (180 / PI)   # radians to degrees
    pos   = bone[i].position[action=0][key=0]

    if bone[i].parent < 0:  # root bone
        BoneFixup[i].matrix    = AngleMatrix(angle)
        BoneFixup[i].world_org = pos
    else:
        parent = bone[i].parent
        local_m = AngleMatrix(angle)
        BoneFixup[i].matrix    = R_ConcatTransforms(BoneFixup[parent].matrix, local_m)
        p = VectorTransform(pos, BoneFixup[parent].matrix)
        BoneFixup[i].world_org = (p[0] + BoneFixup[parent].world_org[0],
                                  p[1] + BoneFixup[parent].world_org[1],
                                  p[2] + BoneFixup[parent].world_org[2])

For each vertex:
    world_pos = VectorTransform(vertex.position, BoneFixup[vertex.node].matrix)
                + BoneFixup[vertex.node].world_org

For each normal:
    world_normal = normalize(VectorRotate(normal, BoneFixup[normal.node].matrix))
```

### Math Functions (Ported from C++)

All math functions were ported from `ZzzMathLib.cpp`:

**AngleMatrix** (lines 228-256): Converts Euler angles (degrees, ZYX convention)
to a 3x4 rotation matrix. This is the standard Quake/Half-Life implementation.

```python
def angle_matrix(angles):
    """angles = (pitch, yaw, roll) in degrees"""
    pitch, yaw, roll = [a * (pi / 180) for a in angles]
    sp, cp = sin(pitch), cos(pitch)
    sy, cy = sin(yaw), cos(yaw)
    sr, cr = sin(roll), cos(roll)
    return [
        [cp*cy,  sr*sp*cy + cr*-sy,  cr*sp*cy + -sr*-sy, 0],
        [cp*sy,  sr*sp*sy + cr*cy,   cr*sp*sy + -sr*cy,  0],
        [-sp,    sr*cp,              cr*cp,               0],
    ]
```

**R_ConcatTransforms** (lines 288-314): Concatenates two 3x4 transformation matrices.

**VectorTransform** (lines 340-345): Transforms a 3D point by a 3x4 matrix (with translation).

**VectorRotate**: Transforms a 3D vector by a 3x4 matrix (rotation only, no translation).

**VectorNormalize** (lines 179-198): Normalizes a 3D vector to unit length.

### GLTF/GLB Emission

After computing world-space positions, the converter emits GLB (glTF Binary) files.

**De-indexing**: BMD uses separate index arrays for vertices, normals, and texture
coordinates per triangle. GLTF requires unified vertex indices. The converter builds
a combined vertex buffer by iterating triangles and deduplicating
`(vert_idx, norm_idx, tc_idx)` tuples.

**Quad triangulation**: When `polygon == 4`, split into 2 triangles:
`(0, 1, 2)` and `(0, 2, 3)`.

**UV flip**: `v_gltf = 1.0 - v_bmd` (BMP texture convention to GL convention).
Reference: `BMD_SMD.cpp:755`.

**GLB binary layout**:
```
[Header: 12 bytes]
  magic:     0x46546C67 ("glTF")
  version:   2
  total_len: <computed>

[JSON Chunk]
  length:    <json_size>
  type:      0x4E4F534A ("JSON")
  data:      <gltf json, padded to 4 bytes with spaces>

[BIN Chunk]
  length:    <bin_size>
  type:      0x004E4942 ("BIN\0")
  data:      <binary buffer, padded to 4 bytes with zeros>

Binary buffer layout:
  POSITION:  N x 12 bytes (VEC3 FLOAT)
  NORMAL:    N x 12 bytes (VEC3 FLOAT)
  TEXCOORD:  N x  8 bytes (VEC2 FLOAT)
  INDICES:   M x  2 bytes (UNSIGNED_SHORT) or x 4 bytes (UNSIGNED_INT if >65535 verts)
```

### Edge Cases and Non-Model BMDs

| Case | Action |
|------|--------|
| `NumMeshs == 0` | Skip (no renderable geometry) |
| `NumBones == 0` or `NumActions == 0` | Identity transforms (vertices already in world space) |
| Degenerate triangles | Skipped silently |
| `vertex.node >= NumBones` | Clamped to 0 with warning |
| Non-`BMD` magic | Skipped (data table files) |
| Version `0x00` | Skipped (corrupt) |

**Non-model BMD stems** (data tables that share `.bmd` extension):
`item`, `minimap`, `itemsetting`, `petdata`, `gate`, `movereq`, `dialog`,
`filter`, `quest`, `npcname`, `servercode`, `itemaddoption`, `socketitem`,
`masterskill`, `masterpassive`, `character`.

---

## Terrain Height Format Deep Dive

### OZB Container Format

TerrainHeight files use the `.OZB` extension but are simply BMP images with
a 4-byte prefix prepended:

```
[4 bytes: OZB prefix] + [standard Windows BMP file]
```

No encryption is applied to terrain height files.

**Source**: `GraphicConvertor.h` confirms OZB uses `PAD=4`.

### 8-bit Height Maps (Legacy)

Used by the majority of maps. Loaded by `OpenTerrainHeight()` in `ZzzLodTerrain.cpp:619-670`.

**Loading process**:
1. Open `.OZB` file
2. Seek to offset 4 (skip OZB prefix)
3. Read 1080 + 65536 bytes (BMP header + pixel data)
4. Skip first 1080 bytes (BMP file header + info header + 256-entry palette)
5. Each pixel byte = raw height value (0-255)
6. Scale: `height = byte * 1.5` (normal maps) or `byte * 3.0` (login scene World55)

**BMP structure**: 256x256 pixels, 8-bit indexed, 256-entry grayscale palette.
File size: `4 (OZB) + 14 (BITMAPFILEHEADER) + 40 (BITMAPINFOHEADER) + 1024 (palette) + 65536 (pixels)` = **66,618 bytes** (some files have 66,620 due to alignment).

### 24-bit Height Maps (Extended)

Used by maps that need larger height range. Loaded by `OpenTerrainHeightNew()` in
`ZzzLodTerrain.cpp:702-751`.

**Which maps use 24-bit**: Determined by `IsTerrainHeightExtMap()` (line 609):
- World 42 (`WD_42CHANGEUP3RD_2ND`)
- World 63 (`WD_63PK_FIELD`, via `IsPKField()`)
- World 66 (`WD_66DOPPLEGANGER2`)

However, the actual 24-bit BMP files in the data directory are:
**World43, World64, World67** (full 196,666-byte files with 24-bit pixel data).

**Loading process**:
1. Read entire `.OZB` file
2. Skip first 4 bytes (OZB prefix)
3. Read `BITMAPFILEHEADER` (14 bytes) + `BITMAPINFOHEADER` (40 bytes)
4. For each pixel, read 3 bytes in BGR order from BMP:
   ```c
   // pbysrc[0]=Blue, pbysrc[1]=Green, pbysrc[2]=Red (BMP byte order)
   DWORD dwHeight = 0;
   BYTE* pbyHeight = (BYTE*)&dwHeight;
   pbyHeight[0] = pbysrc[2];   // R -> byte 0 (least significant)
   pbyHeight[1] = pbysrc[1];   // G -> byte 1
   pbyHeight[2] = pbysrc[0];   // B -> byte 2
   // Result (little-endian): dwHeight = R + G*256 + B*65536
   ```
5. Apply offset: `height = (float)dwHeight + g_fMinHeight` where `g_fMinHeight = -500.0`

**Composite height formula**: `height = R + G*256 + B*65536 - 500.0`

**Example** (World43): Dominant pixel BGR = (0, 1, 244):
- Composite = 244 + 1*256 + 0*65536 = **500**
- Height = 500 - 500 = **0.0** (base terrain level)

**BMP structure**: 256x256 pixels, 24-bit uncompressed, no palette.
File size: `4 (OZB) + 14 + 40 + 256*256*3` = **196,666 bytes**.

### Corrupted and Flat Files

| World | Issue | Root Cause | Resolution |
|-------|-------|------------|------------|
| World1 | BMP header zeroed, 2 bytes short | File corruption (first 1080 bytes are zeros) | Fallback: raw pixel extraction from offset 1080, pad 2 missing bytes with zeros. 250 unique height values recovered. |
| World11 | 24-bit BMP header but only 66,620 bytes | Truncated file (needs 196,666 for full 24-bit) | Fallback to legacy 8-bit reading. All bytes = 0xBE. Genuinely flat terrain. |
| World19 | Entire file is zeros | Empty/corrupted source file | Fallback produces flat terrain at height 0. |
| World73 | Valid 8-bit BMP, 1 unique value (113) | Genuinely flat terrain in original game | No fix needed. |
| World74 | Identical to World73 | Genuinely flat terrain (copy of World73) | No fix needed. |

---

## Conversion Pipeline

### Scripts and Tools

| Script | Lines | Purpose |
|--------|------:|---------|
| `convert_all_assets.sh` | ~179 | Bash orchestrator for the full pipeline |
| `assets_convert.py` | ~2,600 | Texture/terrain/sidecar converter (map, att, obj, cws, config) |
| `bmd_converter.py` | ~1,100 | Pure Python BMD to GLB converter (0x0A, 0x0C, 0x0E, 0x0F) |
| `validate_assets.py` | ~310 | Structural validation for all output files |
| `mu_terrain_decrypt.cpp` | ~180 | C++ Season16+ ModulusDecrypt tool (ATT/MAP) |

**Dependencies**:
- Python 3.8+, Pillow (`pip install Pillow`)
- For Season16+ terrain: `libcryptopp-dev` + compiled `mu_terrain_decrypt`
  ```bash
  g++ -O2 -o mu_terrain_decrypt mu_terrain_decrypt.cpp -lcryptopp
  ```

### Running the Pipeline

```bash
# Full pipeline (textures + models)
./convert_all_assets.sh

# With options
./convert_all_assets.sh --force --verbose

# Textures only
./convert_all_assets.sh --textures-only

# Models only
./convert_all_assets.sh --models-only

# Individual scripts
python3 assets_convert.py \
    --legacy-root cpp/MuClient5.2/bin/Data \
    --output-root rust/assets/data \
    --skip-models --force --verbose \
    --report rust/assets/reports/textures_report.json

python3 bmd_converter.py \
    --bmd-root cpp/MuClient5.2/bin/Data \
    --output-root rust/assets/data \
    --format glb --force --verbose \
    --report rust/assets/reports/models_report.json

# Skip removing embedded PNG textures after GLB conversion
python3 bmd_converter.py \
    --bmd-root cpp/MuClient5.2/bin/Data \
    --output-root rust/assets/data \
    --format glb --force --verbose \
    --disable-embedded-texture-cleanup \
    --report rust/assets/reports/models_report.json

# Blend decision probe (per primitive/material diagnostics)
python3 bmd_converter.py \
    --bmd-root cpp/MuClient5.2/bin/Data \
    --output-root rust/assets/data \
    --format glb --force \
    --blend-probe \
    --probe-output rust/assets/reports/conversion_blend_probe.json

python3 validate_assets.py \
    --assets-root rust/assets \
    --report rust/assets/reports/validation_report.json \
    --verbose
```

The blend probe report includes per-primitive material decisions with alpha
signals plus RGB-key heuristics (`black_ratio`, `bright_ratio`, `mean_luma`)
used to infer legacy additive materials when no explicit alpha channel exists.

**Environment variables**:
- `LEGACY_ROOT` (default: `cpp/MuClient5.2/bin/Data`)
- `OUTPUT_ROOT` (default: `rust/assets`)
- `BMD_CONVERTER` (default: `bmd_converter.py`)

### Output Directory Structure

```
rust/assets/
├── data/
│   ├── Effect/              # Effect textures (PNG)
│   ├── Interface/           # UI textures (PNG)
│   ├── Item/                # Item textures (PNG)
│   ├── Logo/                # Logo textures (PNG)
│   ├── Monster/             # Monster textures (PNG)
│   ├── NPC/                 # NPC textures (PNG)
│   ├── Object1..56/         # Scene object models (GLB) + textures (PNG)
│   ├── Player/              # Player textures (PNG)
│   ├── Skill/               # Skill effect textures (PNG)
│   ├── World1..80/          # Per-world terrain data
│   │   ├── terrain_height.json    # Height map (256x256 samples)
│   │   ├── TerrainHeight.png      # Height map as image
│   │   ├── TerrainLight.png       # Lightmap texture
│   │   ├── TileGrass01.png        # Terrain tile textures
│   │   └── ...
│   └── ...
├── reports/
│   ├── textures_report.json       # Texture conversion stats
│   ├── models_report.json         # Model conversion stats
│   └── validation_report.json     # Structural validation results
└── shaders/
    └── terrain.wgsl               # Bevy terrain shader
```

Only clean output formats are stored: **PNG**, **GLB**, **JSON**, **WGSL**.
No legacy formats (BMD, OZJ, OZT, OZB, ATT, MAP, etc.) are copied to the output.

---

## Conversion Results

### Season20 Build (latest run, 0 validation failures)

| Asset Category | Emitted | Failed | Notes |
|----------------|--------:|-------:|-------|
| Models (GLB) | 12,258 | 0 | From 12,367 BMD files (36 no-geom, 73 non-model) |
| Textures (PNG) | 18,634 | 3 | 3 corrupt source files |
| Terrain Height (JSON) | 80 | 0 | |
| Terrain Map (JSON) | 82 | 0 | |
| Terrain Attributes (JSON) | 88 | 0 | Includes Season16+ ModulusDecrypt |
| Scene Objects (JSON) | 82 emitted | 82 failed | .obj encryption unknown for Season20 |
| **Validated** | **31,389** | **0** | |

### 5.2 Build (legacy reference)

| Metric | Value |
|--------|------:|
| Legacy source files | 11,991 |
| Converted output files | ~11,500 |
| Validation pass rate | 100% |

### Textures

Deduplication resolves cases where the same texture exists in multiple formats
(e.g., `tree.OZT` and `tree.OZJ`). Priority order: OZJ > JPG > PNG > BMP > TGA > OZT > OZB > OZP.

---

## JSON Sidecar Formats

Full field-by-field schemas are documented in
[`rust/docs/client/assets_data.md`](../../docs/client/assets_data.md).

### terrain_height.json

256x256 grid of height samples with metadata for Rust client reconstruction.

```json
{
  "width": 256,
  "height": 256,
  "heights": [[...], ...],
  "metadata": {
    "source": "TerrainHeight.OZB",
    "source_bits_per_pixel": 8,
    "height_multiplier": 1.5,
    "height_offset": 0.0,
    "legacy_terrain_scale": 100.0,
    "source_unique_values": 100,
    "source_min": 85,
    "source_max": 207,
    "normalized_sample_min": 85.0,
    "normalized_sample_max": 207.0
  }
}
```

**Height reconstruction in the Rust client**:
- 8-bit: `world_height = sample * height_multiplier`
- 24-bit: `world_height = sample + height_offset` (typically -500.0)

### *.map.json (terrain tile mapping)

Per-tile texture layer indices and alpha blending values (256x256).

```json
{
  "header": { "version": 0, "map_number": 1 },
  "terrain_size": 256,
  "layer_stats": { "layer1": {...}, "layer2": {...}, "alpha": {...} },
  "layer1": [[...], ...],
  "layer2": [[...], ...],
  "alpha": [[...], ...]
}
```

### *.att.json (terrain attributes)

Per-tile attribute flags (TW_SAFEZONE, TW_NOMOVE, TW_WATER, etc.). Supports
both 8-bit (standard) and 16-bit (extended) formats.

```json
{
  "header": { "version": 0, "map_number": 1, "width": 0, "height": 0 },
  "is_extended": false,
  "terrain_size": 256,
  "terrain_data": [[...], ...]
}
```

### scene_objects.json

All 3D objects placed in a world with resolved GLB model paths. Coordinates are
swizzled to Bevy Y-up convention.

```json
{
  "objects": [
    {
      "id": "obj_00000", "type": 148,
      "model": "data/Object1/Object149.glb",
      "position": [1234.5, 170.0, 5678.9],
      "rotation": [0.0, 45.0, 0.0],
      "scale": [1.0, 1.0, 1.0],
      "properties": { "model_renderable": true }
    }
  ],
  "metadata": { "source": "...", "version": 0, "object_count": 500, ... }
}
```

### camera_tour.json (normalized)

Camera flythrough path with Bevy coordinates, seconds instead of frames.

```json
{
  "waypoints": [
    {
      "index": 32896,
      "position": [12800.0, 490.0, 12800.0],
      "look_at": [13150.0, 170.0, 12800.0],
      "move_acceleration": 16.0,
      "distance_level": 8.0,
      "delay": 0.0
    }
  ],
  "loop": true,
  "blend_distance": 300.0,
  "interpolation": "smooth"
}
```

### terrain_config.json

Auto-generated placeholder with terrain size, height multiplier, texture layer
references, alpha map, and lightmap paths.

---

## Known Issues and Limitations

1. **Animation export is skeletal-only and normalized**: The converter now exports
   BMD actions as glTF animation clips with skin/joints. Current limitations:
   keyframe timestamps are normalized at 30 FPS, action `PlaySpeed`/`Loop` tuning
   from runtime code is not serialized, and vertices are emitted with single-joint
   weights (`WEIGHTS_0 = [1,0,0,0]`).

2. **Material conversion remains approximate**: The converter emits basic glTF
   PBR materials (`metallicFactor=0`, `roughnessFactor=1`) with texture binding
   and embedded payloads when available. Legacy additive behavior is inferred
   from alpha profiles and RGB-key texture signals, with fallback hints for
   known models. UV animation and special legacy shader programs are not
   preserved.

3. **Season20 OBJ encryption unsolved**: The 82 `.obj` scene object files in the
   Season20 build use an unknown encryption scheme. All known methods
   (MapFileDecrypt, Xor3Byte, ModulusDecrypt, LEA-256, all combinations) produce
   garbage. File sizes match a `4 + N*30` pattern (same-size encryption, not
   ModulusDecrypt which changes size). The converter emits placeholder scene objects
   for these worlds.

4. **World1 and World19 terrain heights**: World1 has a corrupted BMP header
   (recovered via fallback). World19 is entirely zeros (empty file). Both produce
   valid but potentially inaccurate terrain.

5. **Off-by-one in terrain files**: Unencrypted `.att` and `.map` files may be
   1 byte short due to a bug in the original C++ `SaveTerrainAttribute()` /
   `SaveTerrainMapping()`. The converter pads with a zero byte.

6. **LEA-256 performance**: The pure Python LEA-256 implementation processes
   ~500 BMD files/min. A C extension could significantly speed this up.

7. **BMP row order**: BMP files store rows bottom-to-top by default. The converter
   handles this by reversing row order during extraction. Top-down BMPs
   (negative height) are also supported.

---

## C++ Source References

All format reverse engineering was based on these C++ source files:

### BMD Parsing
| File | Key Lines | Content |
|------|-----------|---------|
| `cpp/MuClient5.2/source/ZzzBMD.cpp` | 2530-2625 | `Open()` — BMD v0x0A parsing |
| `cpp/MuClient5.2/source/ZzzBMD.cpp` | 2730-2830 | `Open2()` — BMD v0x0C parsing |
| `cpp/MuClient5.2/source/ZzzBMD.h` | full | Struct definitions (Triangle_t, Triangle_t2) |
| `cpp/MuClientTools16/_src_/Core/BMD_SMD.cpp` | 156-195 | `FixUpBones()` — bone hierarchy transform |
| `cpp/MuClientTools16/_src_/Core/BMD_SMD.cpp` | 330-556 | `ReadBmd()` — binary parsing |
| `cpp/MuClientTools16/_src_/Core/BMD_SMD.cpp` | 601-763 | `Bmd2Smd()` — vertex transform + UV flip |
| `cpp/MuClientTools16/_src_/Core/BMD.h` | full | Tools struct definitions (with `DWORD Nothing[7]`) |

### Math
| File | Key Lines | Content |
|------|-----------|---------|
| `cpp/MuClient5.2/source/Math/ZzzMathLib.cpp` | 228-256 | `AngleMatrix()` |
| `cpp/MuClient5.2/source/Math/ZzzMathLib.cpp` | 288-314 | `R_ConcatTransforms()` |
| `cpp/MuClient5.2/source/Math/ZzzMathLib.cpp` | 340-345 | `VectorTransform()` |
| `cpp/MuClient5.2/source/Math/ZzzMathLib.cpp` | 179-198 | `VectorNormalize()` |

### Terrain
| File | Key Lines | Content |
|------|-----------|---------|
| `cpp/MuClient5.2/source/ZzzLodTerrain.cpp` | 619-670 | `OpenTerrainHeight()` — 8-bit loading |
| `cpp/MuClient5.2/source/ZzzLodTerrain.cpp` | 702-751 | `OpenTerrainHeightNew()` — 24-bit loading |
| `cpp/MuClient5.2/source/ZzzLodTerrain.cpp` | 609-617 | `IsTerrainHeightExtMap()` — format selection |
| `cpp/MuClient5.2/source/ZzzLodTerrain.cpp` | 68 | `g_fMinHeight = -500.f` |
| `cpp/MuClient5.2/source/MapManager.cpp` | 1395-1403 | `CreateTerrain()` call site |
| `cpp/MuClient5.2/source/_define.h` | 78-93 | `TW_*` attribute flag definitions |

### Encryption
| File | Key Lines | Content |
|------|-----------|---------|
| `cpp/MuClientTools16/_src_/Core/MuCrypto.cpp` | 65-77 | `MapFileDecrypt()` |
| `cpp/MuClientTools16/_src_/Core/MuCrypto.cpp` | 79-160 | `InitModulusCrypto()` — cipher selection |
| `cpp/MuClientTools16/_src_/Core/MuCrypto.cpp` | 220-262 | `ModulusDecrypt()` |
| `cpp/MuClientTools16/_src_/Core/MuCrypto.h` | full | Key definitions |
| `cpp/MuClient5.2/source/ZzzLodTerrain.cpp` | 113-119 | `BuxConvert()` / Xor3Byte |
| `cpp/MuClientTools16/_src_/Core/ATT.cpp` | full | ATT decrypt wrapper |
| `cpp/MuClientTools16/_src_/Core/MAP.cpp` | full | MAP decrypt wrapper |
| `cpp/MuClientTools16/_src_/Core/OBJ.cpp` | full | OBJ decrypt wrapper (MapFileDecrypt only) |

### OZB/Texture Containers
| File | Key Lines | Content |
|------|-----------|---------|
| `cpp/MuClientTools16/_src_/Core/GraphicConvertor.h` | 71-165 | Template class with PAD-based encrypt/decrypt |
| `cpp/MuClientTools16/_src_/Core/OZB.h` | full | `OZB = GraphicConvertor<EXT_OZB, 4>` |
