# Item Names in Texts Design

> **2026-11-06.** From nolt's proposal on sven-n/MuMain#877 (a text names an item by its number). Written from
> MuMain `upstream/main` @ `dde2efc1d`. The decisions were asked one by one with mockups and answered by the user.
>
> This document is temporary. Its last task deletes it, after what stays true has moved into
> `docs/translation-system.md`, `docs/item-data.md` and the code (see "Tasks").

## Goal

Texts that name items follow the item data and **Translate item names**. Today every text spells its item names per
language, so with the option off a Polish player sees "Jewel of Bless" in the inventory and "Klejnot
Błogosławieństwa" in the jewel bundle window; German mixes "Jewel of Soul" and "Juwel der Seele". A text writes
`{item:14,13}` instead, and the game puts the name in when it shows the text.

## Decisions

| # | Decision |
|---|---|
| D1 | All texts: the Game and Dialog `.resx` texts and the item data's texts (description lines, name lines, `Tooltip.json` shared texts) use one syntax. Not the Editor and Metadata groups (the editor takes names from the data). |
| D2 | Grammar: case forms in the item data (`nameForms` per language, per case), picked by `:case` in the text. A missing form shows the name itself and is a Data Check warning. Item Translations edits the forms. English has no forms. |
| D3 | Syntax: `{item:14,25}`, level `{item:13,20+3}` (the item level as the data's `levels`), case `{item:14,25:acc}`, both `{item:14,21+2:acc}`. Cases `gen`, `dat`, `acc`, `ins`, `loc`. |
| D4 | Migration: all five kinds of mention in en, de, es, pt, pl (the name as spelled; declined, the form moved into `nameForms`; only capitals differ; an English name in a translation; other wording than the item data). Kinds 1-2 in a commit a comparison test proves looks the same with the option on; kinds 3-5 in their own commit, listed in the PR. ru, uk, ja, zh-CN, zh-TW, id, tl untouched (no item names), except D5. |
| D5 | The 43 Game texts that are only an item name (25 used, 18 unused) hold item names the item data lacks (id, ja, ru, zh-TW: 26 items; uk 24; tl 21): they move into the items' names; copies equal to English or wrong (ja "調和の宝石の使い方", "how to use ...") are left out and listed. |
| D6 | The 45 Game texts that name items and that no code uses (18 names, 27 sentences of features this client lacks) are deleted from all 12 languages, in a commit of their own. |
| D7 | Editor: a Data Checks area "Texts"; the forms in Item Translations with the count of texts that name the item; a texts window that edits and saves the `.resx` files. |
| D8 | The texts window covers all Game and Dialog texts (about 3380 keys in 12 languages), with a filter for texts that name items, "Insert item…", a live preview with the option on or off, and a check that a text keeps English's `%d`, `%ls`, `%%`. |
| D9 | Three pull requests: PR 1 the placeholders (from main); PR 3 the texts window (on PR 1); PR 2 the migration (on PR 1 and #877, after #877 is merged). |

## Design

- **UI texts.** Game and Dialog keys are `const wchar_t*` globals that `ApplyLocale` points at the language's literal
  (`tools/ResxGen/CppEmitter.cs`). ResxGen (`--item-text-groups Game,Dialog`) checks the syntax, stops the build on a
  malformed reference, and lists the keys that name items (`I18N::GetItemTexts`). `SetLocale` points those keys at
  their expansion (`I18N::ExpandItemTexts`) before it calls the observers, which copy texts (CSQuest's dialog lines).
  The expansion is interned and kept, so a copy of an older pointer stays readable.
- **Names.** `Data::Translation::ItemTexts` asks the resolver `CItemDataHandler` installs after the item data is
  loaded: `ResolveReferencedName` (`ItemReferenceNames.h`) finds the name, the level's name line or the form in the
  item name language of the text's language, from the item data itself (a language switch expands texts before the
  handler's own observer updates the shown names). A language whose file has no text of its own for the key shows
  the English text and expands it with English names. Before the item data is loaded a text shows `{item:...}`.
- **Re-expansion.** On a language switch; after the item data is loaded; when Translate item names changes
  (`SetTranslateItemNames`); in the editor every frame the item database's name version changed
  (`RefreshItemTextsIfNamesChanged`). A change calls the texts-changed observers
  (`ItemTexts::RegisterTextsChangedObserver`; CSQuest's dialog lines), not the locale observers: the language stayed.
- **Item data texts.** A placeholder may be an item reference (`ReadPlaceholder`); a translation names the same
  items as English, its case may differ (`ToParityPlaceholders`). The tooltip compiles a text with its references
  replaced by names in the item name language, or in English for the English text of a line without a translation
  of its own (`ItemTooltipText.cpp`, cache also reset on the name version).
- **Forms.** `ItemNameForms` on `ItemDefinition` and `ItemNameLine`; `"nameForms"` after `"name"` and after a name
  line's `"text"`. `Tooltips/NameCases.json` gives each language's cases; `ValidateNameForms` warns about forms that
  can never show.
- **Checks.** `ItemReferenceChecks.h`: unknown item, a level whose name needs game values, a level without a name of
  its own, a case the language lacks, a missing form, a translation naming other items. Shown in Data Checks under
  "Texts".

## Tasks

### PR 1: placeholders

- [x] ResxGen: syntax check and the table of texts that name items; `SetLocale` expands before the observers.
- [x] `ItemReference`, `ItemNameForms`, `ItemNameCases` (`NameCases.json`), `ItemReferenceNames`, `ItemTextExpansion`.
- [x] Wiring: item data load, Translate item names, the editor's name edits.
- [x] Item data texts: item references as placeholders, parity without the case, tooltip names in the item name language.
- [x] Validation of forms; Data Checks "Texts"; Item Translations' name forms with the count of texts.
- [x] Editing `NameCases.json` in the editor (Item Translations, **Cases…**), with undo, saved by Save Items.
- [x] Docs; demo texts: the jewel window's ten jewel names (Game "Jewel of Bless" to "Higher Refining Stone", en,
      de, es, pt, pl; "Jewel of Harmony" is also Elpis's menu button), Dialog `Answer_65_Slot_0` (en, de, es, pl).
- [x] Tests pass in the Release editor build; in-game check, Polish and German, the option on and off: the jewel
      window and Elpis's menu (in-game test server, 2026-11-09). Not tried in game: the quest dialog with the
      Polish answer (only in tests).

### PR 3: texts window

- [ ] A `.resx` document that keeps the file's bytes and changes only an edited value (every shipped file written
      back byte for byte).
- [ ] The printf placeholder check against English; the item reference syntax check.
- [ ] Edits with undo (as `ServerListEdits`), Save Texts (changed files only), Revert.
- [ ] The window: all Game and Dialog texts, filters, per-language editing, live preview, Insert item….

### PR 2: migration (after #877)

- [ ] A one-off script with a decisions file (namesakes: 55 English names belong to 128 items; single-word names;
      the Polish and German cases), and a review report.
- [ ] Fixture of the texts before; kinds 1-2 with the comparison test; kinds 3-5; D5 name copies; D6 deletions.

### Last

- [ ] Move what stays true into `docs/translation-system.md` and `docs/item-data.md`, point the links there, and
      delete this document.
