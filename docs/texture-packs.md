# Texture packs

A texture pack replaces the game's textures with its own files, for example the same art at 2
or 4 times the resolution. Packs never change the files in `Data/`, so going back to the
original textures is a matter of choosing them in the options.

## Installing a pack

Each pack is a folder in `Data/TexturePacks/` that mirrors `Data/`:

```
Data/World1/TileGrass01.OZJ                          original
Data/TexturePacks/upscaled/World1/TileGrass01.OZJ    read instead while "upscaled" is active
```

1. Copy the pack folder into `Data/TexturePacks/`.
2. Open the options, **Interface** tab, and choose the pack under **Texture Pack**.
3. Restart the game.

One pack is active at a time. For each `.OZJ` and `.OZT` texture the client reads the copy in the
active pack when there is one and the original otherwise, so a pack may cover only part of the
art. File and folder names in a pack match in any case, also on Linux. The terrain lightmaps
(`TerrainLight.OZJ`) are map data, not art, and are always read from `Data/`.

A copy in the pack that can't be loaded (a damaged file, or an image larger than the limits below)
is written to the log with its path in the pack, and the client loads the original instead.

The choice is stored as `TexturePack` in the `[Render]` section of `config.ini`. The client lists
the active pack's files when it starts, so files added to a pack while the game runs are used
after a restart. The list in the options is read from `Data/TexturePacks/` each time the window
opens.

A pack's folder name may contain only ASCII letters, digits, `-` and `_`; other folders are not
listed. A pack that is chosen but no longer installed falls back to the original textures, with a
warning in the log.

## Removing a pack

Choose **Original** under **Texture Pack** and restart the game, then delete the pack's folder.

## Making a pack

A pack holds OZJ and OZT files under the same paths as in `Data/`. A file at the original size
needs nothing more. A file at 2 or 4 times the original size has to be marked, so the game keeps
working with the original size: its first 4 bytes are `M`, `U`, `X` and the scale (2 or 4). The
loaders skip those bytes, so they carry no image data. Without the marker the client takes the
larger image for the original size, which breaks how it is drawn.

- A marked image must be exactly the scale times the original in both directions.
- A marked image may be up to the scale times 1024 pixels on each side.
- Textures the game draws on at runtime (`Interface/guild.tga` for guild marks and
  `Object31/Flag.tga` for the castle flag) are brought back to the original size when they load,
  so upscaling them gives no gain.
