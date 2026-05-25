# Translation system

This document describes the `.resx` -> generated C++ translation system used by
the client. It replaces the older `text.bmd` / `GlobalText` and
`Translations/*.json` / `EDITOR_TEXT` paths, which were removed during the
migration tracked in sven-n/MuMain#347.

If you only want to add or change a string, jump to
[Adding a new string](#adding-a-new-string).

---

## Source files

All translation data lives in `src/Localization/` as standard ResX XML:

```
src/Localization/
  Editor.en.resx     Editor.de.resx     ...
  Game.en.resx       Game.de.resx       ...
  Dialog.en.resx     Dialog.de.resx     ...
```

Filename convention: `<Group>.<locale>.resx`.

- **Group** is a freeform PascalCase name. It becomes the C++ namespace
  `I18N::<Group>` and ends up in `Generated/I18N/<Group>.{h,cpp}`.
- **Locale** is a BCP-47-ish code (`en`, `de`, `pt`, `zh-TW`, ...). The default
  locale is `en` and every group must ship that file; non-default locales are
  optional and fall back to `en` at runtime for any key they don't supply.

A `.resx` file contains `<data name="...">` entries. The `name` attribute is
the **key** (free text), the inner `<value>` is the translated string, and an
optional `<comment>legacy_id=N[,N,...]</comment>` carries integer IDs that the
old `GlobalText[N]` call sites still need for lookups by index.

Example (`src/Localization/Game.en.resx`):

```xml
<data name="Beep sound for whispering" xml:space="preserve">
  <value>Beep sound for whispering</value>
  <comment>legacy_id=387</comment>
</data>
<data name="Sound volume" xml:space="preserve">
  <value>Sound volume</value>
</data>
```

The matching German file uses the same `name=` keys with translated `<value>`s:

```xml
<data name="Beep sound for whispering" xml:space="preserve">
  <value>Piepton zum Flüstern</value>
  <comment>legacy_id=387</comment>
</data>
<data name="Sound volume" xml:space="preserve">
  <value>Soundlautstärke</value>
</data>
```

## Build-time generator

`Tools/ResxGen` is a small .NET console app driven from `src/CMakeLists.txt`.
On every build it scans `src/Localization/*.resx`, groups files by stem, and
emits:

```
${CMAKE_BINARY_DIR}/Generated/I18N/
  All.h    All.cpp       <- master entry points (SetLocale, Format, ...)
  Editor.h Editor.cpp
  Game.h   Game.cpp
  Dialog.h Dialog.cpp
  Metadata.h
```

The custom command is wired up in `src/CMakeLists.txt` under the comment
"ResxGen: .resx -> typed C++ accessors". `add_dependencies(Main ResxGen)`
guarantees the generator runs before the client compiles, and the generated
`.cpp` files are added to the `Main` target via `target_sources`. Adding,
renaming, or removing a `.resx` re-runs the generator automatically because
CMake reglobs with `CONFIGURE_DEPENDS`.

The generator takes two flags:

- `--input <dir>` - the `.resx` source directory (always `src/Localization`).
- `--output <dir>` - where to write `Generated/I18N/`.
- `--wide-groups <Group,...>` - groups whose strings are `wchar_t*` instead of
  `char*`. Today: `Game,Dialog`. Add a new group here if it needs wide chars
  (most do; the `Editor` group is the only narrow one).

## Generated C++ API

### Per-group accessors

For every group, ResxGen emits one `extern` slot pointer per resource entry.
The slot is updated in place when the locale switches, so call sites never
have to be re-read:

```cpp
#include "I18N/All.h"

g_pRenderText->RenderText(x, y, I18N::Game::SoundVolume);
ImGui::Text("%s", I18N::Editor::SaveSkills);
```

The identifier is derived from the resx `name=` key by stripping non-ASCII
and PascalCasing it. For example:

| Resx key                         | C++ identifier                  |
|----------------------------------|---------------------------------|
| `Sound volume`                   | `I18N::Game::SoundVolume`       |
| `Beep sound for whispering`      | `I18N::Game::BeepSoundForWhispering` |
| `Close388`                       | `I18N::Game::Close388`          |
| `[error5] User has selected ...` | `I18N::Game::Error5UserHasSelectedTheExitButton` |

If you need to know the exact identifier before building, look at
`Generated/I18N/<Group>.h` after one build, or follow the rule above. Two keys
that slug to the same identifier are a build error from the loader.

### Lookup by legacy integer ID

Code paths that historically used `GlobalText[N]` keep working through a
binary-search lookup:

```cpp
mu_swprintf_s(buf, L"(%ls)", I18N::Game::Lookup(guildTextIndex));
```

`Lookup(int)` is generated only for groups that have at least one entry with
a `legacy_id=` comment, and it's `static_assert`-checked to be strictly sorted
so duplicate IDs fail the build (`Generated/I18N/<Group>.cpp`).

### Format / placeholder substitution

For strings with `{0}`, `{1}`, ... placeholders, use `I18N::Format`:

```cpp
const auto msg = I18N::Format(I18N::Editor::ErrorIndexAlreadyInUse, { idxStr });
```

`{{` and `}}` are escaped to literal `{` and `}`. The first argument is a
`const char*` format string (UTF-8); the second is an
`std::initializer_list<std::string_view>` of values to splice in. Result is a
`std::string` (UTF-8). Wide-group strings use `%s` / `%ls` directly through
`mu_swprintf` / `wsprintf` rather than `Format`.

### Master entry points

`I18N/All.h` exposes the global API:

```cpp
namespace I18N {
    // Switches every group to the given locale, then fires observers.
    // Unknown locale falls back to the default ("en").
    void SetLocale(const char* locale) noexcept;

    // Returns the locale code passed to the last successful SetLocale
    // (or the default before any explicit call). Never null.
    const char* GetCurrentLocale() noexcept;

    // Returns the BCP-47 codes this build was generated with, default
    // locale first. Span backs static storage.
    std::span<const char* const> GetAvailableLocales() noexcept;

    // Returns the position of `locale` in GetAvailableLocales (0-based,
    // default at 0). Returns -1 for unknown.
    int LocaleIndex(const char* locale) noexcept;

    // Returns the display name of `locale` in that locale's own language
    // (e.g. "Deutsch" for "de"). Returns `locale` itself for codes the
    // generator has no display name for.
    const char* GetLanguageDisplayName(const char* locale) noexcept;

    // {0},{1}-style substitution for narrow strings.
    std::string Format(const char* format,
                       std::initializer_list<std::string_view> args);

    // Observer callback used by UI widgets that cache I18N strings.
    using LocaleObserver = void (*)(void* context) noexcept;
    void RegisterLocaleObserver(LocaleObserver cb, void* ctx) noexcept;
    void UnregisterLocaleObserver(LocaleObserver cb, void* ctx) noexcept;
}
```

## Locale switching at runtime

The active locale is selected through the Option window's language dropdown
(see `src/source/UI/NewUI/Options/NewUIOptionWindow.cpp`). It calls
`I18N::SetLocale(code)` and persists the choice via `GameConfig.UILocale`
(the single source of truth for the active locale; on startup
`GameLogic/Config/GameConfig` re-applies it).

`SetLocale` does three things:

1. Resolves `locale` to a `LocaleIndex` (unknown -> default 0).
2. Calls each group's generated `ApplyLocale(int)` (a data-driven `kSlots`
   table that overwrites every `extern` pointer in the group).
