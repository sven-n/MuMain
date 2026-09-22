# Lorencia ground tiles

**Exported and validated offline; client verification pending.**

Seventeen existing `Tile*.OZJ` filenames are replaced; each payload is a 512×512 RGB JPEG.
See the [inventory](../inventory.md) for original dimensions, surfaces and loader indices.
The source paintings were generated with the built-in imagegen tool; exact prompts are in
`../generation-manifest.json`. They are retained in `paintings/`.

`build_source.py` uses Blender image APIs to resize and prepare repeat sampling, then packs
the resulting 512×512 PNG masters into `source.blend`. A 24-pixel cubic correction band
matches opposite boundary samples without mirroring whole images. The original generated
paintings remain unmodified. `package_assets.py` encodes RGB JPEG at quality 100, wraps with
`mu_texture.py`, checks every export and measures the **decoded JPEG** edges.

`seam-report.json`: worst mean edge difference is 0.424/255; maximum channel difference is
5/255 after lossy compression. `repeat-review.jpg` shows every final JPEG repeated 3×3 at
128 pixels per tile. Repetition and material differences were reviewed offline; these are
not proof of gameplay-distance readability or terrain-layer compatibility in the engine.

Alpha strips are unchanged: Grass01 256×64, Grass02 256×64, Grass03 256×128, Ground01 256×64.
TerrainLight.OZJ (256×256) is unchanged. No height, mapping, placement, walkability or minimap
data was edited. The source Blender planes are review aids, not replacement terrain models.

Bounds and bmdconv comparison are not applicable to diffuse terrain texture replacements.
