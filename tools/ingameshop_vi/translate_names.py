# -*- coding: utf-8 -*-
"""Translate a cash shop item/package name into Vietnamese.

A name is <prefix tags><core><decorations>. Only the core is translated; the
tags the operators use to tell variants apart (_Red, _CARD, _Platinum, the
Korean sale markers, ...) are left exactly as they are so the rows stay
identifiable in the script files.
"""
import re
from glossary import CORE, SET, SLOT, CLASSES

# Odd rows that don't follow any pattern.
OVERRIDES = {
    "Talisman of Luck_1% Increases - 20th day": "Bùa May Mắn_Tăng 1% - 20th day",
    "Gold Channel 30 Days - Card": "Kênh Vàng 30 Ngày - Card",
    "Seed (Water) DEF Accuracy": "Hạt Giống Nước (Chính Xác PT)",
    "Seed (Water) DEF Increase": "Hạt Giống Nước (Tăng PT)",
    "Seal of Wealth(Normal)_7Day_Event": "Ấn Phú Quý(Thường)_7 Ngày_Event",
    # Internal / test rows the operators never show: left as they are.
    "testddd": "testddd",
    "Test_GB": "Test_GB",
    "Brave ITEM_판매불가": "Brave ITEM_판매불가",
    "Penrill_판매불가": "Penrill_판매불가",
    "10000 GP": "10000 GP",
    "GP-500": "GP-500",
}

# Letter prefixes the scripts use to mark a row's origin (G- = goblin point,
# D- = daily, B- = bonus). They stay in front of the translated core.
_LETTER_PREFIX = re.compile(r"^(?P<tag>[BDG]-(?:\d\s+)?)(?P<rest>[A-Za-z].*)$")

_CORE_KEYS = sorted(CORE, key=len, reverse=True)

# "<set> <slot>" equipment, e.g. "Great Dragon Boots".
def _equipment(name):
    for slot_en, slot_vi in SLOT.items():
        if not name.endswith(" " + slot_en):
            continue
        base = name[: -(len(slot_en) + 1)].strip()
        for set_en, set_vi in SET.items():
            if base.lower() == set_en.lower():
                return f"{slot_vi} {set_vi}"
    return None

_DAY = re.compile(r"^(\d+)\s*(?:day|days)$", re.I)

def _duration(text):
    """'7Day' -> '7 Ngày', '3days' -> '3 Ngày'."""
    m = _DAY.match(text.strip())
    return f"{m.group(1)} Ngày" if m else None

def _core_exact(name):
    """CORE lookup that tolerates the double spaces in the scripts."""
    return CORE.get(re.sub(r"\s{2,}", " ", name.strip()))


def _core(name):
    """Translate the bare core of a name, or return None."""
    name = name.strip()
    if not name:
        return ""

    name = re.sub(r"\s{2,}", " ", name)

    if name in CORE:
        return CORE[name]

    # "Talisman of Luck 10%"
    m = re.match(r"^Talisman of Luck (\d+%)$", name)
    if m:
        return f"Bùa May Mắn {m.group(1)}"

    # "Open Access Ticket to Varka 7"
    m = re.match(r"^(Open Access Ticket to \w+) (\d+)$", name)
    if m and m.group(1) in CORE:
        return f"{CORE[m.group(1)]} {m.group(2)}"

    # "Sign of Lord 255", "Gold Key-1EA" - a known core plus a plain marker.
    m = re.match(r"^(?P<base>.+?)(?P<tail>[- ]\d+(?:EA)?)$", name)
    if m and _core_exact(m.group("base")):
        return _core_exact(m.group("base")) + m.group("tail")

    # "Jewel of Life Bundle" / "Bundle of Jewel of Harmony" without a count.
    m = re.match(r"^(Jewel of \w+) Bundle$", name)
    if m and m.group(1) in CORE:
        return f"Bó {CORE[m.group(1)]}"

    m = re.match(r"^Bundle of (Jewel of \w+)$", name)
    if m and m.group(1) in CORE:
        return f"Bó {CORE[m.group(1)]}"

    eq = _equipment(name)
    if eq:
        return eq

    # "Box of kundun +5" (the scripts spell 'kundun' both ways).
    m = re.match(r"^Box of kundun\s*(\+\d+)$", name, re.I)
    if m:
        return f"Hộp Kundun {m.group(1)}"

    # "6000 Goblin Point(s)" / "100000 Goblin Point"
    m = re.match(r"^(\d+)\s+Goblin Points?$", name, re.I)
    if m:
        return f"{m.group(1)} Điểm Goblin"

    # "Bundle of Jewel of Bless (10)" / "Jewel of Chaos Bundle (30)"
    m = re.match(r"^Bundle of (Jewel of \w+)\s*\((\d+)\)\)?$", name)
    if not m:
        m = re.match(r"^(Jewel of \w+) Bundle\s*\((\d+)\)$", name)
    if m and m.group(1) in CORE:
        return f"Bó {CORE[m.group(1)]} ({m.group(2)})"

    # "Jewel of Soul [20]"
    m = re.match(r"^(Jewel of \w+)\s*\[(\d+)\]$", name)
    if m and m.group(1) in CORE:
        return f"{CORE[m.group(1)]} [{m.group(2)}]"

    # "1st Lucky Helm Ticket"
    m = re.match(r"^(\d)(?:st|nd|rd|th) Lucky (\w+) Ticket$", name, re.I)
    if m and m.group(2).capitalize() in SLOT:
        return f"Vé May Mắn {SLOT[m.group(2).capitalize()]} Bậc {m.group(1)}"

    # "Rare Item Ticket 8"
    m = re.match(r"^Rare Item Ticket\s*(\d+)$", name, re.I)
    if m:
        return f"Vé Vật Phẩm Hiếm {m.group(1)}"

    # "Skill book package for Dark Knight"
    m = re.match(r"^Skill book package for (.+)$", name, re.I)
    if m and m.group(1) in CLASSES:
        return f"Gói Sách Kỹ Năng {CLASSES[m.group(1)]}"

    # "Event Reward - Dragon Helm" (and its '-Dragon Helm' / lowercase spellings)
    m = re.match(r"^Event [Rr]eward\s*-\s*(.+)$", name)
    if m:
        inner = _core(m.group(1))
        if inner:
            return f"Phần Thưởng Sự Kiện - {inner}"

    # "[Master] eX700 OBT - Celebration Package"
    m = re.match(r"^eX700 OBT - Celebration Package$", name, re.I)
    if m:
        return "Gói Mừng eX700 OBT"

    # A core that only differs from a known one by a trailing duration,
    # e.g. "Seal of Wealth 3days" or "Gold Channel 30 Days".
    for key in _CORE_KEYS:
        if name.lower().startswith(key.lower()):
            rest = name[len(key):].strip(" -_")
            if not rest:
                return CORE[key]
            dur = _duration(rest)
            if dur:
                return f"{CORE[key]} {dur}"
    return None


