from pathlib import Path
import re
import sys


ROOT = Path(sys.argv[1])
MAX_MONSTER_NAME = 64
FILES = (
    "src/bin/Data/Local/Eng/NpcName_Eng.txt",
    "src/bin/Data/Local/Por/NpcName_Por.txt",
    "src/bin/Data/Local/Spn/NpcName_Spn.txt",
)
ENTRY_PATTERN = re.compile(r'^\s*(\d+)\s+\d+\s+"([^"]*)"', re.MULTILINE)
REQUIRED_RUNTIME_IDS = {42, 297, 318, 464}


parsed = {}
for relative_path in FILES:
    text = (ROOT / relative_path).read_text(encoding="utf-8", errors="strict")
    entries = [(int(identifier), name) for identifier, name in ENTRY_PATTERN.findall(text)]
    names_by_id = dict(entries)
    assert len(names_by_id) == len(entries), f"{relative_path}: duplicate monster IDs"
    assert REQUIRED_RUNTIME_IDS <= names_by_id.keys(), f"{relative_path}: missing runtime monster IDs"
    assert max(map(len, names_by_id.values())) <= MAX_MONSTER_NAME, relative_path
    parsed[relative_path] = names_by_id.values()

assert "Esqueleto Mágico" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
assert "Administrador da Guilda Mercenária Tercia" in parsed["src/bin/Data/Local/Por/NpcName_Por.txt"]
assert "Tercia de Encargados del Gremio Mercenario" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
