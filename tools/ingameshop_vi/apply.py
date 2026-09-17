# -*- coding: utf-8 -*-
"""Rewrite the cash shop script files with Vietnamese text.

Input : the original English scripts (CP949 bytes, '@' separated rows).
Output: the same rows with only the player-visible fields translated,
        written as UTF-8 without a BOM and LF line endings.

Translated fields
  IBSCategory.txt  1  tab name
  IBSPackage.txt   3  package name        6  description
                  14  cash name          15  price unit
  IBSProduct.txt   1  product name        4  unit name
Everything else - sequence numbers, flags, dates, the internal Korean
property names - is copied through byte for byte.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from glossary import CATEGORIES, UNITS, PRICE_UNITS
from descriptions import DESCRIPTIONS
from translate_names import translate_name

ENCODING_IN = "cp949"  # the scripts were authored in the Korean code page


class Unknown(Exception):
    pass


def _name(value):
    lead = value[: len(value) - len(value.lstrip())]
    trail = value[len(value.rstrip()):]
    out = translate_name(value)
    if out is None:
        raise Unknown(f"name: {value!r}")
    return lead + out + trail


def _lookup(table, value, what):
    stripped = value.strip()
    if stripped in table:
        lead = value[: len(value) - len(value.lstrip())]
        trail = value[len(value.rstrip()):]
        return lead + table[stripped] + trail
    raise Unknown(f"{what}: {value!r}")


def _description(value):
    stripped = value.strip()
    if not stripped:
        return value
    if stripped not in DESCRIPTIONS:
        raise Unknown(f"description: {value!r}")
    lead = value[: len(value) - len(value.lstrip())]
    trail = value[len(value.rstrip()):]
    return lead + DESCRIPTIONS[stripped] + trail


def translate_rows(lines, kind):
    out = []
    for line in lines:
        if not line.strip():
            out.append(line)
            continue
        f = line.split("@")
        if kind == "category":
            f[1] = _lookup(CATEGORIES, f[1], "category")
        elif kind == "package":
            f[3] = _name(f[3])
            f[6] = _description(f[6])
            f[14] = _lookup(PRICE_UNITS, f[14], "cash name")
            f[15] = _lookup(PRICE_UNITS, f[15], "price unit")
        elif kind == "product":
            f[1] = _name(f[1])
            f[4] = _lookup(UNITS, f[4], "unit")
        out.append("@".join(f))
    return out


FILES = [
    ("IBSCategory.txt", "category"),
    ("IBSPackage.txt", "package"),
    ("IBSProduct.txt", "product"),
]


def main(source, targets):
    source = Path(source)
    for filename, kind in FILES:
        lines = (source / filename).read_text(encoding=ENCODING_IN).split("\n")
        translated = translate_rows(lines, kind)
        blob = "\n".join(translated)
        widest = max((len(l.encode("utf-8")) for l in translated), default=0)
        if widest >= 1024:
            raise SystemExit(f"{filename}: a row grew to {widest} bytes, the client reads 1024 per line")
        for target in targets:
            Path(target, filename).write_bytes(blob.encode("utf-8"))
        print(f"{filename}: {len(translated)} rows, widest row {widest} bytes")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
