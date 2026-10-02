# Effect catalogue tool

`effect_types.py` scans the client code for the effect, particle, joint and
sprite types it uses and generates the compiled symbol list and the
catalogue files of the effect catalogue ([docs/effect-data.md](../../docs/effect-data.md)).
Run it again when a change adds or removes a type.

## Steps

From the repository root, with any folder outside the repository as the
work folder:

```bash
python tools/effect-catalogue/effect_types.py collect <work>
```

Writes `<work>/candidates.json` (every type expression of each kind and
where it comes from) and `<work>/probe.cpp`. Compile the probe with
`src/source` as the include folder and save what it prints:

```bash
cl -nologo -std:c++20 -EHsc -utf-8 -DUNICODE -D_UNICODE -I src/source <work>/probe.cpp -Fe:<work>/probe.exe
```

```bash
<work>/probe.exe > <work>/values.txt
```

(`g++ -std=c++20 -I src/source` works as well.) Then:

```bash
python tools/effect-catalogue/effect_types.py generate <work>
```

Writes `src/source/Data/GameData/EffectData/EffectTypeSymbols.cpp` and the
four files in `src/bin/Data/Effects/`. Names that are already in the files
are kept; new types get a generated name, which is checked by hand before
the change is committed. Name collisions and names that are not letters
and digits are printed.

```bash
python tools/effect-catalogue/effect_types.py unused <work>
```

Lists the types that the code of a kind handles but that no call creates.

## Where the types come from

- the case labels of the switches on the type in the code of each kind
  (`ZzzEffect.cpp` and the move handlers, `ZzzEffectParticle.cpp`,
  `ZzzEffectJoint.cpp`, `zzzeffectsprite.cpp`),
- the rows of the effect registry,
- the first argument of every call that creates a kind; `BASE + rand() % n`
  and loop counters are expanded,
- `EXTRA_TYPES` in the script, for types the code computes in a way the
  scan cannot follow.

Expressions with the same number are one type; a plain enum name is
preferred as its `code`. The naming rules (`WORDS`, `NAME_OVERRIDES`) are
at the top of the generate section of the script.
