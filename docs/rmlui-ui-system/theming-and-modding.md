# Theming & Modding

How to edit and select themes, use shared design tokens, and supply custom assets. The client
ships `legacy`, `modern` and `classic` (the 0.97 interface, below), plus `legacy-inherited-test`
for checking inheritance without visual overrides. Additional theme folders work without recompilation and can inherit an existing theme.
A separate user/mod override layer is not implemented.

## Shared markup and theme-specific structure

Start with a window's shared RML and `legacy` stylesheet to understand its compatibility layout.
The C++ bindings define the required ids, model fields and event callbacks; the theme defines
their presentation. Keep shared markup theme-neutral and use RCSS when the same structure serves
both themes. A theme-specific RML override is appropriate when a different composition cannot
reasonably use the shared structure. Preserve the behavioral contract in either case.

The shared file stays theme-neutral: theme-specific class vocabulary (`modern-frame`,
`modern-panel`, …) belongs in that theme's fork, never in the shared `.rml` (`login.rml`,
`msg_win.rml` and `remember_password_prompt.rml` are modern's earliest forks for this reason).

**Forking safely needs a check.** Run `python tools/check_rml_rcss_drift.py` before shipping
theme edits; it also runs during the build. Each theme must provide the literal ids the window
looks up and callbacks its view registers, including linked templates and shared-document
fallback. Fields offered by a model must appear in at least one of its document/theme variants,
so different full/current readouts remain legal. Comments and quoted expression text cannot
satisfy a missing name. The report includes document/theme/file counts; zero discovery fails.

Use `--review` to inspect discovered names and intentional omissions with their reasons. A
control intentionally absent from a variant can carry
`<!-- rml-contract-optional-id: name: reason -->` or
`<!-- rml-contract-optional-callback: name: reason -->`, after confirming its C++ path tolerates
the omission. Unused model fields need a `// rml-contract-unused: name: reason` comment in the
owning source. Exceptions fail when their names disappear from the discovered contract or become
used again; repair required controls instead of exempting them.

The guard checks literal source contracts, not actual visibility, class styling, model-name
correctness or interactions. Dynamic ids, selectors, struct members and arbitrary registration
helpers require manual review and running-client checks.

## How theme resolution works

A theme is identified by **folder name**, not a closed C++ enum, specifically so adding one
(including a fully programmatic, sprite-free one) is normally a zero-source-change,
drop-a-folder operation. Reopen Options after adding or removing a theme to refresh its list;
the list also refreshes after switching themes.