# Decorations that must survive untouched at the end of a name.
_KEEP_SUFFIX = re.compile(
    r"("
    r"\s*\(1 [Ww]coin p\)|\s*\(1 Wcoin p\)"
    r"|_Blue 1\+1|_BLUE 1\+1|_BLUE|_Blue|_CARD|_Platinum|_Red|_RED|_Event|_EVENT"
    r"|_Stamp [Ee]vent|_Time Attack|_Nov_Event|_BUG Event|_Compensation"
    r"|_Reward for DC|_판매불가|_미판매|\(미사용\)|_P|_S|-P|-ALL|\*+|_$"
    r"|_Bonus\(\d\)|_Bonus|_\d+ Bundle|_\d+|_Day|\s+Event|\s*\(\d+\)|\(Normal\)"
    r"|\s*Ex 700 UBT|\s*EX 700 UBT|\s*-\s*\d+(?:st|nd|rd|th) day|\s*Sale|\s*Platinum"
    r"|\s*-\s*Limited|-Limited"
    r")$"
)

# Trailing duration markers that DO get translated, e.g. '_7Day', '(3day)',
# '[28 Day]', '-1Day', ' 30 Days'.
_DUR_SUFFIX = re.compile(
    r"(?P<sep>[_\-\s]|\s*\(|\s*\[)(?P<num>\d+)\s*(?P<unit>[Dd]ays?|DAYS|Day\(s\))(?P<close>\)|\])?$"
)

def translate_name(name):
    """Return the Vietnamese name, or None when the core is unknown."""
    name = name.strip()
    if not name:
        return ""

    if name in OVERRIDES:
        return OVERRIDES[name]

    # "+11 Wing of Storm_CARD", "+13 Cape of Fighter_Red"
    m = re.match(r"^(\+\d+\s+)(\S.*)$", name)
    if m:
        rest = translate_name(m.group(2))
        return None if rest is None else m.group(1) + rest

    # "G-Scroll of Health", "B-1 Silver Key"
    m = _LETTER_PREFIX.match(name)
    if m:
        rest = translate_name(m.group("rest"))
        if rest is not None:
            return m.group("tag") + rest

    # Leading operator tags: "[+GP]", "[+GP_EVENT] ", "[10+3]", "[Discount]",
    # "[GP]", "[Blue] ", "[Master] ", "[5+1]".
    prefix = ""
    m = re.match(r"^((?:\[[^\]]*\]\s*)+)(?=\S)", name)
    if m and name.endswith("]") and m.end() >= len(name.rstrip("]")):
        m = None  # the brackets wrap the whole name, they are not leading tags
    if m and not (name.startswith("[") and name.endswith("]") and name.count("[") == 1):
        # Keep every leading bracket group except a single one wrapping the
        # whole name (that one is handled below).
        groups = re.findall(r"\[[^\]]*\]", m.group(1))
        rest = name[m.end():].strip()
        if rest:
            prefix = "".join(groups) + (" " if m.group(1).endswith(" ") else "")
            name = rest

    # A single pair of brackets or double brackets around the whole name.
    wrap = None
    m = re.match(r"^(\[+)(.*?)(\]+)$", name)
    if m and "[" not in m.group(2) and "]" not in m.group(2):
        wrap = (m.group(1), m.group(3))
        name = m.group(2)

    # Peel the decorations off the end, keeping their order.
    kept = []
    while True:
        m = _KEEP_SUFFIX.search(name)
        if m and m.group(0):
            kept.insert(0, m.group(0))
            name = name[: m.start()]
            continue
        m = _DUR_SUFFIX.search(name)
        if m:
            sep = m.group("sep")
            close = m.group("close") or ""
            open_ = "(" if close == ")" else ("[" if close == "]" else "")
            if open_:
                kept.insert(0, f" {open_}{m.group('num')} Ngày{close}")
            else:
                kept.insert(0, f"{sep}{m.group('num')} Ngày")
            name = name[: m.start()]
            continue
        break

    core = _core(name)
    if core is None:
        return None

    out = prefix
    if wrap:
        out += wrap[0] + core + "".join(kept) + wrap[1]
    else:
        out += core + "".join(kept)
    return re.sub(r"\s+", " ", out).strip()
