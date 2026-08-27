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
NAME_PATTERN = re.compile(r'^\s*\d+\s+\d+\s+"([^"]*)"', re.MULTILINE)


parsed = {}
for relative_path in FILES:
    text = (ROOT / relative_path).read_text(encoding="utf-8", errors="strict")
    names = NAME_PATTERN.findall(text)
    assert len(names) == 532, f"{relative_path}: expected 532 names, got {len(names)}"
    assert max(map(len, names)) <= MAX_MONSTER_NAME, relative_path
    parsed[relative_path] = names

assert "Esqueleto Mágico" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
assert "Administrador da Guilda Mercenária Tercia" in parsed["src/bin/Data/Local/Por/NpcName_Por.txt"]
assert "Tercia de Encargados del Gremio Mercenario" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
