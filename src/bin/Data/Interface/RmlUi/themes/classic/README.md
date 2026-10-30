# Classic theme

The interface of the original 0.97 client, drawn with its own art.

- **Inherits from `legacy`** (`theme.ini`, `[Theme] Base=legacy`). Ship a window's `.rcss` (or a
  `.rml` fork) here only when restyling that window; everything else, tokens included, comes
  from `legacy`.
- **Art** is the original files, unconverted:
  - `Data/Interface/Classic/` — the 0.97 client's `Data/Interface`;
  - `Data/Interface/Classic/099b/` — classic-era files the 0.99b client adds (not in 0.97);
  - `Data/Interface/Classic/s1/`, `s2/` — files of the Season 1 and Season 2 clients, which kept
    the classic interface, for the parts and windows 0.97 does not have (the bar's thirds with the
    command window and friend buttons' boxes and the SD column, the quest, pet and personal shop
    buttons, the duel crest, the quest book, Season 1's text button, the Season 2 cash shop's
    pieces);
  - `Data/Logo/Classic/` — the 0.97 login box and character select art.
- **Paths** are relative to the stylesheet: from a stylesheet here, `../../../Classic/Menu01_new.jpg`
  and `../../../../Logo/Classic/0box.jpg`. The engine opens `.jpg` as `.OZJ` and `.tga` as `.OZT`.
- **Sizes**: an `@spritesheet` gives each sprite its real size.
  `tools/classic_theme/oz_texture_sizes.py ../../../Classic --recursive` lists them.
- **Derived textures**: where 0.97 tiles or stretches a texture over a known size, the result ships
  beside the originals (`Nis_vframe_tall`, `Message_box2_268`, `Message_box2_230`); where the
  original drew a texture additively (its black ground vanishing), a copy with that as alpha
  (`Progress_add`, `s1/DuelWindow_add`). All built by
  `tools/classic_theme/make_classic_derived_textures.py`.
- **C++**: the HUD's extra parts are fields of `CMainFrameWindow`'s model (`UI/HUD/MainFrameWindow`).

Comments in each stylesheet name the 0.97 client function it follows (its address in `main.exe`)
and the positions in the original 640x480 frame. A window 0.97 does not have follows the Season 1
or Season 2 client when that client has it (the function's address in that client's `main.exe`),
else it takes the parts of the closest classic window.
