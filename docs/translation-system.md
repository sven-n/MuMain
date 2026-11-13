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
the **key** (free text) and the inner `<value>` is the translated string. An
optional `<comment>` is a note for translators; the generator ignores it.

Example (`src/Localization/Game.en.resx`):

```xml
<data name="Beep sound for whispering" xml:space="preserve">
  <value>Beep sound for whispering</value>
</data>
<data name="Sound volume" xml:space="preserve">
  <value>Sound volume</value>
</data>
```

The matching German file uses the same `name=` keys with translated `<value>`s:

```xml
<data name="Beep sound for whispering" xml:space="preserve">
  <value>Piepton zum Flüstern</value>
</data>
<data name="Sound volume" xml:space="preserve">
  <value>Soundlautstärke</value>
</data>
```

## Build-time generator

`tools/ResxGen` is a small .NET console app driven from `src/CMakeLists.txt`.
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

### Texts chosen by a number

Some texts are still chosen by a number that comes from outside the code: a
server packet's message number, or a recipe's name in `mix.bmd`. The code
turns that number into a key where it arrives, with a table of slots, and
shows no text for a number outside the table:

```cpp
// Network/Server/ServerMessageTexts.cpp: the Rena hints, by message number.
const wchar_t* const* const kRenaLines[] = {
    &I18N::Game::AreYouThrowingRenaOnThe,
    &I18N::Game::RyanTheMaidOfTheBar,
    &I18N::Game::RenaIsATypeOfCoin,
};
```

Store the slot (`&I18N::Game::X`) and read it when the text is shown, so a
language switch reaches it.

### Format / placeholder substitution

For strings with `{0}`, `{1}`, ... placeholders, use `I18N::Format`:

```cpp
const auto msg = I18N::Format(I18N::Editor::ErrorIndexAlreadyInUse, { idxStr });
```

`{{` and `}}` are escaped to literal `{` and `}`. The first argument is a
`const char*` format string (UTF-8); the second is an
`std::initializer_list<std::string_view>` of values to splice in. Result is a
`std::string` (UTF-8). Wide-group strings use `%s` / `%ls` directly through
the bounds-checked `mu_swprintf_s` (or `swprintf_s`) rather than `Format`.
Avoid `wsprintf` and `mu_swprintf` - they don't take a destination size and
will overflow the buffer if a translated string is longer than expected.

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
(see `src/source/UI/Options/OptionWindow.cpp`). It calls
`I18N::SetLocale(code)` and persists the choice via `GameConfig.UILocale`
(the single source of truth for the active locale; on startup
`GameLogic/Config/GameConfig` re-applies it).

`SetLocale` does four things:

1. Resolves `locale` to a `LocaleIndex` (unknown -> default 0).
2. Calls each group's generated `ApplyLocale(int)` (a data-driven `kSlots`
   table that overwrites every `extern` pointer in the group).