3. Fires every registered `LocaleObserver`.

Most rendering picks up the change for free, because text is read straight
from the now-updated `I18N::<Group>::Name` slot on the next frame. Widgets
that **cache** strings (button tooltips, list labels assembled once at
construction) need to re-read them on locale change. The pattern is:

```cpp
class CCharInfoBalloon {
public:
    CCharInfoBalloon() {
        I18N::RegisterLocaleObserver(&OnLocaleChanged, this);
    }
    ~CCharInfoBalloon() {
        I18N::UnregisterLocaleObserver(&OnLocaleChanged, this);
    }
private:
    static void OnLocaleChanged(void* ctx) noexcept {
        auto* self = static_cast<CCharInfoBalloon*>(ctx);
        if (self->m_pCharInfo != nullptr) {
            self->SetInfo();  // re-reads I18N::Game::* into its caches
        }
    }
};
```

`NewUIButton`, `NewUIRadioButton`, and `NewUICheckBox` also have
`ChangeText(const wchar_t* const*)` / `ChangeToolTipText(...)` overloads that
take a slot pointer; widgets created with those overloads refresh
automatically without needing their own observer.

## Fallback behaviour

- Active locale missing a key -> falls back to the default locale (`en`) for
  that key. The fallback is built into the generated `kSlots` table, so it
  costs nothing at runtime.
- Active locale entirely absent -> `SetLocale` resolves it to the default
  locale silently.
- Unknown locale code -> default locale.
- Lookup by legacy ID that doesn't exist in the group -> returns an empty
  string. The generator's static_assert prevents duplicate IDs from compiling.

There's no per-key "key name shown in red" debug mode today; missing keys
just fall back. If you want a hard-fail mode for new development, the
generator is the place to add it (`Tools/ResxGen/CppEmitter.cs`).

## Adding a new string

1. Add a `<data name="...">` entry to `src/Localization/<Group>.en.resx`.
   Pick a key that reads as English text - it doubles as the source-of-truth
   value and as the slug for the C++ identifier.
2. If you have translations ready, add entries with the same `name=` to the
   other `<Group>.<locale>.resx` files. Missing translations fall back to
   English - it's fine to ship the English entry alone and translate later.
3. Use it from C++ as `I18N::<Group>::<PascalCaseSlug>`. No rebuild dance
   needed; CMake re-runs the generator automatically on the next build.

If your new string contains placeholders, prefer `{0}`/`{1}` and `I18N::Format`
for narrow groups. For wide groups, `%s` / `%ls` via `mu_swprintf` is the
existing pattern.

## Adding a new locale

1. Create `src/Localization/<Group>.<newLocale>.resx` for every group you want
   the new locale to cover. Missing groups fall back to `en` at runtime.
2. Add a display name for the locale to
   `Tools/ResxGen/CppEmitter.cs#KnownLanguageDisplayNames` so the language
   dropdown shows it in its own language (e.g. `["fr"] = "Français"`).
3. Build. `ResxGen` picks up the new locale automatically from the filename;
   `GetAvailableLocales` will include it next run.

## Migration history

If you're trying to understand why a string lives in resx instead of the old
data files, two commits are the headlines:

- `4a1fd53f` - removed `Translator` / `Translations/*.json` (Editor group).
- `a58ac50d` - removed `GlobalText` / `Text_*.bmd` (Game group).
- `19ad1888` - removed the per-language `Dialog_*.bmd` files after the Dialog
  group migration.

The leftover binary `text.bmd` / `NPCDialogue.bmd` / per-language
`Dialog_*.bmd` files are pure data; nothing in the C++ source loads them
anymore.