`config.ini`'s `[UI] RmlTheme=<name>` selects the active theme, read once at startup by
`GameConfig::GetRmlTheme()` and cached by `UI::RmlBridge::GetActiveThemeName()`. That cache is
also live-mutable via `UI::RmlBridge::SetActiveThemeName()` — see
["Switching themes without relaunching"](#switching-themes-without-relaunching) below. A
missing or invalid saved theme falls back to `legacy`, with a diagnostic in the error log. If
`legacy` is also unavailable, restore the bundled theme files before launching. Loading a
window goes through its `ThemedView`, which hands `"Data/Interface/RmlUi/login.rml"` to
`UI::RmlBridge::ThemedDocumentLoader::Load()`:

1. It reads `login.rml` from the active theme, then its ancestors, then the shared markup.
2. It builds a synthetic source URL, `Data/Interface/RmlUi/themes/<name>/login.rml` — this path
   need not exist on disk.
3. It calls `Rml::Context::LoadDocumentFromMemory(text, syntheticSourceUrl)`.
4. RmlUi resolves the document's `<link href="login.rcss"/>` relative to that synthetic URL,
   landing on `Data/Interface/RmlUi/themes/<name>/login.rcss`. The file can come from that theme
   or an ancestor. Its `token(name)` markers use the selected theme's merged tokens; relative
   links and assets keep the selected theme's virtual path so child overrides apply throughout.

This is confirmed against RmlUi's own source: `Context::LoadDocumentFromMemory` wraps the string
in a `StreamMemory` and calls `SetSourceURL()` on it before parsing; `XMLNodeHandlerHead.cpp`'s
`MakeExternalResource()` resolves `<link href>` via `AbsolutePath(path, parser->GetSourceURL())`.

Every migrated window renders its chrome through RmlUi, in every theme — no native sprite path
draws underneath it. A "legacy-look" theme reproduces the
original art by pointing its own RCSS decorators at the same image files the old sprites used;
a "modern" theme uses flat colors/vector shapes instead.

## Forking a theme's RML

`UI::RmlBridge::ThemedDocumentLoader::Load()` ([`RmlTheme.cpp`](../../src/source/UI/RmlBridge/RmlTheme.cpp))
looks for `themes/<theme>/<name>.rml` first, then each ancestor, then the shared `<name>.rml`.
`modern` forks a
few dozen documents; `themes/modern/*.rml` is the live list. Most of these exist so `modern`'s own `.modern-frame`/`.modern-frame-crimson`/`.title-glow` class vocabulary
doesn't have to live in the shared, theme-neutral file (the Shared markup section above).

`main_frame.rml` is an example of shared markup with different presentation. Legacy shows the
original bottom button row; modern uses `main_frame_top.rml` and flat backing elements.
The shared document carries every part either
theme uses, and each theme's `main_frame.rcss` hides the others (`display: none`). Where the two
themes bind different values to the same spot, the markup carries both variants and each theme
shows one: a gauge's readout as `.value-current` or `.value-full`. The document is also split into parts (`.hud-part`: the strip backing, each
gauge group, EXP, buttons, item hotkeys, skill row, skill list), each placed by its own rule in
the theme's `main_frame.rcss` with its contents laid out from its own corner, so a theme moves
or hides a part without touching the others. The skill list's grid fans out from its part's box,
the first cell's position, so it docks wherever the theme puts that box.

**Fork the RML only when a theme needs markup the shared file cannot reasonably carry** — prefer
adding the element to the shared file and letting the other theme hide it. A fork's
ids/classes/bindings have to be kept in step with the shared file by hand;
`tools/check_rml_rcss_drift.py` catches missing required literal ids/callbacks and unused model
fields; visual structure and interaction still need testing in the client.

## Adding a theme folder

For development builds, keep themes under `src/bin/Data/Interface/RmlUi/themes/`.
Asset staging replaces the runtime RmlUi directory with this source tree, so a theme added
only beside `Main.exe` is removed on the next asset copy. `legacy-inherited-test` supplies
only `theme.ini` and should look identical to `legacy`, including the login scene.

A custom theme can contain only the resources it changes. For example, a `classic` theme can
inherit `legacy` by putting this in `themes/classic/theme.ini`:

```ini
[Theme]
Base=legacy

[Meta]
DisplayName=Classic
```

Resolution is **child → declared ancestors**, with shared markup as the final document fallback.
RML, templates, RCSS and theme-local images use the first matching whole file. An overridden
stylesheet replaces its ancestor's file; rules are not automatically combined. Paths to common
assets outside the theme folder keep their ordinary meaning.

`[Tokens]`, `[Capabilities]`, `[Fonts]` and `[Stacking]` inherit individual keys. A child value wins, including
`NativeTextSize=0` or an empty font entry. Display names stay specific to the child. Names use
letters, digits, `_` and `-` and are normalized to lowercase. Missing ancestors, invalid names
and inheritance cycles make a theme unavailable for selection.

`[Stacking]` moves one of the theme's documents in the stacking order, when its layout makes two
documents overlap the other way round. The classic theme puts its chat input over the windows,
the HUD and the command windows, and under the settings window and the tooltips
(`chat_input.rml=10.66`; the command windows are 10.65, settings 10.67, tooltips 10.69). Avoid a
depth another document already has: documents at the same depth are ordered by focus. The depths
are the original windows' layer depths, listed in `UI/RmlBridge/RmlStackingOrder.cpp`; a document
the theme does not list keeps its own. For a document shown in every scene (`option_window.rml`),
the entry applies in the main scene only: outside it the document takes its depth for the other
scenes from that table.

