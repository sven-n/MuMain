# Vietnamese cash shop scripts

The in-game shop reads `Data/InGameShopScript/<zone>.<year>.<version>/`
(`IBSCategory.txt`, `IBSPackage.txt`, `IBSProduct.txt`) and shows the names and
descriptions it finds there, so the shop is localized by translating those
files rather than through `src/Localization/*.resx`.

`apply.py` regenerates the translated files from the English originals, which
are kept next to them in `original-en/`:

```sh
cd tools/ingameshop_vi
SCRIPT=../../src/bin/Data/InGameShopScript/512.2012.084
python3 apply.py $SCRIPT/original-en \
  $SCRIPT \
  ../../../MuMain/build-linux/src/Data/InGameShopScript/512.2012.084 \
  ../../../OpenMU/src/Startup/Data/InGameShopScript/512.2012.084
```

It only rewrites the player-visible fields (tab name; package name,
description, cash and price-unit name; product name and unit) and copies every
sequence number, flag and date through unchanged. It fails loudly on a name it
cannot translate, so a new script version reports exactly which rows need a
glossary entry.

* `glossary.py` — item terms, following `src/Localization/Items.vi.resx`.
* `translate_names.py` — splits a row's name into operator tags (`_Red`,
  `_CARD`, `[+GP]`, ...) and a translatable core; the tags stay as they are.
* `descriptions.py` — the package descriptions, keyed by the English text.
  `#` is the client's line break, so a translation keeps the same `#` layout.

The output is UTF-8 without a byte-order mark; `CShopList::GetDecodedString`
decodes these files as UTF-8.
