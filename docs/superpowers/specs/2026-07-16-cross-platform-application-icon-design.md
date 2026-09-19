# Cross-Platform Application Icon Design

## Goal

Give MuMain a recognizable application and window icon on Windows, macOS, and Linux while preserving the existing Windows icon's identity.

## Artwork

- Retrace `src/source/App/Platform/Windows/icon1.ico` as high-resolution vector-style artwork.
- Preserve the existing star/emblem composition, central red details, gold-orange palette, and transparent outer silhouette.
- Correct uneven geometry, symmetry, jagged edges, muddy color transitions, and details that collapse at small sizes.
- Use restrained metallic highlights and dark separation for clarity.
- Do not add a shield, badge, background tile, glow-heavy treatment, monogram, or replacement symbol.

## Outputs

- One canonical high-resolution source image.
- Windows multi-resolution `.ico` containing standard desktop icon sizes.
- macOS `.icns` containing Retina and non-Retina sizes.
- Linux PNG icon sizes suitable for SDL window managers and desktop packaging.

## Platform Integration

### Windows

- Replace `src/source/App/Platform/Windows/icon1.ico` with the restored multi-resolution icon.
- Keep the existing `resource.rc` icon binding.

### macOS

- Build `Main` as a macOS application bundle.
- Set `MACOSX_BUNDLE_ICON_FILE` and copy the `.icns` file into bundle resources.
- Preserve current command-line launch support through `./ctl run` by teaching it the bundle executable path when needed.

### Linux

- Load a bundled PNG after SDL window creation.
- Call `SDL_SetWindowIcon()` so supported window managers show the MuMain icon in task switchers and window decorations.
- Keep desktop-entry/AppImage packaging outside current scope unless packaging already exists.

## Failure Handling

- Missing or unreadable icon assets must not stop game startup.
- Log one warning and continue with platform default icon.

## Validation

- Windows executable shows restored icon in Explorer and taskbar.
- macOS application shows restored icon in Finder, Dock, and app switcher.
- Linux window shows restored icon in supported taskbars and switchers.
- Icon remains recognizable at 16, 32, 48, 128, 256, 512, and 1024 pixels.
- Debug and Release launch paths still work.

## Scope Ceiling

No installer, launcher UI, desktop-entry generator, AppImage, DMG, or code-signing work. Add those when distributable packaging becomes an explicit requirement.