3. Points the texts that name items at their expansion
   ([Texts that name items](#texts-that-name-items)).
4. Fires every registered `LocaleObserver`.

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

There's no per-key "key name shown in red" debug mode today; missing keys
just fall back. If you want a hard-fail mode for new development, the
generator is the place to add it (`tools/ResxGen/CppEmitter.cs`).

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
for narrow groups. For wide groups, `%s` / `%ls` via the bounds-checked
`mu_swprintf_s` is the existing pattern.

When translating a string with `printf`-style specifiers, keep them exactly
as in English: the same specifiers in the same order, `%ls` stays `%ls`, and
`%%` stays `%%` (a single `%` followed by text is read as a specifier). Chat
commands such as `/warp`, `/trade` or `/party` are compared against the typed
text, so they stay untranslated.

### German terminology

The German texts use the same terms as the German resources of the OpenMU
server (`src/**/Properties/*.de.resx` in
[MUnique/OpenMU](https://github.com/MUnique/OpenMU)), so players see the same
words in the client and in server messages. The player is addressed formally
("Sie"); NPC dialogues (`Dialog.de.resx`) keep their old-fashioned "Ihr/Euch".
Some fixed terms:

| English | German |
|---|---|
| Item | Gegenstand (Artikel only in the item shop) |
| Party | Party |
| Skill | Fähigkeit |
| Vault / Storage | Tresor / Lager |
| Socket | Sockel |
| Command (stat) | Führung |
| Elf, Muse Elf, High Elf | Elfe, Musenelfe, Hochelfe |
| Lord Emperor | Lord-Imperator |
| Blood Castle | Blutburg |
| Devil Square, Chaos Castle, Castle Siege | not translated |
| Illusion Temple | Illusionstempel |
| Fortress of Empire Guardians | Festung der Kaiserlichen Wächter |
| Refinery Tower | Raffinerieturm |
| Jewel of Bless, Jewel of Soul, … | Juwel des Segens, Juwel der Seele, … |
| Helm, Armor, Pants, Gloves, Boots, Mask | Helm, Rüstung, Hose, Handschuhe, Stiefel, Maske |
| Great (in item names) | Großartig (*Großartiger Drachenhelm*) |
| Seed, Sphere, Seed Sphere | Samen, Sphäre, Samensphäre |
| Refining Stone | Verfeinerungsstein |
| Transformation Ring | Verwandlungsring |

The German item names (the `"de"` names in `src/bin/Data/Items/*.json`) are the
same as the German item names of the OpenMU server (`ItemNames.de.resx`). Texts
that mention an item use its German name. Armor set names are joined with the
piece (*Drachenhelm*), proper names get a hyphen (*Hades-Helm*), and adjectives
are declined (*Legendärer Helm*, *Legendäre Rüstung*). Skill, monster and event
names stay English; items that teach a skill name it after a colon
(*Schriftrolle: Evil Spirit*, *Pergament: Chain Lightning*,
*Kugel: Twisting Slash*).

## Adding a new locale

1. Create `src/Localization/<Group>.<newLocale>.resx` for every group you want
   the new locale to cover. Missing groups fall back to `en` at runtime.
2. Add a display name for the locale to
   `tools/ResxGen/CppEmitter.cs#KnownLanguageDisplayNames` so the language
   dropdown shows it in its own language (e.g. `["fr"] = "Français"`).
3. Add it to the game's language dropdown: the `s_Languages` list in
   `src/source/UI/Options/OptionWindow.cpp`. The editor's language
   menu lists every locale from `GetAvailableLocales` by itself.
4. Build. `ResxGen` picks up the new locale automatically from the filename;
   `GetAvailableLocales` will include it next run.
5. To show item, monster and NPC names and buff names and descriptions in the
   new language, add them to the item data, the monster list and the buffs;
   see [Item names](#item-names), [Monster and NPC names](#monster-and-npc-names)
   and [Buff names](#buff-names).

## Item names

Item names are not in the `.resx` files. They are part of the item data
([item-data.md](item-data.md)): each item's `name` in
`src/bin/Data/Items/*.json` holds the names by language code, using the same
codes as the UI texts. The game shows the name for the active locale and
falls back to English, like the UI texts; switching the locale at runtime
updates item names too. Players who know the items by their English names can
untick **Translate item names** under the language in the options window
(Interface tab): item names then stay English, also in the item editor, while
every other text follows the language.

### In the JSON files

Each item's `name` lists its names by language code:

```json
"name": {
  "en": "Blade",
  "de": "Klinge",
  "es": "Espada",
  "pt": "Lâmina"
},
```

- **Change a name:** edit the text.
- **Add a translation:** add a line `"<code>": "<name>"`, e.g. `"pl": "Ostrze"`.
- **Remove a translation:** delete its line; the game then shows the English
  name for that language.
- `en` is required and must stay. A name that is the same in a language
  (*Kris*, *Zen*) is written like the English one (`"de": "Kris"`), so it
  is not counted as missing.
- The order of the lines does not matter when editing by hand; the item
  editor's **Save Items** writes English first and the rest sorted by code.
- Write the files as UTF-8. Accented and non-Latin letters can be typed as
  they are.
- Names must not contain `||`, and names longer than 49 characters are cut
  in the game (a warning at startup).

The game reads `Data/Items` next to `Main`. The build copies
`src/bin/Data/Items` there, so after editing the files in `src/bin`, build
again (or edit the copy in the build folder and copy it back before
committing).

### In the item editor

The **Item Translations** window (editor builds, **Windows → Items → Item
Translations**) shows the item names as a grid: one row per item, one column per UI
language, English first.

- Each language's column header says how many items have no name of their
  own in it; **Missing in** lists only those items. A filled name keeps its
  row until **Refresh** or another filter, so the list can be worked
  through.
- Edit a cell and leave it (click elsewhere, Tab or Enter) to store the
  name. An empty cell shows the English name it falls back to, greyed.
- Right-click a cell: **Same as English** writes the English name for a name
  that is the same in that language; **Remove translation** removes it.
- The search finds an item by any of its names (A to Z in either case), by
  `group,number` or by its item type. Clicking the number selects the item
  in the other item tools and shows the **Item Browser**. The item the
  tools show is marked, and an item selected in another tool is scrolled
  to (the search and filter are cleared when they hide it).
- Columns can be hidden from the header's right-click menu. A name cannot
  contain `||` (the item checks refuse it). The English name
  cannot be emptied, and the names are stored whole, also when they are
  longer than the 49 characters the game shows.
- **Save Items** writes the files and lists each language that misses item
  names, with how many.

**Item Basics** edits the English name. Each name change is one step of
the item tools' Undo and Redo.

Above the grid, **Name forms** shows the selected item's forms for the texts that
name it with a case ([Texts that name items](#texts-that-name-items)): a row for
its name and one for each name of a level, per language that declines names,
with a field per case. An empty field shows the name it falls back to, greyed;
each change is a step of Undo. The line above the table counts the texts that
name the item; hover it for the list.

### Translating items into a language the game already has

The game's UI languages are `en`, `de`, `es`, `id`, `ja`, `pl`, `pt`, `ru`,
`tl`, `uk`, `zh-CN` and `zh-TW`. Items currently have German (`de`), Portuguese (`pt`)
and Spanish (`es`) names. To translate items into another of these languages,
add names with that code (by hand or in the Item Translations window); nothing else is
needed. The names show as soon as a player picks that language.

### Adding a new language

Item names follow the UI language, so a new language has to exist as a UI
language first:

1. Add the UI language as described in [Adding a new locale](#adding-a-new-locale).
2. Add item names with the same code.

Item names for a code that is not a UI language are never shown; loading
warns about them. Import from bmd and Export as bmd only cover English,
Portuguese and Spanish (`Data/Local/Eng`, `Por`, `Spn`); names in other
languages exist only in the JSON files.

### Checking your changes

Start the game: problems in the item data stop the start with a message
that names the file, the item and the field, and all problems are in
`MuError.log`. With a test build (`-DBUILD_TESTING=ON`), the item data tests
also check the files in `src/bin/Data/Items` without starting the game:

```bash
ctest --test-dir <build folder> --build-config Debug -R items --output-on-failure
```

## Texts that name items

A text names an item by its number instead of spelling its name, and the game
puts the item's name in when it shows the text:

| Written | Shows (English, German) |
|---|---|
| `{item:14,13}` | Jewel of Bless, Juwel des Segens |
| `{item:13,20+3}` | the name of item 13,20 at level 3: Ring of glory |
| `{item:14,25:acc}` | the accusative of the name, where the language has one |
| `{item:14,21+2:gen}` | both |

The numbers are the item's group and number, as the editor shows them (`14,13`
in Item Translations, `Jewel of Bless (14,13)` in logs). `+level` picks the
item's name at that level (its name line, [item-data.md](item-data.md#name-lines)),
for items whose name changes with the level, such as Box of Kundun +1 (`14,11+8`).

- **Where:** the `Game` and `Dialog` texts (`ResxGen --item-text-groups`), and
  the item data's own texts: description lines, name lines, and the shared texts
  and layout lines of `Tooltips/Tooltip.json`. Not the editor's texts.
- **Which name:** the item name language, so the names follow **Translate item
  names**: with it off a Polish text shows `Jewel of Bless`, as the inventory
  does, while the rest of the text stays Polish. A language whose file has no
  text of its own for the key shows the English text, and names the items in
  English like it; so does an item data text without a translation of its own.
- **When:** a text is expanded when the language switches, when the item data
  is loaded, when Translate item names changes and when the editor changes a
  name; the text's pointer then shows the expansion, so the code that shows a
  text needs nothing. Code that keeps a copy of a text (an NPC dialog's lines)
  registers with `ItemTexts::RegisterTextsChangedObserver` as well as with
  `RegisterLocaleObserver`: a change of names calls only the first, since the
  language stayed. Before the item data is loaded a text shows `{item:...}`
  as written (no such text is shown that early). A number no item has, or a
  level above +15, stays as written, and the rest of the text is still filled
  in; so does a name with `%` in a Game or Dialog text, a printf format that
  would read it as a conversion.
- **Grammar:** languages that decline names (German, Polish) can ask for a form
  with `:gen`, `:dat`, `:acc`, `:ins` or `:loc`. The forms are part of the item
  data (`nameForms`, [item-data.md](item-data.md#name-forms)) and edited in Item
  Translations; `Tooltips/NameCases.json` gives each language its cases. A
  missing form shows the name itself. English has no forms.
- **Translations** of an item's own text must name the same items as the English
  text; the case may differ (`{item:14,25}` in English, `{item:14,25:acc}` in
  Polish). The UI texts may spell a name in a language that has no item names
  yet (Russian, Japanese, ...): a reference there would show the English name.

A malformed reference in a `.resx` file (`{item:14;13}`, an unknown case) stops
the build with the file, key and text, and so does a reference in the texts of
another group (the editor's), which the game would show as written. In the
editor, **Data Checks** lists the texts under **Texts**: a number or a level no
item has, a level whose name needs game values (a skill's or monster's name), a
level without a name of its own, a missing form, a translation that names other
items than the English text, and one that still spells the names the English
text writes by number, in a language the item data has those names in. A click selects the item and opens Item
Translations (or the tool of an item's own text).

## Simplified Chinese (`zh-CN`)

Simplified Chinese is being added incrementally. Translated screens include the
Options window and its graphics tuning labels and help text, and the system menu
at character selection and in game (Esc). The login window also translates account
and password fields, remember-account/password controls, the password-saving
confirmation, server titles, login progress and common account errors. Character
selection, creation and deletion include class and advancement names, class
descriptions, base attributes, guild roles, controls and validation messages.
Shared labels such as OK, Cancel and Close also appear translated wherever other
screens use the same resource keys, including other dialogs and windows.
Untranslated text falls back to English. Item names and server messages are
separate data; translated item tooltips are also available.

Use official Chinese terminology as a reference, check item IDs and version-specific
behavior, and preserve the current English meaning and placeholders. Do not import
a newer client's rules merely to match its wording.

Rebuild the client, then select **简体中文** in the Options language dropdown.
Select **Noto Sans SC** in the font dropdown to use Simplified Chinese glyph
forms. The regular and bold faces are bundled under `fonts/` and provide
missing-glyph fallback for other font choices; no system font install is needed.
These fonts come from Noto CJK's `Sans/SubsetOTF/SC` distribution and are shipped
with their SIL Open Font License in `fonts/NotoSansSC-LICENSE`.

Alternatively, set the executable directory's `config.ini`:

```ini
[UI]
Locale=zh-CN
```

After rebuilding, check the Options tabs and graphics tuning, and open the
system menu at character selection and with Esc in game, in both themes.
Chinese glyphs should render without boxes, and labels and help should fit at
75% and 150% UI scale. Switch back to English and restart to check that the
language choice is retained.

The password-saving warning wraps within the dialog in both themes. Server
names are preserved; untranslated connection and subscription errors use English.

Character-selection captions, creation headings and class descriptions follow
the selected language. Class descriptions also refresh while the window is open.
Character names require at least four characters. Fruit names, attribute names
and descriptions are translated; enable **Translate item names** to display the
translated item names.

The inventory and expanded inventory controls, common item-use restrictions,
selling confirmations and repair prices are translated. Equipment tooltips show
Chinese damage, defense, durability, resistance and requirement labels, including
missing attribute amounts. These shared tooltips also appear in shops and other
item windows. Item names still depend on **Translate item names** and the item's
available name translations.

The storage and expanded-vault windows translate their titles, fees, Zen deposit
and withdrawal controls, vault locking and password prompts, and related error
messages.

The player-trade window translates its title, partner and item warnings, Zen
offer prompt, incoming request, cancellation reasons and reinforced-item
restriction.

Personal stores translate their owner labels, opening and closing controls,
price and currency dialogs, sale and purchase confirmations, restrictions and
status notices.

NPC shops translate the merchant title, purchase confirmation, real-time tax
rate and reinforced-item selling restriction; their repair and ordinary selling
messages reuse the inventory translations.

The main HUD translates life, mana, AG, SD and experience tooltips, its character,
friends, party, guild, command, menu and item-shop buttons, the MU Helper controls,
and buff duration and cancellation prompts.

The character information window translates level and reset counts, available
points, combat, defense, magic and curse statistics, HP and MP, and its quest,
pet and master-skill controls.

Add future translations to `Game.zh-CN.resx` by screen or feature. Keep English
resource keys, formatting arguments and command tokens unchanged.
Missing translations intentionally use the existing English fallback.

## Monster and NPC names

Monster and NPC names are not in the `.resx` files either. They are in the
monster list, `src/bin/Data/Monsters/Monsters.json`
([monster-data.md](monster-data.md)): each monster's `name` holds the names by
language code, like an item's. English (`en`) is the name
every monster has; Portuguese (`pt`) and Spanish (`es`) come from the original
client's name files.

The game shows the name in the UI language and falls back to English. When
the language changes, the monsters and NPCs on screen are renamed at once.
Summons ("Budge Dragon of <owner>") and the Kalima gates keep the name they
appeared with until they appear again; transformed players show the player's
name. **Translate item names** does not change monster names; it only changes
the monster's name inside a transformation ring's item name.

The original client read the names from `Data/Local/<Language>/NpcName_<Language>.txt`
in the `[LOGIN] Language`, apart from the UI language.

The game holds up to 64 characters of a name (UTF-16 units); a longer name is
cut, and loading warns about it. A Kalima gate shows "Magic stone of <owner>",
which is the UI text `MagicStoneOfOwner` of the `Game` group, not a monster
name.

### In the JSON file

Each monster's `name` lists its names by language code, as an item's does
(see [In the JSON files](#in-the-json-files)):

```json
"name": {"en": "Budge Dragon", "es": "Dragón Escurridizo", "pt": "Dragão Lento"},
```

- Change, add and remove names as for items. Without an English name the
  game shows no name for the monster in a language without a translation (a
  warning at startup).
- Names must not contain `||`, and names longer than 64 characters are cut
  in the game (a warning at startup). Names for a code that is not a UI
  language are never shown; loading warns about them.
- The game reads `Data/Monsters` next to `Main`. The build copies
  `src/bin/Data/Monsters` there, so after editing the file in `src/bin`,
  build again (or edit the copy in the build folder and copy it back before
  committing).

### In the monster editor

The **Monster Translations** window (editor builds, **Windows → Monsters →
Monster Translations**) shows the monster names as a grid: one row per
monster, one column per UI language, English first. A monster with several
numbers has one row for the names they share ("145 (3)": its first number
and how many it has), and a row for each of its numbers with names of its
own or made its own in the Monster Browser. It works like
[Item Translations](#in-the-item-editor):

- Each language's column header says how many monsters have no name of
  their own in it; **Missing in** lists only those monsters. A filled name
  keeps its row until **Refresh** or another filter.
- Edit a cell and leave it (click elsewhere, Tab or Enter) to store the
  name. An empty cell shows the English name it falls back to, greyed.
- Right-click a cell: **Same as English** writes the English name for a name
  that is the same in that language; **Remove translation** removes it.
- The search finds a monster by any of its numbers or by any of its names (A
  to Z in either case). Clicking the number selects the monster in the other
  monster tools (a monster's row: the monster as a whole) and shows the
  **Monster Browser**. The monster the tools show is marked, a number that
  shares its monster's names on the monster's row, and a monster selected in
  another tool is scrolled to (the search and filter are cleared when they
  hide it).
- A name cannot contain `||`, and the English name cannot be emptied (252,
  whose English name is empty, stays so). Names are stored whole, also when
  they are longer than the 64 characters the game shows.
- Each name change is one step of the monster tools' Undo and Redo, and it
  reaches the monsters and NPCs of that number on screen at once.
- **Save Monsters** writes the monster list file.

### Old name files (NpcName txt)

**Import names from NpcName txt** and **Export as NpcName txt** in Monster
Translations read and write the original client's name files,
`Data/Local/<Eng|Por|Spn>/NpcName_<Eng|Por|Spn>.txt` next to the game:
English, Portuguese and Spanish only. The import sets the names by number
and language on the monsters that have an entry, as one undo step; the
export writes the names of every monster, in English where a language has
none. Names in other languages exist only in the monster list. The rules
are in
[monster-data.md → Import and export of NpcName txt](monster-data.md#import-and-export-of-npcname-txt).

## Buff names

Buff names and descriptions are not in the `.resx` files either. They are in
`src/bin/Data/Buffs/Buffs.json` ([buff-data.md](buff-data.md)): each buff's
`displayName` and `description` hold the texts by language code. The game
shows them in the UI language and falls back to English.

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