1. Create `src/bin/Data/Interface/RmlUi/themes/<your-theme-name>/`.
2. Declare `Base=legacy` or `Base=modern` in `[Theme]`, or supply your own complete stylesheets.
   Every window links `base.rcss` for shared rules such as buttons, checkbox states, backdrops
   and the `body { pointer-events: none; }` reset. It must be available in the theme's chain.
3. Override only the `.rcss` or `.rml` files you want to change, preserving the required ids and
   bindings. Without a base, supply every stylesheet the markup links; a missing sheet leaves
   that window partly unstyled. Run the contract guard before testing in the client.
4. Select it in **Options → UI → Theme**, edit `config.ini`'s `[UI]` section:
   `RmlTheme=<your-theme-name>`, or preview it live without
   editing anything — type `$theme <your-theme-name>` in chat (see
   ["Switching themes without relaunching"](#switching-themes-without-relaunching) below).
5. Relaunch (if you edited `config.ini`). No rebuild needed either way — this is a pure data/config
   change.

## Map-name banner

The world-entry banner shows localized map names in both maintained themes. Its original
166 x 90 ornament uses a swirl on each side and the map's own icon between them. Text has a
separate, wider field (up to 384dp and 90% of the viewport) so long names keep their proportions.
Adding a map name no longer needs an individual name image. Unrecognized maps and the GM area do
not show a banner; other map-name displays use “Unknown map” when no name is defined. The Gens
warning still appears at its original readable size on eligible battle maps.

The pieces are in `Data/Interface/MapNameOrnament`, cut out of the original images by
`tools/MapNameFont/ornament.py`:

- `SwirlLeft.OZT`, `SwirlRight.OZT` and one icon per original image (`lorencia.OZT`, ...).
- `Layout.json`: where each piece sat in the 166 x 90 image, in its pixels; the banner places
  each map's icon there.
- `MapIcons.json`: which icon each map shows, by map number. Give a new map an icon by adding
  `{ "map": <number>, "icon": "<icon file name>" }`; a map without an entry shows only the swirls.

Edit a theme's `map_name.rcss` to change the font size, colors, swirl places or the banner's
placement. A name wider than the text field is reduced by changing its font size, preserving
the letters' aspect ratio. Text is rasterized at its final screen size with a thin dark outline,
without a glow or a transform that stretches a smaller glyph bitmap. The silver gradient follows
the fitted font size. The banner uses `dp` layout directly; icon coordinates in `Layout.json`
are reference pixels interpreted as `dp`.

The map banner uses MU Map Name, a font bundled in `fonts/MuMapName-Italic.otf` and generated
from the original client's map-name images (`tools/MapNameFont`), so any name in Latin, Polish
or Cyrillic is written in the original lettering. It is italic only, at normal weight. The banner
uses the bundled Noto fallback faces for Chinese. Japanese names are drawn in Noto Sans JP
(`font-map-name-ja`, chosen while the game's language is Japanese) so that one name does not mix
the Chinese and Japanese forms of the same characters; they keep the banner's baseline. When the
`font-map-name` family isn't available, the banner uses the token named by
`font-map-name-fallback` (the bundled themes name `font-title`, so it gets the title font,
including the player's selected title font) in that token's style. Liberation Sans is the final
fallback if no face is available. An unavailable RmlUi document leaves the banner hidden; there is
no bitmap fallback.

Change the family with `font-map-name` in the theme's `tokens.ini`, keeping the paired
`font-family` and `font-style` token references in `map_name.rcss`. `font-map-name-style=italic`
names the face to prefer; a family token with a `-style` setting may resolve to an italic-only
family, while a token without one needs a regular face. A theme can name a locally installed font
such as Monotype Corsiva after loading its file in `theme.ini`'s `[Fonts]` section (see Optional
system fonts). If the named style is unavailable, an available style is used so the name remains
visible. The same `<font-token>-style` and `<font-token>-fallback` settings work for other theme
font tokens; a fallback chain stops after a few steps. Other UI fonts retain their existing
selection settings.

## Switching themes without relaunching

`$theme <name>` (typed into the chat box, handled locally by `CmuConsoleDebug::CheckCommand` —
never sent to the server, same bucket as `$fps`/`$vsync`) hot-swaps the active theme for the
current session: `$theme modern`, `$theme legacy`, or any modder-supplied folder name.

Mechanically: it validates the inheritance chain and a readable `base.rcss`, supplied by the
theme or an ancestor (`UI::RmlBridge::ThemeExists()`) —
an unknown name is rejected with no state change, not left to fail silently per-window — then
calls `UI::RmlBridge::SetActiveThemeName()` to update the live cache and
`UI::RmlBridge::ReloadAllThemedDocuments()` to invoke every registered theme-reload callback.
Every themed document belongs to a `UI::RmlBridge::ThemedView` (`component-catalog.md`'s Theming
section), which registers for theme switches the first time it builds. On a switch each view keeps
its model's values, unloads its documents, loads them again from the active theme and shows again
the ones that were visible; views rebuild from the bottom of their context's stack up, so the
stacking survives. A window that was never opened has nothing to rebuild and picks up the active
theme when it first builds. The free-function modules (the tooltip, notices, the remember-password
prompt) hold a namespace-scope view and work the same way.

**The chat command is session-only.** `$theme` does not write to `config.ini`. For a selection
that survives a restart, use **Options → UI → Theme** or edit `[UI] RmlTheme` while the client
is closed. The Options control uses `UI::Theme::Select(..., Persistence::Saved)` and rebuilds
documents after the click dispatch has finished.

**The maintained first-party themes are `legacy` and `modern`.** Custom folders use the same
loading mechanism. The architecture policy's Custom/Test-theme checks seek to expose visual
coupling; the current project uses modern's structural differences and the contract guard for
that coverage. Update both built-in themes when window content or behavior changes.

### `#backdrop` usage

```html
<div id="backdrop"></div>
<div id="panel" data-model="...">
    ...
</div>
```

Placed as the *first* child of `<body>`, before the panel, so it paints underneath. Unlike the
panel's children, `#backdrop` sets `pointer-events: auto` directly (not inherited) — a full-screen
dim backdrop is meant to consume clicks underneath it (matching how a full-screen legacy dialog
already treated the whole screen as "in this window"), not create a click-through hole.

## Bringing your own images to a theme

A theme is free to reuse the original game art or bring entirely new image assets (a custom
background, custom button art, a logo). Both go through the same path: any image an RCSS file
references loads through this engine's normal texture pipeline
(`RmlUiRenderInterface::LoadTexture` → `CGlobalBitmap::LoadImageExclusive`). No engine changes are needed
to use it — but there are real constraints worth knowing first.

**Path resolution**: a relative image path inside an RCSS rule resolves against *that
stylesheet's own location*, not the shared `.rml`'s synthetic per-theme URL — so a custom theme's
image can simply sit next to its own `.rcss`:

```
themes/<your-theme-name>/
    login.rcss
    panel_bg.tga      <- referenced from login.rcss below
```

```rcss
#panel {
    decorator: image( panel_bg.tga );
}
```

**Only this engine's proprietary OZT/OZJ containers are actually read, not plain PNG/JPG/BMP.**
`CGlobalBitmap::LoadImage` only recognizes two extensions, `.jpg`/`.tga` — and **both internally
swap the extension to `.OZJ`/`.OZT` before opening.** Concretely: if your RCSS references
`panel_bg.tga`, the file that actually needs to exist on disk is `panel_bg.OZT`, not a real
`.tga`. There is no PNG/JPG→OZT/OZJ converter in this repository — producing one needs an
external tool from the wider MU private-server modding community (the format predates this
project).

**RmlUi file images load at their original dimensions**, including non-power-of-two images.
Use `@spritesheet` when selecting a rectangle from an atlas, or when a named sprite makes the
stylesheet clearer:

```rcss
@spritesheet panel-bg-sheet
{
    src: panel_bg.tga;
    panel-bg-image: 0px 0px 329px 245px;  /* the image's REAL, unpadded pixel size */
}

#panel {
    decorator: image(panel-bg-image);   /* the sprite name, not "panel_bg.tga" directly */
}
```

File images and atlas sprites support `repeat`, `repeat-x` and `repeat-y` in this fork.
Repeating an atlas sprite generates geometry per tile; see
[Engine Findings](engine-findings.md#rmlui-build) for the behavior and remaining limits.

**Failures are completely silent.** A missing file or unsupported extension returns `false` from
`LoadImage` with no log line at all — the element just renders with no image, no error, no
diagnostic. If a custom theme's image isn't showing up, check the extension-swap rule above (a
real `.tga` file, or a file with the wrong internal container, both fail exactly the same silent
way) before assuming something else is wrong.

Plain-image loading is not implemented for theme assets. A custom theme must supply the
containers described above.

## Design tokens: `themes/<theme>/tokens.ini`

Both built-in themes have a `tokens.ini` file (`[Tokens]` section, plain `name=value` lines) that a theme's own `.rcss`
files reference via a `token(name)` marker instead of repeating a literal value everywhere it's
used. RmlUi follows each document's ordinary external `<link>` and loads the themed `.rcss`
through the game's file interface. That interface resolves `token(name)` from the selected
theme's `tokens.ini`, then its ancestors, before RmlUi parses and caches it: plain text substitution, not a CSS
feature. CSS custom properties (`var()`) and `calc()` work too, for a value derived from another
inside the theme (the item grids' `--item-cell`, `item_grid.rcss`). A new theme needs only its own
files; it does not need a C++ change. The stylesheet cache is cleared when a theme is selected
again, so editing theme files and reselecting that theme refreshes their values.

If a stylesheet refers to a token absent from the entire chain's `[Tokens]` sections, the client logs
the token name and stylesheet path in `MuError.log`. The missing value still expands to an empty
string. Add the key to `tokens.ini` and reselect the theme to reload the stylesheet.

**What a token is for**: a *reusable, theme-level semantic choice* — the standard body text color,
a shared muted/secondary text tier, the common tooltip backing, a shared accent/highlight, a
warning/danger color, shared scrollbar track/thumb colors, a shared corner-radius. The test isn't
"does this literal appear more than once" — it's "does changing this value represent one coherent
design decision a theme author would actually want to make in one place." `legacy`'s own
`tokens.ini` documents, token by token, which real recurring selector(s) motivated it; a token with
no real call site backing it (invented for taxonomy-completeness alone) doesn't belong.

**What should stay ordinary RCSS, not a token**:
- **One-off decorative colors** — a single window's own specific accent choice with no cross-window
  or cross-selector reuse. Most of a theme's literal colors are legitimately this; not every color
  needs a lever.
- **Gameplay colours** — gauge gradient stops around a base fill, the cooldown wipe, a status
  colour such as poisoned HP or server load, and white-outlined text over the 3D world. They mean
  something in the game, not in the theme.
- **Content-driven/asset-driven palettes** — `tooltip.rcss`'s `.tt-blue`/`.tt-red`/`.tt-yellow`/etc.
  rich-text colors (and their `.tt-hl-*` background-highlight pairs) are the exact RGB values
  `RenderTipTextList()`'s native `TEXT_COLOR_*` switch (`ZzzInventory.cpp`) already used — they
  identify a *content type* (an item's rarity tier, a warning message), not a theme aesthetic
  choice, and both `legacy`'s and `modern`'s own copies of this file deliberately leave them as
  literals with a comment saying so. A ninepatch/gradient decorator recipe tightly bound to one
  specific sprite composition (`server_select.rcss`'s own button-tint overlays) is the same
  category — asset-specific, not a theme palette lever.
- **Structural geometry** — panel width/height, row height, pixel offsets, icon pitch, spritesheet
  rects, native-companion-synchronized positions (anything [`README.md`](README.md)'s Known limits records as an exception). A value becoming
  `token(foo)` does not make duplicated geometry architecturally themeable — that requires actually
  deriving the position live (or, where that's currently impractical, an explicit pinned
  cross-reference comment on both sides). Tokens are a color/typography/radius mechanism, not a substitute
  for the geometry-decoupling work tracked separately.

Each theme's own token meanings: `themes/modern/README.md`; `themes/legacy/tokens.ini`'s comments.

**Cross-theme naming**: reuse a `modern` token's NAME for a `legacy` token when the semantic ROLE
genuinely matches (`text-primary`, `font-body`, `radius-sm` all exist in both, with each theme's
own appropriate value — `legacy`'s `text-primary` is `#ffffff`, `modern`'s is a warm off-white).
Don't force a shared name onto two concepts that don't actually match, and don't rename an existing
token in one theme purely for cosmetic cross-theme symmetry — a real naming inconsistency is worth
fixing, a theme's own genuinely distinct concept (an accent family the other theme doesn't have)
just gets its own name.

## Optional system fonts

List font files in `theme.ini`'s `[Fonts]` section, with one file per key. For example:

```ini
[Fonts]
body=%WINDIR%\Fonts\verdana.ttf
body-bold=%WINDIR%\Fonts\verdanab.ttf
```

Select the loaded family in `tokens.ini`:

```ini
[Tokens]
font-body=Verdana
font-title=Verdana
```

Paths support `%ENVIRONMENT_VARIABLE%`, absolute filenames, and paths relative to the theme
that declared the entry. Missing files or unresolved variables are skipped. Inherited font
entries retain their declaring directory; a child can replace a key or leave its value empty
to disable it. Load each style you need, such as regular and bold.

`font-*` tokens are family names. If the requested family is unavailable, the client uses the
first available family for that token in an ancestor, then bundled Liberation Sans. The bundled
CJK glyph fallbacks remain loaded. Custom literal `font-family` rules must still name a loaded
family; use the tokens for optional system fonts.

The player's font choice in the options (`[UI] Font`) overrides `font-body` and `font-title` in
every theme, as long as RmlUi has that family loaded. With `Default`, the theme's own families
apply. Other `font-*` tokens always keep the theme's value. A new choice applies right away: every
themed document is rebuilt with it, the same way as after a theme switch.

`font-native` always names the family the native text renderer uses (the `Font` choice, and
DejaVu Sans for `Default`). It is meant for text RmlUi draws inside a box the native renderer
measured, such as the world labels. Themes do not set it.

Fonts load before themed documents at startup and on theme selection. RmlUi retains loaded
families for the session, so a family loaded by an earlier theme remains available. Reselecting
a theme retries optional files; restart the client to replace a font file whose family is
already loaded. Referencing a player's installed font does not package it with the theme.

## The classic (0.97) theme

`RmlTheme=classic` gives the interface of the original 0.97 client, drawn with its own art. It
inherits `legacy` and restyles the login box, server list, character select and creation, HUD,
inventory, character, party, NPC shop, storage, trade, Chaos Machine, guild, event entries,
message boxes, confirmations, the Esc menus, the command window (D) and chat. The NPC menus that
share the Esc menu's document take the Season 2 client's boxes, or the trainer's box where no
classic client has the menu. Where the Season 1 and
2 clients, which kept the classic interface, add a part for something MuMain has, the theme draws
it as they did instead of hiding it: on the HUD the SD column and the command window and friend
buttons, in the character window the quest and pet buttons, in the inventory the personal shop
button. A window 0.97 does not have follows those clients, else the closest classic window. The
art lives outside the RmlUi tree, in `Data/Interface/Classic` and `Data/Logo/Classic`, so its file
names cannot collide with S6 files of the same name.

What changes for the player, besides the look:

- **Character creation:** the class is chosen with arrows, which step through the classes the
  client enables.
- **World:** no map title when entering a map.

The HUD model also exposes what the classic bar shows and the S6 bar does not: `party_open` /
`guild_open` / `command_open` with `mainframe_party_click` / `mainframe_guild_click` /
`mainframe_command_click` (the windows under the P, G and D keys' rules, `CHotKey`) and their
tooltips, and `coord_x` / `coord_y`.

The character creation model has `job_previous` / `job_next`: the enabled class before and after
the selected one, or -1, for a theme that steps through the classes with arrows
(`charmake_select_job(job_next)`) instead of a button per class.

The pet / durability HUD (`item_endurance.rml`) marks `#pets` with `.in-party` while the hero is
in a party, so a theme can keep the pet column clear of the party list. The classic theme also
places the pet column, the party list and the event HUDs from `--world-left` / `--world-right`, so
they move aside when windows open, as in 0.97.

## Coordinates, scaling, and positioning — what a theme actually controls

### Scaling: a global user setting, opt-in per element

**See [Layout, Anchoring & Scaling](layout-and-scaling.md) for the full policy** — condensed here:
`[UI] UIScalePercent` (default 100) contributes to
`Context::SetDensityIndependentPixelRatio()` through the shared scale hook. A coordinate written
in `dp` uses the context's ratio, which combines viewport fit, OS content scale and the user
setting. A `px` length bypasses that ratio but still follows any ancestor transform. Use `dp`
for directly laid-out scalable content, reference `px` inside transformed panels, and
counter-scaled physical text where the native typography policy applies. Raster assets follow
their element's drawn size; choose their scaling deliberately.

### Panel position vs. element layout vs. draggability — three different owners

- **Layout of elements *within* the panel** — expressed in RCSS, which the theme controls. A few
  windows still take an image file or texel rect from C++ (CryWolf's event art; see
  [README.md](README.md#known-limits)), so a theme
  can rearrange that art but not replace it.
- **Where windows and HUD parts go** — the theme's `workspace.rml`/`workspace.rcss`: regions and
  one slot per window or HUD part, laid out in RCSS; the game places each open window on its slot
  ([window-placement.md](window-placement.md)). What a theme can do there:
  - move a slot to another region, or reorder slots, to change the arrangement — including the
    main HUD, chat, header corners and event HUDs;
  - mark a shell region `data-participation="reserve"` (windows avoid it) or `overlay`;
  - list in a slot's `data-closes` the windows that close for lack of room when it opens;
  - give a slot `data-fit="fill"` and a size in `%`, for windows that support it;
  - cap a dock region with `max-height: 100%` so its windows scale down to fit.

  Worked examples (split docks, centred inventory, a side bar, chat on the right, the HUD at the
  top) are in window-placement.md's "Theme recipes". Windows without a slot keep the rule below.
- **The panel's own position on screen** for a window without a slot — its own RCSS anchoring
  (`.center-both`, `position: absolute; right: 0; bottom: 0;`, …), or, for the few that follow
  the pointer or a target (help, item explanations, quick command, tooltips), C++.
- **A button's hint** — markup: `data-hint` (or a bound `data-attr-data-hint`) on the element. The
  shared tooltip places it and `tooltip.rcss` styles it, so a fork that moves or adds a button
  keeps its hint with no C++.
- **Draggability** — `UI::RmlBridge::MakeDraggable()`, wired per window in C++; RCSS has no
  "make this draggable" property. Which windows drag is decided in
  [window-placement.md](window-placement.md) section 8.

## Known limitations

- **The `$theme` command is session-only.** `$theme <name>` (see
  ["Switching themes without relaunching"](#switching-themes-without-relaunching) above) rebuilds
  every open window's document in place, but doesn't persist to `config.ini` — a relaunch still
  uses whatever's saved there. The model survives the rebuild, so a window needs no resync of its
  own; what it sized or placed from the old theme's metrics goes in its view's `afterReload`.
- **Custom theme images require the engine's proprietary OZT/OZJ format, not plain PNG/JPG** —
  see [Bringing your own images to a theme](#bringing-your-own-images-to-a-theme) above. No
  converter tool exists in this repo, and a missing/wrong-format image fails silently.
- **Every themed document belongs to a `ThemedView`**, so `$theme` covers all of them; a new
  window declares one.
- **Theme identity must never drive C++ branching** — `architecture-principles.md` §30. Where a
  theme must change C++ behaviour, it declares a capability in `theme.ini` (`NativeTextSize` is the
  one in use). Prefer removing the need: the main frame's `ProvidesOwnIconChrome` existed only so
  C++ could skip drawing legacy chrome for the modern theme, and went away once that chrome was
  RCSS in the legacy theme's own stylesheet.
- **RmlUi renders last in the frame**, so native content that must sit *in* a window's stacking
  goes into a `UI::RmlBridge::RenderTarget` the document shows: live 3D and a window's remaining
  native 2D alike, so every document is in the one RmlUi context.
