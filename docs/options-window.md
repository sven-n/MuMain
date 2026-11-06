# Options Window and Config

Behaviour changes that landed in PR
[#335](https://github.com/sven-n/MuMain/pull/335) for the options window
and the `config.ini` it writes to.

For the camera architecture see [`camera-system.md`](camera-system.md).

---

## What changed

The options window is now **the same window in every scene** (login,
character select, gameplay) - it shares one layout, one apply path, and one
backing config object. There is no separate per-scene options dialog.

What was added on top of that:

- **Resolution and windowed/fullscreen toggle apply without restarting.**
  Switch them at any time from the options window.
- **Volume and render-level sliders round to the nearest level** instead of
  truncating, so the slider endpoints actually reach 0 (mute) and the
  configured maximum.
- **Resolution combo-box clicks no longer leak through** to the checkboxes
  behind the dropdown.
- **Resolution changes go through SDL** (PR
  [#530](https://github.com/sven-n/MuMain/pull/530)), which makes the live
  apply actually work in windowed mode on Windows. Windowed resizes re-center
  the window on the display, except on Wayland, where the compositor places
  windows (see [Linux - Terminal](build/linux/console.md#wayland)).
  Fullscreen picks the closest real display mode (or borderless desktop when
  nothing matches), and the combo and `config.ini` record the mode that
  actually resulted, not the one requested.
- **The resolution list comes from the current monitor.** Opening the options
  window queries SDL for that display's fullscreen modes, collapses duplicate
  refresh-rate entries to one width/height choice, and sorts the result.
- **The UI reflows immediately after resolution changes.** Docked side menus
  scale up to 2.25 times their reference size on large displays, while
  centered dialogs remain capped at 2 times their reference size. Text grows
  with the active layout up to the dock scale, shrinking only when needed to
  fit an existing control.

## Layout

The window has five tabs:

| Tab | Settings |
|---|---|
| Display | Resolution, windowed mode, VSync, FPS limit, UI scale |
| Graphics | Quality: Enhanced graphics, anti-aliasing, anisotropic filtering, render level. Effects: the effect toggles. Shadows: the shadow toggles |
| Audio | Sound and music volume, whisper sound |
| Gameplay | Auto attack, slide help |
| Interface | Language, translate item names, font, UI theme, FPS counter, debug info |

Each setting is a row with its name on the left and its control on the
right, aligned in two columns. A tab's settings scroll when they don't fit,
so a tab can take more rows.

- **Legacy theme:** the window keeps the same size on every tab, so it does
  not move when you switch tabs. A long setting name, for example in a
  translation or with a larger font, wraps to a second line. On a screen too
  short for the window, it shrinks to fit and the settings scroll below the
  tab row. The small cross in the top-right corner closes it. The window can
  be dragged by any part that is not a control.
- **Modern theme:** a full-screen settings page with the tabs on the left.
  The page scrolls when its rows don't fit.

Rows with a hint (UI Scale, Anti-aliasing, Anisotropic filtering and the
Enhanced tuning sliders) show it while the pointer is over the row.

## What it stores in `config.ini`

Settings are written next to the executable as `config.ini`. The options
window writes to these sections:

- `[Window]` - width, height, windowed flag.
- `[Render]` - set from the Display and Graphics tabs, each applied at once:
  - `VSync`: `$vsync on` and `$vsync off` update it too; startup and
    fullscreen/resolution changes reapply it before the next frame begins.
  - `FpsCap`: the frame limit while VSync is off, `-1` for none.
  - `Look` (default `Classic`): the "Enhanced graphics" checkbox, ticked for
    the Enhanced look; see [`enhanced-look.md`](enhanced-look.md).
  - `Antialiasing` (`0`, `2`, `4`, `8`; default `4`): the Anti-aliasing row,
    the samples of the Enhanced look. Classic always renders without it.
  - `Anisotropy` (`1` is off, up to `16`; default `8`): the Anisotropic
    filtering row, for every texture with mip levels: world textures in the
    Enhanced look, upscaled textures in both.
  - `EnhancedLighting`, `DynamicLights`, `LightMood`, `PostProcessing`
    (default `1`): the checkboxes of the rows under "Enhanced graphics", each a
    feature of the Enhanced look; see [`enhanced-look.md`](enhanced-look.md).
    Clicking a row's name opens its tuning sliders instead.
  - `DisableEffects`, `DisableParticles`, `DisableSkillEffectModels`,
    `DisableBoids`, `DisableBodyShadows`, `DisableWingShadow`: the Graphics
    tab's toggles. A toggle another one already covers shows ticked and
    greyed out: Disable Effects covers skill effects and wildlife; without
    Render full effects, or with Disable Shadows, no wing shadow is drawn
    (and without full effects no shadow at all).

  Set from the Interface tab, applied at the next start:
  - `TexturePack` (default empty, the original textures): the Texture Pack
    row, the [texture pack](texture-packs.md) to read textures from first.

  Set by hand only, never written by the game:
  - `Backend` (`default`/`vulkan`/`direct3d12`/`metal`, case-insensitive,
    `d3d12` accepted as an alias for `direct3d12`) forces a specific SDL_gpu
    GPU backend at startup, falling back to SDL's own auto-pick with a logged
    warning if the requested one is unavailable.
  - `SortParticleDraws` (`0`/`1`) sorts particle draws back to front.
- `[EnhancedTuning]` - only the tuning sliders under "Enhanced graphics" the
  player moved, one global key per value applying on every map and after restart;
  no per-map player setup is needed. "Defaults" means no personal override;
  Reset removes the key again. Written on its
  own, through a temporary file, when a slider is let go; see "Tuning it
  yourself" in [`enhanced-look.md`](enhanced-look.md).
- `[UI]` - `Locale`, `Font`, `RmlTheme` and `UIScalePercent` from the Display and
  Interface tabs. `TranslateItemNames` (default `1`): the "Translate item names"
  checkbox under the language, applied at once; unticked, item names stay
  English while the rest of the UI is in the chosen language. Monster and NPC
  names follow the language either way; only the monster in a transformation
  ring's name follows the checkbox.
  `EnableAnimationTaskPool` (`0`/`1`, set by hand only, like the
  `--enable-taskpool` command-line flag) animates characters on worker
  threads.
- `[Audio]` - volumes.
- `[Login]` - language, and the remembered-credential keys (`RememberMe`,
  `SavePassword`, `EncryptedUsername`, `EncryptedPassword`); see
  "Remembering login credentials" below.
- `[ConnectionSettings]`
- `[Camera]` - orbital wheel-zoom radius (`Zoom`). **New in this PR**, the
  only key the camera rework added.

Missing keys fall back to compile-time defaults from
`src/source/Data/GameConfig/GameConfigConstants.h`. If `config.ini` doesn't exist,
the build seeds it from `src/bin/config.ini.template`, which lists every key the
game writes, in the order `Save()` writes them, plus `Backend`.

## Remembering login credentials

The login screen has two separate checkboxes:

- **Remember Username** - fills in your account name next time. Safe to leave
  on; only the username is stored.
- **Remember Password** - also stores the password. Because a saved password
  lets anyone using the same computer log into your account, ticking it opens a
  confirmation dialog first; the box only stays ticked if you accept it. Leave
  this off on shared machines (e.g. an internet cafe).

Editing the account or password field clears any stored credentials and turns
Remember Password back off, so an out-of-date password is never left behind.

Stored credentials are obfuscated (not plaintext) and tied to the current
machine and user account, so copying `config.ini` to another machine does not
reveal them. This is protection against a casual reader of the file, not a
determined attacker - which is why saving the password is opt-in.
