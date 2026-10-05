# Options Window and Config

Behaviour changes that landed in PR
[#335](https://github.com/sven-n/MuMain/pull/335) for the options window
and the `config.ini` it writes to.

For the camera architecture see [`camera-system.md`](camera-system.md).

---

## What changed

The options window is now **the same window in every scene** (login,
character select, gameplay) - it shares one layout, one apply path, and one
backing config object. There is no separate per-scene options dialog. The
frame is **380 by 319** reference pixels (the old window was a single 190-wide
column) and lays the same controls out in two columns: attack, audio, and
resolution on the left; render toggles, font, language, and windowed mode on
the right.

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

## What it stores in `config.ini`

Settings are written next to the executable as `config.ini`. The options
window writes to these sections:

- `[Window]` - width, height, windowed flag.
- `[Render]` - the persisted `VSync` preference. The options checkbox **VSync**
  and the commands `$vsync on` / `$vsync off` update it. Startup, the checkbox,
  and fullscreen/resolution changes reapply it before the next frame: on uses
  the swapchain's vsync present mode, off presents immediately. Also
  `WeatherEffects`: the options checkbox **Weather effects**. Checked (the
  default) keeps rain, snow, mist, and the map haze overlays, including the
  Tarkan sandstorm and its dust vents. Unchecked turns those off on every map
  and writes the key immediately, the same way the volume sliders save. Wind
  is unchanged. Equipment, skin, and item sparkles stay on, and so does map
  fire: braziers and torches, and the floating embers on Battle Castle, the
  login scene, Balgas' Refuge, Vulcanus, and Doppelganger. Also `ShowFps`:
  the options checkbox **Show FPS**. Unchecked (the default) hides the
  counter. Checked draws `FPS:` in the top-left of the screen and writes the
  key immediately. `$fpscounter on` / `$fpscounter off` still toggle that
  readout for the current session; opening Options loads the saved checkbox
  again.
- `[Audio]` - volumes.
- `[Login]` - language, and the remembered-credential keys (`RememberMe`,
  `SavePassword`, `EncryptedUsername`, `EncryptedPassword`); see
  "Remembering login credentials" below.
- `[ConnectionSettings]`
- `[Camera]` - orbital wheel-zoom radius (`Zoom`). **New in this PR**, the
  only key the camera rework added.

Missing keys fall back to compile-time defaults from
`src/source/Data/GameConfig/GameConfigConstants.h`. If `config.ini` doesn't exist,
it's created the first time anything calls `Save()`.

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
